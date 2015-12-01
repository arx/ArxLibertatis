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

// TODO extern globals
extern bool bFadeInOut;
extern bool bFade;
extern int iFadeAction;


struct CreditsTextInformations {
	
	CreditsTextInformations() {
		sPos = Vec2i_ZERO;
		fColors = Color::none;
		sourceLineNumber = -1;
	}
	
	std::string  sText;
	Color fColors;
	Vec2i sPos;
	int sourceLineNumber;
	
};


struct CreditsInformations {
	
	CreditsInformations() : iFirstLine(0), iFontAverageHeight(-1), sizex(0), sizey(0) { }
	
	int iFirstLine;
	int iFontAverageHeight;
	
	int sizex, sizey; // save the screen size so we know when to re-initialize the credits
	
	std::vector<CreditsTextInformations> aCreditsInformations;
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
	
	// When the screen is resized, try to keep the credits scrolled to the 'same' position
	static int anchorLine = -1;
	static float offset;
	typedef std::vector<CreditsTextInformations>::iterator Iterator;
	if(CreditsData.iFontAverageHeight != -1 && CreditsData.iFirstLine >= 0
	   && size_t(CreditsData.iFirstLine) < CreditsData.aCreditsInformations.size()) {
		// We use the first line that is still visible as our anchor
		Iterator it = CreditsData.aCreditsInformations.begin() + CreditsData.iFirstLine;
		anchorLine = it->sourceLineNumber;
		// Find the first credits line that comes from this source line
		Iterator first = it;
		while(first != CreditsData.aCreditsInformations.begin()
		      && (first - 1)->sourceLineNumber == anchorLine) {
			--first;
		}
		// Find the first credits line that comes from this source line
		Iterator last = it;
		while((last + 1) != CreditsData.aCreditsInformations.end()
		      && (last + 1)->sourceLineNumber == anchorLine) {
			++last;
		}
		// Remember the offset from the anchor line to the current scroll position
		float pos = (first->sPos.y + last->sPos.y) * 0.5f;
		offset = (pos + ARXmenu.mda->creditspos) / float(CreditsData.iFontAverageHeight);
	}
	
	CreditsData.sizex = g_size.width();
	CreditsData.sizey = g_size.height();
	
	CreditsData.aCreditsInformations.clear();
	
	LogDebug("InitCredits");
	
	CalculAverageWidth();
	ExtractAllCreditsTextInformations();
	
	LogDebug("Credits lines " << CreditsData.aCreditsInformations.size());
	
	if(anchorLine >= 0) {
		// Find the first credits line that comes from our source anchor line
		Iterator first = CreditsData.aCreditsInformations.begin();
		while(first != CreditsData.aCreditsInformations.end()
		      && first->sourceLineNumber != anchorLine) {
			++first;
		}
		if(first != CreditsData.aCreditsInformations.end()) {
			// Find the last credits line that comes from our source anchor line
			Iterator last = first;
			while((last + 1) != CreditsData.aCreditsInformations.end()
			      && (last + 1)->sourceLineNumber == anchorLine) {
				++last;
			}
			// Restore the scroll positon using the offset to our anchor line
			float pos = (first->sPos.y + last->sPos.y) * 0.5f;
			ARXmenu.mda->creditspos = offset * float(CreditsData.iFontAverageHeight) - pos;
		}
	}
	
}

