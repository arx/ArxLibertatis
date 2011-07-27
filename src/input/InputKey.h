#ifndef ARX_INPUT_INPUTKEY_H
#define ARX_INPUT_INPUTKEY_H

#include <string>

typedef int InputKeyId;

#define INPUT_KEYBOARD_MASK			0x0000FFFF
#define INPUT_MOUSE_MASK			0x2000000F
#define INPUT_MOUSEWHEEL_MASK		0x1000000F
#define INPUT_MASK					(INPUT_KEYBOARD_MASK | INPUT_MOUSE_MASK | INPUT_MOUSEWHEEL_MASK)
#define INPUT_COMBINATION_MASK		(~INPUT_MASK)


struct KeyDescription {
	InputKeyId id;
	std::string name;
};

#endif // ARX_INPUT_INPUTKEY_H
