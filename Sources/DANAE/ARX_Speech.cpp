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
// ARX_Speech
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Speech & Conversation Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "danae.h"
#include "ARX_Interface.h"
#include "ARX_Speech.h"
#include "ARX_Text.h"
#include "ARX_Script.h"
#include "ARX_Sound.h"
#include "ARX_Input.h"
#include "ARX_Text.h"
#include "ARX_Loc.h"
#include "ARX_Time.h"

#include "EERIEDRAW.h"

#include "hermesmain.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


//-----------------------------------------------------------------------------
extern TextureContainer *	arx_logo_tc;
extern long ARX_CONVERSATION;
extern long EXTERNALVIEW;
extern long REQUEST_SPEECH_SKIP;

//-----------------------------------------------------------------------------
ARX_SPEECH aspeech[MAX_ASPEECH];
long HIDESPEECH = 0;
STRUCT_SPEECH speech[MAX_SPEECH];


//-----------------------------------------------------------------------------
void ARX_SPEECH_Init()
{
	memset(speech, 0, sizeof(STRUCT_SPEECH)*MAX_SPEECH);
}

//-----------------------------------------------------------------------------
void ARX_SPEECH_MoveUp()
{
	if (speech[0].timecreation != 0)
	{
		if (speech[0].lpszUText != NULL)
		{
			free(speech[0].lpszUText);
			speech[0].lpszUText = NULL;
		}
	}

	for (long j = 0; j < MAX_SPEECH - 1; j++)
	{
		memcpy(&speech[j], &speech[j+1], sizeof(STRUCT_SPEECH));
	}

	memset(&speech[MAX_SPEECH-1], 0, sizeof(STRUCT_SPEECH));
}

//-----------------------------------------------------------------------------
void ARX_SPEECH_ClearAll()
{
	for (long i = 0; i < MAX_SPEECH; i++)
	{
		if (speech[i].timecreation != 0)
		{
			if (speech[i].lpszUText != NULL)
			{
				free(speech[i].lpszUText);
				speech[i].lpszUText = NULL;
			}

			speech[i].timecreation = 0;
		}
	}
}

