//////////////////////////////////////////////////////////////////////
//
// Input.h
//
// vstup - klavesy,mys
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef INPUT_H
#define INPUT_H

#include "types.h"

struct CKeyStruct{
	CKeyStruct(char *_name, UC scankod, int sr,int sl, int cr,int cl, int ar,int al )
	{
		name = _name;
		flags = sr + (sl<<1) + (cr<<2) + (cl<<3) + (ar<<4) + (al<<5);
		key=scankod;
	}
	~CKeyStruct(){
		SAFE_DELETE_ARRAY(name);
	}

	int shift() { return flags&3; }
	int ctrl() { return (flags>>2)&3; }
	int alt() { return (flags>>4)&3; }

	char *name;
	UC key;
	UC flags;
};

class CKeyCfg
{
public:
	CKeyCfg();
	~CKeyCfg();

	int ReadCfg( char *filename = "$DATA$/keyboard.cfg" );

	int RegisterKernelKeys();

protected:
	CListK<CKeyStruct*> keylist;
};

class CInput  
{
public:
	CInput();
	virtual ~CInput();

	int Create( HWND hwnd );
	int GetKeyState();

	int KeyPressed();
    
	int IsKeyDown( int key ){return keybuffer[key]&0x80;}

	int IsKeyDown( class CKerName *keyname );
	void DisableGameKeys(int disable=1){gameKeyDisabled=disable;}
	int IsGameKeysDisabled(){return gameKeyDisabled;}

	int GetMouseState(DIMOUSESTATE2 **mousestate);

	void Acquire(){
		if(lpDIMouseDevice) lpDIMouseDevice->Acquire();
		if(lpDIKeyDevice) lpDIKeyDevice->Acquire();
	}


	HANDLE GetMouseEvent(){return MouseEvent;}

	int RegisterKernelKeys(){if(keycfg) return keycfg->RegisterKernelKeys(); else return 0;}

protected:

	int Destroy();

	unsigned char keybuffer[256];
	
	LPDIRECTINPUT8  lpDI;
	LPDIRECTINPUTDEVICE8 lpDIKeyDevice;
	LPDIRECTINPUTDEVICE8 lpDIMouseDevice;

	HANDLE MouseEvent;

	DIMOUSESTATE2 MouseState;

	CKeyCfg *keycfg;

	int gameKeyDisabled;
};

extern CInput* Input; //globalni objekt pro vstup

#endif
