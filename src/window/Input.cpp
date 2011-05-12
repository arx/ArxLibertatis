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

#include "window/Input.h"

#include <cstdio>

#include "core/Config.h"

#include "gui/MenuWidgets.h" //controls

#include "io/Logger.h"
#include "window/DXInput.h"

extern CDirectInput * pGetInfoDirectInput;
extern long STOP_KEYBOARD_INPUT;

bool ARX_INPUT_Init(HINSTANCE hInst, HWND hWnd) {
	
	DXI_Init(hInst);
	
	if(!DXI_GetKeyboardInputDevice(hWnd, DXI_MODE_NONEXCLUSIF_OURMSG)) {
		LogWarning << "could not grab the keyboeard";
		return false;
	}
	
	if(!DXI_GetMouseInputDevice(hWnd, DXI_MODE_NONEXCLUSIF_ALLMSG, 2, 2)) {
		LogWarning << "could not grab the mouse";
		return false;
	}
	
	if(!DXI_SetMouseRelative()) {
		LogWarning << "could not set mouse relative mode";
		return false;
	}
	
	return true;
}

void ARX_INPUT_Release() {
	DXI_Release();
}
 
//-----------------------------------------------------------------------------
bool ARX_IMPULSE_NowPressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			for (long j = 0; j < 2; j++)
			{
				if (config.actions[ident].key[j] != -1)
				{
					if (config.actions[ident].key[j] & 0x80000000)
					{
						if (pGetInfoDirectInput->GetMouseButtonNowPressed(config.actions[ident].key[j]&~0x80000000))
							return true;
					}
					else
					{
						if (config.actions[ident].key[j] & 0x40000000)
						{
							if (config.actions[ident].key[j] == 0x40000001)
							{
								if (pGetInfoDirectInput->iWheelSens < 0) return true;
							}
							else
							{
								if (pGetInfoDirectInput->iWheelSens > 0) return true;
							}
						}
						else
						{
							bool bCombine = true;

							if (config.actions[ident].key[j] & 0x7FFF0000)
							{
								if (!pGetInfoDirectInput->IsVirtualKeyPressed((config.actions[ident].key[j] >> 16) & 0xFFFF))
									bCombine = false;
							}

							if (pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(config.actions[ident].key[j] & 0xFFFF))
								return true & bCombine;
						}
					}
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
static unsigned int uiOneHandedMagicMode = 0;
static unsigned int uiOneHandedStealth = 0;

bool ARX_IMPULSE_Pressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			if (config.misc.forceToggle)
			{
				for (long j = 0; j < 2; j++)
				{
					if (config.actions[ident].key[j] != -1)
					{
						if (config.actions[ident].key[j] & 0x80000000)
						{
							if (pGetInfoDirectInput->GetMouseButtonRepeat(config.actions[ident].key[j]&~0x80000000))
								return true;
						}
						else
						{
							if (config.actions[ident].key[j] & 0x40000000)
							{
								if (config.actions[ident].key[j] == 0x40000001)
								{
									if (pGetInfoDirectInput->iWheelSens < 0) return true;
								}
								else
								{
									if (pGetInfoDirectInput->iWheelSens > 0) return true;
								}
							}
							else
							{
								bool bCombine = true;

								if (config.actions[ident].key[j] & 0x7FFF0000)
								{
									if (!pGetInfoDirectInput->IsVirtualKeyPressed((config.actions[ident].key[j] >> 16) & 0xFFFF))
										bCombine = false;
								}

								if (pGetInfoDirectInput->IsVirtualKeyPressed(config.actions[ident].key[j] & 0xFFFF))
								{
									bool bQuit = false;

									switch (ident)
									{
										case CONTROLS_CUST_MAGICMODE:
										{
											if (bCombine)
											{
												if (!uiOneHandedMagicMode)
												{
													uiOneHandedMagicMode = 1;
												}
												else
												{
													if (uiOneHandedMagicMode == 2)
													{
														uiOneHandedMagicMode = 3;
													}
												}

												bQuit = true;
											}
										}
										break;
										case CONTROLS_CUST_STEALTHMODE:
										{
											if (bCombine)
											{
												if (!uiOneHandedStealth)
												{
													uiOneHandedStealth = 1;
												}
												else
												{
													if (uiOneHandedStealth == 2)
													{
														uiOneHandedStealth = 3;
													}
												}

												bQuit = true;
											}
										}
										break;
										default:
										{
											return true & bCombine;
										}
										break;
									}

									if (bQuit)
									{
										break;
									}
								}
								else
								{
									switch (ident)
									{
										case CONTROLS_CUST_MAGICMODE:
										{
											if ((!j) &&
											        (pGetInfoDirectInput->IsVirtualKeyPressed(config.actions[ident].key[j+1] & 0xFFFF)))
											{
												continue;
											}

											if (uiOneHandedMagicMode == 1)
											{
												uiOneHandedMagicMode = 2;
											}
											else
											{
												if (uiOneHandedMagicMode == 3)
												{
													uiOneHandedMagicMode = 0;
												}
											}
										}
										break;
										case CONTROLS_CUST_STEALTHMODE:
										{
											if ((!j) &&
											        (pGetInfoDirectInput->IsVirtualKeyPressed(config.actions[ident].key[j+1] & 0xFFFF)))
											{
												continue;
											}

											if (uiOneHandedStealth == 1)
											{
												uiOneHandedStealth = 2;
											}
											else
											{
												if (uiOneHandedStealth == 3)
												{
													uiOneHandedStealth = 0;
												}
											}
										}
										break;
									}
								}
							}
						}
					}
				}

				switch (ident)
				{
					case CONTROLS_CUST_MAGICMODE:

						if ((uiOneHandedMagicMode == 1) || (uiOneHandedMagicMode == 2))
						{
							return true;
						}

						break;
					case CONTROLS_CUST_STEALTHMODE:

						if ((uiOneHandedStealth == 1) || (uiOneHandedStealth == 2))
						{
							return true;
						}

						break;
				}
			}
			else
			{
				for (long j = 0; j < 2; j++)
				{
					if (config.actions[ident].key[j] != -1)
					{
						if (config.actions[ident].key[j] & 0x80000000)
						{
							if (pGetInfoDirectInput->GetMouseButtonRepeat(config.actions[ident].key[j]&~0x80000000))
								return true;
						}
						else
						{
							if (config.actions[ident].key[j] & 0x40000000)
							{
								if (config.actions[ident].key[j] == 0x40000001)
								{
									if (pGetInfoDirectInput->iWheelSens < 0) return true;
								}
								else
								{
									if (pGetInfoDirectInput->iWheelSens > 0) return true;
								}
							}
							else
							{
								bool bCombine = true;

								if (config.actions[ident].key[j] & 0x7FFF0000)
								{
									if (!pGetInfoDirectInput->IsVirtualKeyPressed((config.actions[ident].key[j] >> 16) & 0xFFFF))
										bCombine = false;
								}

								if (pGetInfoDirectInput->IsVirtualKeyPressed(config.actions[ident].key[j] & 0xFFFF))
									return true & bCombine;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool ARX_IMPULSE_NowUnPressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			for (long j = 0; j < 2; j++)
			{
				if (config.actions[ident].key[j] != -1)
				{
					if (config.actions[ident].key[j] & 0x80000000)
					{
						if (pGetInfoDirectInput->GetMouseButtonNowUnPressed(config.actions[ident].key[j]&~0x80000000))
							return true;
					}
					else
					{
						bool bCombine = true;

						if (config.actions[ident].key[j] & 0x7FFF0000)
						{
							if (!pGetInfoDirectInput->IsVirtualKeyPressed((config.actions[ident].key[j] >> 16) & 0xFFFF))
								bCombine = false;
						}

						if (pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(config.actions[ident].key[j] & 0xFFFF))
							return true & bCombine;
					}
				}
			}
		}
	}

	return false;
}
