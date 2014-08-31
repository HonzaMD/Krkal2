//////////////////////////////////////////////////////////////////////
//
// DXhighlight.cpp
//
// vyhighlightuje text
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXHighLight.h"

#include "refmgr.h"
#include "dxfont.h"

#include "comcompiler.h"

#define HL_NUM 7

#define HL_DEFAULT 0
#define HL_KEYWORD 1
#define HL_STRING 2
#define HL_COMMENT 3
#define HL_KN 4
#define HL_EDIT 5
#define HL_DIRECTIVE 6

CDXHighLightFormat *DXHighLightFormat=NULL;

#define HLF DXHighLightFormat

CDXHighLightFormat::CDXHighLightFormat()
{
	fonts = new CDXFont*[HL_NUM];
	colors = new DWORD[HL_NUM];
	
	for(int i=0;i<HL_NUM;i++)
		fonts[i]=NULL;
}

CDXHighLightFormat::~CDXHighLightFormat()
{
	if(fonts)
	{
		for(int i=0;i<HL_NUM;i++)
			SAFE_RELEASE(fonts[i]);
		delete[] fonts;
	}
	SAFE_DELETE_ARRAY(colors);
}

int CDXHighLightFormat::SetHL(int type,char *fontname, DWORD color)
{
	CDXFont *font = (CDXFont*)RefMgr->Find(fontname);
	SAFE_RELEASE(fonts[type]);
	font->AddRef();
	fonts[type]=font;
	colors[type]=color;
	return 1;
}


int CDXHighLightFormat::SetAllToDefault()
{
	SetDefault();
	SetKeyword();
	SetString();
	SetComment();
	SetKnownNames();
	SetEditTag();
	SetDirective();
	return 1;
}

int CDXHighLightFormat::SetDefault(char *fontname, DWORD color)
{
	return SetHL(HL_DEFAULT,fontname,color);
}

int CDXHighLightFormat::SetKeyword(char *fontname, DWORD color)
{
	return SetHL(HL_KEYWORD,fontname,color);
}

int CDXHighLightFormat::SetString(char *fontname, DWORD color)
{
	return SetHL(HL_STRING,fontname,color);
}

int CDXHighLightFormat::SetComment(char *fontname, DWORD color)
{
	return SetHL(HL_COMMENT,fontname,color);
}

int CDXHighLightFormat::SetKnownNames(char *fontname, DWORD color)
{
	return SetHL(HL_KN,fontname,color);
}
int CDXHighLightFormat::SetEditTag(char *fontname, DWORD color)
{
	return SetHL(HL_EDIT,fontname,color);
}
int CDXHighLightFormat::SetDirective(char *fontname, DWORD color)
{
	return SetHL(HL_DIRECTIVE,fontname,color);
}

void CDXHighLightFormat::GetDefault(CDXFont *&font, DWORD &color)
{
	font = fonts[HL_DEFAULT];
	color = colors[HL_DEFAULT];
}

void CDXHighLightFormat::GetKeyword(CDXFont *&font, DWORD &color)
{
	font = fonts[HL_KEYWORD];
	color = colors[HL_KEYWORD];
}
void CDXHighLightFormat::GetString(CDXFont *&font, DWORD &color)
{
	font = fonts[HL_STRING];
	color = colors[HL_STRING];
}
void CDXHighLightFormat::GetComment(CDXFont *&font, DWORD &color)
{
	font = fonts[HL_COMMENT];
	color = colors[HL_COMMENT];
}
void CDXHighLightFormat::GetKnownNames(CDXFont *&font, DWORD &color)
{
	font = fonts[HL_KN];
	color = colors[HL_KN];
}
void CDXHighLightFormat::GetEditTag(CDXFont *&font, DWORD &color)
{
	font = fonts[HL_EDIT];
	color = colors[HL_EDIT];
}
void CDXHighLightFormat::GetDirective(CDXFont *&font, DWORD &color)
{
	font = fonts[HL_DIRECTIVE];
	color = colors[HL_DIRECTIVE];
}

////////////////////////////////////////////////////////////////////////////////////


CDXHighlighter::CDXHighlighter(CFormatedText *txt)
{
	text=txt;

}

CDXHighlighter::~CDXHighlighter()
{
}

//highlightuje cely text
int CDXHighlighter::Highlight()
{
	int comment=0;
	CTextBlock *b=text->GetFirstBlock();
	while(b->text)
	{
		HighlightBlock(b,comment);
		b->WordWrap(text,text->GetWidth());
		b->CmpBlockSize(text,0);
		b=b->next;
	}
	text->GetFirstBlock()->CmpYStarts();
	return 1;
}