//-----------------------------------------------------------------------------
long ARX_SPEECH_Add(INTERACTIVE_OBJ * io, _TCHAR * _lpszUText, long duration)
{
	if (_lpszUText == NULL) return -1;

	if (_lpszUText[0] == 0) return -1;

	unsigned long tim = ARXTimeUL(); 

	if (tim == 0) tim = 1;

	if (speech[MAX_SPEECH-1].timecreation != 0)
		ARX_SPEECH_MoveUp();

	for (long i = 0; i < MAX_SPEECH; i++)
	{
		if (speech[i].timecreation == 0)
		{
			long length = _tcslen(_lpszUText);

			// We allocate memory for new speech
			speech[i].lpszUText = (_TCHAR *) malloc(__min(length + 1, 4096) * sizeof(_TCHAR)); 

			// Sets creation time
			speech[i].timecreation = tim;

			// Sets/computes speech duration
			if (duration == -1) speech[i].duration = 2000 + length * 60; 
			else speech[i].duration = duration;

			if (length > 4095)
			{
				memcpy(&speech[i].lpszUText, &_lpszUText, 4095 * sizeof(_TCHAR));
				speech[i].lpszUText[4095] = 0;
			}
			else _tcscpy(speech[i].lpszUText, _lpszUText);

			// Sets speech color
			if (io == NULL)
			{
				speech[i].io = NULL;
				strcpy(speech[i].name, " ");
			}
			else
			{
				speech[i].io = io;
				strcpy(speech[i].name, GetName(io->filename));
			}

			speech[i].color = D3DRGB(1.f, 1.f, 1.f);
			// Successfull allocation
			return speech[i].duration;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
bool CheckLastSpeech(int _iI)
{
	for (long i = _iI + 1; i < MAX_SPEECH; i++)
	{
		if ((speech[i].timecreation != 0) &&
		        (speech[i].lpszUText != NULL))
		{
			return false;
		}
	}

	return true;
}
//-----------------------------------------------------------------------------
void ARX_SPEECH_Render(LPDIRECT3DDEVICE7 pd3dDevice)
{
	_TCHAR temp[4096];
	long igrec = 14;

	HDC	hDC;
	SIZE sSize;

	if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
	{
		SelectObject(hDC, InBookFont);

		GetTextExtentPoint32W(hDC,
		                      _T("p"),
		                      1,
		                      &sSize);
		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);

		sSize.cy *= 3;
	}
	else
	{
		sSize.cy = DANAESIZY >> 1;
	}

	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(pd3dDevice, TRUE);

	int iEnd = igrec + sSize.cy;

	for (long i = 0; i < MAX_SPEECH; i++)
	{
		if (speech[i].timecreation != 0)
		{
			if (speech[i].lpszUText != NULL)
			{

				if ((speech[i].name) && (speech[i].name[0] != ' '))
					_stprintf(temp, _T("%S > %s"), speech[i].name, speech[i].lpszUText);
				else
					_stprintf(temp, _T(" %s"), speech[i].lpszUText);//>



				EERIEDrawBitmap(GDevice,
				                120 * Xratio - 16 * Xratio, ARX_CLEAN_WARN_CAST_FLOAT(igrec),
				                16 * Xratio, 16 * Xratio,
				                0.00001f,
				                arx_logo_tc,
				                D3DCOLORWHITE);


				igrec += ARX_TEXT_DrawRect(pd3dDevice, InBookFont,
				                           120.f * Xratio, (float)igrec,
				                           -3, 0,
				                           500 * Xratio, 200 * Yratio, temp, speech[i].color, NULL, 0x00FF00FF, 1);
				if ((igrec > iEnd) &&
				        !CheckLastSpeech(i))
				{
					ARX_SPEECH_MoveUp();
					break;
				}
			}
		}
	}

	SETALPHABLEND(pd3dDevice, FALSE);
}

void ARX_SPEECH_Check(LPDIRECT3DDEVICE7 pd3dDevice)
{
	bool bClear = false;
	long exist = 0;

	for (long i = 0; i < MAX_SPEECH; i++)
	{
		if (speech[i].timecreation != 0)
		{
			if (ARXTime > speech[i].timecreation + speech[i].duration)
			{
				ARX_SPEECH_MoveUp();
				i--;
			}
			else exist++;

			bClear = true;
		}
	}

	if (bClear)
	{
		if (pTextManage)
		{
			pTextManage->Clear();
		}
	}

	if (exist) ARX_SPEECH_Render(pd3dDevice);
}

//-----------------------------------------------------------------------------
void ARX_SPEECH_Launch_No_Unicode_Seek(char * string, INTERACTIVE_OBJ * io_source, long mood)
{
	mood = ANIM_TALK_NEUTRAL;
	long speechnum = ARX_SPEECH_AddSpeech(io_source, string, PARAM_LOCALISED, mood, 4);

	if (speechnum >= 0)
	{
		aspeech[speechnum].scrpos = -1;
		aspeech[speechnum].es = NULL;
		aspeech[speechnum].ioscript = io_source;
		aspeech[speechnum].flags = 0;
		ARX_CINEMATIC_SPEECH acs;
		acs.type = ARX_CINE_SPEECH_NONE;
		memcpy(&aspeech[speechnum].cine, &acs, sizeof(ARX_CINEMATIC_SPEECH));
	}
}


ARX_CONVERSATION_STRUCT main_conversation;
void ARX_CONVERSATION_FirstInit()
{
	main_conversation.actors_nb = 0;
	main_conversation.current = -1;
}
void ARX_CONVERSATION_Reset()
{
	main_conversation.actors_nb = 0;
	main_conversation.current = -1;
}

void ARX_CONVERSATION_CheckAcceleratedSpeech()
{
	if (REQUEST_SPEECH_SKIP) 
	{
		for (long i = 0; i < MAX_ASPEECH; i++)
		{
			if ((aspeech[i].exist) && !(aspeech[i].flags & ARX_SPEECH_FLAG_UNBREAKABLE))
			{
				aspeech[i].duration = 0;
			}
		}

		REQUEST_SPEECH_SKIP = 0;
	}
}


void ARX_SPEECH_FirstInit()
{
	memset(aspeech, 0, sizeof(ARX_SPEECH)*MAX_ASPEECH);
}
long ARX_SPEECH_GetFree()
{
	for (long i = 0; i < MAX_ASPEECH; i++)
	{
		if (!aspeech[i].exist)
		{
			aspeech[i].cine.type = 0;
			return i;
		}
	}

	return -1;
}


long ARX_SPEECH_GetIOSpeech(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < MAX_ASPEECH; i++)
	{
		if ((aspeech[i].exist)
		        &&	(aspeech[i].io == io))
			return i;
	}

	return -1;
}

void ARX_SPEECH_Release(long i)
{
	if (aspeech[i].exist)
	{
		ARX_SOUND_Stop(aspeech[i].sample);

		if (aspeech[i].text != NULL)
			free(aspeech[i].text);

		if ((ValidIOAddress(aspeech[i].io))
		        &&	(aspeech[i].io->animlayer[2].cur_anim))
		{
			AcquireLastAnim(aspeech[i].io);
			aspeech[i].io->animlayer[2].cur_anim = NULL;
		}

		memset(&aspeech[i], 0, sizeof(ARX_SPEECH));
	}
}
void ARX_SPEECH_ReleaseIOSpeech(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < MAX_ASPEECH; i++)
	{
		if ((aspeech[i].exist)
		        &&	(aspeech->io == io))
		{
			ARX_SPEECH_Release(i);
		}
	}
}


 
void ARX_SPEECH_Reset()
{
	for (long i = 0; i < MAX_ASPEECH; i++)
	{
		ARX_SPEECH_Release(i);
	}
}