static void addCreditsLine(std::string & phrase, float & drawpos, int sourceLineNumber) {
	
	CreditsTextInformations infomations;
	infomations.sourceLineNumber = sourceLineNumber;
	
	// Determnine the type of the line
	bool isSimpleLine = false;
	if(!phrase.empty() && phrase[0] == '~') {
		// Heading
		drawpos += CreditsData.iFontAverageHeight * 0.6f;
		phrase[0] = ' ';
		infomations.fColors = Color::white;
	} else if(phrase[0] == '&') {
		// Heading continued
		infomations.fColors = Color::white;
	} else {
		// Name or text
		isSimpleLine = true;
		infomations.fColors = Color(232, 204, 143);
	}
	
	static const int Margin = 20;
	Rect linerect(g_size.width() - Margin - Margin, hFontCredits->getLineHeight());
	
	while(!phrase.empty()) {
		
		// Calculate height position
		infomations.sPos.y = static_cast<int>(drawpos);
		drawpos += CreditsData.iFontAverageHeight;
		
		// Split long lines
		long n = ARX_UNICODE_ForceFormattingInRect(hFontCredits, phrase, linerect);
		arx_assert(n >= 0 && size_t(n) < phrase.length());
		
		// Long lines are not simple
		isSimpleLine = isSimpleLine && size_t(n + 1) == phrase.length();
		
		infomations.sText = phrase.substr(0, size_t(n + 1) == phrase.length() ? n + 1 : n);
		phrase = phrase.substr(n + 1);
		
		// Center the text on the screen
		int linesize = hFontCredits->getTextSize(infomations.sText).x;
		infomations.sPos.x = (g_size.width() - linesize) / 2;
		
		if(isSimpleLine) {
			
			// Check if there is a suffix that should be styled differently
			size_t p = size_t(-1);
			for(;;) {
				p = infomations.sText.find_first_of("-(0123456789", p + 1);
				if(p == std::string::npos) {
					break;
				}
				if(p != 0 && infomations.sText[p  - 1] != ' ') {
					continue;
				}
				if(infomations.sText[p] == '(') {
					if(infomations.sText[infomations.sText.length() - 1] != ')') {
						continue;
					}
					if(infomations.sText.find_first_of('(', p + 1) != std::string::npos) {
						continue;
					}
				}
				if(infomations.sText[p] >= '0' && infomations.sText[p] < '9') {
					if(infomations.sText.find_first_not_of(".", p) == std::string::npos) {
						continue;
					}
					if(infomations.sText.find_first_not_of("0123456789.", p) != std::string::npos) {
						continue;
					}
				}
				break;
			}
			
			if(p != std::string::npos) {
				CreditsTextInformations prefix = infomations;
				prefix.sText.resize(p);
				int prefixsize = hFontCredits->getTextSize(prefix.sText).x;
				CreditsData.aCreditsInformations.push_back(prefix);
				infomations.sPos.x = prefix.sPos.x + prefixsize + 20;
				infomations.sText = infomations.sText.substr(p);
				infomations.fColors = Color::gray(0.7f);
			}
			
		}
		
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

static void strip(std::string & str) {
	
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
	std::string phrase;

	//Use to calculate the positions
	float drawpos = static_cast<float>(g_size.height());
	
	for(int sourceLineNumber = 0; std::getline(iss, phrase); sourceLineNumber++) {
		
		strip(phrase);
		
		if(phrase.empty()) {
			// Separator line
			drawpos += 0.4f * CreditsData.iFontAverageHeight;
		} else {
			addCreditsLine(phrase, drawpos, sourceLineNumber);
		}
	}
}

void Credits::render() {
	
	//We initialize the datas
	InitCredits();
	
	int iSize = CreditsData.aCreditsInformations.size() ;
	
	//We display them
	if(CreditsData.iFontAverageHeight != -1) {
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::Fog, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		//Draw Background
		if(ARXmenu.mda->pTexCredits) {
			Rectf rect(Vec2f_ZERO, g_size.width(), g_size.height() + 1);
			
			EERIEDrawBitmap2(rect, .999f, ARXmenu.mda->pTexCredits, Color::white);
		}
		
		// Use time passed between frame to create scroll effect
		float time = arxtime.get_updated(false);
		float dtime = (float)(time - ARXmenu.mda->creditstart);
		ARXmenu.mda->creditspos -= 0.03f * g_sizeRatio.y * dtime;
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



	if(iSize <= CreditsData.iFirstLine && iFadeAction != AMCM_MAIN) {
		ARXmenu.mda->creditspos = 0;
		ARXmenu.mda->creditstart = 0 ;
		CreditsData.iFirstLine = 0 ;
		
		bFadeInOut = true;
		bFade = true;
		iFadeAction = AMCM_MAIN;
		
		ARX_MENU_LaunchAmb(AMB_MENU);
	}

	if(ProcessFadeInOut(bFadeInOut,0.1f) && iFadeAction == AMCM_MAIN) {
		ARXmenu.currentmode = AMCM_MAIN;
		iFadeAction = -1;
		bFadeInOut = false;
		bFade = true;
	}
	
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	
}

void Credits::reset() {
	ARXmenu.mda->creditstart = arxtime.get_updated(false);
	ARXmenu.mda->creditspos = 0;
	CreditsData.iFirstLine = 0;
}

