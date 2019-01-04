/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "profiler/ui/ArxProfiler.h"

#include "ui_ArxProfiler.h"

#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>

#include <QDebug>
#include <QHash>

#include <QClipboard>
#include <QFileDialog>

#include <QMouseEvent>
#include <QWheelEvent>

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsItemGroup>

#include "platform/profiler/ProfilerDataFormat.h"
#include "util/String.h"

ArxProfiler::ArxProfiler(QWidget * parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags)
	, ui(new Ui::ArxProfilerClass)
{
	ui->setupUi(this);

	// Fix missing menu bar in Ubuntu
	ui->menuBar->setNativeMenuBar(false);

	view = new ProfilerView(this);
	setCentralWidget(view);

	connect(ui->action_Open, SIGNAL(triggered()), this, SLOT(openFile()));
}

ArxProfiler::~ArxProfiler() {
	delete ui;
}

template <typename T>
void readStruct(T & out, QByteArray & data, int & pos) {
	
	out = *reinterpret_cast<const T *>(data.mid(pos, sizeof(T)).constData());
	pos += int(sizeof(T));
}

void ArxProfiler::openFile() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Arx performance profiler log (*.arxprof)"));
	
	QFile file(fileName);

	if(!file.open(QIODevice::ReadOnly))
		return;
	
	m_threads.clear();
	m_strings.clear();
	
	QByteArray fileData = file.readAll();
	int filePos = 0;
	
	SavedProfilerHeader header;
	readStruct(header, fileData, filePos);
	
	if(strcmp(profilerMagic, header.magic) == 0) {
		qDebug() << "Header magic found";
	} else {
		qDebug() << "Invalid magic";
		return;
	}
	
	qDebug() << "Found file version:" << header.version;
	
	u32 minimumVersion = 1;
	if(header.version < minimumVersion) {
		qWarning() << "File version too old, aborting";
		return;
	}
	
	while(filePos < fileData.size()){
		SavedProfilerChunkHeader chunk;
		readStruct(chunk, fileData, filePos);
		
		int chunkSize = int(chunk.size);
		qDebug() << "Reading chunk at offset" << filePos << "type" << chunk.type << "size" << chunkSize;
		QByteArray chunkData = fileData.mid(filePos, chunkSize);
		if(chunkData.size() != chunkSize) {
			qWarning() << "Chunk too short, expected" << chunkSize << "got" << chunkData.size();
			return;
		}
			
		filePos += chunkData.size();
		
		if(chunk.type == ArxProfilerChunkType_Strings) {
			int chunkPos = 0;
			while(chunkPos < chunkData.size()) {
				int foo = chunkData.indexOf('\0', chunkPos);
				if(foo == -1)
					break;
				
				QString string = QString::fromLatin1(chunkData.mid(chunkPos, foo));
				m_strings.append(string);
				chunkPos = foo + 1;
			}
		}
		
		if(chunk.type == ArxProfilerChunkType_Threads) {
			int chunkPos = 0;
			while(chunkPos < chunkData.size()) {
				SavedProfilerThread saved;
				readStruct(saved, chunkData, chunkPos);
				
				ProfileThread & thread = m_threads[saved.threadId].info;
				thread.threadId = saved.threadId;
				thread.threadName = m_strings.at(saved.stringIndex);
				thread.startTime = saved.startTime;
				thread.endTime = saved.endTime;
			}
		}
		
		if(chunk.type == ArxProfilerChunkType_Samples) {
			int pos = 0;
			while(pos < chunkData.size()) {
				SavedProfilerSample saved;
				readStruct(saved, chunkData, pos);
				
				ProfileSample sample;
				sample.tag = m_strings.at(saved.stringIndex);
				sample.threadId = saved.threadId;
				sample.startTime = saved.startTime;
				sample.endTime = saved.endTime;
				
				m_threads[sample.threadId].profilePoints.push_back(sample);
			}
		}
	}
	
	view->setData(&m_threads);
}


class QGraphicsProfilePoint : public QGraphicsRectItem {
	
public:
	QGraphicsProfilePoint(const QRectF & rect, QGraphicsItem * parent = 0)
		: QGraphicsRectItem(rect, parent)
	{}
	
