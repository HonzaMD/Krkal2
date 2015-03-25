//////////////////////////////////////////////////////////////////////
//
// krkal.h
//
// KRKAL main
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////


#ifndef KRKAL_H
#define KRKAL_H

#include "types.h"

enum lang {langCS, langEN};

class CKrkalCfg
{
public:
	CKrkalCfg();
	~CKrkalCfg();

	int ReadCfg( char *filename = "$DATA$/krkal.cfg" );

	int SetAttribute(char *name, char *value);

	int game_resx, game_resy, game_fullscreen;
	int editor_resx, editor_resy, editor_fullscreen;
	
	int music,sound;
	float musicvolume,soundvolume;

	int showFPS, debugmessage;

	char *defaultscript;

	lang lang,lang2;// primarni jazyk		
	char *langStr;
//	lang lang2;		// sekundarni jazyk
	char *langStr2;

	int DXnumbackbuff;
};

class CKrkal{
public:
	CKrkal();
	~CKrkal();

	int Init(HINSTANCE hInstance);
	int Run();

	void Quit();

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void SetTimes(UI mintime, UI maxtime){SetMinTime(mintime);SetMaxTime(maxtime);}

	void SetMinTime(UI time); //min. cas na kolo v milisekundach
	void SetMaxTime(UI time); //max. cas na kolo v milisekundach

	void SetTimeOuts(UI t1, UI t2); //timeout1: max cas, kdy se jeste budu snazit dohonit zpozdeni
									//timeout2: jak dlouho se ma snazit dohonit zpozdeni

	void GetScreenRes(int &sx, int &sy); //vrati rozliseni
	int SetScreenRes(int sx, int sy); //nastavi rozliseni
	int SetFullscreen(int fullscreen=1); //prepne do/z fullscreenu

	void SetCapture();
	void ReleaseCapture();
	void SetCursorPos(int x, int y);

	void GameVictory();
	
	void ResetGameVictory(int gv=-1) {gameVictory=gv;}
	int GetGameVictory(){return gameVictory;}

	void PauseGame(){paused=1;}
	void ResumeGame(){paused=0;}
	bool IsPaused() { return paused != 0;}

	void DeleteKeyBoardMessages();
	void Redraw();

	void DeleteTextureLater(LPDIRECT3DTEXTURE8 pTexture) { texturesToDelete.Add(pTexture); }

private:

	class CDXapp *DXapp;
	class CGUI* gui;
	class CGEnMain* GEnMain;
	class CComCompiler* compiler;
	class CDXHighLightFormat* highlightformat;
	CListK<LPDIRECT3DTEXTURE8> texturesToDelete;

	int isrunning;

	void Stop(){isrunning=0;}
	
	int MakeTurn();
	void DeleteTextures();

	UI MinTime,MaxTime; //min a max cas na kolo v milisekundach
	UI TimeOut1; //max cas, kdy se jeste budu snazit dohonit zpozdeni
	UI TimeOut2; //jak dlouho se ma snazit dohonit zpozdeni

	LONGLONG LastKernelRunTimeT; //posledni cas pusteni kernelu

	LONGLONG MinTimeT,MaxTimeT;
	LONGLONG TimeOut1T,TimeOut2T;

	LONGLONG RenderTimeT; //cas na vykresleni sceny

	LONGLONG LastRenderTimeT; //cas posledniho vykresleni

#define numRT 5
	LONGLONG RenderTimes[numRT];
	int nextRT;


	int catching; //snazim se to dohonit
	LONGLONG CatchSinceT; //od tohodle casu se to snazim dohonit

	int rendflag;

	LONGLONG tps,tpms;

	class CTimer *timer;

	HANDLE KrkalMutex;

	HANDLE RenderThread;

	UI kernelTime;
	LONGLONG kernelRealTimeT;

	int quit;

	int gameVictory;
	int paused;

public:
	HANDLE RenderEvent;
	HANDLE RenderReadyEvent;
	LPDIRECT3DDEVICE8 DXDevice;

	CKrkalCfg cfg;

};

extern CKrkal *KRKAL;

#endif
