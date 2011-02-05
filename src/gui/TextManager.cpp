/*
 * TextManager.cpp
 *
 *  Created on: Feb 3, 2011
 *      Author: bmonkey
 */

#include "TextManager.h"
#include "Text.h"

#include "core/Core.h"

TextManager::TextManager() {
	vText.clear();
}

TextManager::~TextManager() {
	vector<ARX_TEXT *>::iterator itManage;

	for (itManage = vText.begin(); itManage < vText.end(); itManage++)
	{
		if (*itManage)
		{
			(*itManage)->lpszUText.clear();

			free((void *)(*itManage));
			*itManage = NULL;
		}
	}

	vText.clear();
}

bool TextManager::AddText(HFONT _hFont, const std::string& _lpszUText, RECT & _rRect, long _lCol, long _lBkgCol, long _lTimeOut, long _lTimeScroll, float _fSpeedScroll, int iNbLigneClipp)
{
//	if ((!_lpszUText.empty()) && (_hFont))
	{
		ARX_TEXT * pArxText = new ARX_TEXT();

//		if (pArxText)
		{
//			pArxText->lpszUText.clear();

//			if (!pArxText->lpszUText.empty())
			{
				pArxText->hFont = _hFont;
				pArxText->lpszUText = _lpszUText;
				pArxText->eType			= ARX_TEXT_STAY;
				pArxText->rRect			= _rRect;
				pArxText->lCol			= _lCol;
				pArxText->lBkgCol		= _lBkgCol;
				pArxText->lTimeScroll	= _lTimeScroll;
				pArxText->fDeltaY		= 0.f;
				pArxText->fSpeedScrollY	= _fSpeedScroll;
				pArxText->lTimeOut		= _lTimeOut;

				if (iNbLigneClipp)
				{
					HDC hDC;
					SIZE sSize;

					if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
					{
						SelectObject(hDC, pArxText->hFont);
						GetTextExtentPoint(hDC,
						                      pArxText->lpszUText.c_str(),
						                      pArxText->lpszUText.length(),
						                      &sSize);
						danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
						pArxText->lTailleLigne = sSize.cy;
						sSize.cy *= iNbLigneClipp;
					}
					else
					{
						sSize.cy = _rRect.bottom - _rRect.top;
						pArxText->lTailleLigne = sSize.cy;
					}

					SetRect(&pArxText->rRectClipp,
							pArxText->rRect.left,
							pArxText->rRect.top,
							pArxText->rRect.right,
							pArxText->rRect.top + sSize.cy);
				}
				else
				{
					pArxText->rRectClipp = pArxText->rRect;
				}

				vText.insert(vText.end(), pArxText);
				return true;
			}

			delete pArxText;
		}
	}

	return false;
}

bool TextManager::AddText(ARX_TEXT * _pArxText)
{
	if ((_pArxText) && (!_pArxText->lpszUText.empty()) && (_pArxText->hFont))
	{
		ARX_TEXT * pArxText = (ARX_TEXT *) malloc(sizeof(ARX_TEXT));

		if (pArxText)
		{
			pArxText->lpszUText.clear();

			if (!pArxText->lpszUText.empty())
			{
				pArxText->hFont			= _pArxText->hFont;
				pArxText->lpszUText = _pArxText->lpszUText;
				pArxText->eType			= _pArxText->eType;
				pArxText->rRect			= _pArxText->rRect;
				pArxText->lCol			= _pArxText->lCol;
				pArxText->lBkgCol		= _pArxText->lBkgCol;
				pArxText->lTimeScroll	= _pArxText->lTimeScroll;
				pArxText->fDeltaY		= _pArxText->fDeltaY;
				pArxText->fSpeedScrollY	= _pArxText->fSpeedScrollY;
				pArxText->lTimeOut		= _pArxText->lTimeOut;
				pArxText->iNbLineClip   = _pArxText->iNbLineClip;

				if (pArxText->iNbLineClip)
				{
					HDC hDC;
					SIZE sSize;

					if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
					{
						SelectObject(hDC, pArxText->hFont);
						GetTextExtentPoint(hDC,
						                      pArxText->lpszUText.c_str(),
						                      pArxText->lpszUText.length(),
						                      &sSize);
						danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
						pArxText->lTailleLigne = sSize.cy;
						sSize.cy *= pArxText->iNbLineClip;
					}
					else
					{
						sSize.cy = pArxText->rRect.bottom - pArxText->rRect.top;
						pArxText->lTailleLigne = sSize.cy;
					}

					SetRect(&pArxText->rRectClipp,
							pArxText->rRect.left,
							pArxText->rRect.top,
							pArxText->rRect.right,
						pArxText->rRect.top + sSize.cy);
				}
				else
				{
					pArxText->rRectClipp = pArxText->rRect;
				}

				vText.insert(vText.end(), pArxText);
				return true;
			}

			free((void *)pArxText);
			pArxText = NULL;
		}
	}

	return false;
}

