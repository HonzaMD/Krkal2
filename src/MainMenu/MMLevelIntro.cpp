#include "stdafx.h"
#include "MMLevelIntro.h"
#include "MainMenuDesktop.h"
#include "gui.h"
#include "window.h"
#include "widgets.h"
#include "refmgr.h"
#include "krkal.h"
#include "fs.h"
#include "tinyxml.h"




MMLevelIntro::MMLevelIntro(const char *levelDirPath, const char *levelUserName)
{

	cw = 0; font=0; fontB=0; fontN=0;
	int screenx,screeny;
	KRKAL->GetScreenRes(screenx,screeny);

	char path[MAX_PATH];
	sprintf(path, "%s/LevelIntro.xml", levelDirPath);

	int size = FS->GetFileSize(path);
	if (!size) return;

	char *buffer = new char[size+1];
	if (1 == FS->ReadFile(path, buffer, size)) {
		buffer[size] = 0;

		cw=new CGUIStdWindow(20,20,(float)screenx-40,(float)screeny-(int)STD_GAMEMENU_HEIGHT - 40,0xDD202070);
		desktop->AddBackElem(cw);
		cw->FocusMe();

		font=(CGUIFont*) RefMgr->Find("GUI.F.Verdana.10");
		fontB=(CGUIFont*) RefMgr->Find("GUI.F.Verdana.10.B");
		fontN=(CGUIFont*) RefMgr->Find("GUI.F.Courier.18.B");


		TiXmlDocument doc("LevelIntro.xml");
		doc.Parse(buffer);
		
		if (doc.Error()) {
			ShowXmlError(doc);
		} else {
			DisplayIntro(SelectRootByLanguage(doc), levelUserName);
		}
	}

	SAFE_DELETE(buffer);



	//CGUIStaticText *st;

	//float mx=(float)screenx-30;
	//float xx=10;
	//float yy=10;
	//float sxx,syy;
	//int ti=0;
	//UI tc=0xFFFFFFFF;

	//CGUIStaticPicture* krkal = new CGUIStaticPicture(0,0,"$TEX$/MainMenu/krkal.png");
	//krkal->GetSize(sxx,syy);
	//krkal->Move((screenx-sxx-10)/2,0);
	//yy+=syy+10;
	//cw->AddBackElem(krkal);


	//CGUIFont *f=(CGUIFont*) RefMgr->Find("GUI.F.Verdana.10");
	//CGUIFont *fb=(CGUIFont*) RefMgr->Find("GUI.F.Verdana.10.B");
	//CGUIFont *fn=(CGUIFont*) RefMgr->Find("GUI.F.Courier.14.B");

	////hra krkal
	//yy+=10;
	//st=new CGUIStaticText(helptext[ti++],fn,xx,yy,tc,mx);
	//st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	//
	//st=new CGUIStaticText(helptext[ti++],fb,xx,yy,tc,mx);
	//st->GetSize(sxx,syy);yy+=syy;cw->AddBackElem(st);
	//st=new CGUIStaticText(helptext[ti++],f,xx+20,yy,tc,mx-20);
	//st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);
	//st=new CGUIStaticText(helptext[ti++],fb,xx,yy,tc,mx);
	//st->GetSize(sxx,syy);yy+=syy;cw->AddBackElem(st);
	//st=new CGUIStaticText(helptext[ti++],f,xx+20,yy,tc,mx-20);
	//st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);

	//for(int i=0;i<8;i++)
	//{
	//	st=new CGUIStaticText(helptext[ti],fb,xx,yy,tc,mx);
	//	st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);
	//	ti++;
	//}

	//yy+=5;
	//for(int i=0;i<2;i++)
	//{
	//	st=new CGUIStaticText(helptext[ti],f,xx,yy,tc,mx);
	//	st->GetSize(sxx,syy);yy+=syy+10;cw->AddBackElem(st);
	//	ti++;
	//}

 //   //konfigurace
	//yy+=20;
	//st=new CGUIStaticText(helptext[ti++],fn,xx,yy,tc,mx);
	//st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	//st=new CGUIStaticText(helptext[ti++],f,xx,yy,tc,mx);
	//st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);

	////technicka podpora
	//yy+=15;
	//st=new CGUIStaticText(helptext[ti++],fn,xx,yy,tc,mx);
	//st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	//for(int i=0;i<4;i++)
	//{
	//	st=new CGUIStaticText(helptext[ti],f,xx,yy,tc,mx);
	//	st->GetSize(sxx,syy);yy+=syy+10;cw->AddBackElem(st);
	//	ti++;
	//}

	//yy+=10;
	//cw->SetBackWindowNormalSize(mx+10,yy);
}

