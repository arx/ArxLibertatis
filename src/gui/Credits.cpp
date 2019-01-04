/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <stddef.h>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Version.h"

#include "gui/Menu.h"
#include "gui/Text.h"
#include "gui/MenuWidgets.h"
#include "gui/menu/MenuFader.h"

#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/font/Font.h"

#include "input/Input.h"

#include "io/log/Logger.h"
#include "io/resource/PakReader.h"

#include "math/Vector.h"

#include "scene/GameSound.h"

#include "util/Unicode.h"

namespace credits {

namespace {

struct CreditsLine {
	
	CreditsLine()
		: fColors(Color::none)
		, sPos(0)
		, sourceLineNumber(-1)
	{ }
	
	std::string  sText;
	Color fColors;
	Vec2i sPos;
	int sourceLineNumber;
	
};

class Credits {
	
public:
	
	Credits()
		: m_background(NULL)
		, m_scrollPosition(0.f)
		, m_lastUpdateTime(0)
		, m_firstVisibleLine(0)
		, m_lineHeight(-1)
		, m_windowSize(0)
	{ }
	
	void setLibraryCredits(const std::string & subsystem, const std::string & credits);
	
	void setMessage(const std::string & message) { m_message = message; }
	
	void render();
	
	void reset();
	
private:
	
	TextureContainer * m_background;
	
	typedef boost::unordered_map<std::string, std::string> Libraries;
	Libraries m_libraries;
	
	std::string m_message;
	
	std::string m_text;
	
	float m_scrollPosition;
	PlatformInstant m_lastUpdateTime;
	
	size_t m_firstVisibleLine;
	int m_lineHeight;
	
	Vec2i m_windowSize; // save the screen size so we know when to re-initialize the credits
	
	std::vector<CreditsLine> m_lines;
	
	bool load();
	
	bool init();
	
	void addLine(std::string & phrase, float & drawpos, int sourceLineNumber);
	