void ARX_SPEECH_ClearIOSpeech(INTERACTIVE_OBJ * io)
{
	if (!io) return;

	for (long i = 0; i < MAX_ASPEECH; i++)
	{
		if ((aspeech[i].exist)
		        &&	(aspeech[i].io == io))
		{
			EERIE_SCRIPT * es = aspeech[i].es;
			INTERACTIVE_OBJ * io = aspeech[i].ioscript;
			long scrpos = aspeech[i].scrpos;
			ARX_SPEECH_Release(i);

			if ((es)
			        &&	(ValidIOAddress(io)))
			{
				SendScriptEvent(es, SM_EXECUTELINE, "", io, NULL, scrpos);
			}
		}
	}
}


long ARX_SPEECH_AddSpeech(INTERACTIVE_OBJ * io, char * data, long param, long mood, long flags)
{
	if (!data || !data[0]) return -1;

	long num = ARX_SPEECH_GetFree();

	if (num < 0) return -1;

	long ioo = ARX_SPEECH_GetIOSpeech(io);

	if (ioo != -1)
	{
		for (long i = 0; i < MAX_ASPEECH; i++)
			if (aspeech[i].exist && aspeech[i].io == io)
			{
				EERIE_SCRIPT * es = aspeech[i].es;
				INTERACTIVE_OBJ * io = aspeech[i].ioscript;
				long scrpos = aspeech[i].scrpos;
				ARX_SPEECH_Release(i);

				if ((es)
				        &&	(ValidIOAddress(io)))
					SendScriptEvent(es, SM_EXECUTELINE, "", io, NULL, scrpos);
			}
	}

	aspeech[num].exist = 1;
	aspeech[num].time_creation = ARX_TIME_GetUL();
	aspeech[num].io = io; // can be NULL
	aspeech[num].duration = 2000; // Minimum value
	aspeech[num].flags = flags;
	aspeech[num].sample = -1;
	aspeech[num].fDeltaY = 0.f;
	aspeech[num].iTimeScroll = 0;
	aspeech[num].fPixelScroll = 0.f;
	aspeech[num].color = 0xFFFFFFFF;
	aspeech[num].mood = mood;

	long flg = 0;

	_TCHAR lpszUSection[512];
	MultiByteToWideChar(CP_ACP, 0, data, -1, lpszUSection, 512);

	if (!(flags & ARX_SPEECH_FLAG_NOTEXT))
	{
		_TCHAR _output[4096];
		ZeroMemory(_output, 4096 * sizeof(_TCHAR));

		flg = HERMES_UNICODE_GetProfileString(lpszUSection,
		                                      _T("string"),
		                                      _T(""),
		                                      _output,
		                                      4096,
		                                      NULL,
		                                      io->lastspeechflag
		                                     );


		io->lastspeechflag = (short)flg;
		aspeech[num].text = (_TCHAR *) malloc((_tcslen(_output) + 1) * sizeof(_TCHAR));
		_tcscpy(aspeech[num].text, _output);
		aspeech[num].duration = __max(aspeech[num].duration, (_tcslen(_output) + 1) * 100);
	}

	char speech_label[256];
	char speech_sample[256];

	strcpy(speech_label, data + 1);
	speech_label[strlen(speech_label) - 1] = 0;

	if (flags & ARX_SPEECH_FLAG_NOTEXT)
	{
		long count = 0;

		count = HERMES_UNICODE_GetProfileSectionKeyCount(lpszUSection);

		F2L((float)(rnd() *(float)count), &flg);

		while ((io->lastspeechflag == flg) && (count > 1))
		{
			F2L((float)(rnd() *(float)count), &flg);
		}

		if (flg > count) flg = count;
		else if (flg <= 0) flg = 1;

		io->lastspeechflag = (short)flg;
	}

	if (flg > 1)
		sprintf(speech_sample, "%s%d", speech_label, flg);
	else
		strcpy(speech_sample, speech_label);

	if (aspeech[num].flags & ARX_SPEECH_FLAG_OFFVOICE)
		aspeech[num].sample = ARX_SOUND_PlaySpeech(speech_sample);
	else
		aspeech[num].sample = ARX_SOUND_PlaySpeech(speech_sample, io);

	//Next lines must be removed (use callback instead)
	aspeech[num].duration = (unsigned long)ARX_SOUND_GetDuration(aspeech[num].sample);

	//This div is not good in the intro because speakpitch value is bad
	//But correct it inside arx_script ligne 7913 create a crash
	float fDiv = aspeech[num].duration / io->_npcdata->speakpitch;
	//ARX_CHECK_ULONG(fDiv);

	if ((io->ioflags & IO_NPC) && !(aspeech[num].flags & ARX_SPEECH_FLAG_OFFVOICE))
		aspeech[num].duration = ARX_CLEAN_WARN_CAST_ULONG(fDiv);



	if (aspeech[num].duration < 500) aspeech[num].duration = 2000;

	if (ARX_CONVERSATION && io)
		for (long j = 0; j < main_conversation.actors_nb; j++)
			if (main_conversation.actors[j] >= 0 && io == inter.iobj[main_conversation.actors[j]])
				main_conversation.current = num;

	return num;
}


