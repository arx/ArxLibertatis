/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_TOOLS_ARXEDIT_RENDERWIDGET_H
#define ARX_TOOLS_ARXEDIT_RENDERWIDGET_H

// Ogre
#include <Ogre/OgreCommon.h>
#include <Ogre/OgreRoot.h>
#include <Ogre/OgreRenderWindow.h>

// Qt
#include <QWidget>
#include <QFrame>

class RenderWidget : public QWidget {
	Q_OBJECT

public:
	RenderWidget(QWidget* parent=0, Qt::WindowFlags f=0);
	~RenderWidget();

	Ogre::RenderWindow* getOgreRenderWindow() const;

	void initialise(const Ogre::NameValuePairList *miscParams = 0);

protected:
	QPaintEngine *paintEngine() const;
	void paintEvent(QPaintEvent* evt);
	void resizeEvent(QResizeEvent* evt);

public:
	Ogre::RenderWindow* m_pOgreRenderWindow;

private:
	bool mIsInitialised;
};

class EventHandler
{
public:	
	virtual void onKeyPress(QKeyEvent* event) {}
	virtual void onKeyRelease(QKeyEvent* event) {}

	virtual void onMousePress(QMouseEvent* event) {}
	virtual void onMouseRelease(QMouseEvent* event) {}
	virtual void onMouseDoubleClick(QMouseEvent* event) {}
	virtual void onMouseMove(QMouseEvent* event) {}

	virtual void onWheel(QWheelEvent* event) {}
};

class EventHandlingRenderWidget : public RenderWidget
{
	Q_OBJECT

public:
	EventHandlingRenderWidget(QWidget* parent=0, Qt::WindowFlags f=0);
	~EventHandlingRenderWidget();

	void setEventHandler(EventHandler* eventHandler);

	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

	void wheelEvent(QWheelEvent* event);

private:
	EventHandler* mEventHandler;
};

#endif // ARX_TOOLS_ARXEDIT_RENDERWIDGET_H