	//! Parse the credits text and compute line positions
	void layout();
	
};

Credits g_credits;

void Credits::setLibraryCredits(const std::string & subsystem,
                                const std::string & credits) {
	m_libraries[subsystem] = credits;
}

bool Credits::load() {
	
	LogDebug("Loading credits");
	
	std::string creditsFile = "localisation/ucredits_" +  config.language + ".txt";
	
	std::string credits = g_resources->read(creditsFile);
	
	std::string englishCreditsFile;
	if(credits.empty()) {
		// Fallback if there is no localised credits file
		englishCreditsFile = "localisation/ucredits_english.txt";
		credits = g_resources->read(englishCreditsFile);
	}
	
	if(credits.empty()) {
		if(!englishCreditsFile.empty() && englishCreditsFile != creditsFile) {
			LogWarning << "Unable to read credits files " << creditsFile
			           << " and " << englishCreditsFile;
		} else {
			LogWarning << "Unable to read credits file " << creditsFile;
		}
		return false;
	}
	
	LogDebug("Loaded credits file: " << creditsFile << " of size " << credits.size());
	
	m_text = arx_credits;
	
	m_text += "\n\n\n" + arx_copyright;
	
	if(!m_libraries.empty()) {
		m_text += "\n\n\n" "~This build was made using the following tools and libraries:\n\n";
		Libraries::const_iterator compiler = m_libraries.find("compiler");
		if(compiler != m_libraries.end()) {
			m_text += "Compiler: ";
			m_text += compiler->second;
			m_text += '\n';
		}
		Libraries::const_iterator stdlib = m_libraries.find("stdlib");
		if(stdlib != m_libraries.end()) {
			m_text += stdlib->second;
			m_text += '\n';
		}
		std::vector<std::string> libraries;
		BOOST_FOREACH(const Libraries::value_type & library, m_libraries) {
			if(library.first != "compiler" && library.first != "stdlib" && !library.second.empty()) {
				boost::char_separator<char> sep("\n");
				boost::tokenizer< boost::char_separator<char> > tokens(library.second, sep);
				std::copy(tokens.begin(), tokens.end(), std::back_inserter(libraries));
			}
		}
		std::sort(libraries.begin(), libraries.end());
		BOOST_FOREACH(const std::string & library, libraries) {
			m_text += library;
			m_text += '\n';
		}
	}
	
	m_text += "\n\n\n\n~ORIGINAL ARX FATALIS CREDITS:\n\n\n";
	
	m_text += util::convert<util::UTF16LE, util::UTF8>(credits);
	
	LogDebug("Final credits length: " << m_text.size());
	
	if(!m_message.empty()) {
		m_text = "~" + m_message + "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" + m_text;
	}
	
	return true;
}

bool Credits::init() {
	
	if(!m_background) {
		m_background = TextureContainer::LoadUI("graph/interface/menus/menu_credits");
	}
	
	if(m_text.empty() && !load()) {
		return false;
	}
	
	if(m_lineHeight != -1 && m_windowSize == g_size.size()) {
		return true;
	}
	
	LogDebug("Layout credits");
	
	// When the screen is resized, try to keep the credits scrolled to the 'same' position
	int anchorLine = -1;
	float offset;
	typedef std::vector<CreditsLine>::iterator Iterator;
	if(m_lineHeight != -1 && m_firstVisibleLine < m_lines.size()) {
		// We use the first line that is still visible as our anchor
		Iterator it = m_lines.begin() + m_firstVisibleLine;
		anchorLine = it->sourceLineNumber;
		// Find the first credits line that comes from this source line
		Iterator first = it;
		while(first != m_lines.begin() && (first - 1)->sourceLineNumber == anchorLine) {
			--first;
		}
		// Find the last credits line that comes from this source line
		Iterator last = it;
		while((last + 1) != m_lines.end()
		      && (last + 1)->sourceLineNumber == anchorLine) {
			++last;
		}
		// Remember the offset from the anchor line to the current scroll position
		float pos = (first->sPos.y + last->sPos.y) * 0.5f;
		offset = (pos + m_scrollPosition) / float(m_lineHeight);
	}
	
	m_windowSize = g_size.size();
	
	layout();
	arx_assert(m_lineHeight != -1);
	
	if(anchorLine >= 0) {
		// Find the first credits line that comes from our source anchor line
		Iterator first = m_lines.begin();
		while(first != m_lines.end()
		      && first->sourceLineNumber != anchorLine) {
			++first;
		}
		if(first != m_lines.end()) {
			// Find the last credits line that comes from our source anchor line
			Iterator last = first;
			while((last + 1) != m_lines.end()
			      && (last + 1)->sourceLineNumber == anchorLine) {
				++last;
			}
			// Restore the scroll positon using the offset to our anchor line
			float pos = (first->sPos.y + last->sPos.y) * 0.5f;
			m_scrollPosition = offset * float(m_lineHeight) - pos;
		}
	}
	
	return true;
}

void Credits::addLine(std::string & phrase, float & drawpos, int sourceLineNumber) {
	
	CreditsLine infomations;
	infomations.sourceLineNumber = sourceLineNumber;
	
	// Determnine the type of the line
	bool isSimpleLine = false;
	if(!phrase.empty() && phrase[0] == '~') {
		// Heading
		drawpos += m_lineHeight * 0.6f;
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
		drawpos += m_lineHeight;
		
		// Split long lines
		long n = ARX_UNICODE_ForceFormattingInRect(hFontCredits, phrase.begin(), phrase.end(), linerect);
		arx_assert(n >= 0 && size_t(n) < phrase.length());
		
		// Long lines are not simple
		isSimpleLine = isSimpleLine && size_t(n + 1) == phrase.length();
		
		infomations.sText = phrase.substr(0, size_t(n + 1) == phrase.length() ? n + 1 : n);
		phrase = phrase.substr(n + 1);
		
		// Center the text on the screen
		int linesize = hFontCredits->getTextSize(infomations.sText).width();
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
					if(infomations.sText.find_first_not_of('.', p) == std::string::npos) {
						continue;
					}
					if(infomations.sText.find_first_not_of("0123456789.", p) != std::string::npos) {
						continue;
					}
				}
				break;
			}
			
