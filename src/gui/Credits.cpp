
#include "gui/Credits.h"

#include <sstream>

#include "core/Core.h"
#include "core/Time.h"

#include "gui/Menu.h"
#include "gui/Text.h"
#include "gui/MenuWidgets.h"

#include "graphics/Draw.h"
#include "graphics/font/Font.h"

#include "io/Logger.h"

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
		sPos.x = 0;
		sPos.y = 0;
		fColors = 0;
	}
	
	string  sText;
	Color fColors;
	Vec2i sPos;
};


struct CreditsInformations {
	
	CreditsInformations() {
	iFontAverageHeight = -1;
	iFirstLine = 0 ;
	}
	
	int iFirstLine;
	int iFontAverageHeight;
	vector<CreditsTextInformations> aCreditsInformations;
};


static CreditsInformations CreditsData;

static void InitCredits();
static void CalculAverageWidth();
static void ExtractAllCreditsTextInformations();
static void ExtractPhraseColor(string & phrase, CreditsTextInformations & infomations);
static void CalculTextPosition(const string & phrase, CreditsTextInformations & infomations, float & drawpos);

static void InitCredits() {

	LogDebug << "InitCredits";
	
	CalculAverageWidth();
	ExtractAllCreditsTextInformations();
	
	LogDebug << "Credits lines " << CreditsData.aCreditsInformations.size();
	
}

static void CalculTextPosition(const string & phrase, CreditsTextInformations & infomations, float & drawpos) {
	
	//Center the text on the screen
	infomations.sPos = hFontCredits->GetTextSize(phrase);
	if(infomations.sPos.x < DANAESIZX) 
		infomations.sPos.x = static_cast<int>((DANAESIZX - infomations.sPos.x) * ( 1.0f / 2 ));
	
	//Calcul height position (must be calculate after GetTextSize because sPos is writted)
	infomations.sPos.y = static_cast<int>(drawpos) ;
	drawpos += CreditsData.iFontAverageHeight;
}

static void ExtractPhraseColor(string & phrase, CreditsTextInformations &infomations )
{
	//Get the good color
	if(!phrase.empty() && phrase[0] == '~') {
		phrase[0] = ' ';
		infomations.fColors = Color(255,255,255);
	} else {
		//print in gold color
		infomations.fColors = Color(232,204,143);
	}
}

//Use to calculate an Average height for text fonts
static void CalculAverageWidth() {
	
	// Calculate the average value
	Vec2i size = hFontCredits->GetTextSize("aA(");
	CreditsData.iFontAverageHeight = size.y;
}


//Use to extract string info from src buffer
static void ExtractAllCreditsTextInformations() {
	
	// Retrieve the rows to display
	std::istringstream iss(ARXmenu.mda->str_cre_credits);
	string phrase;

	//Use to calculate the positions
	float drawpos = static_cast<float>(DANAESIZY);

	while(std::getline(iss, phrase)) {
	
		//Case of separator line
		if(phrase.length() == 0) {
			drawpos += CreditsData.iFontAverageHeight >> 3;
			continue ;
		}
		
		//Create a data containers
		CreditsTextInformations infomations ;
		
		ExtractPhraseColor(phrase, infomations);
		CalculTextPosition(phrase, infomations, drawpos);
		
		//Assign the text modified by ExtractPhase Color
		infomations.sText = phrase;
		
		//Bufferize it
		CreditsData.aCreditsInformations.push_back(infomations);
	}
}

void Credits::render() {

	//We initialize the datas
	if(CreditsData.iFontAverageHeight == -1) {
		InitCredits();
	}
	
	int iSize = CreditsData.aCreditsInformations.size() ;
	
	//We display them
	if(CreditsData.iFontAverageHeight != -1) {
		
		//Set the device
		if(!danaeApp.DANAEStartRender()) return;
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::Fog, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		//Draw Background
		if(ARXmenu.mda->pTexCredits)
		{
			EERIEDrawBitmap2( 0, 0, static_cast<float>(DANAESIZX), static_cast<float>(DANAESIZY + 1), .999f, ARXmenu.mda->pTexCredits, 0xFFFFFFFF);
		}    

		//Use time passed between frame to create scroll effect
		//ARX_LOG("CreditStart %f, CreditGet ARX_TIME_Get %f  ", ARXmenu.mda->creditstart,ARX_TIME_Get( false ));
		ARXmenu.mda->creditspos-=0.025f*(float)(ARX_TIME_Get( false )-ARXmenu.mda->creditstart);
		ARXmenu.mda->creditstart=ARX_TIME_Get( false );
		
		std::vector<CreditsTextInformations>::const_iterator it = CreditsData.aCreditsInformations.begin() + CreditsData.iFirstLine ;
		for (; it != CreditsData.aCreditsInformations.end(); ++it)
		{
			//Update the Y word display
			float yy = it->sPos.y + ARXmenu.mda->creditspos;

			//Display the text only if he is on the viewport
			if ((yy >= -CreditsData.iFontAverageHeight) && (yy <= DANAESIZY)) 
			{
				hFontCredits->Draw(it->sPos.x, ARX_CLEAN_WARN_CAST_INT(yy), it->sText, it->fColors);
			}
			
			if (yy <= -CreditsData.iFontAverageHeight)
			{
				++CreditsData.iFirstLine;
			}
			
			if ( yy >= DANAESIZY )
				break ; //it's useless to continue because next phrase will not be inside the viewport
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
	
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	
}

void Credits::reset() {
	ARXmenu.mda->creditstart = ARX_TIME_Get();
	ARXmenu.mda->creditspos = 0;
	CreditsData.iFirstLine = 0;
}

