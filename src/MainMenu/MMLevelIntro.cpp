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
#include "Input.h"




MMLevelIntro::MMLevelIntro(const char *levelDirPath, const char *fileName, const char *levelUserName, const char *author)
{

	cw = 0; font=0; fontB=0; fontN=0;
	int screenx,screeny;
	KRKAL->GetScreenRes(screenx,screeny);
	int x1 = 0, y1 = 0;
	int maxX = 1024;
	int maxY = 768;
	if (screenx > maxX) {
		x1 = (screenx - maxX) / 2;
		screenx = maxX;
	}
	if (screeny > maxY) {
		y1 = (screeny - maxY) / 2;
		screeny = maxY;
	}

	char path[MAX_PATH];
	sprintf(path, "%s/%s", levelDirPath, fileName);

	int size = FS->GetFileSize(path);
	if (!size) return;

	char *buffer = new char[size+1];
	if (1 == FS->ReadFile(path, buffer, size)) {
		buffer[size] = 0;

		cw=new CGUIStdWindow(20+x1,20+y1,(float)screenx-40,(float)screeny-(int)STD_GAMEMENU_HEIGHT - 40,0xDD202070);
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
			DisplayIntro(SelectRootByLanguage(doc), levelUserName, author);
		}
	}

	SAFE_DELETE(buffer);

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


void MMLevelIntro::DisplayIntro(TiXmlElement * root, const char *levelUserName, const char *author) {
	if (!root)
		return;

	float windowSX, windowSY;
	cw->GetVPSize(windowSX, windowSY);
	UI tc = 0xFFFFFFFF;

	if (!levelUserName || !*levelUserName)
	{
		headerSize = 0;
	}
	else 
	{
		CGUIStaticText *st = new CGUIStaticText(levelUserName, fontN, 0, 50, tc, windowSX, windowSY, aCenter, 0, 0, DT_NOPREFIX);
		cw->AddBackElem(st);
	}

	if (author && *author) {
		CGUIStaticText *auth = new CGUIStaticText(author, (CGUIFont*)RefMgr->Find("GUI.F.Arial.14PX"), 30, windowSY - 30, 0xFFA0A0A0, windowSX - 60, 30, aRight, 0, 0, DT_NOPREFIX);
		cw->AddBackElem(auth);
	}


	TiXmlElement *elem;
	for (elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement()) {
		if (elem->ValueStr() == "Text") {
			DisplayText(elem);
		} else if (elem->ValueStr() == "Image") {
			DisplayImage(elem);
		}
	}
}


void AppendToBuff(char *buff, char *&buffptr, const char *str) {
	if (!str)
		return;
	if (buffptr > buff && buffptr + 1 < buff + 128 * 1024) {
		*buffptr++ = ' ';
		*buffptr = 0;
	}
	int len = strlen(str);
	strcpy_s(buffptr, 128 * 1024 - (buffptr - buff), str);
	buffptr += len;
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

	char *buff = new char[128 * 1024];
	*buff = 0;
	char *buffptr = buff;

	for (TiXmlNode *node = text->FirstChild(); node; node = node->NextSibling()) {
		if (node->Type() == TiXmlNode::TEXT) {
			AppendToBuff(buff, buffptr, node->ToText()->Value());
		} 
		else if (node->Type() == TiXmlNode::ELEMENT)
		{
			TiXmlElement *elem = node->ToElement();
			if (elem->ValueStr() == "br") {
				st = new CGUIStaticText(buff, font, xx, yy, tc, mx - xx, my - yy);
				st->GetSize(sxx, syy); yy += syy + 2; cw->AddBackElem(st);
				*buff = 0;
				buffptr = buff;
			}
			else if (elem->ValueStr() == "key")
			{
				const char *key;
				if (Input && (key = Input->FindKeyDisplayName(elem->Attribute("name")))) {
					AppendToBuff(buff, buffptr, key);
				}
				else {
					AppendToBuff(buff, buffptr, elem->Attribute("default"));
				}
			}
		}
	}

	st = new CGUIStaticText(buff, font, xx, yy, tc, mx - xx, my - yy);
	st->GetSize(sxx, syy); yy += syy + 2; cw->AddBackElem(st);
	*buff = 0;
	buffptr = buff;


	SAFE_DELETE_ARRAY(buff);
}



void MMLevelIntro::DisplayImage(TiXmlElement * image) {
	float windowSX, windowSY;
	cw->GetVPSize(windowSX, windowSY);
	windowSY -= headerSize;

	float x1=0,x2=0,y1=0,y2=0;
	int vAlign = 0, hAlign = 0;
	image->QueryFloatAttribute("x1", &x1);
	image->QueryFloatAttribute("x2", &x2);
	image->QueryFloatAttribute("y1", &y1);
	image->QueryFloatAttribute("y2", &y2);

	image->QueryIntAttribute("vAlign", &vAlign);
	image->QueryIntAttribute("hAlign", &hAlign);

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

		if (x2 != 0 && y2 != 0) {
			float sx2 = sy / syy * sxx;
			float sy2 = sx / sxx * syy;
			if (sx2 < sx && hAlign != 0) {
				if (hAlign == 1)
					picture->MoveRel(floorf((sx - sx2) / 2), 0);
				if (hAlign == 2)
					picture->MoveRel(floorf(sx - sx2), 0);
				sx = sx2;
			}
			if (sy2 < sy && vAlign != 0) {
				if (vAlign == 1)
					picture->MoveRel(0, floorf((sy - sy2) / 2));
				if (vAlign == 2)
					picture->MoveRel(0, floorf(sy - sy2));
				sy = sy2;
			}
		}
		else {
			if (sx == 0)
				sx = sy / syy * sxx;
			if (sy == 0)
				sy = sx / sxx * syy;
		}

		picture->Resize(ceilf(sx),ceilf(sy));
	}

	cw->AddBackElem(picture);
}
