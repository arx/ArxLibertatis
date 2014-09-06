/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include <limits>

#include <QDebug>
#include <QHash>

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

ArxProfiler::ArxProfiler(QWidget *parent, Qt::WindowFlags flags)
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

void ArxProfiler::openFile() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Ax performance log (*.perf)"));
	
	QFile file(fileName);

	if(!file.open(QIODevice::ReadOnly))
		return;
	
	threadsData.clear();
	
	SavedThreadInfoHeader theadsHeader;
	file.read((char*)&theadsHeader, sizeof(SavedThreadInfoHeader));
	
	for(quint32 i = 0; i < theadsHeader.size; i++) 	{
		SavedThreadInfo saved;
		file.read((char*)&saved, sizeof(SavedThreadInfo));
		
		ThreadInfo& threadInfo = threadsData[saved.threadId].info;
		threadInfo.threadId = saved.threadId;
		threadInfo.threadName = QString::fromLatin1(util::loadString(saved.threadName).c_str());
		threadInfo.startTime = saved.startTime;
		threadInfo.endTime = saved.endTime;
	}
	
	SavedProfilePointHeader pointsHeader;
	file.read((char*)&pointsHeader, sizeof(SavedProfilePointHeader));
	
	for(quint32 i = 0; i < pointsHeader.size; i++) {
		SavedProfilePoint saved;
		file.read((char*)&saved, sizeof(SavedProfilePoint));
		
		// TODO: String table ftw
		ProfilePoint point;
		point.tag = QString::fromLatin1(util::loadString(saved.tag).c_str());
		point.threadId = saved.threadId;
		point.startTime = saved.startTime;
		point.endTime = saved.endTime;
		
		threadsData[point.threadId].profilePoints.push_back(point);
	}
	
	view->setData(&threadsData);
}


class QGraphicsProfilePoint : public QGraphicsRectItem
{
	static const int Type = UserType + 1;
	int type() const {
		// Enable the use of qgraphicsitem_cast with this item.
		return Type;
	}
	
public:
	QGraphicsProfilePoint(const QRectF &rect, QGraphicsItem *parent = 0)
		: QGraphicsRectItem(rect, parent)
	{}
	
	~QGraphicsProfilePoint()
	{}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) {
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
	
	void setText(const QString& text) {
		m_Text = text;
	}
	
private:
	Q_DISABLE_COPY(QGraphicsProfilePoint)
	
	QString m_Text;
};


const quint32 ITEM_HEIGHT = 15;
const quint32 THREAD_SPACING = 50;

ProfilerView::ProfilerView(QWidget* parent)
	: QGraphicsView(parent)
	, m_data(NULL)
{
	setBackgroundBrush(QBrush(QColor(160, 160, 160)));
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setRenderHint(QPainter::HighQualityAntialiasing, true);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setOptimizationFlags(QGraphicsView::DontSavePainterState);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_scene = new QGraphicsScene(this);
	setScene(m_scene);

	QFont font;
	font.setPixelSize(11);
	setFont(font);
}

void ProfilerView::setData(ThreadsData * data) {
	
	quint64 firstTimestamp = std::numeric_limits<quint64>::max();
	quint64 lastTimestamp = std::numeric_limits<quint64>::min();
	
	for(ThreadsData::const_iterator it = data->begin(); it != data->end(); ++it) {
		
		if(it->second.profilePoints.empty()) {
			continue;
		}
		
		if(firstTimestamp > it->second.profilePoints[0].startTime) {
			firstTimestamp = it->second.profilePoints[0].startTime;
		}
		
		if(lastTimestamp < it->second.profilePoints.back().endTime) {
			lastTimestamp = it->second.profilePoints.back().endTime;
		}
	}
	
	m_scene->clear();
	
	// reverse iterate
	int nextPos = 8;
	
	QPen profilePointPen(Qt::black);
	profilePointPen.setCosmetic(true);
	
	for(ThreadsData::iterator it = data->begin(); it != data->end(); ++it) {
		ThreadData& threadData = it->second;
		
		QGraphicsItemGroup * group = new QGraphicsItemGroup();
		m_scene->addItem(group);
		
		std::vector<quint64> threadStack;
		
		for(std::vector<ProfilePoint>::const_reverse_iterator it = threadData.profilePoints.rbegin(); it != threadData.profilePoints.rend(); ++it) {
			
			while(!threadStack.empty()) {
				if(it->endTime <= threadStack.back()) {
					threadStack.pop_back();
				} else {
					break;
				}
			}
			qreal offset = ITEM_HEIGHT * threadStack.size();
			
			threadStack.push_back(it->startTime);
			if(threadStack.size() > threadData.maxDepth)
				threadData.maxDepth = threadStack.size();
			
			double duration = it->endTime - it->startTime;
			
			const char* unitName = humanReadableTime(duration);
			
			// Round to get 2 decimals of precision
			duration = (int)(duration * 100);
			duration /= 100;
			
			QRectF rect(it->startTime - firstTimestamp, offset, it->endTime - it->startTime, ITEM_HEIGHT);
			QGraphicsProfilePoint* profilePoint = new QGraphicsProfilePoint(rect, group);
			
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
		
		nextPos += threadData.maxDepth * ITEM_HEIGHT + THREAD_SPACING;
	}
	
	setSceneRect(0, 0, lastTimestamp - firstTimestamp, nextPos);

	scale(size().width() / (qreal)(lastTimestamp - firstTimestamp), 1.0);
	
	setDragMode(ScrollHandDrag);
	setInteractive(false);

	m_data = data;
}

void ProfilerView::paintEvent(QPaintEvent * event) {
	
	QGraphicsView::paintEvent(event);
	
	if(!m_data)
		return;
	
	QPainter painter(viewport());
	painter.setPen(Qt::white);
	
	int nextY = 5;
	
	for(ThreadsData::iterator it = m_data->begin(); it != m_data->end(); ++it) {
		ThreadData& threadData = it->second;
		
		painter.drawLine(QPointF(0, nextY), QPointF(viewport()->width(), nextY));
		painter.drawText(QPointF(0, nextY + 14), threadData.info.threadName);
		
		nextY += threadData.maxDepth * ITEM_HEIGHT + THREAD_SPACING;
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
		scale(scaleFactor, 1.0f);
	} else {
		scale(1.0 / scaleFactor, 1.0f);
	}
	
	QPointF offset = oldScenePos - mapToScene(mousePos);
	centerOn(viewCenter() + offset);
}

void ProfilerView::wheelEvent(QWheelEvent* event) {
	zoomEvent(event->pos(), event->delta() > 0);
}

void ProfilerView::keyPressEvent(QKeyEvent* event) {
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

const char * ProfilerView::humanReadableTime(double & duration) {
	
	static const qint32 NUM_UNITS = 6;
	static const char*  UNIT_NAME[NUM_UNITS] = {"us", "ms", "s", "m", "h", "d"};
	static const qint64 UNIT_NEXT[NUM_UNITS] = {1000, 1000,  60,  60,  24,   1};
	
	int i;
	for(i = 0; i < NUM_UNITS; i++) {
		if(duration <= UNIT_NEXT[i])
			break;
		
		duration /= UNIT_NEXT[i];
	}
	
	return UNIT_NAME[i];
}
