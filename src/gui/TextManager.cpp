/*
 * TextManager.cpp
 *
 *  Created on: Feb 3, 2011
 *      Author: bmonkey
 */

#include "gui/TextManager.h"

#include <cassert>

#include "core/Core.h"

#include "gui/Text.h"

#include "io/Logger.h"

using std::string;
using std::vector;

struct TextManager::ManagedText {
	Font* pFont;
	RECT rRect;
	RECT rRectClipp;
	string lpszUText;
	float fDeltaY;
	float fSpeedScrollY;
	long lCol;
	long lTimeScroll;
	long lTimeOut;
};

TextManager::TextManager() {
}

TextManager::~TextManager() {
	Clear();
}

bool TextManager::AddText(Font* _pFont, const string & _lpszUText, const RECT & _rRect, long _lCol, long _lTimeOut, long _lTimeScroll, float _fSpeedScroll, int iNbLigneClipp) {
	
	if(_lpszUText.empty()) {
		return false;
	}
	
	if(!_pFont) {
		LogWarning << "Adding text with NULL font.";
		return false;
	}
	
	ManagedText * pArxText = new ManagedText();
	if(!pArxText) {
		return false;
	}
	
	pArxText->pFont = _pFont;
	pArxText->lpszUText = _lpszUText;
	pArxText->rRect = _rRect;
	pArxText->lCol = _lCol;
	pArxText->lTimeScroll = _lTimeScroll;
	pArxText->fDeltaY = 0.f;
	pArxText->fSpeedScrollY = _fSpeedScroll;
	pArxText->lTimeOut = _lTimeOut;
	pArxText->rRectClipp = pArxText->rRect;
	
	if(iNbLigneClipp) 
	{
		Vector2i sSize = _pFont->GetTextSize(pArxText->lpszUText);
		sSize.y *= iNbLigneClipp;
	
		pArxText->rRectClipp.bottom = pArxText->rRect.top + sSize.y;
	}
	
	entries.push_back(pArxText);
	
	return true;
}

bool TextManager::AddText( Font* font, const std::string& str, long x, long y, long fgcolor)
{
	RECT r;
	r.left = x;
	r.top = y;
	r.right = SHRT_MAX;
	r.bottom = SHRT_MAX;
	return AddText(font, str, r, fgcolor);
}

void TextManager::Update(float _fDiffFrame) {
	
	ARX_CHECK_INT(_fDiffFrame);
	int _iDiffFrame = ARX_CLEAN_WARN_CAST_INT(_fDiffFrame);
	
	// TODO-slussier: Until we fix the ARX_TIME_Get() mess, it's easy to have a FrameDiff of 0...
	if(_iDiffFrame == 0)
		_iDiffFrame = 1;
	
	vector<ManagedText *>::iterator itManage;
	for(itManage = entries.begin(); itManage != entries.end();) {
		
		ManagedText * pArxText = *itManage;
		
		if(pArxText->lTimeOut < 0) {
			delete pArxText;
			itManage = entries.erase(itManage);
			continue;
		}
		
		pArxText->lTimeOut -= _iDiffFrame;
		
		if(pArxText->lTimeScroll < 0 &&
		   pArxText->fDeltaY < (pArxText->rRect.bottom - pArxText->rRectClipp.bottom)) {
			pArxText->fDeltaY += pArxText->fSpeedScrollY * (float)_iDiffFrame;
			
			if(pArxText->fDeltaY >= (pArxText->rRect.bottom - pArxText->rRectClipp.bottom)) {
				pArxText->fDeltaY = static_cast<float>(pArxText->rRect.bottom - pArxText->rRectClipp.bottom);
			}
		} else {
			pArxText->lTimeScroll -= _iDiffFrame;
		}
		
		itManage++;
	}
	
}

void TextManager::Render() {
	vector<ManagedText *>::const_iterator itManage = entries.begin();
	for(; itManage != entries.end(); itManage++) {
		
		ManagedText * pArxText = *itManage;
		
		RECT* pRectClip = NULL;
		if(pArxText->rRectClipp.right != SHRT_MAX || pArxText->rRectClipp.bottom != SHRT_MAX)
			pRectClip = &pArxText->rRectClipp;

		long height = ARX_UNICODE_DrawTextInRect( pArxText->pFont, static_cast<float>(pArxText->rRect.left),
		                                         pArxText->rRect.top - pArxText->fDeltaY,
		                                         static_cast<float>(pArxText->rRect.right),
		                                         pArxText->lpszUText, pArxText->lCol, pRectClip);
		
		pArxText->rRect.bottom = pArxText->rRect.top + height;
	}
}

void TextManager::Clear() {
	
	vector<ManagedText *>::iterator itManage;
	for(itManage = entries.begin(); itManage < entries.end(); itManage++) {
		delete *itManage;
	}
	
	entries.clear();
}

bool TextManager::Empty() const 
{
	return entries.empty();
}
