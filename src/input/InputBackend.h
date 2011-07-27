#ifndef ARX_INPUT_INPUTBACKEND_H
#define ARX_INPUT_INPUTBACKEND_H

class InputBackend {
public:
	virtual ~InputBackend() {}

	virtual bool init() = 0;
	virtual void acquireDevices() = 0;
	virtual void unacquireDevices() = 0;

	virtual bool update() = 0;	

	// Mouse 
	virtual void getMouseCoordinates(int & absX, int & absY, int & wheelDir) const = 0;
	virtual void setMouseCoordinates(int absX, int absY) = 0;
	virtual bool isMouseButtonPressed(int buttonId, int & _iDeltaTime) const = 0;
	virtual void getMouseButtonClickCount(int buttonId, int & _iNumClick, int & _iNumUnClick) const = 0;

	// Keyboard
	virtual bool isKeyboardKeyPressed(int keyId) const = 0;
	virtual bool getKeyAsText(int keyId, char& result) const = 0;
};

#endif // ARX_INPUT_INPUTBACKEND_H