			// Center names around the surname start
			size_t s = std::string::npos;
			if(p != std::string::npos && p > 2) {
				if(infomations.sText[p] == '-' || infomations.sText[p] == '(') {
					s = p - 1; // Skip space before the suffix
				} else {
					s = p;
				}
			} else if(p != 0) {
				s = infomations.sText.length();
			}
			if(s != std::string::npos && s != 0) {
				if(std::count(infomations.sText.begin(), infomations.sText.begin() + s, ' ') > 2) {
					s = std::string::npos; // A sentence
				} else if(infomations.sText.find_last_of(',', s - 1) != std::string::npos) {
					s = std::string::npos; // An inline list
				} else {
					s = infomations.sText.find_last_of(' ', s - 1);
				}
			}
			bool centered = false;
			if(s != std::string::npos && s != 0) {
				int firstsize = hFontCredits->getTextSize(infomations.sText.substr(0, s)).width();
				if(firstsize < g_size.width() / 2 && linesize - firstsize < g_size.width() / 2) {
					infomations.sPos.x = g_size.width() / 2 - firstsize;
					centered = true;
				}
			}
			
			if(p != std::string::npos) {
				CreditsLine prefix = infomations;
				prefix.sText.resize(p);
				int prefixsize = hFontCredits->getTextSize(prefix.sText).width();
				if(!centered && p != 0 && prefixsize / 2 < g_size.width() / 2
					 && linesize - prefixsize / 2 < g_size.width() / 2) {
					prefix.sPos.x = (g_size.width() - prefixsize) / 2;
				}
				m_lines.push_back(prefix);
				infomations.sPos.x = prefix.sPos.x + prefixsize + 20;
				infomations.sText = infomations.sText.substr(p);
				infomations.fColors = Color::gray(0.7f);
			}
			
		}
		
		m_lines.push_back(infomations);
	}
	
}

void Credits::layout() {
	
	m_lineHeight = hFontCredits->getLineHeight();
	
	m_lines.clear();
	
	std::istringstream iss(m_text);
	std::string phrase;

	float drawpos = static_cast<float>(g_size.height());
	
	for(int sourceLineNumber = 0; std::getline(iss, phrase); sourceLineNumber++) {
		
		boost::trim(phrase);
		
		if(phrase.empty()) {
			// Separator line
			drawpos += 0.4f * m_lineHeight;
		} else {
			addLine(phrase, drawpos, sourceLineNumber);
		}
	}
	
	LogDebug("Credits lines: " << m_lines.size());
	
}

