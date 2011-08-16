
#ifndef ARX_INPUT_SDLINPUTBACKEND_H
#define ARX_INPUT_SDLINPUTBACKEND_H

#include "input/Input.h"
#include "input/InputBackend.h"

union SDL_Event;

class SDLInputBackend : public InputBackend {
	
public:
	
	SDLInputBackend();
	~SDLInputBackend();
	
	bool init();
	bool update();
	
	void acquireDevices();
	void unacquireDevices();
	
	// Mouse
	void getMouseCoordinates(int & absX, int & absY, int & wheelDir) const;
	void setMouseCoordinates(int absX, int absY);
	bool isMouseButtonPressed(int buttonId, int & deltaTime) const;
	void getMouseButtonClickCount(int buttonId, int & numClick, int & numUnClick) const;
	
	// Keyboard
	bool isKeyboardKeyPressed(int dikkey) const;
	bool getKeyAsText(int keyId, char& result) const;
	
private:
	
	void onInputEvent(const SDL_Event & event);
	
	int wheel;
	Vec2i cursor;
	bool keyStates[Keyboard::KeyCount];
	bool buttonStates[Mouse::ButtonCount];
	size_t clickCount[Mouse::ButtonCount];
	size_t unclickCount[Mouse::ButtonCount];
	
	int currentWheel;
	size_t currentClickCount[Mouse::ButtonCount];
	size_t currentUnclickCount[Mouse::ButtonCount];
	
	friend class SDLWindow;
};

#endif // ARX_INPUT_SDLINPUTBACKEND_H
