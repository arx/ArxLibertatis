/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "arxedit/ui/ArxRenderWidget.h"

#include "graphics/opengl/OpenGLUtil.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QTimer>

#include <cstdio>
#include <functional>
#include <iomanip>
#include <math.h>
#include <string>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "animation/Animation.h"
#include "animation/AnimationRender.h"
#include "core/Application.h"
#include "core/GameTime.h"

#include "game/Camera.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/effect/ParticleSystems.h"

#include "graphics/DrawLine.h"
#include "graphics/Renderer.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/FTL.h"
#include "graphics/font/Font.h"
#include "graphics/font/FontCache.h"
#include "graphics/particle/ParticleSystem.h"
#include "graphics/opengl/OpenGLRenderer.h"

#include "input/Input.h"
#include "io/fs/FilePath.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "platform/WindowsMain.h"
#include "scene/Light.h"
#include "scene/Object.h"
#include "window/SDL2Window.h"

#include "arxedit/model/AnimationLayersModel.h"
#include "arxedit/model/LightsModel.h"

extern Rect g_size;

class TurntableCamera {
public:
	Camera viewerCamera;
	float latitude;
	float longitude;
	float height;
	int cameraDistance;
	
	TurntableCamera() {
		init();
	}
	
	void init() {
		latitude = 0.f;
		longitude = 0.f;
		height = 0.f;
		cameraDistance = 500;
		
		viewerCamera.angle = Anglef::ZERO;
		viewerCamera.m_pos = Vec3f_ZERO;
		viewerCamera.focal = 350.f;
		viewerCamera.cdepth = 2000.f;
	}
	
	void updateCamera() {
		height = glm::clamp(height, -100.f, 100.f);
		cameraDistance = glm::clamp(cameraDistance, 50, 1900);
		
		// Update viewport
		//viewerCamera.clip = Rect(0, 0, g_size.width(), g_size.height());
		//viewerCamera.center = viewerCamera.clip.center();
		
		// Update position
		Vec2f polar = Vec2f(glm::radians(latitude), glm::radians(180.f + longitude));
		viewerCamera.m_pos = Vec3f(0.f, -height, 0.f) + glm::euclidean(polar) * float(cameraDistance);
		
		// TODO This does not seem to do what is expected !
		// viewerCamera.setTargetCamera(Vec3f(0, 0, 0));
		
		// Set the angle manually
		viewerCamera.angle.setPitch(-latitude);
		viewerCamera.angle.setYaw(-longitude);
		
		SetActiveCamera(&viewerCamera);
		PrepareCamera(&viewerCamera, g_size);
	}
};

class DummyWindow : public RenderWindow {
public:
	virtual void setTitle(const std::string &) { ARX_DEAD_CODE();}
	virtual void setFullscreenMode(const DisplayMode &) {ARX_DEAD_CODE();}
	virtual void setWindowSize(const Vec2i & size) { m_size = size; }
	virtual bool initialize() {ARX_DEAD_CODE();return true;}
	virtual void tick() {ARX_DEAD_CODE();}
	virtual void hide() {ARX_DEAD_CODE();}
	virtual bool initializeFramework() {ARX_DEAD_CODE();return true;}
	virtual bool setVSync(int) {ARX_DEAD_CODE();return true;}
	virtual bool setGamma(float gamma = 1.f) {ARX_DEAD_CODE();return true;}
	
	virtual void showFrame() {ARX_DEAD_CODE();}
	virtual InputBackend * getInputBackend() {ARX_DEAD_CODE();return 0;}
	
	virtual void setMinimizeOnFocusLost(bool enabled) {ARX_DEAD_CODE();}
	virtual MinimizeSetting willMinimizeOnFocusLost() {ARX_DEAD_CODE(); return Disabled;}
	virtual std::string getClipboardText() {ARX_DEAD_CODE(); return "";}
};

class DummyApp : public Application {
public:
	DummyApp(RenderWindow * win)
	: Application()
	{
		m_MainWindow = win;
	}
	
	virtual bool initialize() {ARX_DEAD_CODE();return true;}
	virtual void run() {ARX_DEAD_CODE();}
	virtual void setWindowSize(bool) {ARX_DEAD_CODE();}
};

class RenderListener : public Renderer::Listener {
public:
	virtual ~RenderListener()
	{ }
	
	virtual void onRendererInit(Renderer & renderer) {
		GRenderer = &renderer;
		
		size_t size = 1024;
		VertexBuffer<TexturedVertex> * vb = renderer.createVertexBufferTL(size, Renderer::Stream);
		
		extern CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX;
		pDynamicVertexBuffer_TLVERTEX = new CircularVertexBuffer<TexturedVertex>(vb);
	}
	