void Credits::render() {
	
	// Initialze the data on demand
	if(!init()) {
		LogError << "Could not initialize credits";
		reset();
		ARXmenu.requestMode(Mode_MainMenu);
		MenuFader_start(Fade_Out, -1);
	}
	
	// Draw the background
	if(m_background) {
		Rectf rect(Vec2f(0.f), g_size.width(), g_size.height() + 1);
		UseRenderState state(render2D().noBlend());
		EERIEDrawBitmap(rect, .999f, m_background, Color::white);
	}
	
	// Use time passed between frame to create scroll effect
	PlatformInstant now = g_platformTime.frameStart();
	float elapsed = toMs(now - m_lastUpdateTime);
	
	static PlatformInstant lastKeyPressTime   = 0;
	static PlatformInstant lastUserScrollTime = 0;
	static float scrollDirection = 1.f;
	
	PlatformDuration keyRepeatDelay  = PlatformDurationMs(256); // delay after key press before continuous scrolling
	PlatformDuration autoScrollDelay = PlatformDurationMs(250); // ms after user input before resuming normal scrolling
	
	// Process user input
	float userScroll = 20.f * GInput->getMouseWheelDir();
	if(GInput->isKeyPressed(Keyboard::Key_UpArrow)) {
		userScroll += 0.2f * elapsed;
	}
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_PageUp)) {
		userScroll += 150.f;
		lastKeyPressTime = now;
	} else if(GInput->isKeyPressed(Keyboard::Key_PageUp)) {
		if(now - lastKeyPressTime > keyRepeatDelay) {
			userScroll += 0.5f * elapsed;
		}
	}
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_PageDown)) {
		userScroll -= 150.f;
		lastKeyPressTime = now;
	} else if(GInput->isKeyPressed(Keyboard::Key_PageDown)) {
		if(now - lastKeyPressTime > keyRepeatDelay) {
			userScroll -= 0.5f * elapsed;
		}
	}
	if(GInput->isKeyPressed(Keyboard::Key_DownArrow)) {
		userScroll -= 0.2f * elapsed;
	}
	m_scrollPosition += g_sizeRatio.y * userScroll;
	
	// If the user wants to scroll up, also change the automatic scroll direction …
	if(userScroll > 0.f) {
		lastUserScrollTime = now;
		scrollDirection = -1.f;
	}
	// … but restore normal scrolling after a short delay.
	if(now - lastUserScrollTime > autoScrollDelay) {
		scrollDirection = 1.f;
	}
	
	m_scrollPosition -= 0.03f * g_sizeRatio.y * elapsed * scrollDirection;
	m_lastUpdateTime = now;
	
	// Don't scroll past the credits start
	m_scrollPosition = std::min(0.f, m_scrollPosition);
	
	std::vector<CreditsLine>::const_iterator it = m_lines.begin() + m_firstVisibleLine;
	
	for(; it != m_lines.begin(); --it, --m_firstVisibleLine) {
		float yy = (it - 1)->sPos.y + m_scrollPosition;
		if (yy <= -m_lineHeight) {
			break;
		}
	}
	
	for(; it != m_lines.end(); ++it) {
		
		// Update the Y word display
		float yy = it->sPos.y + m_scrollPosition;
		
		// Display the text only if it is inside the viewport
		if(yy >= float(-m_lineHeight) && yy <= float(g_size.height())) {
			hFontCredits->draw(it->sPos.x, static_cast<int>(yy), it->sText, it->fColors);
		}
		
		if(yy <= float(-m_lineHeight)) {
			++m_firstVisibleLine;
		}
		
		if(yy >= float(g_size.height())) {
			// It's useless to continue because next line will not be inside the viewport
			break;
		}
		
	}
	
	if(m_firstVisibleLine >= m_lines.size() && iFadeAction != Mode_MainMenu) {
		
		MenuFader_start(Fade_In, Mode_MainMenu);
		ARX_SOUND_PlayMenuAmbiance(AMB_MENU);
	}

	if(MenuFader_process() && iFadeAction == Mode_MainMenu) {
		reset();
		ARXmenu.requestMode(Mode_MainMenu);
		MenuFader_start(Fade_Out, -1);
	}
	
}

void Credits::reset() {
	LogDebug("Reset credits");
	m_lastUpdateTime = g_platformTime.frameStart();
	m_scrollPosition = 0;
	m_firstVisibleLine = 0;
	m_lineHeight = -1;
	m_windowSize = Vec2i(0);
	m_lines.clear();
	delete m_background, m_background = NULL;
	m_text.clear();
	m_message.clear();
}

} // anonymous namespace

void setLibraryCredits(const std::string & subsystem, const std::string & credits) {
	g_credits.setLibraryCredits(subsystem, credits);
}

void setMessage(const std::string & message) {
	g_credits.setMessage(message);
}

void render() {
	g_credits.render();
}

void reset() {
	g_credits.reset();
}

} // namespace credits
