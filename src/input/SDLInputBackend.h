
#ifndef ARX_INPUT_SDLINPUTBACKEND_H
#define ARX_INPUT_SDLINPUTBACKEND_H

#include "input/Input.h"
#include "input/InputBackend.h"

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
	
};

#endif // ARX_INPUT_SDLINPUTBACKEND_H
