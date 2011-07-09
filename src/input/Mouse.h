#ifndef ARX_INPUT_MOUSE_H
#define ARX_INPUT_MOUSE_H

class Mouse
{
public:
	enum Button
	{
		ButtonBase = 0x20000000,

		Button_0 = ButtonBase,
		Button_1,
		Button_2,
		Button_3,
		Button_4,
		Button_5,
		Button_6,
		Button_7,

		ButtonMax,
		ButtonCount = ButtonMax - ButtonBase
	};

	enum Wheel
	{
		WheelBase = 0x10000000,

		Wheel_Up = WheelBase,
		Wheel_Down
	};
};

#endif // ARX_INPUT_MOUSE_H
