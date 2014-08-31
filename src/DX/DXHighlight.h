//////////////////////////////////////////////////////////////////////
//
// DXhighlight.h
//
// vyhighlightuje text
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef DXHIGHLIGHT_H
#define DXHIGHLIGHT_H

#include "dxtext.h"

//popisuje, jak se ma higlightovat - fonty a barvy
class CDXHighLightFormat{
public:
	CDXHighLightFormat();
	~CDXHighLightFormat();

	int SetAllToDefault();

	int SetDefault(char *fontname="DX.F.Courier.10", DWORD color=0xFFFFFFFF);
	int SetKeyword(char *fontname="DX.F.Courier.10", DWORD color=0xFFFFFF00);
	int SetString(char *fontname="DX.F.Courier.10", DWORD color=0xFFEE0000);
	int SetComment(char *fontname="DX.F.Courier.10", DWORD color=0xFF00CC00);
	int SetKnownNames(char *fontname="DX.F.Courier.10", DWORD color=0xFFFF9900);
	int SetEditTag(char *fontname="DX.F.Courier.10", DWORD color=0xFFAAAAAA);
	int SetDirective(char *fontname="DX.F.Courier.10", DWORD color=0xFF00FF88);

	void GetDefault(CDXFont *&font, DWORD &color);
	void GetKeyword(CDXFont *&font, DWORD &color);
	void GetString(CDXFont *&font, DWORD &color);
	void GetComment(CDXFont *&font, DWORD &color);
	void GetKnownNames(CDXFont *&font, DWORD &color);
	void GetEditTag(CDXFont *&font, DWORD &color);
	void GetDirective(CDXFont *&font, DWORD &color);

	CDXFont** GetFonts(){return fonts;}
	DWORD* GetColors(){return colors;}


protected:

	int SetHL(int type,char *fontname,DWORD color);

	CDXFont **fonts;
	DWORD *colors;
};
extern CDXHighLightFormat *DXHighLightFormat;


class CDXHighlighter{
public:
	CDXHighlighter(CFormatedText *txt);
	~CDXHighlighter();

	int Highlight(); //highlightuje cely text
	int HighlightFromBlock(CTextBlock *b); //highlightuje od zadaneho bloku
	int HighlightBlock(CTextBlock *b, int &comment); //highlightuje blok

protected:
	CFormatedText *text;
};

#endif