	virtual void onRendererShutdown(Renderer &) {
		
	}
};

QGLFormat ArxRenderWidget::desiredFormat() {
	QGLFormat fmt;
	fmt.setDoubleBuffer(true);
	fmt.setDepthBufferSize(24);
	fmt.setProfile(QGLFormat::CompatibilityProfile);
	fmt.setSwapInterval(1);
	fmt.setSamples(8);
	return fmt;
}

ArxRenderWidget::ArxRenderWidget(AnimationLayersModel * animationLayersModel, LightsModel * lightsModel, QWidget *parent)
	: QGLWidget(desiredFormat(), parent)
	, m_showVertexNormals(false)
	, m_showFaceNormals(false)
{
	m_animationLayersModel = animationLayersModel;
	m_lightsModel = lightsModel;
	
	connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
	timer.setInterval(0);
	timer.start();
	
	entities.init();
	
	//g_gameTime.init();
	
	m_dummyWindow = new DummyWindow;
	mainApp = new DummyApp(m_dummyWindow);
	
	m_renderer = new OpenGLRenderer;
	
	RenderListener * listener = new RenderListener;
	m_renderer->addListener(listener);
	
	cam = new TurntableCamera;
	
	setAcceptDrops(true);
	
	m_backgroundColor = QColor(83, 106, 127);
	m_currentObject = NULL;
	
	m_playbackSpeed = 1.f;
	
	particleParametersInit();
}

ArxRenderWidget::~ArxRenderWidget() {
}

void ArxRenderWidget::openFile2(QString name) {
	
	if(name.startsWith("game/")) {
		name = name.mid(5);
	}
	
	m_currentObject = ARX_FTL_Load(name.toStdString());
}

bool m_particleSystemVisible = false;
ParticleParams m_particleParameters = ParticleParams();
ParticleSystem * m_particleSystem = nullptr;

void ArxRenderWidget::particleSystemReset()
{
	delete m_particleSystem;
	m_particleSystem = nullptr;
}

void ArxRenderWidget::particleSystemLoad(int id)
{
	if(id == -1) {
		m_particleSystemVisible = false;
	} else {
		m_particleSystemVisible = true;
		m_particleParameters = g_particleParameters[id];
	}
	
	delete m_particleSystem;
	m_particleSystem = NULL;
}

void ArxRenderWidget::initializeGL() {
	
	m_renderer->initialize();
	m_renderer->afterResize();
	
	cam->init();
	m_lightsModel->updateBackingData();
	
	m_frameTimer.invalidate();
}


static Color toColor(QColor in) {
	int r, g, b, a;
	in.getRgb(&r, &g, &b, &a);
	return Color(r, g, b, a);
}

void ArxRenderWidget::paintGL() {
	
	qint64 frameTime;
	if(m_frameTimer.isValid()) {
		frameTime = m_frameTimer.restart();
	} else {
		m_frameTimer.start();
		frameTime = 0;
	}
	
	if(!m_play)
		frameTime = 0;
	
	frameTime *= m_playbackSpeed;
	
	if(m_particleSystemVisible) {
		if(m_particleSystem) {
			if(!m_particleSystem->IsAlive()) {
				delete m_particleSystem;
				m_particleSystem = NULL;
			}
		}
		
		if(!m_particleSystem) {
			m_particleSystem = new ParticleSystem();
			m_particleSystem->SetParams(m_particleParameters);
		}
	} else {
		delete m_particleSystem;
		m_particleSystem = NULL;
	}
	
	// TODO for some reason the frametime gets larger every frame, find out why !
	//arxtime.update_frame_time();
	//frameTime = arxtime.get_frame_delay();
	
	cam->updateCamera();
	
	if(m_particleSystem) {
		m_particleSystem->SetPos(Vec3f_ZERO);
		m_particleSystem->Update(GameDurationMs(frameTime));
	}
	
	//GRenderer->SetScissor(Rect::ZERO);
	GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, toColor(m_backgroundColor));
	//GRenderer->SetFogColor(Color::none);
	//GRenderer->SetRenderState(Renderer::Fog, false);
	
	drawModel(frameTime);
	//drawLights();
	
	//GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	//GRenderer->SetRenderState(Renderer::DepthTest, true);
	//GRenderer->SetRenderState(Renderer::AlphaBlending, false);
