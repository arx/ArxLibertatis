/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Input
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Input interface (uses MERCURY input system)
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_WINDOW_INPUT_H
#define ARX_WINDOW_INPUT_H

//-----------------------------------------------------------------------------
class InputDevice
{
};

class Keyboard : public InputDevice
{
public:
    //! Key enumeration.
    enum Key
    {
		KeyBase = 0,

        Key_0 = KeyBase,
        Key_1,
        Key_2,
        Key_3,
        Key_4,
        Key_5,
        Key_6,
        Key_7,
        Key_8,
        Key_9,
        
        Key_A,
        Key_B,
        Key_C,
        Key_D,
        Key_E,
        Key_F,
        Key_G,
        Key_H,
        Key_I,
        Key_J,
        Key_K,
        Key_L,
        Key_M,    
        Key_N,
        Key_O,
        Key_P,
        Key_Q,
        Key_R,
        Key_S,
        Key_T,
        Key_U,
        Key_V,
        Key_W,
        Key_X,
        Key_Y,
        Key_Z,

        Key_F1,
        Key_F2,
        Key_F3,
        Key_F4,
        Key_F5,
        Key_F6,
        Key_F7,
        Key_F8,
        Key_F9,
        Key_F10,
        Key_F11,
        Key_F12,
        Key_F13,
        Key_F14,
        Key_F15,

        Key_UpArrow,
        Key_DownArrow,
        Key_LeftArrow,
        Key_RightArrow,

        Key_Home,
        Key_End,
        Key_PageUp,
        Key_PageDown,
        Key_Insert,
        Key_Delete,

        Key_Escape,
        
        Key_NumLock,
        Key_NumPad0,
        Key_NumPad1,
        Key_NumPad2,
        Key_NumPad3,
        Key_NumPad4,
        Key_NumPad5,
        Key_NumPad6,
        Key_NumPad7,
        Key_NumPad8,
        Key_NumPad9,
        Key_NumPadEnter,
        Key_NumSubtract,        // (-) on numeric keypad 
        Key_NumAdd,             // (+) on numeric keypad 
        Key_NumMultiply,        // (*) on numeric keypad 
        Key_NumDivide,          // (/) on numeric keypad 
        Key_NumPoint,           // PERIOD (decimal point) on numeric keypad 

        Key_LeftBracket,        // Left square bracket [ 
        Key_LeftCtrl,           // Left CTRL         
        Key_LeftAlt,            // Left ALT 
        Key_LeftShift,          // Left SHIFT 
        Key_LeftWin,            // Left Microsoft® Windows® logo key 

        Key_RightBracket,       // Right square bracket ] 
        Key_RightCtrl,          // Right CTRL 
        Key_RightAlt,           // Right ALT 
        Key_RightShift,         // Right SHIFT 
        Key_RightWin,           // Right Windows logo key 

        Key_PrintScreen,
        Key_ScrollLock,
        Key_Pause,
        
        Key_Spacebar,
        Key_Backspace,
        Key_Enter,              // ENTER on main keyboard 
        Key_Tab,            

        Key_Apps,               // Application key 
        Key_CapsLock,

        Key_Slash,              // (/) On main keyboard 
        Key_Backslash,          // (\) 
        Key_Comma,              // (,)
        Key_Semicolon,          // (;)
        Key_Period,             // (.) On main keyboard 
        Key_Grave,              // (`) Grave accent 
        Key_Apostrophe,         // (')
        Key_Minus,              // (-) On main keyboard 
        Key_Equals,             // (=) On main keyboard 

        KeyMax,
		KeyCount = KeyMax - KeyBase
    };

    //! State of a keyboard key.
    enum KeyState
    {
        Key_Up,                 //!< Key is up.
        Key_Pressed,            //!< Key was up and is now down.
        Key_Down,               //!< Key is down.
        Key_Released            //!< Key was down and is now up.
    };
};

class Mouse : public InputDevice
{
public:
	enum Button
	{
		ButtonBase = 0x80000000,

		Button_1 = ButtonBase,
		Button_2,
		Button_3,
		Button_4,
		Button_5,
		Button_6,
		Button_7,
		Button_8,

		ButtonMax,
		ButtonCount = ButtonMax - ButtonBase
	};

	enum Wheel
	{
		WheelBase = 0x40000000,

		Wheel_Up = WheelBase,
		Wheel_Down
	};
};

typedef int InputKeyId;

struct KeyDescription {
	InputKeyId id;
	std::string name;
};

class Input
{
public:
	static std::string	getKeyName(InputKeyId key, bool localizedName = false);
	static InputKeyId	getKeyId(const std::string& keyName);

public:
	static const std::string KEY_NONE;
};


class CDirectInput {
	
public:
	
	// Keyboard
	bool	bKeyTouched;		// Was a key pressed in the last update
	int		iKeyId;				// Id of the key pressed in the last update

	// Mouse
	bool	bMouseMoved;
	int		iMouseRX;
	int		iMouseRY;
	int		iMouseAX;
	int		iMouseAY;
 
public:
	CDirectInput();
	virtual ~CDirectInput();

	void GetInput();

	void SetMousePosition(int mouseX, int mouseY);

	void SetSensibility(int);
	int  GetSensibility() const;

	bool GetMouseButton(int buttonId) const;
	int  GetMouseButtonClicked() const;
	bool GetMouseButtonRepeat(int buttonId) const;
	bool GetMouseButtonNowPressed(int buttonId) const;
	bool GetMouseButtonNowUnPressed(int buttonId) const;
	bool GetMouseButtonDoubleClick(int buttonId, int timeMs) const;
	
	bool IsVirtualKeyPressed(int) const;
	bool IsVirtualKeyPressedNowPressed(int) const;
	bool IsVirtualKeyPressedNowUnPressed(int) const;
	 
	void ResetAll();
	int GetWheelDir() const;
	
private:
	int	iOneTouch[256];

	float fMouseAXTemp;
	float fMouseAYTemp;

	int	iSensibility;
	int	iWheelDir;

	bool bMouseButton[Mouse::ButtonCount];
	bool bOldMouseButton[Mouse::ButtonCount];

	int	iMouseTime[Mouse::ButtonCount];
	int	iMouseTimeSet[Mouse::ButtonCount];
	int	iOldNumClick[Mouse::ButtonCount];
};



bool ARX_INPUT_Init();
void ARX_INPUT_Release();

bool ARX_IMPULSE_NowPressed(long ident);
bool ARX_IMPULSE_Pressed(long ident);
bool ARX_IMPULSE_NowUnPressed(long ident);
 
#endif // ARX_WINDOW_INPUT_H
