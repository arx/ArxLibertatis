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

#ifndef ARX_TOOLS_PROFILER_UI_ARXPROFILER_H
#define ARX_TOOLS_PROFILER_UI_ARXPROFILER_H

#include <QMainWindow>
#include <QGraphicsView>

#include <map>

struct ProfileSample {
	QString tag;
	quint64 threadId;
	qint64 startTime;
	qint64 endTime;
};

struct ProfileThread {
	QString threadName;
	quint64 threadId;
	qint64 startTime;
	qint64 endTime;
};

struct ThreadData {
	
	ThreadData() {
		info.threadId = 0;
		info.startTime = 0;
		info.endTime = 0;
		maxDepth = 0;
	}

	ProfileThread info;
	std::vector<ProfileSample> profilePoints;
	size_t maxDepth;
	
};

typedef std::map<quint64, ThreadData> ThreadsData;

class ProfilerView : public QGraphicsView {
	
	Q_OBJECT

public:
	
	explicit ProfilerView(QWidget * parent = NULL);
	
	void setData(ThreadsData * threadsData);
	
protected:
	
	void paintEvent(QPaintEvent * event);
	
	virtual void wheelEvent(QWheelEvent * event);
	virtual void keyPressEvent(QKeyEvent * event);
	void contextMenuEvent(QContextMenuEvent * event);
	
private slots:
	
	void copyToClipboard();
	
private:
	
	ThreadsData * m_data;
	QGraphicsScene * m_scene;
	
	QPointF viewCenter() const;
	void zoomEvent(QPoint mousePos, bool zoomIn);
	const char * humanReadableTime(qreal & duration);
	
};

namespace Ui {
	class ArxProfilerClass;
}

class ArxProfiler : public QMainWindow {
	
	Q_OBJECT
	
public:
	
	ArxProfiler(QWidget * parent = 0, Qt::WindowFlags flags = 0);
	~ArxProfiler();
	
private slots:
	
	void openFile();
	
private:
	
	Ui::ArxProfilerClass * ui;
	ProfilerView * view;
	
	QStringList m_strings;
	ThreadsData m_threads;
	
};

#endif // ARX_TOOLS_PROFILER_UI_ARXPROFILER_H
