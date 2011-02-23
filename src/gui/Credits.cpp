
#include "gui/Credits.h"

#include <string>

#include "core/Core.h"
#include "core/Time.h"

#include "gui/Menu.h"
#include "gui/Text.h"
#include "gui/MenuWidgets.h"

#include "graphics/Draw.h"

#include "io/Logger.h"

#include "scene/GameSound.h"

using std::string;
using std::vector;

// TODO extern globals
extern bool bFadeInOut;
extern bool bFade;
extern int iFadeAction;
void ARX_MENU_LaunchAmb(char *_lpszAmb);


struct CreditsTextInformations {
	
	CreditsTextInformations() {
		sPos.cx = 0;
		sPos.cy = 0;
		fColors = 0;
	}
	
	string  sText;
	COLORREF fColors;
	SIZE sPos;
};


struct CreditsInformations {
	
	CreditsInformations() {
	iFontAverageHeight = -1;
	iFirstLine = 0 ;
  }

  int iFirstLine ;
  int iFontAverageHeight ;
  vector<CreditsTextInformations> aCreditsInformations ;
};


static CreditsInformations CreditsData;

static void InitCredits();
static void CalculAverageWidth(HDC _hDC);
static void ExtractAllCreditsTextInformations(HDC _hDC);
static void ExtractPhraseColor(string & phrase, CreditsTextInformations & infomations);
static void CalculTextPosition(HDC _hDC, const string & phrase, CreditsTextInformations & infomations, float & drawpos);

static void InitCredits() {
	HDC hDC;
	
	LogDebug << "InitCredits";
	
	if(SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC))) {
		CalculAverageWidth(hDC);
		ExtractAllCreditsTextInformations(hDC);
		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
	}
	
	LogDebug << "Credits lines " << CreditsData.aCreditsInformations.size();
	
}

static void CalculTextPosition(HDC _hDC, const string & phrase, CreditsTextInformations & infomations, float & drawpos) {
	//Center the text on the screen
	GetTextExtentPoint32(_hDC, phrase.c_str(), phrase.length(), &(infomations.sPos));
	
	if(infomations.sPos.cx < DANAESIZX) 
		infomations.sPos.cx = static_cast<int>((DANAESIZX - infomations.sPos.cx) * ( 1.0f / 2 ));
	
	//Calcul height position (must be calculate after GetTextExtendPoint32 because sPos is writted)
	infomations.sPos.cy = static_cast<int>(drawpos) ;
	drawpos += CreditsData.iFontAverageHeight ;
}

static void ExtractPhraseColor(string & phrase, CreditsTextInformations &infomations )
{
	//Get the good color
	if(!phrase.empty() && phrase[0] == '~') {
		phrase[0] = ' ';
		infomations.fColors = RGB(255,255,255);
	} else {
		//print in gold color
		infomations.fColors = RGB(232,204,143);
	}
}

	//Use to calculate an Average height for text fonts
static void CalculAverageWidth(HDC _hDC) {
	
	SelectObject(_hDC, hFontCredits);
	SIZE size;
	
	//calculate the average value
	GetTextExtentPoint32(_hDC, "aA(",3, &size);
	CreditsData.iFontAverageHeight = size.cy;
}


//Use to extract string info from src buffer
static void ExtractAllCreditsTextInformations(HDC _hDC) {
	
	// Retrieve the rows to display
	std::istringstream iss(ARXmenu.mda->str_cre_credits);
	string phrase;

	//Use to calculate the positions
	float drawpos = static_cast<float>(DANAESIZY);
	bool firstLine = true ;

	
	while(std::getline(iss, phrase)) {
		
		if(!phrase.empty() && phrase[phrase.size() - 1] == '\r') {
			phrase.resize(phrase.size() - 1);
		}
		
		//Remove the first tild
		if(firstLine) {
			firstLine = false;
			phrase[0] = ' ';
		}
		
		//Case of separator line
		if(phrase.length() == 0) {
			drawpos += CreditsData.iFontAverageHeight >> 3;
			continue ;
		}
		
		//Create a data containers
		CreditsTextInformations infomations ;
		
		ExtractPhraseColor(phrase, infomations);
		CalculTextPosition(_hDC, phrase, infomations, drawpos);
		
		//Assign the text modified by ExtractPhase Color
		infomations.sText = phrase;
		
		//Bufferize it
		CreditsData.aCreditsInformations.push_back(infomations);
	}
}

void Credits::render() {
	int drawn = 0 ;
	
	//We initialize the datas
	if(CreditsData.iFontAverageHeight == -1) {
		InitCredits();
	}
	
	int iSize = CreditsData.aCreditsInformations.size() ;
	
	//We display them
	if(CreditsData.iFontAverageHeight != -1) {
		
		HDC hDC;
		COLORREF oldRef  = RGB(0,0,0);
		
		//Set the device
		if(!danaeApp.DANAEStartRender()) return;
		
		SETALPHABLEND(GDevice,false);
		GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, false);
		SETZWRITE(GDevice,true);
		GDevice->SetRenderState( D3DRENDERSTATE_ZENABLE,false);
		
		//Draw Background
		if(ARXmenu.mda->pTexCredits)
		{
			EERIEDrawBitmap2(GDevice, 0, 0, static_cast<float>(DANAESIZX), static_cast<float>(DANAESIZY + 1), .999f, ARXmenu.mda->pTexCredits, 0xFFFFFFFF);
		}    

		danaeApp.DANAEEndRender();

		//Use time passed between frame to create scroll effect
		//ARX_LOG("CreditStart %f, CreditGet ARX_TIME_Get %f  ", ARXmenu.mda->creditstart,ARX_TIME_Get( false ));
		ARXmenu.mda->creditspos-=0.025f*(float)(ARX_TIME_Get( false )-ARXmenu.mda->creditstart);
		ARXmenu.mda->creditstart=ARX_TIME_Get( false );
		
		if( SUCCEEDED( danaeApp.m_pddsRenderTarget->GetDC(&hDC) ) )
		{
			SetBkMode(hDC,TRANSPARENT);    
		

			std::vector<CreditsTextInformations>::const_iterator it = CreditsData.aCreditsInformations.begin() + CreditsData.iFirstLine ;

			for (;
				it != CreditsData.aCreditsInformations.end();
				++it)
			{
				//Update the Y word display
				float yy = it->sPos.cy + ARXmenu.mda->creditspos;

				//Display the text only if he is on the viewport
				if ((yy >= -CreditsData.iFontAverageHeight) && (yy <= DANAESIZY)) 
				{
					if (oldRef != it->fColors) //Little optimization
					{
						SetTextColor(hDC, it->fColors);
						oldRef = it->fColors;
					}

					SelectObject(hDC, hFontCredits);

					//Display the text on the screen
					TextOut( hDC,
						it->sPos.cx,
						ARX_CLEAN_WARN_CAST_INT(yy),
						it->sText.c_str(),
						it->sText.length()	);

					++drawn;
				}
				
				if (yy <= -CreditsData.iFontAverageHeight)
				{
					++CreditsData.iFirstLine;
				}
				
				if ( yy >= DANAESIZY )
					break ; //it's useless to continue because next phrase will not be inside the viewport


			}
			danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
		}
	} else {
		LogWarning << "error initializing credits";
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

	danaeApp.DANAEStartRender();

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
	
	danaeApp.DANAEEndRender();
	
	SETZWRITE(GDevice,true);
	danaeApp.EnableZBuffer();
	
}

