///////////////////////////////////////////////
//
// widget.cpp
//
// Implementace pro elementy okna (widgety)
//
// CGUIStaticText
// CGUIStaticPicture
// CGUILine
// CGUIContextHelp
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "widgets.h"
#include "primitives.h"
#include "gui.h"

#include "dxbliter.h"
#include "krkal.h"


//////////////////////////////////////////////////////////////////////
// CGUIStaticText
//////////////////////////////////////////////////////////////////////

CGUIStaticText::CGUIStaticText(float _x, float _y, char *_text, char *fontName, int size, bool pixelSize, int color)
	: CGUIWidget(_x,_y)
{
	HFONT font;
	
	if(!_text || !fontName)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIStaticText::CGUIStaticText> Text or Font not assigned");
	}

	int len = strlen(_text);
	originalText = new char[len+1];
	strcpy(originalText,_text);
	numChars = len;
	SIZE vel;

	if(len==0)	// text nulove delky
	{
		SetSize(0,0);
		throw_going=true;
		focusable=false;	// tento widget nelze focusovat (kliknutim mysi)
		texts=0;
		text=0;
		guiFont=0;
		numChars=0;
		numLines=0;
		AddToTrash();
		return;
	}

	guiFont = new CGUIFont();

	if(!pixelSize)
		guiFont->CreateFont(fontName,size);
	else
		guiFont->CreateFontPS(fontName,size);

	font = guiFont->font;

	SelectObject(mainGUI->dc,font);
	GetTextExtentPoint32(mainGUI->dc,_text,len,&vel);

	SetSize((float)vel.cx,(float)vel.cy);
	text = new CBltText(x,y,(float)vel.cx,(float)vel.cy,originalText,len,font, color,0,0);
	throw_going=true;
	focusable=false;	// tento widget nelze focusovat (kliknutim mysi)
	texts = 0;

	maxX=maxY=linesSpaces=0;
	alignment=aLeft;
	maxNumLines=0;

	AddToTrash();
}


CGUIStaticText::CGUIStaticText(float _x, float _y, float maxx, char *_text, char *fontName, int size, bool pixelSize, EGUIAlignment align, int color)
	: CGUIWidget(_x,_y)
{
	HFONT font;
	if(!_text || !fontName)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIStaticText::CGUIStaticText> Text or Font not assigned");
	}

	int len = strlen(_text);
	originalText = new char[len+1];
	strcpy(originalText,_text);

	SIZE vel;

	if(len==0)	// text nulove delky
	{
		SetSize(0,0);
		throw_going=true;
		focusable=false;	// tento widget nelze focusovat (kliknutim mysi)
		texts=0;
		text=0;
		guiFont=0;
		numLines=0;
		numChars=0;
		AddToTrash();
		return;
	}

	guiFont = new CGUIFont();

	if(!pixelSize)
		guiFont->CreateFont(fontName,size);
	else
		guiFont->CreateFontPS(fontName,size);

	font = guiFont->font;
	SelectObject(mainGUI->dc,font);

	// oriznuti textu na maximalni delku podle max. velikosti obdelniku
	while(len)
	{
		GetTextExtentPoint32(mainGUI->dc,_text,len,&vel);
		if((float)vel.cx > maxx)
			len--;
		else
			break;
	}
	numChars = len;

    SetSize((float)vel.cx,(float)vel.cy);
	throw_going=true;
	focusable=false;	// tento widget nelze focusovat (kliknutim mysi)
	texts=0;

	switch(align)
	{
		case aLeft :
			break;
		case aRight :
			x += maxx - (float)vel.cx;
			break;
		case aCenter :
			x += floorf((maxx - (float)vel.cx)/2 + 0.5f);
			break;
	}
	text = new CBltText(x,y,(float)vel.cx,(float)vel.cy,originalText,len,font, color,0,0);

	maxX=maxx;
	maxY=linesSpaces=0;
	alignment=align;
	maxNumLines=0;

	AddToTrash();
}

