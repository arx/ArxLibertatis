/*
 * TextManager.h
 *
 *  Created on: Feb 3, 2011
 *      Author: bmonkey
 */

#ifndef ARX_GUI_TEXTMANAGER_H
#define ARX_GUI_TEXTMANAGER_H

#include <windows.h> // for RECT
#include <vector>
#include <string>

class Font;

class TextManager {
	
public:
	
	TextManager();
	~TextManager();
	
	bool AddText(Font *, const std::string &, const RECT &, long _lCol = -1,
	             long _lTimeOut = 0, long _lTimeScroll = 0,
	             float _fSpeedScroll = 0.f, int iNbLigneClipp = 0);
	
	bool AddText(Font* , const std::string &, long x, long y, long fgcolor);
	void Update(float);
	void Render();
	void Clear();
	bool Empty() const;
	
private:
	
	struct ManagedText;
	std::vector<ManagedText *> entries;
	
};

#endif // ARX_GUI_TEXTMANAGER_H
