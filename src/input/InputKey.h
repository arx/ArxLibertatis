#ifndef ARX_INPUT_INPUTKEY_H
#define ARX_INPUT_INPUTKEY_H

#include <string>

typedef int InputKeyId;

struct KeyDescription {
	InputKeyId id;
	std::string name;
};

#endif // ARX_INPUT_INPUTKEY_H
