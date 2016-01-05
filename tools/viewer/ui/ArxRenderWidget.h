/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QColor>
#include <QElapsedTimer>
#include <QTimer>
#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>

class AnimationLayersModel;
class LightsModel;

class RenderWindow;
class Renderer;
class TurntableCamera;

class EERIE_3DOBJ;

class ArxRenderWidget : public QGLWidget {
	Q_OBJECT
	
public:
	QGLFormat desiredFormat();
	ArxRenderWidget(
		AnimationLayersModel * animationLayersModel,
		LightsModel * lightsModel,
		QWidget *parent = 0);
	
	~ArxRenderWidget();
	
	void setAnimationModel(AnimationLayersModel * model);
	
	void openFile2(QString name);
	
	bool m_showVertexNormals;
	bool m_showFaceNormals;
	bool m_play;
	float m_playbackSpeed;
	
protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void wheelEvent(QWheelEvent * event);
	
private:
	QTimer timer;
	
	QElapsedTimer m_frameTimer;
	
	AnimationLayersModel * m_animationLayersModel;
	LightsModel * m_lightsModel;
	
	RenderWindow * m_dummyWindow;
	Renderer * m_renderer;
	TurntableCamera * cam;
	
	EERIE_3DOBJ * m_currentObject;
	
	void drawModel(qint64 frameTime);
	void drawLights();
	void drawCoordinates();
	
	QColor m_backgroundColor;
	
	QPoint m_lastPos;
};

#endif
