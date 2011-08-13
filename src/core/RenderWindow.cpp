
#include "core/RenderWindow.h"

#include <algorithm>

void RenderWindow::addListener(RendererListener * listener) {
	renderListeners.push_back(listener);
}

void RenderWindow::removeListener(RendererListener * listener) {
	renderListeners.erase(std::remove(renderListeners.begin(), renderListeners.end(), listener), renderListeners.end());
}

void RenderWindow::onRendererInit() {
	for(RendererListeners::iterator i = renderListeners.begin(); i != renderListeners.end(); ++i) {
		(*i)->onRendererInit(*this);
	}
}

void RenderWindow::onRendererShutdown() {
	for(RendererListeners::iterator i = renderListeners.begin(); i != renderListeners.end(); ++i) {
		(*i)->onRendererShutdown(*this);
	}
}
