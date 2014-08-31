//////////////////////////////////////////////////////////////////////
//
// DXfont.h
//
// font vyrendrovany do textury
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef DXFONT_H
#define DXFONT_H

#include "types.h"
#include "dxbliter.h"

struct CBltTextVertices;

class CDXFont: public CRefCnt
{
	friend class CBltFormatedText;
public:

	CDXFont( HFONT font );
	CDXFont( char *fontname, int height=12, int weight=FW_NORMAL, int italic=0, int underline=0);


	float texcoord[256][4];
	ABC charABC[256];
	int charW[256]; //A+B+C

	TEXTMETRIC metric;

	int texwidth, texheight;

	float texaddx,texaddy;

	CBltTextVertices *vertices;

protected:

	int CreateFont( HFONT font );

	LPDIRECT3DTEXTURE8 tex;


};


#endif


