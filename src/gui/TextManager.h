/*
 * TextManager.h
 *
 *  Created on: Feb 3, 2011
 *      Author: bmonkey
 */

#ifndef TEXTMANAGER_H_
#define TEXTMANAGER_H_

#include <windef.h>
#include <vector>
#include <string>

using std::vector;

struct ARX_TEXT;

class TextManager {
public:
	vector<ARX_TEXT *> vText;

	TextManager();
	~TextManager();
	bool AddText(HFONT, const std::string&, RECT &, long _lCol = -1,
			long _lBkgCol = 0, long _lTimeOut = 0, long _lTimeScroll = 0,
			float _fSpeedScroll = 0.f, int iNbLigneClipp = 0);
	bool AddText(ARX_TEXT *);
	void Update(float);
	void Render();
	void Clear();
};

#endif /* TEXTMANAGER_H_ */