CGUIStaticText::CGUIStaticText(float _x, float _y, float maxx, float maxy, char *_text, char *fontName, int size, bool pixelSize, EGUIAlignment align, int color, float LinesSpaces)
	: CGUIWidget(_x,_y)
{
	HFONT font;
	float gsx=0, gsy=0;
	numLines = 0;
	int i,len,l;

	char* startLine[200];
	int lengthLine[200];

	if(!_text || !fontName)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIStaticText::CGUIStaticText> Text or Font not assigned");
	}


	int lengthText = strlen(_text);
	originalText = new char[lengthText+1];
	strcpy(originalText,_text);
	SIZE vel;


	guiFont = new CGUIFont();

	if(!pixelSize)
		guiFont->CreateFont(fontName,size);
	else
		guiFont->CreateFontPS(fontName,size);

	font = guiFont->font;
	SelectObject(mainGUI->dc,font);


	for( i=0;i<200;i++)
	{
		startLine[i]=0;
		lengthLine[i]=0;
	}

	startLine[0] = originalText;

	while(lengthText>0)
	{
		len = 0;
		while(1)
		{
			GetTextExtentPoint32(mainGUI->dc,startLine[numLines],len,&vel);

			if(startLine[numLines][len] == '\n')
			{
				lengthLine[numLines] = len;
				startLine[numLines+1] = startLine[numLines] + lengthLine[numLines] + 1;
				lengthText -= lengthLine[numLines]+1;
				break;
			}

			if(maxx>0 && ((float)vel.cx > maxx))
			{
				l = len-1;
				while((startLine[numLines][l] != ' ') && (startLine[numLines] != &startLine[numLines][l]) )
					l--;
				if(startLine[numLines] == &startLine[numLines][l])
				{
					lengthLine[numLines] = len-1;
					startLine[numLines+1] = startLine[numLines] + lengthLine[numLines];
					lengthText -= lengthLine[numLines];
				}else{
					lengthLine[numLines] = l;
					startLine[numLines+1] = startLine[numLines] + lengthLine[numLines] + 1;
					lengthText -= lengthLine[numLines]+1;
				}
				break;
			}else
				len++;
			
			if(len>lengthText)
			{
				lengthLine[numLines] = lengthText;
				lengthText -= lengthLine[numLines];
				break;
			}
		}
		numLines++;
		gsy += (float)vel.cy + LinesSpaces;
		if(maxy && gsy>maxy)
			break;	// pretekla mi max. vyska textu => dalsi text jiz nezpracovavam
	}

	if(numLines==0)
	{
		text = 0;
		texts = 0;
		SetSize(0,0);
		throw_going=true;
		focusable=false;	// tento widget nelze focusovat (kliknutim mysi)
		numLines=0;
		numChars=0;
		AddToTrash();
		guiFont->Release();
		guiFont = 0;
		return;
	}

	numChars=0;
	texts = new CBltText*[numLines];
	for(i=0;i<numLines;i++)
	{
		numChars+=lengthLine[i];
		GetTextExtentPoint32(mainGUI->dc,startLine[i],lengthLine[i],&vel);
		if(gsx < (float)vel.cx)
			gsx = (float)vel.cx;
		switch(align)
		{
			case aLeft :
				texts[i] = new CBltText(x,_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
				break;
			case aRight :
				texts[i] = new CBltText(x+maxx - (float)vel.cx,_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
				break;
			case aCenter :
				texts[i] = new CBltText(x+floorf((maxx - (float)vel.cx)/2 + 0.5f),_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
				break;
		}
		_y += (float)vel.cy + LinesSpaces;
	}

	if(numLines<=1)
	{
		SetSize((float)vel.cx,(float)vel.cy);
	}else{
		if((maxx > 0) && (align != aLeft))
			SetSize(maxx,gsy - LinesSpaces);
		else
			SetSize(gsx,gsy - LinesSpaces);	// pokud neni sirka radky nastavena, nastavi se na nejsirsi pouzity radek 
											//(odradkovani se provadi jen na '\n')
	}

	throw_going=true;
	focusable=false;	// tento widget nelze focusovat (kliknutim mysi)
	text = 0;

	maxX=maxx;
	maxY=maxy;
	linesSpaces=LinesSpaces;
	alignment=align;
	maxNumLines=0;

	AddToTrash();
}

CGUIStaticText::CGUIStaticText(const char *_text, CGUIFont* _font, float _x, float _y, int color, float maxx, float maxy, EGUIAlignment align, int maxLines, float LinesSpaces)
	: CGUIWidget(_x,_y)
{
	float gsx=0, gsy=0;
	numLines = 0;
	int i,len,l;
	HFONT font;

	char* startLine[256];
	int lengthLine[256];

	if(!_text || !_font || !_font->font)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIStaticText::CGUIStaticText> Text or Font not assigned");
	}

	guiFont = _font;
	font = guiFont->font;

	originalText = KRKAL->CopyOutStrByLanguage(_text);
	int lengthText = strlen(originalText);
	SIZE vel;

	SelectObject(mainGUI->dc,font);


	for( i=0;i<200;i++)
	{
		startLine[i]=0;
		lengthLine[i]=0;
	}

	startLine[0] = originalText;

	while(lengthText>0)
	{
		len = 0;
		while(1)
		{
			GetTextExtentPoint32(mainGUI->dc,startLine[numLines],len,&vel);

			if(startLine[numLines][len] == '\n')
			{
				lengthLine[numLines] = len;
				startLine[numLines+1] = startLine[numLines] + lengthLine[numLines] + 1;
				lengthText -= lengthLine[numLines]+1;
				break;
			}

			if(maxx>0 && ((float)vel.cx > maxx))
			{
				l = len-1;
				while((startLine[numLines][l] != ' ') && (startLine[numLines] != &startLine[numLines][l]) )
					l--;
				if(startLine[numLines] == &startLine[numLines][l])
				{
					lengthLine[numLines] = len-1;
					startLine[numLines+1] = startLine[numLines] + lengthLine[numLines];
					lengthText -= lengthLine[numLines];
				}else{
					lengthLine[numLines] = l;
					startLine[numLines+1] = startLine[numLines] + lengthLine[numLines] + 1;
					lengthText -= lengthLine[numLines]+1;
				}
				break;
			}else
				len++;
			
			if(len>lengthText)
			{
				lengthLine[numLines] = lengthText;
				lengthText -= lengthLine[numLines];
				break;
			}
		}
		gsy += (float)vel.cy + LinesSpaces;
		if(maxy && gsy>maxy)
			break;	// pretekla mi max. vyska textu => dalsi text jiz nezpracovavam
		numLines++;
		if(maxLines && maxLines<=numLines)
			break;
	}

	if(numLines==0)
	{
		text = 0;
		texts = 0;
		SetSize(0,0);
		throw_going=true;
		focusable=false;	// tento widget nelze focusovat (kliknutim mysi)
		guiFont=0;
		numChars=0;
		AddToTrash();
		return;
	}

	numChars=0;
	texts = new CBltText*[numLines];
	for(i=0;i<numLines;i++)
	{
		GetTextExtentPoint32(mainGUI->dc,startLine[i],lengthLine[i],&vel);
		numChars+=lengthLine[i];
		if(gsx < (float)vel.cx)
			gsx = (float)vel.cx;
		switch(align)
		{
			case aLeft :
				texts[i] = new CBltText(floorf(x),floorf(_y),(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
				break;
			case aRight :
				texts[i] = new CBltText(x+maxx - (float)vel.cx,_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
				break;
			case aCenter :
				texts[i] = new CBltText(x+floorf((maxx - (float)vel.cx)/2 + 0.5f),_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
				break;
		}
		_y += (float)vel.cy + LinesSpaces;
	}

	if(numLines<=1)
	{
		SetSize((float)vel.cx,(float)vel.cy);
	}else{
		if((maxx > 0) && (align != aLeft))
			SetSize(maxx,gsy - LinesSpaces);
		else
			SetSize(gsx,gsy - LinesSpaces);	// pokud neni sirka radky nastavena, nastavi se na nejsirsi pouzity radek 
											//(odradkovani se provadi jen na '\n')
	}

	throw_going=true;
	focusable=false;	// tento widget nelze focusovat (kliknutim mysi)
	text = 0;
	guiFont->AddRef();

	maxX=maxx;
	maxY=maxy;
	linesSpaces=LinesSpaces;
	alignment=align;
	maxNumLines=maxLines;

	AddToTrash();
}

CGUIStaticText::~CGUIStaticText()
{
	RemoveFromTrash();

	if(text){
		CBltWindow *wnd = text->GetParent();
		if(wnd){
			wnd->DeleteChild(text);
		}else{
			delete text;
		}
	}

	if(texts){
		for(int i=0;i<numLines;i++)
		{
			CBltWindow *wnd = texts[i]->GetParent();
			if(wnd){
				wnd->DeleteChild(texts[i]);
			}else{
				delete texts[i];
			}
		}
		SAFE_DELETE_ARRAY(texts);
	}

	if(originalText)
		delete[] originalText;

	if(guiFont)
		guiFont->Release();
}

void CGUIStaticText::Resize(float _sx, float _sy)
{
	CheckResize(_sx,_sy);
	if(text)
	{
		text->Resize(_sx,_sy);
		SetSize(_sx,_sy);
	}

	// !@#$ viceradkovy text nepodporuje resize !!!
}

void CGUIStaticText::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	if(text)
		text->Move(_x-x,_y-y);

	if(texts){
		for(int i=0;i<numLines;i++)
		{
			texts[i]->Move(_x-x,_y-y);
		}
	}

	SetPos(_x,_y);
}

void CGUIStaticText::AddToEngine(CBltWindow *rootwnd)
{
	if(text)
		rootwnd->AddChild(text);

	if(texts){
		for(int i=0;i<numLines;i++)
		{
			rootwnd->AddChild(texts[i]);
		}
	}
}

void CGUIStaticText::RemoveFromEngine()
{
	if(text)
	{
		CBltWindow *wnd = text->GetParent();
		if(wnd)
			wnd->RemoveChild(text);
	}

	if(texts){
		CBltWindow *wnd = texts[0]->GetParent();
		if(wnd)
			for(int i=0;i<numLines;i++)
			{
				wnd->RemoveChild(texts[i]);
			}
	}
}

void CGUIStaticText::BringToTop()
{
	if(text)
	{
		text->BringToTop();
	}

	if(texts){
		for(int i=0;i<numLines;i++)
		{
			texts[i]->BringToTop();
		}
	}
}


void CGUIStaticText::SetVisible(int vis)
{
	if(text)
		text->SetVisible(vis);

	if(texts){
		for(int i=0;i<numLines;i++)
		{
			texts[i]->SetVisible(vis);
		}
	}
	
	SetVisibility(vis);
}

void CGUIStaticText::AddToEngineBackPosition(CBltWindow *rootwnd, CBltElem *pos)
{
	if(text && pos)
		rootwnd->AddChildToBackOfEl(text,pos);

	if(texts && pos){
		for(int i=0;i<numLines;i++)
		{
			rootwnd->AddChildToBackOfEl(texts[i],pos);
		}
	}
}

CBltElem* CGUIStaticText::GetElemPosition()
{
	if(text)
		return text->GetFrontElem();

	if(texts)
		return texts[numLines-1]->GetFrontElem();

	return 0;
}

void CGUIStaticText::SetColor(UINT color)
{
	if(text)
		text->SetCol(color);

	if(texts){
		for(int i=0;i<numLines;i++)
		{
			texts[i]->SetCol(color);
		}
	}
}

UINT CGUIStaticText::GetColor()
{
	if(text)
		return text->GetCol();
	else if(texts)
		return texts[0]->GetCol();

	return 0;
}

void CGUIStaticText::CreateText(char *_text, CGUIFont* _font, float _x, float _y, int color, float maxx, float maxy, EGUIAlignment align, int maxLines, float LinesSpaces)
{
	float gsx=0, gsy=0;
	int numLinesNEW = 0;
	int i,len,l;
	HFONT font;

	char* startLine[256];
	int lengthLine[256];

	if(!_text || !_font || !_font->font)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIStaticText::CreateText> Text or Font not assigned");
	}

	if(guiFont != _font)
	{
		guiFont->Release();
		_font->AddRef();
	}
	guiFont = _font;
	font = guiFont->font;

	int lengthText = strlen(_text);
	char *newText = new char[lengthText+1];
	strcpy(newText,_text);
	SIZE vel;

	SelectObject(mainGUI->dc,font);


	for( i=0;i<200;i++)
	{
		startLine[i]=0;
		lengthLine[i]=0;
	}

	startLine[0] = newText;

	while(lengthText>0)
	{
		len = 0;
		while(1)
		{
			GetTextExtentPoint32(mainGUI->dc,startLine[numLinesNEW],len,&vel);

			if(startLine[numLinesNEW][len] == '\n')
			{
				lengthLine[numLinesNEW] = len;
				startLine[numLinesNEW+1] = startLine[numLinesNEW] + lengthLine[numLinesNEW] + 1;
				lengthText -= lengthLine[numLinesNEW]+1;
				break;
			}

			if(maxx>0 && ((float)vel.cx > maxx))
			{
				l = len-1;
				while((startLine[numLinesNEW][l] != ' ') && (startLine[numLinesNEW] != &startLine[numLinesNEW][l]) )
					l--;
				if(startLine[numLinesNEW] == &startLine[numLinesNEW][l])
				{
					lengthLine[numLinesNEW] = len-1;
					startLine[numLinesNEW+1] = startLine[numLinesNEW] + lengthLine[numLinesNEW];
					lengthText -= lengthLine[numLinesNEW];
				}else{
					lengthLine[numLinesNEW] = l;
					startLine[numLinesNEW+1] = startLine[numLinesNEW] + lengthLine[numLinesNEW] + 1;
					lengthText -= lengthLine[numLinesNEW]+1;
				}
				break;
			}else
				len++;
			
			if(len>lengthText)
			{
				lengthLine[numLinesNEW] = lengthText;
				lengthText -= lengthLine[numLinesNEW];
				break;
			}
		}
		gsy += (float)vel.cy + LinesSpaces;
		if(maxy && gsy>maxy)
			break;	// pretekla mi max. vyska textu => dalsi text jiz nezpracovavam
		numLinesNEW++;
		if(maxLines && maxLines<=numLinesNEW)
			break;
	}

	if(numLinesNEW==0)
	{
		if(text){
			CBltWindow *wnd = text->GetParent();
			if(wnd){
				wnd->DeleteChild(text);
			}else{
				delete text;
			}
			text=0;
		}

		if(texts){
			for(int i=0;i<numLines;i++)
			{
				CBltWindow *wnd = texts[i]->GetParent();
				if(wnd){
					wnd->DeleteChild(texts[i]);
				}else{
					delete texts[i];
				}
			}
			SAFE_DELETE_ARRAY(texts);
		}

		SAFE_DELETE(originalText);

		if(guiFont)
			guiFont->Release();


		SetSize(0,0);
		guiFont=0;
		numChars=0;
		numLines=0;
		return;
	}

	CBltWindow *rootWnd=0;
	CBltElem* pos=0;

	if(texts)
	{
		rootWnd = texts[numLines-1]->GetParent();
		pos = texts[numLines-1];
	}

	numChars=0;
	CBltText **textsNEW = new CBltText*[numLinesNEW];
	for(i=0;i<numLinesNEW;i++)
	{
		GetTextExtentPoint32(mainGUI->dc,startLine[i],lengthLine[i],&vel);
		numChars+=lengthLine[i];
		if(gsx < (float)vel.cx)
			gsx = (float)vel.cx;
		if(i<numLines)
		{
			switch(align)
			{
				case aLeft :
					texts[i]->SetText(floorf(x),floorf(_y),(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
					break;
				case aRight :
					texts[i]->SetText(x+maxx - (float)vel.cx,_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
					break;
				case aCenter :
					texts[i]->SetText(x+floorf((maxx - (float)vel.cx)/2 + 0.5f),_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
					break;
			}
			textsNEW[i] = texts[i];
		}
		else
		{
			switch(align)
			{
				case aLeft :
					textsNEW[i] = new CBltText(floorf(x),floorf(_y),(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
					break;
				case aRight :
					textsNEW[i] = new CBltText(x+maxx - (float)vel.cx,_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
					break;
				case aCenter :
					textsNEW[i] = new CBltText(x+floorf((maxx - (float)vel.cx)/2 + 0.5f),_y,(float)vel.cx,(float)vel.cy,startLine[i],lengthLine[i],font, color,0,0);
					break;
			}
			if(rootWnd)
			{
				rootWnd->AddChildToFrontOfEl(textsNEW[i],pos);
				pos=textsNEW[i];
			}
		}
		_y += (float)vel.cy + LinesSpaces;
	}

	if(numLinesNEW<=1)
	{
		SetSize((float)vel.cx,(float)vel.cy);
	}else{
		if((maxx > 0) && (align != aLeft))
			SetSize(maxx,gsy - LinesSpaces);
		else
			SetSize(gsx,gsy - LinesSpaces);	// pokud neni sirka radky nastavena, nastavi se na nejsirsi pouzity radek 
											//(odradkovani se provadi jen na '\n')
	}

	if(numLinesNEW<numLines)
	{
		for(int i=numLinesNEW;i<numLines;i++)
		{
			if(rootWnd)
				rootWnd->RemoveChild(texts[i]);

			delete texts[i];
		}
	}


	SAFE_DELETE_ARRAY(originalText);
	originalText=newText;

	SAFE_DELETE_ARRAY(texts);
	texts = textsNEW;
	numLines = numLinesNEW;
}

void CGUIStaticText::ChangeText(char *_text, CGUIFont* _font)
{
	if(_font==0)
		_font=guiFont;
	CreateText(_text,_font,x,y,GetColor(),maxX,maxY,alignment,maxNumLines,linesSpaces);
}


//////////////////////////////////////////////////////////////////////
// CGUIContextHelp
//////////////////////////////////////////////////////////////////////

CGUIElement* CGUIContextHelp::activator=0;
CGUIContextHelp* CGUIContextHelp::contextHelp=0;

CGUIContextHelp::CGUIContextHelp(float maxx, float maxy, char *_text, char *fontName, int size, bool pixelSize, int textColor, int backColor, int borderColor)
	: CGUIStaticText(0,0,maxx,maxy,_text, fontName, size, pixelSize, aLeft, textColor)
{
	RemoveFromTrash();	// jistotni vyndani z kose
						// rodic (CGUIStaticText) ve svem konstruktoru se umistil do kose,
						// ovsem pokud nastane v tomto konstruktoru vyjimka, objekt nebude
						// spravne vytvoren a tedy nesmi byt v kosi
						// na konci konstruktoru po jistem spravnem vytvoreni se do kose vratim
						// tento princip se pouzije na vsechny potomky predku, kteri se umistuji do kose
	MsgAcceptConsume(MsgTimer);
	MsgSetProduce(MsgNone);

	req = false;
	active=false;

	background = new CGUIRectHost(-3,-1,sx+6,sy+2,backColor);
	SetPos(-3,-1);
	SetSize(sx+6,sy+2);
	border[0] = new CBltLine(x,y,x+sx,y,borderColor);
	border[1] = new CBltLine(x+sx,y,x+sx,y+sy,borderColor);
	border[2] = new CBltLine(x+sx,y+sy,x,y+sy,borderColor);
	border[3] = new CBltLine(x,y+sy,x,y,borderColor);
	background->RemoveFromTrash();

	SetVisible(0);
	Move(3,1);

	AddToTrash();
}

CGUIContextHelp::~CGUIContextHelp()
{
	RemoveFromTrash();
	timerGUI->DeleteAllRequests(this);
	if(background)
		delete background;
	
	CBltWindow *wnd=0;
	if(border[0])
		wnd = border[0]->GetParent();
	if(wnd)
	{
		for(int i=0;i<4;i++)
		{
			if(border[i])
				wnd->DeleteChild(border[i]);
		}
	}else{
		for(int i=0;i<4;i++)
			SAFE_DELETE(border[i]);
	}

	contextHelp=0;
}

void CGUIContextHelp::Refresh(char* text)
{
	ChangeText(text);

/*
	background->Resize(sx+6,sy+2);
	
	background = new CGUIRectHost(-3,-1,sx+6,sy+2,backColor);
	SetPos(-3,-1);
	SetSize(sx+6,sy+2);
	border[0] = new CBltLine(x,y,x+sx,y,borderColor);
	border[1] = new CBltLine(x+sx,y,x+sx,y+sy,borderColor);
	border[2] = new CBltLine(x+sx,y+sy,x,y+sy,borderColor);
	border[3] = new CBltLine(x,y+sy,x,y,borderColor);
	background->RemoveFromTrash();

	SetVisible(0);
	Move(3,1);
*/

}

void CGUIContextHelp::Resize(float _sx, float _sy)
{
}

void CGUIContextHelp::Move(float _x, float _y)
{
	float dx=x, dy=y;
	CGUIStaticText::Move(_x,_y);
	dx=x-dx;dy=y-dy;

	if(background)
		background->Move(x,y);
	for(int i=0;i<4;i++)
		if(border[i])
			border[i]->Move(dx,dy);
}

void CGUIContextHelp::AddToEngine(CBltWindow *rootwnd)
{
	if(background)
		background->AddToEngine(rootwnd);
	for(int i=0;i<4;i++)
		if(border[i])
			rootwnd->AddChild(border[i]);
	CGUIStaticText::AddToEngine(rootwnd);
}

void CGUIContextHelp::RemoveFromEngine()
{
	CGUIStaticText::RemoveFromEngine();

	CBltWindow *wnd=0;

	if(background)
		background->RemoveFromEngine();

	if(border[0])
		wnd = border[0]->GetParent();
	if(wnd)
	{
		for(int i=0;i<4;i++)
		{
			if(border[i])
				wnd->RemoveChild(border[i]);
		}
	}
}

void CGUIContextHelp::BringToTop()
{
	if(background)
		background->BringToTop();
	for(int i=0;i<4;i++)
		if(border[i])
			border[i]->BringToTop();
	CGUIStaticText::BringToTop();
}


void CGUIContextHelp::SetVisible(int vis)
{
	CGUIStaticText::SetVisible(vis);
	if(background)
		background->SetVisible(vis);
	for(int i=0;i<4;i++)
		if(border[i])
			border[i]->SetVisible(vis);
}

void CGUIContextHelp::On(CGUIElement* _activator, char* text)
{
	if(!text)
		return;

	if(contextHelp && activator==_activator && !strcmp(text,contextHelp->GetText()))
	{
		if(!contextHelp->req && !timerGUI->ContainRequest(contextHelp))
		{
			timerGUI->AddRequest(contextHelp,HELP_DELAY);
			contextHelp->req = true;
			contextHelp->active=false;
		}
	}
	else
	{
		if(!contextHelp)
		{
			contextHelp = new CGUIContextHelp(200,0,text,ARIAL,8,false);
			desktop->AddFrontElem(contextHelp);
		}
		else
		{
			if(activator!=_activator)
			{	
				contextHelp->Off(activator);
			}

			desktop->DeleteFrontElem(contextHelp);
			contextHelp = new CGUIContextHelp(200,0,text,ARIAL,8,false);
			desktop->AddFrontElem(contextHelp);

			//contextHelp->ChangeText(text);
		}

		activator = _activator;

		if(!contextHelp->req && !timerGUI->ContainRequest(contextHelp))
		{
			timerGUI->AddRequest(contextHelp,HELP_DELAY);
			contextHelp->req = true;
			contextHelp->active=false;
		}
	}
}

void CGUIContextHelp::Off(CGUIElement* _activator)
{
	if(!contextHelp || activator!=_activator)
		return;

	if(contextHelp->req)
	{
		timerGUI->DeleteRequest(contextHelp);
		contextHelp->req=false;
	}
	
	if(contextHelp->active)
	{
		float xx,yy;
		contextHelp->GetPos(xx,yy);
		contextHelp->active=false;
		contextHelp->Move(-xx,-yy);
		contextHelp->SetVisible(0);
	}
}

void CGUIContextHelp::OnSpecific(CGUIElement* _activator, char* text, char *fontName, int fontSize, float maxx, float maxy, int textColor, int backColor, int borderColor)
{
	if(!text)
		return;

	if(contextHelp && activator==_activator && !strcmp(text,contextHelp->GetText()))
	{
		if(!contextHelp->req && !timerGUI->ContainRequest(contextHelp))
		{
			timerGUI->AddRequest(contextHelp,HELP_DELAY);
			contextHelp->req = true;
			contextHelp->active=false;
		}
	}
	else
	{
		if(!contextHelp)
		{
			contextHelp = new CGUIContextHelp(maxx,maxy,text,fontName,fontSize,false,textColor,backColor,borderColor);
			desktop->AddFrontElem(contextHelp);
		}
		else
		{
			if(activator!=_activator)
			{	
				contextHelp->Off(activator);
			}

			desktop->DeleteFrontElem(contextHelp);
			contextHelp = new CGUIContextHelp(maxx,maxy,text,fontName,fontSize,false,textColor,backColor,borderColor);
			desktop->AddFrontElem(contextHelp);

			//contextHelp->ChangeText(text);
		}

		activator = _activator;

		if(!contextHelp->req && !timerGUI->ContainRequest(contextHelp))
		{
			timerGUI->AddRequest(contextHelp,HELP_DELAY);
			contextHelp->req = true;
			contextHelp->active=false;
		}
	}
}

int CGUIContextHelp::TimerImpulse(typeID timerID, float time)
{
	if(active)
	{
		// prisel impuls ke schovani helpu
		active=false;
		req=false;
		Move(-x,-y);
		SetVisible(0);
	}else{
		// prisel impuls k zobrazeni helpu
		active=true;

		float xx;
		float yy;
		mainGUI->cursor->GetPos(xx,yy);

		float hsx, hsy;
		float dsx, dsy;
		GetSize(hsx,hsy);
		desktop->GetSize(dsx,dsy);
		if(yy + hsy + 18 > dsy)
			yy = yy - 8 - hsy;
		else
			yy = yy + 18;

		if(xx + hsx > dsx)
			xx = dsx - hsx - 5;


		Move(xx,yy);
		BringToTop();
		SetVisible(1);

		req=true;
		timerGUI->AddRequest(this,HELP_TIME);
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////
// CGUIStaticPicture
//////////////////////////////////////////////////////////////////////

const int CGUIStaticPicture::numEvents = 1;
const int CGUIStaticPicture::availableEvents[numEvents] = {EMouseButton};


CGUIStaticPicture::CGUIStaticPicture(float _x, float _y, const char *texName, float _sx, float _sy, UINT color)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	UINT ssx, ssy;
	CTexture *texture = new CTexture;
	rect = 0;
	bool released = false;

	if ( !( texture->CreateFromFile(texName) ) )
	{
		texture->Release();
		throw CExc(eGUI, E_FILE_NOT_FOUND,"CGUIStaticPicture::CGUIStaticPicture> Cannot find texture: %s",texName);
	}

	try{
		texture->GetImageSize(&ssx,&ssy);
		if(_sx>0)
			ssx=(UINT)_sx;
		if(_sy>0)
			ssy=(UINT)_sy;

		rect = new CBltRect(_x, _y, (float)ssx, (float)ssy, color);
		rect->SetTexture(texture);
		texture->Release();
		released=true;

		SetSize((float)ssx,(float)ssy);
		SetPos(_x,_y);

		throw_going=true;
		focusable=false;
		MsgAcceptConsume(MsgMouseL);
	//	MsgSetConsume(MsgNone);
	//	MsgSetProduce(MsgNone);	// preposila elementum pod sebou
		AddToTrash();
	}catch(CExc)
	{
		if(!released)
			texture->Release();
		SAFE_DELETE(rect);
		throw;
	}
}


CGUIStaticPicture::CGUIStaticPicture(float _x, float _y, CTexture *texture, float _sx, float _sy, UINT color)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	UINT ssx, ssy;
	rect = 0;

	try{
		texture->GetImageSize(&ssx,&ssy);
		if(_sx>0)
			ssx=(UINT)_sx;
		if(_sy>0)
			ssy=(UINT)_sy;

		rect = new CBltRect((float)_x, (float)_y, (float)ssx, (float)ssy, color);
		rect->SetTexture(texture);

		SetSize((float)ssx,(float)ssy);
		SetPos(_x,_y);
		throw_going=true;
		focusable=false;
		MsgAcceptConsume(MsgMouseL);
	//	MsgSetConsume(MsgNone);
	//	MsgSetProduce(MsgNone);	// preposila elementum pod sebou
		AddToTrash();
	}catch(CExc)
	{
		SAFE_DELETE(rect);
		throw;
	}
}

CGUIStaticPicture::CGUIStaticPicture(float _x, float _y, float _sx, float _sy, UINT color)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	rect = 0;
	try{
		rect = new CBltRect((float)_x, (float)_y, (float)_sx, (float)_sy, color);
		SetSize(_sx,_sy);
		SetPos(_x,_y);
		throw_going=true;
		focusable=false;
		MsgAcceptConsume(MsgMouseL);
	//	MsgSetConsume(MsgNone);
	//	MsgSetProduce(MsgNone);	// preposila elementum pod sebou
		AddToTrash();
	}catch(CExc)
	{
		SAFE_DELETE(rect);
		throw;
	}
}


CGUIStaticPicture::~CGUIStaticPicture()
{
	RemoveFromTrash();
	if(rect)
	{
		CBltWindow *wnd = rect->GetParent();
		if(wnd)
			wnd->DeleteChild(rect);
		else
			delete rect;
		rect = 0;
	}
}


void CGUIStaticPicture::AddToEngine(CBltWindow *rootwnd)
{
	if(rootwnd && rect)
		rootwnd->AddChild(rect);
}


void CGUIStaticPicture::RemoveFromEngine()
{
	if(!rect)
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIStaticPicture::RemoveFromEngine> rect not found");
	}

	CBltWindow *wnd = rect->GetParent();
	if(wnd)
		wnd->RemoveChild(rect);
	else
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIStaticPicture::RemoveFromEngine> rect not found in window");
	}
}

void CGUIStaticPicture::BringToTop()
{
	if(rect)
		rect->BringToTop();
}

CBltElem* CGUIStaticPicture::GetElemPosition()
{
	if(rect)
		return rect->GetFrontElem();
	else
		return 0;
}

void CGUIStaticPicture::Resize(float _sx, float _sy)
{
	SetSize(_sx,_sy);
	rect->Resize(sx,sy);
}

void CGUIStaticPicture::Move(float _x, float _y)
{
	rect->Move(_x-x,_y-y);
	SetPos(_x,_y);
}

void CGUIStaticPicture::Rotate(float alfa)
{
	
	CMatrix2D r,p,pp,c;
	r.Rotate(alfa);
	p.Translate(-x,-y-sy);
	pp.Translate(x,y);
	c = p*r*pp;
	

	rect->SetTrans(c);

//	SetSize();

}


void CGUIStaticPicture::SetVisible(int vis)
{
	rect->SetVisible(vis);
	SetVisibility(vis);
}

void CGUIStaticPicture::SetTexture(CTexture *tex)
{
	if(rect)
		rect->SetTexture(tex);
}

void CGUIStaticPicture::SetColor(UINT color)
{
	if(rect)
		rect->SetCol(color);
}


void CGUIStaticPicture::RebuildStaticPicture(CTexture *tex, float _x, float _y, float _sx, float _sy)
{
	if(rect)
	{
		if(_x>=0 && _y>=0)
		{
			rect->Move(_x-x,_y-y);
			SetPos(_x,_y);
		}
		if(_sx>=0 && _sy>=0)
		{
			rect->Resize(_sx,_sy);
			SetSize(_sx,_sy);
		}
		if(tex)
		{
			if(_sx<0 || _sy<0)
			{
				UINT ssx, ssy;
				tex->GetImageSize(&ssx,&ssy);
				rect->Resize((float)ssx,(float)ssy);
				SetSize((float)ssx,(float)ssy);
			}
		}
		rect->SetTexture(tex);
	}
}

int CGUIStaticPicture::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{
	EventArise(EMouseButton,0,mouseState);
	return 1;
}
int CGUIStaticPicture::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	return 1;
}



//////////////////////////////////////////////////////////////////////
// CGUILine
//////////////////////////////////////////////////////////////////////

CGUILine::CGUILine(float x1, float y1, float x2, float y2, UINT color)
	: CGUIWidget(x1,y1)
{
	throw_going=true;
	focusable=false;	
	line = new CBltLine(x1,y1,x2,y2,color);
	SetSize(fabsf(x2-x1)+1,fabsf(y2-y1)+1);
	SetPos((x1<x2)? x1 : x2, (y1<y2)? y1 :y2);
	if(y1<y2)
		up_down = true;
	else
		up_down = false;
	if(x1<x2)
		left_right = true;
	else
		left_right = false;
	AddToTrash();
}

CGUILine::~CGUILine()
{
	RemoveFromTrash();
	if(line)
	{
		CBltWindow *wnd = line->GetParent();
		if(wnd)
			wnd->DeleteChild(line);
		else
			delete line;
		line = 0;
	}
}


void CGUILine::AddToEngine(CBltWindow *rootwnd)
{
	if(rootwnd && line)
		rootwnd->AddChild(line);
}


void CGUILine::RemoveFromEngine()
{
	if(line)
	{
		CBltWindow *wnd = line->GetParent();
		if(wnd)
			wnd->RemoveChild(line);
	}
}

void CGUILine::BringToTop()
{
	if(line)
		line->BringToTop();
}

/*
void CGUILine::Resize(float _sx, float _sy)
{
	float lsx, lsy;
	GetSize(lsx,lsy);

	CheckResize(_sx,_sy);

	if(lsy==0)
	{
		SetPoints(x,y,x+_sx,y);
		SetSize(_sx,0);
	}else if(lsx==0)
	{
		SetPoints(x,y,x,y+_sy);
		SetSize(0,_sy);
	}else if(up_down)
	{
		SetPoints(x,y,x+_sx,y+_sy);
		SetSize(_sx,_sy);
	}else if(!up_down)
	{
		SetPoints(x,y+_sy,x+_sx,y);
		SetSize(_sx,_sy);
	}
}
*/

void CGUILine::Resize(float _sx, float _sy)
{
	float lsx, lsy;
	GetSize(lsx,lsy);

	CheckResize(_sx,_sy);

	if(lsy==1)
	{
		if(left_right)
			SetPoints(x,y,x+_sx-1,y);
		else
			SetPoints(x+_sx-1,y,x,y);
		//SetSize(_sx,0);
	}else if(lsx==1)
	{
		if(up_down)
			SetPoints(x,y,x,y+_sy-1);
		else
			SetPoints(x,y+_sy-1,x,y);
		//SetSize(0,_sy);
	}else if(up_down)
	{
		if(left_right)
			SetPoints(x,y,x+_sx-1,y+_sy-1);
		else
			SetPoints(x+_sx-1,y,x,y+_sy-1);
		//SetSize(_sx,_sy);
	}else if(!up_down)
	{
		if(left_right)
			SetPoints(x,y+_sy-1,x+_sx-1,y);
		else
			SetPoints(x+_sx-1,y+_sy-1,x,y);
		//SetSize(_sx,_sy);
	}
}


void CGUILine::Move(float _x, float _y)
{
	if(line)
		line->Move(_x-x,_y-y);
	SetPos(_x,_y);
}

void CGUILine::SetVisible(int vis)
{
	if(line)
		line->SetVisible(vis);
	SetVisibility(vis);
}

void CGUILine::SetColor(UINT color)
{
	if(line)
		line->SetCol(color);
}

void CGUILine::SetPoints(float x1, float y1, float x2, float y2)
{
	if(line)
		line->SetPoints(x1,y1,x2,y2);
	if(y1<y2)
		up_down = true;
	else
		up_down = false;
	if(x1<x2)
		left_right = true;
	else
		left_right = false;

	SetSize(fabsf(x2-x1)+1,fabsf(y2-y1)+1);
	SetPos((x1<x2)? x1 : x2, (y1<y2)? y1 :y2);
}

//////////////////////////////////////////////////////////////////////
// CGUITextButton
//////////////////////////////////////////////////////////////////////

const int CGUITextButton::numEvents = 1;
const int CGUITextButton::availableEvents[numEvents] = {EClicked};

CGUITextButton::CGUITextButton(char *_text, CGUIFont* _font, float _x, float _y, int _color, float maxx, float maxy, EGUIAlignment align, int maxLines, float LinesSpaces)
: CGUIStaticText(_text,_font,_x,_y,_color,maxx,maxy,align,maxLines,linesSpaces), CGUIEventSupport(availableEvents, numEvents)
{
	throw_going=false;
	focusable=true;
	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	color=_color;
	markColor=color;

	psx=psy=0;
}


int CGUITextButton::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	SetColor(markColor);
	if(mouseState)
	{
		EventArise(EClicked);
		OnClick();
	}
	else
	{

	}

	return 0;
}

int CGUITextButton::MouseOver(float x, float y, UINT over, UINT state)
{
	SetColor(markColor);
	return 0;
}

int CGUITextButton::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
		
	SetColor(color);
	return 0;
}

int CGUITextButton::MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID)
{
	SetColor(color);
	return 0;
}

int CGUITextButton::FocusChanged(UINT focus, typeID dualID)
{
	if(!focus)
	{
		SetColor(color);
	}

	return 0;
}

int CGUITextButton::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	SetColor(color);
	return 0;
}

void CGUITextButton::SetPosSize(float _x, float _y, float _sx, float _sy)
{
	psx=x-_x;
	psy=y-_y;
	SetPos(_x,_y);
	SetSize(_sx,_sy);
}


void CGUITextButton::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	if(text)
		text->Move(_x-x+psx,_y-y+psy);

	if(texts){
		for(int i=0;i<numLines;i++)
		{
			texts[i]->Move(_x-x+psx,_y-y+psy);
		}
	}

	SetPos(_x,_y);
}