void TextManager::Update(float _fDiffFrame)
{
	vector<ARX_TEXT *>::iterator itManage;


	ARX_CHECK_INT(_fDiffFrame);
	int _iDiffFrame = ARX_CLEAN_WARN_CAST_INT(_fDiffFrame);



	for (itManage = vText.begin(); itManage < vText.end();)
	{
		ARX_TEXT * pArxText = *itManage;

		if (pArxText)
		{
			if ((pArxText->lTimeOut < 0))
			{
				pArxText->lpszUText.clear();
				free((void *)pArxText);
				pArxText = NULL;
				itManage = vText.erase(itManage);
				continue;
			}

			pArxText->lTimeOut -= _iDiffFrame;

			if ((pArxText->lTimeScroll < 0) &&
					(pArxText->fDeltaY < (pArxText->rRect.bottom - pArxText->rRectClipp.bottom))
			   )
			{
				pArxText->fDeltaY += pArxText->fSpeedScrollY * (float)_iDiffFrame;

				if (pArxText->fDeltaY >= (pArxText->rRect.bottom - pArxText->rRectClipp.bottom))
				{
					pArxText->fDeltaY = ARX_CLEAN_WARN_CAST_FLOAT(pArxText->rRect.bottom - pArxText->rRectClipp.bottom);
				}
			}
			else
			{
				pArxText->lTimeScroll -= _iDiffFrame;
			}
		}

		itManage++;
	}
}

void TextManager::Render()
{
	vector<ARX_TEXT *>::iterator itManage;

	itManage = vText.begin();

	HDC hDC = NULL;

	if (danaeApp.m_pddsRenderTarget && vText.size())
	{
		danaeApp.m_pddsRenderTarget->GetDC(&hDC);
	}

	while (itManage != vText.end())
	{
		ARX_TEXT * pArxText = *itManage;

		if (pArxText)
		{
			HRGN hRgn = NULL;
			hRgn = CreateRectRgn(pArxText->rRectClipp.left,
								 pArxText->rRectClipp.top,
								 pArxText->rRectClipp.right,
								 pArxText->rRectClipp.bottom);

			pArxText->rRect.bottom = pArxText->rRect.top + ARX_UNICODE_DrawTextInRect(ARX_CLEAN_WARN_CAST_FLOAT(pArxText->rRect.left),
									 pArxText->rRect.top - pArxText->fDeltaY,
									 ARX_CLEAN_WARN_CAST_FLOAT(pArxText->rRect.right),
									 0,
									 pArxText->lpszUText.c_str(),
									 pArxText->lCol,
									 pArxText->lBkgCol,
									 pArxText->hFont,
									 hRgn,
									 hDC);


			if (hRgn)
			{
				DeleteObject(hRgn);
			}

			if (pArxText->eType == ARX_TEXT_ONCE)
			{
				pArxText->lpszUText.clear();
				free((void *)pArxText);
				pArxText = NULL;
				itManage = vText.erase(itManage);
				continue;
			}
		}

		itManage++;
	}

	if (hDC)
	{
		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
	}
}

void TextManager::Clear()
{
	vector<ARX_TEXT *>::iterator itManage;

	for (itManage = vText.begin(); itManage < vText.end(); itManage++)
	{
		if (*itManage)
		{
			(*itManage)->lpszUText.clear();
			free((void *)(*itManage));
			*itManage = NULL;
		}
	}

	vText.clear();
}
