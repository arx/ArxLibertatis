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

#include "RenderWidget.h"

// Ogre
#include <Ogre/OgreStringConverter.h>

#include <QCloseEvent>


RenderWidget::RenderWidget(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f | Qt::MSWindowsOwnDC)
    , m_pOgreRenderWindow(0)
    , mIsInitialised(false) {		
    QPalette colourPalette = palette();
    colourPalette.setColor(QPalette::Active, QPalette::WindowText, Qt::black);
    colourPalette.setColor(QPalette::Active, QPalette::Window, Qt::black);
    setPalette(colourPalette);

    
}

RenderWidget::~RenderWidget() {
}
/*
QSize RenderWidget::sizeHint() const {
	return QSize(
}
*/
void RenderWidget::initialise(const Ogre::NameValuePairList *miscParams) {
    //These attributes are the same as those use in a QGLWidget
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    //Parameters to pass to Ogre::Root::createRenderWindow()
    Ogre::NameValuePairList params;
    params["useNVPerfHUD"] = "true";

    //If the user passed in any parameters then be sure to copy them into our own parameter set.
    //NOTE: Many of the parameters the user can supply (left, top, border, etc) will be ignored
    //as they are overridden by Qt. Some are still useful (such as FSAA).
    if(miscParams != 0) {
        params.insert(miscParams->begin(), miscParams->end());
    }

    //The external windows handle parameters are platform-specific
    Ogre::String externalWindowHandleParams;

    //Accept input focus
    setFocusPolicy(Qt::StrongFocus);

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    //positive integer for W32 (HWND handle) - According to Ogre Docs
    externalWindowHandleParams = Ogre::StringConverter::toString((unsigned int)(winId()));
#endif

#if defined(Q_WS_X11)
    //poslong:posint:poslong:poslong (display*:screen:windowHandle:XVisualInfo*) for GLX - According to Ogre Docs
    QX11Info info = x11Info();
    externalWindowHandleParams  = Ogre::StringConverter::toString((unsigned long)(info.display()));
    externalWindowHandleParams += ":";
    externalWindowHandleParams += Ogre::StringConverter::toString((unsigned int)(info.screen()));
    externalWindowHandleParams += ":";
    externalWindowHandleParams += Ogre::StringConverter::toString((unsigned long)(winId()));
    //externalWindowHandleParams += ":";
    //externalWindowHandleParams += Ogre::StringConverter::toString((unsigned long)(info.visual()));
#endif

    //Add the external window handle parameters to the existing params set.
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)		
    params["externalWindowHandle"] = externalWindowHandleParams;
#endif

#if defined(Q_WS_X11)
    params["parentWindowHandle"] = externalWindowHandleParams;
#endif

#if defined(Q_WS_MAC)
    params["macAPI"] = "cocoa";
    params["macAPICocoaUseNSView"] = "true";
#endif 

    //Finally create our window.
    m_pOgreRenderWindow = Ogre::Root::getSingletonPtr()->createRenderWindow("OgreWindow", width(), height(), false, &params);

    mIsInitialised = true;
}

Ogre::RenderWindow* RenderWidget::getOgreRenderWindow() const {
    return m_pOgreRenderWindow;
}

QPaintEngine *RenderWidget:: paintEngine() const {
    return 0;
}

void RenderWidget::paintEvent(QPaintEvent* evt) {
    if(mIsInitialised) {
        Ogre::Root::getSingleton()._fireFrameStarted();
        m_pOgreRenderWindow->update();
        Ogre::Root::getSingleton()._fireFrameRenderingQueued();
        Ogre::Root::getSingleton()._fireFrameEnded();
    }
}

void RenderWidget::resizeEvent(QResizeEvent* evt) {
    if(m_pOgreRenderWindow) {
		int w = width();
		int h = height();
        m_pOgreRenderWindow->resize(width(), height());
        m_pOgreRenderWindow->windowMovedOrResized();

        for(int ct = 0; ct < m_pOgreRenderWindow->getNumViewports(); ++ct) {
            Ogre::Viewport* pViewport = m_pOgreRenderWindow->getViewport(ct);
            Ogre::Camera* pCamera = pViewport->getCamera();
            pCamera->setAspectRatio(static_cast<Ogre::Real>(pViewport->getActualWidth()) / static_cast<Ogre::Real>(pViewport->getActualHeight()));
        }
    }
}

EventHandlingRenderWidget::EventHandlingRenderWidget(QWidget* parent, Qt::WindowFlags f)
	: RenderWidget(parent, f)
	, mEventHandler(0) {		
}

EventHandlingRenderWidget::~EventHandlingRenderWidget() {
}

void EventHandlingRenderWidget::setEventHandler(EventHandler* eventHandler) {
	mEventHandler = eventHandler;
}

void EventHandlingRenderWidget::keyPressEvent(QKeyEvent* event) {
	if(mEventHandler != 0) {
		mEventHandler->onKeyPress(event);
	} else {
		QWidget::keyPressEvent(event);
	}
}

void EventHandlingRenderWidget::keyReleaseEvent(QKeyEvent* event) {
	if(mEventHandler != 0) {
		mEventHandler->onKeyRelease(event);
	} else {
		QWidget::keyReleaseEvent(event);
	}
}

void EventHandlingRenderWidget::mousePressEvent(QMouseEvent* event) {
	if(mEventHandler != 0) {
		mEventHandler->onMousePress(event);
	} else {
		QWidget::mousePressEvent(event);
	}
}

void EventHandlingRenderWidget::mouseReleaseEvent(QMouseEvent* event) {
	if(mEventHandler != 0) {
		mEventHandler->onMouseRelease(event);
	} else {
		QWidget::mouseReleaseEvent(event);
	}
}

void EventHandlingRenderWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	if(mEventHandler != 0) {
		mEventHandler->onMouseDoubleClick(event);
	} else {
		QWidget::mouseDoubleClickEvent(event);
	}
}

void EventHandlingRenderWidget::mouseMoveEvent(QMouseEvent* event) {
	if(mEventHandler != 0) {
		mEventHandler->onMouseMove(event);
	} else {
		QWidget::mouseMoveEvent(event);
	}
}

void EventHandlingRenderWidget::wheelEvent(QWheelEvent* event) {
	if(mEventHandler != 0) {
		mEventHandler->onWheel(event);
	} else {
		QWidget::wheelEvent(event);
	}
}
