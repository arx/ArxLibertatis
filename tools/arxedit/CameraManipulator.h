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

#ifndef ARX_TOOLS_ARXEDIT_CAMERAMANIPULATOR_H
#define ARX_TOOLS_ARXEDIT_CAMERAMANIPULATOR_H

#include <QKeyEvent>

#include <Ogre/Ogre.h>
#include <limits>

#include "RenderWidget.h"


class CameraManipulator : public EventHandler, public Ogre::FrameListener
{
public:
	CameraManipulator(Ogre::Camera* cam) 
		: mCamera(cam)
		, mTopSpeed(10)
		, mVelocity(Ogre::Vector3::ZERO)
		, mGoingForward(false)
		, mGoingBack(false)
		, mGoingLeft(false)
		, mGoingRight(false)
		, mGoingUp(false)
		, mGoingDown(false)
		, mFastMove(false) {

		mCamera->setAutoTracking(false);
		mCamera->setFixedYawAxis(true);
	}
		
	void setTopSpeed(Ogre::Real topSpeed) {
		mTopSpeed = topSpeed;
	}

	Ogre::Real getTopSpeed() {
		return mTopSpeed;
	}

	bool frameRenderingQueued(const Ogre::FrameEvent& evt) {
		// build our acceleration vector based on keyboard input composite
		Ogre::Vector3 accel = Ogre::Vector3::ZERO;
		if (mGoingForward) accel += mCamera->getDirection();
		if (mGoingBack) accel -= mCamera->getDirection();
		if (mGoingRight) accel += mCamera->getRight();
		if (mGoingLeft) accel -= mCamera->getRight();
		if (mGoingUp) accel += mCamera->getUp();
		if (mGoingDown) accel -= mCamera->getUp();

		// if accelerating, try to reach top speed in a certain time
		Ogre::Real topSpeed = mFastMove ? mTopSpeed * 10 : mTopSpeed;
		if (accel.squaredLength() != 0)
		{
			accel.normalise();
			mVelocity += accel * topSpeed * evt.timeSinceLastFrame * 10;
		}
		// if not accelerating, try to stop in a certain time
		else mVelocity -= mVelocity * evt.timeSinceLastFrame * 10;

		Ogre::Real tooSmall = std::numeric_limits<Ogre::Real>::epsilon();

		// keep camera velocity below top speed and above epsilon
		if (mVelocity.squaredLength() > topSpeed * topSpeed) {
			mVelocity.normalise();
			mVelocity *= topSpeed;
		} else if (mVelocity.squaredLength() < tooSmall * tooSmall) {
			mVelocity = Ogre::Vector3::ZERO;
		} else {
			mVelocity =mVelocity ;
		}

		if (mVelocity != Ogre::Vector3::ZERO) {
			mCamera->move(mVelocity * evt.timeSinceLastFrame);
		}

		return true;
	}
		
	void onKeyPress(QKeyEvent* event) {
		updateMovement(event, true);
	}

	void onKeyRelease(QKeyEvent* event) {
		updateMovement(event, false);
	}

	void updateMovement(QKeyEvent* event, bool bPressed) {
		int key = event->key();
		
		if (key == Qt::Key_W || key == Qt::Key_Up) {
			mGoingForward = bPressed;
		} else if (key == Qt::Key_S || key == Qt::Key_Down) {
			mGoingBack = bPressed;
		} else if (key == Qt::Key_A || key == Qt::Key_Left) {
			mGoingLeft = bPressed;
		} else if (key == Qt::Key_D || key == Qt::Key_Right) {
			mGoingRight = bPressed;
		} else if (key == Qt::Key_PageUp) {
			mGoingUp = bPressed;
		} else if (key == Qt::Key_PageDown) {
			mGoingDown = bPressed;
		} 
		
		if (event->modifiers() == Qt::ShiftModifier) {
			mFastMove = bPressed;
		}
	}

	void onMousePress(QMouseEvent* event) {
		mLastPos = event->globalPos();
		QApplication::setOverrideCursor(QCursor( Qt::BlankCursor ));
	}

	void onMouseRelease(QMouseEvent* event) {
		QApplication::restoreOverrideCursor();
	}
	
	void onMouseMove(QMouseEvent* event) {		
		QPoint newPos = event->globalPos();
		QPoint relMove(newPos - mLastPos);
		mCamera->yaw(Ogre::Degree(-relMove.x() * 0.10f));
		mCamera->pitch(Ogre::Degree(-relMove.y() * 0.10f));
		QCursor::setPos(mLastPos);
	}

protected:
	Ogre::Camera* mCamera;

	Ogre::Real mTopSpeed;
	Ogre::Vector3 mVelocity;
	bool mGoingForward;
	bool mGoingBack;
	bool mGoingLeft;
	bool mGoingRight;
	bool mGoingUp;
	bool mGoingDown;
	bool mFastMove;

	QPoint mLastPos;
};

#endif // ARX_TOOLS_ARXEDIT_CAMERAMANIPULATOR_H