//	PopAllTriangleListOpaque();
	//GRenderer->SetRenderState(Renderer::AlphaBlending, true);
//	PopAllTriangleListTransparency();
	
//	if(m_particleSystem) {
//		g_renderBatcher.clear();
//		m_particleSystem->Render();
		
//		//GRenderer->SetFogColor(Color::none);
//		//GRenderer->SetRenderState(Renderer::DepthWrite, false);
//		g_renderBatcher.render();
//		//GRenderer->SetRenderState(Renderer::DepthWrite, true);
//	}
}

void ArxRenderWidget::drawModel(qint64 frameTime) {
	
	if(!m_currentObject) {
		return;
	}
	
	AnimLayer * animationLayers = NULL;
	
	if(m_animationLayersModel) {
		animationLayers = m_animationLayersModel->m_currentAnimation;
	}
	
	Anglef ePlayerAngle = Anglef::ZERO;
	Vec3f pos = Vec3f();
	float invisibility = 0.f;
	
	{
		EERIE_3DOBJ * eobj = m_currentObject;
		AnimationDuration time = AnimationDurationMs(frameTime);
		
		EERIEDrawAnimQuatUpdate(eobj, animationLayers, ePlayerAngle, pos, time, NULL, true);
		EERIEDrawAnimQuatRender(eobj, pos, NULL, invisibility);
		
		PopAllTriangleListOpaque(render3D().fog(false));
		PopAllTriangleListTransparency();
	}
	
	if(m_showVertexNormals || m_showFaceNormals) {
		EERIE_3DOBJ * eobj = m_currentObject;
		
		for(size_t i = 0; i < eobj->facelist.size(); i++) {
			const EERIE_FACE & face = eobj->facelist[i];
			
			Vec3f foo = Vec3f_ZERO;
			
			for(size_t n = 0; n < 3; n++) {
				short vidx = face.vid[n];
				EERIE_VERTEX & vert = eobj->vertexlist[vidx];
				if(m_showVertexNormals)
					drawLine(vert.v, vert.v + vert.norm * 10.f, Color::cyan);
				foo += vert.v;
			}
			Vec3f bar = foo / 3.f;
			
			if(m_showFaceNormals)
				drawLine(bar, bar + face.norm * 10.f, Color::magenta);
		}
	}
	
	if(m_showAxis) {
		drawCoordinates();
	}
}

void ArxRenderWidget::drawLights() {
	
	//GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	//GRenderer->SetCulling(CullNone);
	//GRenderer->SetBlendFunc(BlendOne, BlendOne);
	
	for(auto editorLight : m_lightsModel->lights) {
		if(editorLight.visualize) {
			EERIE_LIGHT & light = editorLight.light;
			
			drawLineSphere(Sphere(light.pos, light.fallstart), Color(Color3<u8>::green, 200));
			drawLineSphere(Sphere(light.pos, light.fallend), Color(Color3<u8>::red, 200));
		}
	}
}

void ArxRenderWidget::drawCoordinates() {
	
	//GRenderer->SetRenderState(Renderer::DepthTest, false);
	drawLine(Vec3f(0, 0, 0), Vec3f(200, 0, 0), Color::red, Color::red);
	drawLine(Vec3f(0, 0, 0), Vec3f(0, 200, 0), Color::green, Color::green);
	drawLine(Vec3f(0, 0, 0), Vec3f(0, 0, 200), Color::blue, Color::blue);
}

void ArxRenderWidget::resizeGL(int width, int height) {
	
	Rect rect = Rect(width, height);
	g_size = rect;
	m_dummyWindow->setWindowSize(rect.bottomRight());
	m_renderer->afterResize();
	m_renderer->SetViewport(rect);
	cam->updateCamera();
}

void ArxRenderWidget::mousePressEvent(QMouseEvent * event) {
	
	m_lastPos = event->pos();
}

void ArxRenderWidget::mouseMoveEvent(QMouseEvent * event) {
	
	QPoint d = event->pos() - m_lastPos;
	
	if(event->buttons().testFlag(Qt::LeftButton)) {
		cam->longitude += d.x() * 0.2f;
		cam->latitude += d.y() * -0.2f;
	} else if(event->buttons().testFlag(Qt::RightButton)) {
		cam->height += d.y() * 2;
	}
	
	m_lastPos = event->pos();
}

void ArxRenderWidget::wheelEvent(QWheelEvent * event) {
	
	if(event->delta() < 0) {
		cam->cameraDistance -= 50;
	} else {
		cam->cameraDistance += 50;
	}
}