//*************************************************************************************
//*************************************************************************************
void ARX_SPEECH_Update(LPDIRECT3DDEVICE7 pd3dDevice)
{
	unsigned long	tim		= ARXTimeUL(); 	
 

	if (CINEMASCOPE || BLOCK_PLAYER_CONTROLS) ARX_CONVERSATION_CheckAcceleratedSpeech();

	for (long i = 0 ; i < MAX_ASPEECH ; i++)
	{
		if (aspeech[i].exist)
		{
			INTERACTIVE_OBJ * io = aspeech[i].io;

			// updates animations
			if (io)
			{
				if (aspeech[i].flags & ARX_SPEECH_FLAG_OFFVOICE)
					ARX_SOUND_RefreshSpeechPosition(aspeech[i].sample);
				else
					ARX_SOUND_RefreshSpeechPosition(aspeech[i].sample, io);

				if (((io != inter.iobj[0]) || ((io == inter.iobj[0])  && (EXTERNALVIEW)))
				        &&	ValidIOAddress(io))
				{
					if (io->anims[aspeech[i].mood] == NULL)	aspeech[i].mood = ANIM_TALK_NEUTRAL;

					if (io->anims[aspeech[i].mood] != NULL)
					{
						if ((io->animlayer[2].cur_anim != io->anims[aspeech[i].mood])
						        ||	(io->animlayer[2].flags & EA_ANIMEND))
						{
							AcquireLastAnim(io);
							ANIM_Set(&io->animlayer[2], io->anims[aspeech[i].mood]);
						}
					}
				}
			}

			// checks finished speech
			if (tim >= aspeech[i].time_creation + aspeech[i].duration)
			{
				EERIE_SCRIPT	*	es		= aspeech[i].es;
				INTERACTIVE_OBJ	* io		= aspeech[i].ioscript;
				long				scrpos	= aspeech[i].scrpos;
				ARX_SPEECH_Release(i);

				if ((es)
				        &&	(ValidIOAddress(io)))
					SendScriptEvent(es, SM_EXECUTELINE, "", io, NULL, scrpos);
			}
		}
	}

	for (int i = 0 ; i < MAX_ASPEECH ; i++)
	{
		ARX_SPEECH * speech = &aspeech[i];

		if (speech->exist)
		{
			if (speech->text != NULL)
			{
				if ((ARX_CONVERSATION) && (speech->io))
				{
					long ok = 0;

					for (long j = 0 ; j < main_conversation.actors_nb ; j++)
					{
						if (main_conversation.actors[j] >= 0)
							if (speech->io == inter.iobj[main_conversation.actors[j]])
							{
								ok = 1;
							}
					}

					if (!ok) goto next;
				}

				if (CINEMASCOPE && !HIDESPEECH)
				{
					if (CINEMA_DECAL >= 100.f)
					{
						HRGN	hRgn;
						HDC		hDC;
						SIZE	sSize;

						sSize.cx = sSize.cy = 0;

						if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
						{
							SelectObject(hDC, InBookFont);

							GetTextExtentPoint32W(hDC,
							                      speech->text,
							                      _tcslen(speech->text),
							                      &sSize);
							danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
						}

						else
						{
							ARX_CHECK_NO_ENTRY();
						}



						float fZoneClippHeight	=	ARX_CLEAN_WARN_CAST_FLOAT(sSize.cy * 3);
						float fStartYY			=	100 * Yratio;
						float fStartY			=	ARX_CLEAN_WARN_CAST_FLOAT(((int)fStartYY - (int)fZoneClippHeight) >> 1);
						float fDepY				=	((float)DANAESIZY) - fStartYY + fStartY - speech->fDeltaY + sSize.cy;
						float fZoneClippY		=	fDepY + speech->fDeltaY;


						float fAdd = fZoneClippY + fZoneClippHeight ;
						ARX_CHECK_INT(fZoneClippY);
						ARX_CHECK_INT(fAdd);

						hRgn = CreateRectRgn(0,
						                     ARX_CLEAN_WARN_CAST_INT(fZoneClippY),
						                     DANAESIZX,
						                     ARX_CLEAN_WARN_CAST_INT(fAdd));


						danaeApp.DANAEEndRender();
						float iTaille = (float)ARX_TEXT_DrawRect(
						                    pd3dDevice,
						                    InBookFont,
						                    10.f,
						                    fDepY + fZoneClippHeight,
						                    -3,
						                    0,
						                    -10.f + (float)DANAESIZX,
						                    0,		//taille recalculée
						                    speech->text,
						                    RGB(255, 255, 255),
						                    hRgn);

						if (hRgn)
						{
							DeleteObject(hRgn);
						}

						danaeApp.DANAEStartRender();

						SETTC(GDevice, NULL);
						GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
						GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
						GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
						GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
						EERIEDrawFill2DRectDegrad(GDevice,
						                          0.f,
						                          fZoneClippY - 1.f, 
						                          ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZX),
						                          fZoneClippY + (sSize.cy * 3 / 4),
						                          0.f,
						                          RGBA_MAKE(255, 255, 255, 255),
						                          RGBA_MAKE(0, 0, 0, 255));

						EERIEDrawFill2DRectDegrad(GDevice,
						                          0.f,
						                          fZoneClippY + fZoneClippHeight - (sSize.cy * 3 / 4),
						                          ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZX),
						                          fZoneClippY + fZoneClippHeight,
						                          0.f,
						                          RGBA_MAKE(0, 0, 0, 255),
						                          RGBA_MAKE(255, 255, 255, 255));

						GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
						GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
				
						danaeApp.EnableZBuffer();
						GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);


						iTaille += (int)fZoneClippHeight;

						if (((int)speech->fDeltaY) <= iTaille)
						{
							//vitesse du scroll
							float fDTime;

							if (speech->sample)
							{
								fDTime				= ((float)iTaille * (float)FrameDiff) / (float)ARX_SOUND_GetDuration(speech->sample);    //speech->duration;
								float fTimeOneLine	= ((float)sSize.cy) * fDTime;

								if (((float)speech->iTimeScroll) >= fTimeOneLine)
								{
									float fResteLine	 = (float)sSize.cy - speech->fPixelScroll;
 
									float fTimePlus		 = ((float)fResteLine * (float)FrameDiff) / (float)ARX_SOUND_GetDuration(speech->sample);
									fDTime				-= fTimePlus;
									speech->fPixelScroll = 0.f;
									speech->iTimeScroll	 = 0;
								}


								ARX_CHECK_INT(speech->iTimeScroll + FrameDiff);
								speech->iTimeScroll	+= ARX_CLEAN_WARN_CAST_INT(FrameDiff);


							}
							else
							{
								fDTime = ((float)iTaille * (float)FrameDiff) / 4000.f;
							}

							speech->fDeltaY			+= fDTime;
							speech->fPixelScroll	+= fDTime;
						}
					}
				}

			
			}

		next:
			;
		}
	}


}

//-----------------------------------------------------------------------------
BOOL ApplySpeechPos(EERIE_CAMERA * conversationcamera, long is)
{
	if (is < 0) return FALSE;

	if (aspeech[is].io == NULL)  return FALSE;


	conversationcamera->d_pos.x = aspeech[is].io->pos.x;
	conversationcamera->d_pos.y = aspeech[is].io->pos.y + PLAYER_BASE_HEIGHT;
	conversationcamera->d_pos.z = aspeech[is].io->pos.z;
	float t = (aspeech[is].io->angle.b);
	conversationcamera->pos.x = conversationcamera->d_pos.x + (float)EEsin(t) * 100.f;
	conversationcamera->pos.y = conversationcamera->d_pos.y;
	conversationcamera->pos.z = conversationcamera->d_pos.z - (float)EEcos(t) * 100.f;

	return TRUE;
}