	~QGraphicsProfilePoint()
	{}
	
	static const int Type = UserType + 1;
	int type() const {
		// Enable the use of qgraphicsitem_cast with this item.
		return Type;
	}
	
	void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) {
		QGraphicsRectItem::paint(painter, option, widget);
		
		if(rect().width() * painter->transform().m11() >= 5) {
			QTransform transBefore = painter->transform();
			painter->setTransform(QTransform());
			
			QRectF newRect = transBefore.mapRect(rect());
			newRect.adjust(4, 1, 0, 0);

			QTextOption textOption;
			textOption.setWrapMode(QTextOption::NoWrap);
			painter->drawText(newRect, m_Text, textOption);
			
			painter->setTransform(transBefore);
		}
	}
	
	QString text() {
		return m_Text;
	}
	
	void setText(const QString & text) {
		m_Text = text;
	}
	
private:
	Q_DISABLE_COPY(QGraphicsProfilePoint)
	
	QString m_Text;
};


const qreal ITEM_HEIGHT = 15;
const qreal THREAD_SPACING = 50;

ProfilerView::ProfilerView(QWidget * parent)
	: QGraphicsView(parent)
	, m_data(NULL)
{
	setBackgroundBrush(QBrush(QColor(160, 160, 160)));
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setOptimizationFlags(QGraphicsView::DontSavePainterState);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_scene = new QGraphicsScene(this);
	// This is a workaround for bad performance using the default BspTreeIndex
	// Maybe we are hitting this bug:
	// http://stackoverflow.com/questions/6164543/qgraphicsscene-item-coordinates-affect-performance
	m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	
	setScene(m_scene);

	QFont font;
	font.setPixelSize(11);
	setFont(font);
}

void ProfilerView::setData(ThreadsData * threadsData) {
	
	qint64 firstTimestamp = std::numeric_limits<qint64>::max();
	qint64 lastTimestamp = std::numeric_limits<qint64>::min();
	BOOST_FOREACH(const ThreadsData::value_type & entry, *threadsData) {
		if(!entry.second.profilePoints.empty()) {
			firstTimestamp = std::min(firstTimestamp, entry.second.profilePoints[0].startTime);
			lastTimestamp = std::max(lastTimestamp, entry.second.profilePoints.back().endTime);
		}
	}
	
	m_scene->clear();
	
	// reverse iterate
	qreal nextPos = 8;
	
	QPen profilePointPen(Qt::black);
	profilePointPen.setCosmetic(true);
	
	BOOST_FOREACH(ThreadsData::value_type & entry, *threadsData) {
		ThreadData & threadData = entry.second;
		
		QGraphicsItemGroup * group = new QGraphicsItemGroup();
		m_scene->addItem(group);
		
		std::vector<qint64> threadStack;
		
		for(std::vector<ProfileSample>::const_reverse_iterator it = threadData.profilePoints.rbegin(); it != threadData.profilePoints.rend(); ++it) {
			
			while(!threadStack.empty()) {
				if(it->endTime <= threadStack.back()) {
					threadStack.pop_back();
				} else {
					break;
				}
			}
			qreal offset = ITEM_HEIGHT * qreal(threadStack.size());
			
			threadStack.push_back(it->startTime);
			if(threadStack.size() > threadData.maxDepth)
				threadData.maxDepth = threadStack.size();
			
			qreal duration = qreal(it->endTime - it->startTime);
			
			const char * unitName = humanReadableTime(duration);
			
			// Round to get 2 decimals of precision
			duration = int(duration * 100);
			duration /= 100;
			
			QRectF rect(qreal(it->startTime - firstTimestamp), offset, qreal(it->endTime - it->startTime), ITEM_HEIGHT);
			QGraphicsProfilePoint * profilePoint = new QGraphicsProfilePoint(rect, group);
			
			QString text = QString("%3 (%1 %2)").arg(duration).arg(unitName).arg(it->tag);
			profilePoint->setText(text);
			profilePoint->setToolTip(text);
			
			qsrand(qHash(it->tag));
			QColor tagColor;
			tagColor.setRed(100 + qrand() % 155);
			tagColor.setGreen(100 + qrand() % 155);
			tagColor.setBlue(100 + qrand() % 155);
			profilePoint->setBrush(QBrush(tagColor));

			profilePoint->setPen(profilePointPen);
		}
		group->setPos(0, nextPos);
		
		nextPos += qreal(threadData.maxDepth) * ITEM_HEIGHT + THREAD_SPACING;
	}
	
	const qreal timeDiff = qreal(lastTimestamp - firstTimestamp);
	
	setSceneRect(0, 0, timeDiff, nextPos);
	
	resetMatrix();
	scale(size().width() / timeDiff, 1.0);
	
	setDragMode(ScrollHandDrag);
	setInteractive(false);

	m_data = threadsData;
}