MMLevelIntro::~MMLevelIntro(void)
{
	if (cw != 0)
		cw->CloseWindow();
	cw = 0;
}







void MMLevelIntro::ShowXmlError(TiXmlDocument &doc) {
}




TiXmlElement * MMLevelIntro::SelectRootByLanguage(TiXmlDocument &doc) {
	TiXmlElement *elem = doc.FirstChildElement();
	TiXmlElement *ret = 0;
	if (elem) {		
		for (elem = elem->FirstChildElement("Language"); elem; elem = elem->NextSiblingElement("Language")) {
			ret = elem;
			const char *attr = elem->Attribute("name");
			if (attr && _stricmp(attr, "en") == 0 && KRKAL->cfg.lang==langEN)
				return elem;
			if (attr && _stricmp(attr, "cs") == 0 && KRKAL->cfg.lang==langCS)
				return elem;
		}
	}
	return ret;
}


void MMLevelIntro::DisplayIntro(TiXmlElement * root, const char *levelUserName) {
	if (!root)
		return;

	float windowSX, windowSY;
	cw->GetVPSize(windowSX, windowSY);
	UI tc = 0xFFFFFFFF;

	CGUIStaticText *st = new CGUIStaticText(levelUserName, fontN, 0, 50, tc, windowSX, windowSY, aCenter);
	cw->AddBackElem(st);


	TiXmlElement *elem;
	for (elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement()) {
		if (elem->ValueStr() == "Text") {
			DisplayText(elem);
		} else if (elem->ValueStr() == "Image") {
			DisplayImage(elem);
		}
	}
}




void MMLevelIntro::DisplayText(TiXmlElement * text){
	CGUIStaticText *st;

	float windowSX, windowSY;
	cw->GetVPSize(windowSX, windowSY);
	windowSY -= headerSize;

	float x1=0,x2=0,y1=0,y2=0;
	text->QueryFloatAttribute("x1", &x1);
	text->QueryFloatAttribute("x2", &x2);
	text->QueryFloatAttribute("y1", &y1);
	text->QueryFloatAttribute("y2", &y2);

	UI tc=0xFFFFFFFF;

	float xx = x1 / 100 * windowSX;
	float yy = (y1 / 100 * windowSY) + headerSize;
	float mx = x2 / 100 * windowSX;
	float my = (y2 / 100 * windowSY) + headerSize;

	float sxx, syy;

	for (TiXmlNode *node = text->FirstChild(); node; node = node->NextSibling()) {
		if (node->Type() == TiXmlNode::TEXT) {
			st=new CGUIStaticText(node->ToText()->Value(),font,xx,yy,tc,mx-xx,my-yy);
			st->GetSize(sxx,syy); yy+=syy+2; cw->AddBackElem(st);
		}
	}
}



void MMLevelIntro::DisplayImage(TiXmlElement * image) {
	float windowSX, windowSY;
	cw->GetVPSize(windowSX, windowSY);
	windowSY -= headerSize;

	float x1=0,x2=0,y1=0,y2=0;
	image->QueryFloatAttribute("x1", &x1);
	image->QueryFloatAttribute("x2", &x2);
	image->QueryFloatAttribute("y1", &y1);
	image->QueryFloatAttribute("y2", &y2);

	float xx = floorf(x1 / 100 * windowSX);
	float yy = floorf(y1 / 100 * windowSY) + headerSize;
	
	CGUIStaticPicture* picture = new CGUIStaticPicture(xx,yy,image->GetText());

	if (x2 != 0 || y2 != 0) {
		float sx=0, sy=0, sxx, syy;
		if (x2 != 0)
			sx = x2 / 100 * windowSX - xx;
		if (y2 != 0)
			sy = y2 / 100 * windowSY + headerSize - yy;

		picture->GetSize(sxx,syy);
		if (sx == 0)
			sx = sy / syy * sxx;
		if (sy == 0)
			sy = sx / sxx * syy;

		picture->Resize(ceilf(sx),ceilf(sy));
	}

	cw->AddBackElem(picture);
}
