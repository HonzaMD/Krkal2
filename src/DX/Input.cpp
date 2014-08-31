//////////////////////////////////////////////////////////////////////
//
// Input.cpp
//
// vstup - klavesy,mys
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Input.h"

#include "types.h"
#include "FS.h"

#include "kernel.h"
#include "names.h"

CInput* Input=NULL;

CInput::CInput()
{
	assert(!Input);
	Input = this;
	lpDI=NULL;
	lpDIKeyDevice=NULL;
	lpDIMouseDevice=NULL;
	MouseEvent = NULL;

	keycfg = 0;
	gameKeyDisabled=0;
}

CInput::~CInput()
{
	SAFE_DELETE(keycfg);
	Destroy();
	Input=NULL;
}

int CInput::Destroy()
{
	if(lpDI)
	{
		if(lpDIKeyDevice)
		{
			lpDIKeyDevice->Unacquire(); 
            lpDIKeyDevice->Release();
            lpDIKeyDevice = NULL;
		}
		if(lpDIMouseDevice)
		{
			lpDIMouseDevice->Unacquire();
			if(MouseEvent) CloseHandle(MouseEvent);
			lpDIMouseDevice->Release();
			lpDIMouseDevice=NULL;
		}
		lpDI->Release();
		lpDI=NULL;
	}
	return 1;
}

int CInput::Create( HWND hwnd )
{

	keycfg = new CKeyCfg();
	if(!keycfg->ReadCfg())
		return -1;

	
	HRESULT hr;
 
	hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&lpDI, NULL);
	if(FAILED(hr)) return 0;

	hr = lpDI->CreateDevice(GUID_SysKeyboard,&lpDIKeyDevice,NULL);
	if(FAILED(hr)) {Destroy();return 0;}

	hr = lpDIKeyDevice->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(hr)) {Destroy();return 0;}

	hr = lpDIKeyDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
	if(FAILED(hr)) {Destroy();return 0;}
 
	hr = lpDIKeyDevice->Acquire();
	//if(FAILED(hr)) {Destroy();return 0;}



	hr = lpDI->CreateDevice(GUID_SysMouse,&lpDIMouseDevice,NULL);
	if(FAILED(hr)) {Destroy();return 0;}

	hr = lpDIMouseDevice->SetDataFormat(&c_dfDIMouse2);
	if(FAILED(hr)) {Destroy();return 0;}

	hr = lpDIMouseDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
	if(FAILED(hr)) {Destroy();return 0;}
 

	MouseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
 
	if (MouseEvent == NULL) { Destroy();return 0; }
 
	hr = lpDIMouseDevice->SetEventNotification(MouseEvent);
	if (FAILED(hr)) { Destroy(); return 0; }

	hr = lpDIMouseDevice->Acquire();
	//if(FAILED(hr)) {Destroy();return 0;}

	return 1;
}

int CInput::GetKeyState()
{
	HRESULT hr;

	if(!lpDIKeyDevice) return 0;

	hr =lpDIKeyDevice->GetDeviceState(sizeof(keybuffer),(LPVOID)&keybuffer); 
	if(FAILED(hr))
    {
		if(hr==DIERR_NOTACQUIRED||hr==DIERR_INPUTLOST)
		{
			hr = lpDIKeyDevice->Acquire();
			if(!FAILED(hr))
				hr = lpDIKeyDevice->GetDeviceState(sizeof(keybuffer),(LPVOID)&keybuffer); 
		}
	}
	if FAILED(hr) return 0;
	

	return 1;

}


int CInput::GetMouseState(DIMOUSESTATE2 **mousestate)
{
	HRESULT hr;

	if(!lpDIMouseDevice) return 0;

	hr =lpDIMouseDevice->GetDeviceState(sizeof(MouseState),(LPVOID)&MouseState); 
	if(FAILED(hr))
    {
		if(hr==DIERR_NOTACQUIRED||hr==DIERR_INPUTLOST)
		{
			hr = lpDIMouseDevice->Acquire();
			if(!FAILED(hr))
				hr =lpDIMouseDevice->GetDeviceState(sizeof(MouseState),(LPVOID)&MouseState); 
		}
	}

	if FAILED(hr) return 0;

	*mousestate = &MouseState;
	
	return 1;
}

