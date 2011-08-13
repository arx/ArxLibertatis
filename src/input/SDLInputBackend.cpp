
#include "input/SDLInputBackend.h"

SDLInputBackend::SDLInputBackend() {
	// TODO implement
}

SDLInputBackend::~SDLInputBackend() {
	// TODO implement
}

bool SDLInputBackend::init() {
	return true; // TODO implement
}

bool SDLInputBackend::update() {
	return true; // TODO implement
}

void SDLInputBackend::acquireDevices() {
	 // TODO implement
}

void SDLInputBackend::unacquireDevices() {
	// TODO implement
}

void SDLInputBackend::getMouseCoordinates(int & absX, int & absY, int & wheelDir) const {
	absX = absY = wheelDir = 0; // TODO implement
}

void SDLInputBackend::setMouseCoordinates(int absX, int absY) {
	ARX_UNUSED(absX), ARX_UNUSED(absY);// TODO implement
}

bool SDLInputBackend::isMouseButtonPressed(int buttonId, int & deltaTime) const  {
	ARX_UNUSED(buttonId), deltaTime = 0;
	return false; // TODO implement
}

void SDLInputBackend::getMouseButtonClickCount(int buttonId, int & numClick, int & numUnClick) const {
	ARX_UNUSED(buttonId), numClick = numUnClick = 0;
	// TODO implement
}

bool SDLInputBackend::isKeyboardKeyPressed(int dikkey) const {
	ARX_UNUSED(dikkey);
	return false; // TODO implement
}

bool SDLInputBackend::getKeyAsText(int keyId, char & result) const {
	ARX_UNUSED(keyId), result = 'F';
	return true; // TODO implement
}