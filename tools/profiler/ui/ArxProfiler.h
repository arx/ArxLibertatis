/*
 * Copyright 2014-2021 Arx Libertatis Team (see the AUTHORS file)
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
	
	ThreadData()
		: maxDepth(0)
	{
		info.threadId = 0;
		info.startTime = 0;
		info.endTime = 0;
	}

	ProfileThread info;
	std::vector<ProfileSample> profilePoints;
	size_t maxDepth;
	
};

typedef std::map<quint64, ThreadData> ThreadsData;

class ProfilerView final : public QGraphicsView {
	
	Q_OBJECT

public:
	
	explicit ProfilerView(QWidget * parent = nullptr);
	
	void setData(ThreadsData * threadsData);
	
protected:
	
	void paintEvent(QPaintEvent * event) override;
	
	void wheelEvent(QWheelEvent * event) override;
	void keyPressEvent(QKeyEvent * event) override;
	void contextMenuEvent(QContextMenuEvent * event) override;
	
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

class ArxProfiler final : public QMainWindow {
	
	Q_OBJECT
	
public:
	
	ArxProfiler(QWidget * parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
	~ArxProfiler() override;
	
private slots:
	
	void openFile();
	
private:
	
	Ui::ArxProfilerClass * ui;
	ProfilerView * view;
	
	QStringList m_strings;
	ThreadsData m_threads;
	
};

#endif // ARX_TOOLS_PROFILER_UI_ARXPROFILER_H
