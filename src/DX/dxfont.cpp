//////////////////////////////////////////////////////////////////////
//
// DXfont.cpp
//
// font vyrendrovany do textury
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "dxfont.h"


CDXFont::CDXFont( HFONT font )
{
	CreateFont(font);
}

CDXFont::CDXFont( char *fontname, int height, int weight, int italic, int underline)
{
	
	HDC dc = CreateCompatibleDC(0);
	SetMapMode(dc,MM_TEXT);
	height = -MulDiv(height, GetDeviceCaps(dc, LOGPIXELSY), 72);
	DeleteDC(dc);

	HFONT font = ::CreateFont(
	height,                    // nHeight
	0,                         // nWidth
	0,                         // nEscapement
	0,                         // nOrientation
	weight,                    // nWeight
	italic,                    // bItalic
	underline,                 // bUnderline
	0,                         // cStrikeOut
	DEFAULT_CHARSET,              // nCharSet
	OUT_DEFAULT_PRECIS,        // nOutPrecision
	CLIP_DEFAULT_PRECIS,       // nClipPrecision
	DEFAULT_QUALITY,           // nQuality
	VARIABLE_PITCH,  // nPitchAndFamily
	fontname);                 // lpszFacename

	CreateFont(font);

	DeleteObject(font);
}

int CDXFont::CreateFont(HFONT font)
{

	HDC hdc = CreateCompatibleDC( NULL );
	HGDIOBJ old;

	SetMapMode( hdc, MM_TEXT );

	old = SelectObject(hdc,font);

	GetCharABCWidths(hdc,0,255,charABC);
	for(int i=0;i<256;i++)
		charW[i] = charABC[i].abcA + charABC[i].abcB + charABC[i].abcC;

	GetTextMetrics(hdc,&metric);

	if(metric.tmHeight>=35)
		texwidth = texheight = 1024;
	else
	if(metric.tmHeight>=15)
		texwidth = texheight = 512;
	else
		texwidth = texheight = 256;

	texaddx = 1.f/texwidth;
	texaddy = 1.f/texheight;

	LPDIRECT3DDEVICE8 d3ddev =  DXbliter->GetD3DDevice();

	HRESULT hr;

    hr = d3ddev->CreateTexture( texwidth, texheight, 1,
                                      0, D3DFMT_A4R4G4B4,
                                      D3DPOOL_MANAGED, &tex );

    // Prepare to create a bitmap
    DWORD*      pBitmapBits;
    BITMAPINFO bmi;
    ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  (int)texwidth;
    bmi.bmiHeader.biHeight      = -(int)texheight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;

    // create bitmap for the font
    HBITMAP hbmBitmap = CreateDIBSection( hdc, &bmi, DIB_RGB_COLORS,
                                          (VOID**)&pBitmapBits, NULL, 0 );

	SelectObject( hdc, hbmBitmap );

    SetTextColor( hdc, RGB(255,255,255) );
    SetBkColor(   hdc, 0x00000000 );
    SetTextAlign( hdc, TA_TOP );


    // Loop through all printable character and output them to the bitmap..
    // compute tex coords for each character
    int x = 0;
    int y = 0;
	int w;

	char str;

    for( int c=32; c<256; c++ )
    {
		str = c;
		w = charABC[c].abcB;

		if(charABC[c].abcA<0) x+=-charABC[c].abcA;

		if( x+w+1 >= texwidth )
		{
			x=0;
			y+=metric.tmHeight+1;
		}
      
        ExtTextOut( hdc, x, y, ETO_OPAQUE, NULL, &str, 1, NULL );

		x +=  charABC[c].abcA;

        texcoord[c][0] = ((FLOAT)x)/texwidth;
        texcoord[c][1] = ((FLOAT)y)/texheight;
        texcoord[c][2] = ((FLOAT)x+w+1)/texwidth;
        texcoord[c][3] = ((FLOAT)y+metric.tmHeight+1)/texheight;

		x += w+2;
		if(charABC[c].abcC<0) x+=-charABC[c].abcC;

    }

    // Lock the surface and write the alpha values for the set pixels
    D3DLOCKED_RECT d3dlr;
    tex->LockRect( 0, &d3dlr, 0, 0 );
    WORD* pDst16 = (WORD*)d3dlr.pBits;
    BYTE bAlpha; // 4-bit measure of pixel intensity

    for( y=0; y < texheight; y++ )
    {
        for( x=0; x < texwidth; x++ )
        {
            bAlpha = (BYTE)((pBitmapBits[texwidth*y + x] & 0xff) >> 4);
            if (bAlpha > 0)
            {
                *pDst16++ = (bAlpha << 12) | 0x0fff;
            }
            else
            {
                *pDst16++ = 0x0000;
            }
        }
    }

    // Done updating texture, so clean up used objects
    tex->UnlockRect(0);
    DeleteObject( hbmBitmap );
	SelectObject(hdc,old);
	DeleteDC(hdc);


	vertices = NULL;

	return 1;
}

