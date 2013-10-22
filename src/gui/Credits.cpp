/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
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

#include "gui/Credits.h"

#include <sstream>

#include "core/Core.h"
#include "core/GameTime.h"

#include "gui/Menu.h"
#include "gui/Text.h"
#include "gui/MenuWidgets.h"

#include "graphics/Draw.h"
#include "graphics/font/Font.h"

#include "io/log/Logger.h"

#include "scene/GameSound.h"

using std::string;
using std::vector;

// TODO extern globals
extern bool bFadeInOut;
extern bool bFade;
extern int iFadeAction;
void ARX_MENU_LaunchAmb(const string & _lpszAmb);


struct CreditsTextInformations {
	
	CreditsTextInformations() {
		sPos = Vec2i_ZERO;
		fColors = Color::none;
	}
	
	string  sText;
	Color fColors;
	Vec2i sPos;
};


struct CreditsInformations {
	
	CreditsInformations() : iFirstLine(0), iFontAverageHeight(-1), sizex(0), sizey(0) { }
	
	int iFirstLine;
	int iFontAverageHeight;
	
	int sizex, sizey; // save the screen size so we know when to re-initialize the credits
	
	vector<CreditsTextInformations> aCreditsInformations;
};


static CreditsInformations CreditsData;

static void InitCredits();
static void CalculAverageWidth();
static void ExtractAllCreditsTextInformations();

static void InitCredits() {
	
	if(CreditsData.iFontAverageHeight != -1
		&& CreditsData.sizex == g_size.width() && CreditsData.sizey == g_size.height()) {
		return;
	}
	
	CreditsData.sizex = g_size.width();
	CreditsData.sizey = g_size.height();
	
	CreditsData.aCreditsInformations.clear();
	
	LogDebug("InitCredits");
	
	CalculAverageWidth();
	ExtractAllCreditsTextInformations();
	
	LogDebug("Credits lines " << CreditsData.aCreditsInformations.size());
	
}

static Color ExtractPhraseColor(string & phrase) {
	//Get the good color
	if(!phrase.empty() && phrase[0] == '~') {
		phrase[0] = ' ';
		return Color(255,255,255);
	} else {
		//print in gold color
		return Color(232,204,143);
	}
}

static void addCreditsLine(string & phrase, float & drawpos) {
	
	//Create a data containers
	CreditsTextInformations infomations;
	
	infomations.fColors = ExtractPhraseColor(phrase);
	
	//int linesize = hFontCredits->GetTextSize(phrase).x;
	
	static const int MARGIN_WIDTH = 20;
	Rect linerect(g_size.width() - MARGIN_WIDTH - MARGIN_WIDTH, hFontCredits->getLineHeight());
	
	while(!phrase.empty()) {
		
		// Split long lines
		long n = ARX_UNICODE_ForceFormattingInRect(hFontCredits, phrase, linerect);
		arx_assert(n >= 0 && size_t(n) < phrase.length());
		
		infomations.sText = phrase.substr(0, size_t(n + 1) == phrase.length() ? n + 1 : n);
		phrase = phrase.substr(n + 1);
		
		// Center the text on the screen
		int linesize = hFontCredits->getTextSize(infomations.sText).x;
		infomations.sPos.x = (g_size.width() - linesize) / 2;
		
		LogDebug("credit line: '" << infomations.sText << "' (" << linesize << "," << infomations.sText.length() << ")");
		
		// Calculate height position
		infomations.sPos.y = static_cast<int>(drawpos);
		drawpos += CreditsData.iFontAverageHeight;
		
		CreditsData.aCreditsInformations.push_back(infomations);
	}
	
}

//Use to calculate an Average height for text fonts
static void CalculAverageWidth() {
	
	// Calculate the average value
	Vec2i size = hFontCredits->getTextSize("aA(");
	CreditsData.iFontAverageHeight = size.y;
}

static bool iswhitespace(char c) {
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

static void strip(string & str) {
	
	size_t startpos = 0;
	while(startpos < str.length() && iswhitespace(str[startpos])) {
		startpos++;
	}
	
	size_t endpos = str.length();
	while(endpos > startpos && iswhitespace(str[endpos - 1])) {
		endpos--;
	}
	
	if(startpos != 0) {
		str = str.substr(startpos, endpos - startpos);
	} else {
		str.resize(endpos);
	}
}


//Use to extract string info from src buffer
static void ExtractAllCreditsTextInformations() {
	
	// Retrieve the rows to display
	std::istringstream iss(ARXmenu.mda->credits);
	string phrase;

	//Use to calculate the positions
	float drawpos = static_cast<float>(g_size.height());

	while(std::getline(iss, phrase)) {
		
		strip(phrase);
		
		if(phrase.empty()) {
			// Separator line
			drawpos += CreditsData.iFontAverageHeight;
		} else {
			addCreditsLine(phrase, drawpos);
		}
	}
}

void Credits::render() {
	
	//We initialize the datas
	InitCredits();
	
	int iSize = CreditsData.aCreditsInformations.size() ;
	
	//We display them
	if(CreditsData.iFontAverageHeight != -1) {
		
		//Set the device
		GRenderer->BeginScene();
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::Fog, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		//Draw Background
		if(ARXmenu.mda->pTexCredits) {
			EERIEDrawBitmap2(0, 0, static_cast<float>(g_size.width()), static_cast<float>(g_size.height() + 1), .999f, ARXmenu.mda->pTexCredits, Color::white);
		}
		
		// Use time passed between frame to create scroll effect
		float time = arxtime.get_updated(false);
		float dtime = (float)(time - ARXmenu.mda->creditstart);
		ARXmenu.mda->creditspos -= 0.03f * Yratio * dtime;
		ARXmenu.mda->creditstart = time;
		
		std::vector<CreditsTextInformations>::const_iterator it = CreditsData.aCreditsInformations.begin() + CreditsData.iFirstLine ;
		for (; it != CreditsData.aCreditsInformations.end(); ++it)
		{
			//Update the Y word display
			float yy = it->sPos.y + ARXmenu.mda->creditspos;

			//Display the text only if he is on the viewport
			if ((yy >= -CreditsData.iFontAverageHeight) && (yy <= g_size.height())) 
			{
				hFontCredits->draw(it->sPos.x, static_cast<int>(yy), it->sText, it->fColors);
			}
			
			if (yy <= -CreditsData.iFontAverageHeight)
			{
				++CreditsData.iFirstLine;
			}
			
			if ( yy >= g_size.height() )
				break ; //it's useless to continue because next phrase will not be inside the viewport
		}
	} else {
		LogWarning << "Error initializing credits";
	}



	if ( (iSize <= CreditsData.iFirstLine) && ( iFadeAction != AMCM_MAIN ) )
	{
		ARXmenu.mda->creditspos = 0;
		ARXmenu.mda->creditstart = 0 ;
		CreditsData.iFirstLine = 0 ;

		bFadeInOut = true;
		bFade = true;
		iFadeAction=AMCM_MAIN;

		ARX_MENU_LaunchAmb(AMB_MENU);
	}

	if(ProcessFadeInOut(bFadeInOut,0.1f))
	{
		switch(iFadeAction)
		{
		case AMCM_MAIN:
			ARXmenu.currentmode=AMCM_MAIN;
			iFadeAction=-1;
			bFadeInOut=false;
			bFade=true;
			break;
		}
	}
	
	GRenderer->EndScene();
	
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	
}

void Credits::reset() {
	ARXmenu.mda->creditstart = arxtime.get_updated(false);
	ARXmenu.mda->creditspos = 0;
	CreditsData.iFirstLine = 0;
}