//higlightuje od zadaneho bloku
int CDXHighlighter::HighlightFromBlock(CTextBlock *b)
{
	int comment=0;
	CTextBlock *bl=NULL;
	if(b->prev->text)
		comment=b->prev->blockcomment;

	int cchng=1;

	bl=b;
	
	while(b->text)
	{
		if(!cchng&&b->blockcomment!=-1) break;
		cchng = HighlightBlock(b,comment);
		b->WordWrap(text,text->GetWidth());
		if(b->CmpBlockSize(text,0)&&!bl)
			bl=b;
		b=b->next;
	}
	if(bl) bl->CmpYStarts();
	return 1;
}

//highlightuje blok
//comment je uroven zanoreni ve viceradkovych komentarich
int CDXHighlighter::HighlightBlock(CTextBlock *b, int &comment)
{


	CDXFont **fonts;
	DWORD *colors;

	bool hl;
	
	CTextFormat *df=text->GetDefaultFormat();

	fonts=HLF->GetFonts();
	colors=HLF->GetColors();

	char *c=b->text,cc;
	int wspos=0;

	int ln=0;
	int ff=-1,fcp=0;
	int pos=0;
	int ef=0;

	int bs=0;
	int uv=0,ap=0;

	int hgl=0,hltype;
	int wasbackslash=0,wasslash=0,wasmul=0;
	int inlinecomment=0;
	int endofcom=0;

	b->ResetFormat();

	while(1)
	{
		hgl=0;
		
		if(inlinecomment){
			if(*c==0){
				inlinecomment=0;
				hgl=HL_COMMENT;
			}else
				ln++;
		}else
		if(comment){
			if(*c==0){
				hgl=HL_COMMENT;
			}else
			{
				ln++;
				if(*c=='*') wasmul=1; else 
				{					
					if(*c=='/' && wasmul)
					{						
						comment--;
						if(comment==0)
							hgl=HL_COMMENT;
						endofcom=1;
					}
					wasmul=0;
				}
			}				
		}else
		if(uv)
		{			
			ln++;
			if((*c=='"' && !wasbackslash) || *c==0) {
				uv=0;hgl=HL_STRING;
				if(*c==0) ln--;
			}
        }else
		if(ap)
		{
			ln++;
			if((*c=='\'' && !wasbackslash) || *c==0) {
				ap=0;hgl=HL_STRING;
				if(*c==0) ln--;
			}
		}else
		if(*c == '"' )
		{			
			uv=1;
			wspos=pos;fcp=pos-ef;
			ln=1;
		}else
		if(*c =='\'' )
		{
			ap=1;
			wspos=pos;fcp=pos-ef;
			ln=1;
		}else
		if(!((isalpha((UC)*c) || *c=='@' || *c=='#' || *c=='_' || isdigit((UC)*c) )))
		{
			if(ln){
				cc=*c;
				*c = 0;
				hl = compiler->IsHighlight(&b->text[wspos],&hltype);
				if(hl) {
					switch(hltype)
					{
					case pteKeyword: // klicove slovo
						hgl=HL_KEYWORD;
						break;
					case pteDirective: // direktiva preprocesoru - napriklad #ifdef
						hgl=HL_DIRECTIVE;
						break;

					case pteKnownName: // zname jmeno
					case pteKernelService: // sluzba kernelu
					case pteKernelConstant: // konstanta kernelu
						hgl=HL_KN;
						break;

					case pteEditTag: // edit tag
						hgl=HL_EDIT;
						break;
					
					}					
				}
				else ln=0;
				*c = cc;
			}
		}else 
		{
			if(!ln) {wspos=pos;fcp=pos-ef;}
			ln++;
		}

		if(hgl)
		{
			if(ln)
			{
				b->FormatChars(ff,fcp,ln,fonts[hgl],colors[hgl],df);
			}
			ff=b->formatcnt-1;
			ef=wspos+ln;
			ln=0;
			
		}

		if(*c==0) 
			break;
	
		if(*c=='\\' && !wasbackslash)
		{
			wasbackslash=1;
		}else
			wasbackslash=0;

		if(!inlinecomment)
		{
			if(*c=='*' && wasslash){
                comment++;
				if(comment==1)
				{
					wspos=pos-1;fcp=pos-1-ef;
					ln=2;
				}
			}				
		}

		if(*c=='/'  && !endofcom &&!inlinecomment)
		{
			if(!wasslash)
			{
				wasslash=1;
			}else{
				if(!uv && !ap)
				{
					if(!comment){
						wspos=pos-1;fcp=pos-1-ef;
						ln=2;
					}
					inlinecomment=1;					
				}
				wasslash=0;
			}
		}else wasslash=0;

		endofcom=0;

		pos++;
		c++;
	}

	if(b->blockcomment!=comment)
	{
		b->blockcomment=comment;
		return 1;
	}

	return 0;
}