void ProfilerView::paintEvent(QPaintEvent * event) {
	
	QGraphicsView::paintEvent(event);
	
	if(!m_data)
		return;
	
	QPainter painter(viewport());
	painter.setPen(Qt::white);
	
	qreal nextY = 5;
	
	for(ThreadsData::iterator it = m_data->begin(); it != m_data->end(); ++it) {
		ThreadData & threadData = it->second;
		painter.drawLine(QPointF(0, nextY), QPointF(viewport()->width(), nextY));
		painter.drawText(QPointF(0, nextY + 14), threadData.info.threadName);
		nextY += qreal(threadData.maxDepth) * ITEM_HEIGHT + THREAD_SPACING;
	}
	
	painter.end();
}

QPointF ProfilerView::viewCenter() const {
	return mapToScene(viewport()->rect()).boundingRect().center();
}

void ProfilerView::zoomEvent(QPoint mousePos, bool zoomIn) {
	
	QPointF oldScenePos(mapToScene(mousePos));
	
	double scaleFactor = 1.15;
	if(zoomIn) {
		scale(scaleFactor, qreal(1));
	} else {
		scale(qreal(1) / scaleFactor, qreal(1));
	}
	
	QPointF offset = oldScenePos - mapToScene(mousePos);
	centerOn(viewCenter() + offset);
}

void ProfilerView::wheelEvent(QWheelEvent * event) {
	zoomEvent(event->pos(), event->delta() > 0);
}

void ProfilerView::keyPressEvent(QKeyEvent * event) {
	if(!underMouse())
		return;
	
	if(event->key() == Qt::Key_Plus) {
		zoomEvent(QCursor::pos(), true);
	} else if(event->key() == Qt::Key_Minus) {
		zoomEvent(QCursor::pos(), false);
	} else if(event->key() == Qt::Key_Right) {
		centerOn(viewCenter() + QPointF(1000, 0));
	} else if(event->key() == Qt::Key_Left) {
		centerOn(viewCenter() - QPointF(1000, 0));
	}
}

void ProfilerView::contextMenuEvent(QContextMenuEvent * event) {
	
	if(QGraphicsItem * item = itemAt(event->pos())) {
		if(QGraphicsProfilePoint * sample = qgraphicsitem_cast<QGraphicsProfilePoint *>(item)) {
			QMenu menu(this);
			
			QAction * copyAction = new QAction("Copy text", this);
			copyAction->setData(sample->text());
			connect(copyAction, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
			
			menu.addAction(copyAction);
			menu.exec(event->globalPos());
		}
	}
	
}

void ProfilerView::copyToClipboard() {
	if(QAction * action = qobject_cast<QAction *>(QObject::sender())) {
		QString text = action->data().toString();
		qDebug() << "Copy text to clipboard" << text;
		QApplication::clipboard()->setText(text);
	}
}

const char * ProfilerView::humanReadableTime(qreal & duration) {
	
	static const qint32 NUM_UNITS = 5;
	static const char * UNIT_NAME[NUM_UNITS + 1] = {"us", "ms", "s", "m", "h", "d"};
	static const qreal UNIT_NEXT[NUM_UNITS] = {1000, 1000,  60,  60,  24     };
	
	int i;
	for(i = 0; i < NUM_UNITS; i++) {
		if(duration <= UNIT_NEXT[i])
			break;
		
		duration /= UNIT_NEXT[i];
	}
	
	return UNIT_NAME[i];
}
