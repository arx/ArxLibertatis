
#include "core/SDLWindow.h"

#include "graphics/opengl/OpenGLRenderer.h"

SDLWindow::SDLWindow() { }

SDLWindow::~SDLWindow() {
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
	}
	
	OnDestroy();
}

bool SDLWindow::Init(const std::string & title, int width, int height, bool visible, bool fullscreen) {
	
	ARX_UNUSED(title), ARX_UNUSED(width), ARX_UNUSED(height), ARX_UNUSED(visible), ARX_UNUSED(fullscreen);
	
	OnCreate();
	
	renderer = new OpenGLRenderer;
	renderer->Initialize();
	
	onRendererInit();
	
	OnShow(true);
	OnFocus(true);
	
	return true; // TODO implement
}

void * SDLWindow::GetHandle() {
	return NULL; // TODO implement
}

void SDLWindow::SetFullscreen(bool fullscreen) {
	// TODO implement
	if(fullscreen) {
		OnMakeFullscreen();
	} else {
		OnRestore();
	}
}

void SDLWindow::SetSize(Vec2i size) {
	// TODO implement
	OnResize(size.x, size.y);
}

void SDLWindow::Tick() {
	// TODO implement
}

bool SDLWindow::showFrame() {
	return true; // TODO implement
}

void SDLWindow::restoreSurfaces() {
	// TODO implement
}

void SDLWindow::evictManagedTextures() {
	// TODO implement
}