int CInput::IsKeyDown(CKerName *keyname)
{
	if(!keyname || keyname->Type != eKerNTkey ) {
		KerMain->Errors->LogError(eKRTEKeyNameExpected);
		return 0;
	}

	if(gameKeyDisabled) return 0;

	CKeyStruct *ks=keyname->KeyStruct;

	if( ! keybuffer[ks->key] ) return 0;

	if(ks->flags==0) return 1;

	int rl;
	rl = ks->shift();
	if(rl){
		switch(rl)
		{
		case 1: //right
			if(keybuffer[DIK_RSHIFT]==0) return 0;
			break;
		case 2: //left
			if(keybuffer[DIK_LSHIFT]==0) return 0;
			break;
		case 3: // right or left
			if(keybuffer[DIK_RSHIFT]==0 && keybuffer[DIK_LSHIFT]==0) return 0;
			break;
		}
	}
	rl = ks->ctrl();
	if(rl){
		switch(rl)
		{
		case 1: //right
			if(keybuffer[DIK_RCONTROL]==0) return 0;
			break;
		case 2: //left
			if(keybuffer[DIK_LCONTROL]==0) return 0;
			break;
		case 3: // right or left
			if(keybuffer[DIK_RCONTROL]==0 && keybuffer[DIK_LCONTROL]==0) return 0;
			break;
		}
	}
	rl = ks->alt();
	if(rl){
		switch(rl)
		{
		case 1: //right
			if(keybuffer[DIK_RMENU]==0) return 0;
			break;
		case 2: //left
			if(keybuffer[DIK_LMENU]==0) return 0;
			break;
		case 3: // right or left
			if(keybuffer[DIK_RMENU]==0 && keybuffer[DIK_LMENU]==0) return 0;
			break;
		}
	}

	return 1;
}

int CInput::KeyPressed()
{
	for(int i=0;i<256;i++)
		if(keybuffer[i]) return 1;

	return 0;
}

////////////////////////////////////////

CKeyCfg::CKeyCfg()
{
}

CKeyCfg::~CKeyCfg()
{
	DeleteListElems(keylist);
}

int CKeyCfg::ReadCfg(char *filename)
{
	int sz = FS->GetFileSize(filename);

	if(sz<=0) return 0;
	sz++;

	char *buf = new char[sz];

	if(!FS->ReadFile(filename,buf,sz))
	{
		delete[] buf;
		return 0;
	}

	buf[sz-1] = 10; //pridam konec posledni radky

	int incoment = 0;
	UC *ch=(UC*)buf;
	int mode = 0;
	char *token;
	char *name=0;
	int scankod;
	int sr,sl,cr,cl,ar,al;

	int err=0;
	UC tmch;
	CKeyStruct *ks;

	for( ; sz && !err ; sz--,ch++ )
	{
        if(incoment && (*ch==13||*ch==10) ) { 
			incoment=0;
		}
		if(!incoment)
		{
			if(ch[0]=='/' && ch[1]=='/') incoment=1;
			else{
				switch(mode)
				{
				case 0:
					if(isspace(*ch)) break;
					mode = 1;
					token = (char*)ch;
					sr=sl=cr=cl=ar=al=0;
					name=0;
				case 1:
					if(isspace(*ch)||*ch=='=')
					{
						tmch=*ch;
						*ch=0;
						name = newstrdup(token);
						*ch=tmch;
						mode=2;
					}else
						break;
				case 2:
					if(isspace(*ch)) break;
					mode=3;
				case 3:
					if(*ch!='=') err=1;
					mode = 4;
					break;
				case 4:
					if(isspace(*ch)) break;
					mode = 5;
					token = (char*)ch;
				case 5:
					if(isdigit(*ch)) break;
					if(isspace(*ch)) {
						tmch=*ch;
						*ch=0;
						scankod = atoi(token);
						*ch=tmch;
						if(scankod<0||scankod>255)
						{
							err=1;break;
						}
						mode=6;
					}else
					{
						err=1;
						break;
					}
				case 6:
					if(isspace(*ch)) break;
					mode = 7;
				case 7:
					if(*ch=='s') mode=8; else
					if(*ch=='c') mode=9; else
					if(*ch=='a') mode=10; else
						err=1;
					break;
				case 8:
					if(*ch=='r') sr=1; else
					if(*ch=='l') sl=1; else
								 sr=sl=1; 
					mode=6;
					break;
				case 9:
					if(*ch=='r') cr=1; else
					if(*ch=='l') cl=1; else
								 cr=cl=1; 
					mode=6;
					break;
				case 10:
					if(*ch=='r') ar=1; else
					if(*ch=='l') al=1; else
								 ar=al=1; 
					mode=6;
					break;
				}

				if(*ch==13||*ch==10){
					if(mode>0)
					{
						if(mode<6) {err=1; break;}
						ks = new CKeyStruct(name,scankod,sr,sl,cr,cl,ar,al);
						keylist.Add(ks);
					}

					name = 0;
					mode = 0;
				}

			}
		}
	}

	if(err)
	{
		SAFE_DELETE(name);
	}else
		assert(!name);

	delete[] buf;

	return !err;
}

int CKeyCfg::RegisterKernelKeys()
{
	if(!KerMain) return 0;
	CKerNamesMain *kn = KerMain->KerNamesMain;
	if(!kn) return 0;

	CKerName *name;

	CListIterator<CKeyStruct*> it(keylist);
	while(!it.End())
	{
		name = kn->GetNamePointer(it->name);
		if(name)
		{
			if(name->Type==eKerNTvoid)
			{
				name->Type = eKerNTkey;
				name->KeyStruct = it;
			}else{
				KerMain->Errors->LogError(eKRTEbadKeyName,0,it->name);
			}
		}
		it++;
	}
	
	return 1;
}