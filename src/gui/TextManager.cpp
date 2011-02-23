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

struct TextManager::ManagedText {
	HFONT hFont;
	RECT rRect;
	RECT rRectClipp;
	string lpszUText;
	float fDeltaY;
	float fSpeedScrollY;
	long lCol;
	long lBkgCol;
	long lTimeScroll;
	long lTimeOut;
};

TextManager::TextManager() {
}

TextManager::~TextManager() {
	Clear();
}

bool TextManager::AddText(HFONT _hFont, const string & _lpszUText, const RECT & _rRect, long _lCol, long _lBkgCol, long _lTimeOut, long _lTimeScroll, float _fSpeedScroll, int iNbLigneClipp) {
	
	if(_lpszUText.empty()) {
		return false;
	}
	
	if(!_hFont) {
		LogWarning << "Adding text with NULL font.";
		// TODO why is this commented out? ~dscharrer
		//return false;
	}
	
	ManagedText * pArxText = new ManagedText();
	if(!pArxText) {
		return false;
	}
	
	pArxText->hFont = _hFont;
	pArxText->lpszUText = _lpszUText;
	pArxText->rRect = _rRect;
	pArxText->lCol = _lCol;
	pArxText->lBkgCol = _lBkgCol;
	pArxText->lTimeScroll = _lTimeScroll;
	pArxText->fDeltaY = 0.f;
	pArxText->fSpeedScrollY = _fSpeedScroll;
	pArxText->lTimeOut = _lTimeOut;
	pArxText->rRectClipp = pArxText->rRect;
	
	if(iNbLigneClipp) {
		HDC hDC;
		SIZE sSize;
		
		if(SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC))) {
			SelectObject(hDC, pArxText->hFont);
			GetTextExtentPoint(hDC, pArxText->lpszUText.c_str(),
			                   pArxText->lpszUText.length(), &sSize);
			danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
			sSize.cy *= iNbLigneClipp;
		} else {
			sSize.cy = _rRect.bottom - _rRect.top;
		}
		
		pArxText->rRectClipp.bottom = pArxText->rRect.top + sSize.cy;
	}
	
	entries.push_back(pArxText);
	
	return true;
}

bool TextManager::AddText(HFONT font, const std::string& str, long x, long y, long fgcolor)
{
	RECT r;
	r.left = x;
	r.top = y;
	r.right = 9999;
	r.bottom = 9999;
	return AddText(font, str, r, fgcolor, 0x00FF00FF);
}

void TextManager::Update(float _fDiffFrame) {
	
	ARX_CHECK_INT(_fDiffFrame);
	int _iDiffFrame = ARX_CLEAN_WARN_CAST_INT(_fDiffFrame);
	
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
	
	HDC hDC = NULL;
	if(danaeApp.m_pddsRenderTarget && !entries.empty()) {
		danaeApp.m_pddsRenderTarget->GetDC(&hDC);
	}
	
	vector<ManagedText *>::const_iterator itManage = entries.begin();
	for(; itManage != entries.end(); itManage++) {
		
		ManagedText * pArxText = *itManage;
		
		HRGN hRgn = CreateRectRgn(pArxText->rRectClipp.left, pArxText->rRectClipp.top,
		                          pArxText->rRectClipp.right, pArxText->rRectClipp.bottom);
		
		long height = ARX_UNICODE_DrawTextInRect(static_cast<float>(pArxText->rRect.left),
		                                         pArxText->rRect.top - pArxText->fDeltaY,
		                                         static_cast<float>(pArxText->rRect.right),
		                                         pArxText->lpszUText, pArxText->lCol,
		                                         pArxText->lBkgCol, pArxText->hFont, hRgn, hDC);
		
		pArxText->rRect.bottom = pArxText->rRect.top + height;
		
		if(hRgn) {
			DeleteObject(hRgn);
		}
		
	}
	
	if (hDC) {
		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
	}
}

void TextManager::Clear() {
	
	vector<ManagedText *>::iterator itManage;
	for(itManage = entries.begin(); itManage < entries.end(); itManage++) {
		delete *itManage;
	}
	
	entries.clear();
}

bool TextManager::empty() const {
	return entries.empty();
}
