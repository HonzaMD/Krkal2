/////////////////////////////////////////////////////////////////////////////
//
// KRKAL.cpp
//
// KRKAL main
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "krkal.h"

#include "resource.h"

#include "dx.h"
#include "fs.h"
#include "refmgr.h"

#include "gui.h"

#include "version.h"
#include "ks.h"
#include "KerErrors.h"
#include "md_testy.h"
#include "comcompiler.h"
#include "dxhighlight.h"

#include "ident.h"

#include "music.h"

#include "timer.h"

//#include "mainmenudesktop.h"
#include "MMDesktop.h"
#include "levels.h"
#include "editor.h"


#define DEFAULTSCRIPT "krkal2_A1CF_6963_8DA6_D822.kc"
//#define CREATE_TEX


#define WMessageBox SetFullscreen(0);MessageBox

DWORD WINAPI RenderFunction( LPVOID lpParam ) 
{ 
	while(1){
		if(WaitForSingleObject(KRKAL->RenderEvent,INFINITE)==WAIT_OBJECT_0)
		{
		//	OutputDebugString("DXDevice Prersent\n");
			KRKAL->DXDevice->Present( NULL, NULL, NULL, NULL );	
			SetEvent(KRKAL->RenderReadyEvent);
		}
	}
	return 0;
} 


CKrkal *KRKAL=NULL;

CKrkal::CKrkal()
{
	DXapp=NULL;
	gui=NULL;
	GEnMain=NULL;
	timer=NULL;
	compiler=NULL; ::compiler=NULL;
	highlightformat=NULL;
	RefMgr=NULL;


	quit=0;
	
	isrunning=0;
	
	KrkalMutex=0;
	
	RenderThread=0;
	RenderEvent=0;
	RenderReadyEvent=0;

}


void CKrkal::DeleteTextures()
{
	while (!texturesToDelete.IsEmpty()) 
	{
		texturesToDelete.RemoveHead()->Release();
	}
}

CKrkal::~CKrkal()
{

	MMProfiles->Save();

	SAFE_DELETE(MusicEngine);

	if(DXapp){
		SendMessage( DXapp->m_hWnd, WM_CLOSE, 0, 0 );
	}


	SAFE_DELETE(KerMain);
	SAFE_DELETE(compiler);
	::compiler=NULL;

	SAFE_DELETE(gui);
	SAFE_DELETE(highlightformat);
	DXHighLightFormat=NULL;

	SAFE_DELETE(GEnMain);

	SAFE_DELETE(KerErrorFiles);
	SAFE_DELETE(KSMain);
	

	SAFE_DELETE(MMProfiles);
	SAFE_DELETE(MMLevelDirs);
	DeleteTextures();

	CFS::DoneFS(); //deinitace FS

	SAFE_DELETE(timer);
	SAFE_DELETE(DXapp);

	SAFE_DELETE(RefMgr);
	DeleteTextures();

	if(KrkalMutex) CloseHandle(KrkalMutex);
	if(RenderThread) CloseHandle(RenderThread);
	if(RenderEvent) CloseHandle(RenderEvent);
	if(RenderReadyEvent) CloseHandle(RenderReadyEvent);

}

void CKrkal::Quit()
{
	WaitForSingleObject(KRKAL->RenderReadyEvent,INFINITE);

	if(DXapp)
	{
		PostMessage(DXapp->m_hWnd,  WM_CLOSE,0,0);
	}else
		PostQuitMessage(1);

}

int CKrkal::Init(HINSTANCE hInstance)
{

	//zjistim, jestli KRKAL uz nebezi
	KrkalMutex = CreateMutex(NULL,TRUE,"KRKALmutex");
	
	DWORD err=GetLastError();

	if(err!=ERROR_SUCCESS)
	{
		if(err==ERROR_ALREADY_EXISTS)
			MessageBox( NULL,"You can start KRKAL only once!", "KRKAL", MB_ICONERROR|MB_OK );
		else
			MessageBox( NULL,"Error: Can't create KRKAL mutex!", "KRKAL", MB_ICONERROR|MB_OK );

		return 0;
	}
	
	//je to OK - zatim zadnej nebezi

	isrunning=0;
	paused = 0;

	CFS::InitFS(); //initace FileSystemu
	FS->AddFSDir("GAME","$GAMES$/Krkal_4F88_78B7_A01C_48AB",1);

	cfg.ReadCfg("$DATA$/default.cfg");
	if(!cfg.ReadCfg())
	{
		MessageBox( NULL,"Error in krkal.cfg!", "KRKAL", MB_ICONERROR|MB_OK );
		return 0;
	}

	//otestuju jestli jde zapisovat
	char *writetest="WRITETEST";
	if(!FS->WriteFile("$DATA$/writetest",writetest,10,0))
	{
		MessageBox( NULL, "Error! Can't write to disk! (readonly?, access rights?)","KRKAL",MB_ICONERROR|MB_OK );
		return 0;
	}else{
		FS->Delete("$DATA$/writetest");
	}
	if(FS->GetFileSize("$DATA$/krkal.dat/test.wr")==0)
	{
		MessageBox( NULL, "Error! Can't write to disk! (readonly?, access rights?)","KRKAL",MB_ICONERROR|MB_OK );
		return 0;
	}

	FS->WriteFile("$KRKAL$/version",KRKAL_NAME_AND_VERSION_STRING,strlen(KRKAL_NAME_AND_VERSION_STRING),0);


	#ifdef CREATE_TEX
	FS->Delete("$DATA$/tex");
	FS->CreateArchive("$DATA$/tex");
	FS->CopyTree("$KRKAL$/tex","$DATA$/tex",0);
	FS->Defragment("$DATA$/tex");
	#endif

	// create level & profile objects
	MMProfiles = new CMMProfiles;
	MMLevelDirs = new CMMLevelDirs;


	DXapp=new CDXapp;
	
	if( FAILED( DXapp->Create( hInstance ) ) )
		return 0;

	DXDevice=DXapp->m_pd3dDevice;

	RenderEvent = CreateEvent(NULL,FALSE,FALSE,"KRKALrenderEvent");
	if(!RenderEvent){
		MessageBox( NULL,"Error: Can't create KRKAL Render Event!", "KRKAL", MB_ICONERROR|MB_OK );
		return 0;
	}

	RenderReadyEvent = CreateEvent(NULL,FALSE,TRUE,"KRKALrenderReadyEvent");
	if(!RenderReadyEvent){
		MessageBox( NULL,"Error: Can't create KRKAL RenderReady Event!", "KRKAL", MB_ICONERROR|MB_OK );
		return 0;
	}

	DWORD dwThreadId;
	RenderThread=CreateThread( NULL, 0, RenderFunction, 0, 0, &dwThreadId);

	if(!RenderThread)
	{
		MessageBox( NULL,"Can't create reder thread!", "KRKAL", MB_ICONERROR|MB_OK );
		return 0;
	}

	RefMgr = new CRefMgr();

	KSMain = new CKSMain(); // Je treba volat pred Kernelem
	new CKerErrorFiles(); // Je treba volat pred Kernelem

	compiler = new CComCompiler();
	::compiler=compiler;

	CBltWindow* GameWnd;

	ResetGameVictory();

	try{

		new CMusicEngine;

		MDTcreate_files();

		timer = new CTimer();
		tps=timer->GetTicksPerSec();
		tpms=tps/1000;

		GameWnd=new CBltWindow(50,50,540,380);
		GameWnd->CreateCache(32,32,-20,-20,660,500);
		GameWnd->SetGameWindow();
		
		GEnMain=new CGEnMain(GameWnd);				

		gui=new CGUI(DXbliter->rootwnd, DXapp->input);

		highlightformat = new CDXHighLightFormat();
		highlightformat->SetAllToDefault();
		DXHighLightFormat = highlightformat;

		isrunning=1;

		kernelTime=0;
		kernelRealTimeT=timer->GetAbsoluteTimeTicks();
	}
	catch (CKernelPanic){
		SAFE_DELETE(KerMain);
		if(MMDesktop) MMDesktop->KernelPanic();
		if(editor) editor->KernelShutDown();
		else{
			if(KerErrorFiles)
				if(KerErrorFiles->WasError) KerErrorFiles->ShowAllLogs(0,0,640,480);
		}

	}
	catch (CExc e){
		MessageBox( NULL, e.errstr, DXapp->m_strWindowTitle, MB_ICONERROR|MB_OK );
		return 0;
	}
	catch (std::bad_alloc&) {
		MessageBox( NULL, "Error: Not enough memory!", DXapp->m_strWindowTitle, MB_ICONERROR|MB_OK );
		return 0;
	}
	catch (...) {
		MessageBox( NULL, "Error: Caught unknown exception!", "KRKAL", MB_ICONERROR|MB_OK );
		throw;
		return 0;
	}



	SetMinTime(17);	SetMaxTime(17);
	SetTimeOuts(300,600);

	RenderTimeT=0;

	nextRT=0;
	for(int i=0;i<numRT;i++) RenderTimes[i]=0;

	ShowCursor(0);

	return 1;
}


void CKrkal::SetMinTime(UI time)
{
	MinTime=time;
	MinTimeT=(LONGLONG)(MinTime * (timer->GetTicksPerSec() / 1000));
}
void CKrkal::SetMaxTime(UI time)
{
	MaxTime=time;
	MaxTimeT=(LONGLONG)(MaxTime * (timer->GetTicksPerSec() / 1000));
}
void CKrkal::SetTimeOuts(UI t1, UI t2)
{
	TimeOut1=t1;
	TimeOut2=t2;
	TimeOut1T = (LONGLONG)(TimeOut1 * (timer->GetTicksPerSec() / 1000));
	TimeOut2T = (LONGLONG)(TimeOut2 * (timer->GetTicksPerSec() / 1000));
}

int CKrkal::Run()
{
	if(!DXapp) return 0;


	LastKernelRunTimeT = timer->GetAbsoluteTimeTicks();

	// Load keyboard accelerators
    HACCEL hAccel = LoadAccelerators( NULL, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

	// Now we're ready to recieve and process Windows messages.
    BOOL bGotMsg;
    MSG  msg;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message && !quit )
    {

		// Use PeekMessage() if the app is active, so we can use idle time to
        // render the scene. Else, use GetMessage() to avoid eating CPU time.
        if( DXapp->m_bActive )
            bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        else
            bGotMsg = GetMessage( &msg, NULL, 0U, 0U );
			


		if(DXapp->input) DXapp->input->GetKeyState();

		DWORD dwResult = WaitForSingleObject(DXapp->input->GetMouseEvent(), 0); 
		if (dwResult == WAIT_OBJECT_0) 
		{ 
			DIMOUSESTATE2 *ms=NULL;				
			if(DXapp->input->GetMouseState(&ms))
			{
				if(gui) gui->MouseRelativeMove(ms->lX,ms->lY);
			}
		}


        if( bGotMsg )
        {
            // Translate and dispatch the message
            if( 0 == TranslateAccelerator( DXapp->m_hWnd, hAccel, &msg ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            // Render a frame during idle time (no messages are waiting)
            if( isrunning && DXapp->m_bActive && DXapp->m_bReady )
            {
				try{

					if(!MakeTurn()){
						Stop();
						SendMessage( DXapp->m_hWnd, WM_CLOSE, 0, 0 );
					}
									
				}
				catch (CKernelPanic){
					SAFE_DELETE(KerMain);
					if(MMDesktop) MMDesktop->KernelPanic();
					if(editor) editor->KernelShutDown();
					else{
						if(KerErrorFiles)
							if(KerErrorFiles->WasError) KerErrorFiles->ShowAllLogs(0,0,640,480);
					}
				}
				catch (CExc e) {
					WMessageBox( NULL, e.errstr, "KRKAL", MB_ICONERROR|MB_OK );
					Stop();
					SendMessage( DXapp->m_hWnd, WM_CLOSE, 0, 0 );
				}
				catch (std::bad_alloc&) {
					WMessageBox( NULL, "Error: calling new", "KRKAL", MB_ICONERROR|MB_OK );
					Stop();
					SendMessage( DXapp->m_hWnd, WM_CLOSE, 0, 0 );
				}
				catch (...) {
					WMessageBox( NULL, "Error: Caught unknown exception!", "KRKAL", MB_ICONERROR|MB_OK );
					Stop();
					SendMessage( DXapp->m_hWnd, WM_CLOSE, 0, 0 );
					throw;
				}

            }
        }
    }

    return (int)msg.wParam;

}

void CKrkal::DeleteKeyBoardMessages()
{
	MSG msg;

	while( PeekMessage(&msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE) )
		;

}

LRESULT CKrkal::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam )
{

	if(isrunning) 
	{

		try{

			switch( uMsg )
			{
				case WM_MOUSEMOVE:

					if(gui) gui->MouseMove(LOWORD(lParam),HIWORD(lParam),wParam);
					
					break;

				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP:

					if(gui) gui->MouseBut(uMsg,LOWORD(lParam),HIWORD(lParam),wParam);

					break;

				case WM_MOUSEWHEEL:

					POINT p;
					p.x=LOWORD(lParam);
					p.y=HIWORD(lParam);
					ScreenToClient(DXapp->m_hWnd,&p);

					if(gui) gui->MouseWheel(GET_WHEEL_DELTA_WPARAM(wParam),p.x,p.y,LOWORD(wParam));

					break;

				case WM_KEYDOWN:

					/*switch(wParam)
					{
					case VK_LEFT:
					case VK_RIGHT:
					case VK_UP:
					case VK_DOWN:
					case VK_HOME:
					case VK_END:
					case VK_INSERT:
					case VK_DELETE:
					*/
						if(gui) gui->SendKey(wParam,lParam);
					//}
					
					break;

				case WM_KEYUP:

					if(gui) gui->SendKeyUp(wParam,lParam);

					break;

				case WM_CHAR:

					if(gui) gui->SendChar(wParam,lParam);

					break;				
			}		
		}
		catch (CKernelPanic){
			SAFE_DELETE(KerMain);
			if(MMDesktop) MMDesktop->KernelPanic();
			if(editor) editor->KernelShutDown();
			else{
				if(KerErrorFiles)
					if(KerErrorFiles->WasError) KerErrorFiles->ShowAllLogs(0,0,640,480);
			}
		}
		catch (CExc e) {
			WMessageBox( NULL, e.errstr, "KRKAL", MB_ICONERROR|MB_OK );
			Stop();
			SendMessage( hWnd, WM_CLOSE, 0, 0 );
		}
		catch (std::bad_alloc&) {
			WMessageBox( NULL, "Error: calling new", "KRKAL", MB_ICONERROR|MB_OK );
			Stop();
			SendMessage( hWnd, WM_CLOSE, 0, 0 );
		}
		catch (...) {
			WMessageBox( NULL, "Error: Caught unknown exception!", "KRKAL", MB_ICONERROR|MB_OK );
			Stop();
			SendMessage( DXapp->m_hWnd, WM_CLOSE, 0, 0 );
			throw;
		}
	}
	return DXapp->MsgProc(hWnd,uMsg,wParam,lParam);
}


int CKrkal::MakeTurn()
{

	LONGLONG tm,tm2;
	LONGLONG diff;
	LONGLONG to;
	UI df;

	UI ttt;
	
	int hurry=0;

	tm=timer->GetAbsoluteTimeTicks();

	diff=tm-LastKernelRunTimeT;

	if(diff>=MinTimeT)
	{
		
		rendflag=0;


		if(diff>MaxTimeT + 2*tpms )
		{
			//nestiham

			hurry=1;

			to = diff-MaxTimeT;

			df=MaxTime;

			if(to<=TimeOut1T)
			{
				//snazim se to dohonit

				ttt=(UI)(to*1000/tps);
				DebugMessage(3,0xffff0000,"@ %i",ttt);

//				if(ttt>5) rendflag=1;
				rendflag=1;

				if(!catching)
				{
					catching = 1;
					CatchSinceT = tm;
					LastKernelRunTimeT += MaxTimeT;
				}else{

					if(tm - CatchSinceT > TimeOut2T)
					{
						//uz se to snazim dohnat moc dlouho - kaslu na to
						DebugMessage(3,0xffff0000,"!@!@!");
						LastKernelRunTimeT = tm;
						catching=0;

					}else{
						LastKernelRunTimeT += MaxTimeT;
					}

				}

			}else{
				//nestih jsem to o hodne - kaslu na to
				DebugMessage(3,0xffff0000,"!!!!!");
				LastKernelRunTimeT = tm;
			}

		}else
		{
			catching=0;
			DebugMessage(3,0xffff0000,"");
			if(diff>MaxTimeT)
				df=MaxTime;
			else
				df=(UI)(diff/tpms);
			LastKernelRunTimeT = tm;
		}


		kernelRealTimeT=tm;

		if(!paused)
		{
			kernelTime+=df;
			GEnMain->SetKernelTime(kernelTime);

			if(KerMain)
			{
				try{
					KerMain->RunTurn(df,hurry);
				}
				catch (CKernelPanic){
					SAFE_DELETE(KerMain);
					if(MMDesktop) MMDesktop->KernelPanic();
					if(editor) editor->KernelShutDown();
					else{
						if(KerErrorFiles)
							if(KerErrorFiles->WasError) KerErrorFiles->ShowAllLogs(0,0,640,480);
					}
				}

			}
		}

		float fdf= df/1000.f;
		gui->Timer(fdf);
		if(MusicEngine)
			MusicEngine->Timer(fdf);
				
	}	
	
	tm=timer->GetAbsoluteTimeTicks();
	diff=tm-LastKernelRunTimeT;

	if(  (rendflag==0&&diff<MinTimeT) || (diff + RenderTimeT < MinTimeT && tm-LastRenderTimeT>=RenderTimeT-100) )
	{
		if(WaitForSingleObject(KRKAL->RenderReadyEvent,0)==WAIT_OBJECT_0)
		{
			DeleteTextures();

            if(!paused) GEnMain->Update(kernelTime + (UI)((tm-kernelRealTimeT)/tpms));
			DXbliter->Update();

			if( FAILED( DXapp->Render() ) ) 
				return 0;

			LastRenderTimeT = tm;

			tm2=timer->GetAbsoluteTimeTicks();

			tm2 = (LONGLONG)((tm2-tm));
			
			RenderTimes[nextRT] = tm2;
			nextRT++;
			if(nextRT>=numRT) nextRT=0;

			int mini=0;
			LONGLONG minRT;
			minRT=RenderTimes[0];
			for(int i=1;i<numRT;i++)
			{
				if(RenderTimes[i]<minRT) {minRT=RenderTimes[i];mini=i;}
			}

			RenderTimeT=0;
			for(int i=0;i<numRT;i++)
				if(mini!=i) RenderTimeT+=RenderTimes[i];

            RenderTimeT = RenderTimeT/(numRT-1);

			rendflag=1;

			SetEvent(RenderEvent);

		}
	}
	return 1;
}

void CKrkal::Redraw()
{
	if(WaitForSingleObject(KRKAL->RenderReadyEvent,0)==WAIT_OBJECT_0)
	{
		//OutputDebugString("REDRAW\n");

		DXbliter->Update();

		DXapp->Render();

		SetEvent(RenderEvent);


		WaitForSingleObject(KRKAL->RenderReadyEvent, INFINITE);
		SetEvent(KRKAL->RenderReadyEvent);
	}
}

void CKrkal::SetCapture()
{
	::SetCapture(DXapp->m_hWnd);
}

void CKrkal::ReleaseCapture()
{
	::ReleaseCapture();
}

void CKrkal::SetCursorPos(int x, int y)
{
	POINT pp;
	pp.x=x;
	pp.y=y;
	ClientToScreen(DXapp->m_hWnd, &pp);
	::SetCursorPos(pp.x,pp.y);
}

void CKrkal::GetScreenRes(int &sx, int &sy)
{
	g_pDXapp->GetScreenRes(sx,sy);
}

int CKrkal::SetScreenRes(int sx, int sy)
{
	if(WaitForSingleObject(KRKAL->RenderReadyEvent,INFINITE)!=WAIT_OBJECT_0)
		return 0;

	int res = g_pDXapp->ChangeScreenRes(sx,sy);

	SetEvent(RenderReadyEvent);

	return res;
}
int CKrkal::SetFullscreen(int fullscreen)
{
	if((!DXapp->m_bWindowed && !fullscreen)||
	   (DXapp->m_bWindowed && fullscreen))
	{
		SendMessage( DXapp->m_hWnd, WM_COMMAND, IDM_TOGGLEFULLSCREEN, 0 );
		return 1;
	}
	return 0;
}

void CKrkal::GameVictory() 
{
	gameVictory=1;
	if(KerMain)
		KerMain->Errors->LogError(eKRTEgameVictory);
}



const char *CKrkal::GetTranslationCoordinates(const char *str, int &langugeLen, int &textLen, const char *&text)
{
	int pos = 0;
	text = str;
	langugeLen = 0;
	const char *nextStr = 0;

	while (str[pos] != 0)
	{
		if (text == str && str[pos] == '{' && str[pos + 1] == '{') 
		{
			text = str + pos + 2;
			langugeLen = pos;
		}
		if (text != str && str[pos] == '}' && str[pos + 1] == '}') 
		{
			nextStr = str + pos + 2;
			break;
		}
		pos++;
	}

	textLen = str + pos - text;
	return nextStr;
}


char *CKrkal::CopyOutStrByLanguage(const char *str) 
{
	const char *text1=0, *text2=0, *text3=0;
	int len1 = 0, len2 = 0, len3 = 0;

	while (str && *str) 
	{
		const char* lang = str;
		const char* text;
		int langLen, textLen;
		str = GetTranslationCoordinates(str, langLen, textLen, text);

		if (langLen > 0) 
		{
			if (!text1 && strncmp(cfg.langStr, lang, langLen) == 0)
			{
				text1 = text;
				len1 = textLen;
				break;
			}
			if (!text2 && strncmp(cfg.langStr2, lang, langLen) == 0)
			{
				text2 = text;
				len2 = textLen;
			}
		}
		if (!text3)
		{
			text3 = text;
			len3 = textLen;
		}
	}

	if (!text1 && text2) 
	{
		text1 = text2;
		len1 = len2;
	}
	if (!text1 && text3)
	{
		text1 = text3;
		len1 = len3;
	}

	if (!text1)
		return 0;

	char *s;
	if (s = new char[len1 + 1]) {
		strncpy(s, text1, len1);
		s[len1] = 0;
	}

	return s;
}


////////////////////////////

CKrkalCfg::CKrkalCfg()
{
	game_resx=640;
	game_resy=480;
	game_fullscreen=0;
	editor_resx=800;
	editor_resy=600;
	editor_fullscreen=0;
	
	music=100; musicvolume=music/100.f;
	sound=100; musicvolume=sound/100.f;

	showFPS=0;
	debugmessage=0;

	defaultscript = newstrdup(DEFAULTSCRIPT);

	lang=langCS;
	lang2=langEN;
	langStr=newstrdup("cs");
	langStr2=newstrdup("en");
	DXnumbackbuff=1;
}

CKrkalCfg::~CKrkalCfg()
{
	SAFE_DELETE_ARRAY(defaultscript);
	SAFE_DELETE_ARRAY(langStr);
	SAFE_DELETE_ARRAY(langStr2);
}

int CKrkalCfg::ReadCfg(char *filename)
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
	char *name=0,*value=0;

	int err=0;
	UC tmch;

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
					name=0;value=0;
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
					if(!isspace(*ch)) break;
					tmch=*ch;
					*ch=0;
					value = newstrdup(token);
					*ch=tmch;
					mode=6;
					break;
				}

				if(*ch==13||*ch==10){
					if(mode>0)
					{
						if(mode<6) {err=1; break;}

						if(!SetAttribute(name,value))
							err=1;

						SAFE_DELETE_ARRAY(name);
						SAFE_DELETE_ARRAY(value);

					}

					name = 0;
					value = 0;
					mode = 0;
				}

			}
		}
	}

	if(err)
	{
		SAFE_DELETE(name);
		SAFE_DELETE(value);
	}else
	{
		assert(!name);
		assert(!value);
	}

	delete[] buf;

	return !err;
}

int CKrkalCfg::SetAttribute(char *name, char *value)
{
	if(!name||!value) return 0;

	int i;

	if(strcmp(name,"game_resx")==0){
		i = atoi(value);
		if(i<640) return 0;
		game_resx=i;
		return 1;
	}
	if(strcmp(name,"game_resy")==0){
		i = atoi(value);
		if(i<480) return 0;
		game_resy=i;
		return 1;
	}
	if(strcmp(name,"game_fullscreen")==0){
		game_fullscreen=atoi(value);
		return 1;
	}

	if(strcmp(name,"editor_resx")==0){
		i = atoi(value);
		if(i<640) return 0;
		editor_resx=i;
		return 1;
	}
	if(strcmp(name,"editor_resy")==0){
		i = atoi(value);
		if(i<480) return 0;
		editor_resy=i;
		return 1;
	}
	if(strcmp(name,"editor_fullscreen")==0){
		editor_fullscreen=atoi(value);
		return 1;
	}

	if(strcmp(name,"music")==0){
		music=atoi(value);
		if(music<0) music=0; else
		if(music>100) music=100;
		musicvolume = music/100.f;
		return 1;
	}
	
	if(strcmp(name,"sound")==0){
		sound=atoi(value);
		if(sound<0) sound=0; else
		if(sound>100) sound=100;
		soundvolume = sound/100.f;
		return 1;
	}

	if(strcmp(name,"showFPS")==0){
		showFPS=atoi(value);
		return 1;
	}

	if(strcmp(name,"debugmessage")==0){
		debugmessage=atoi(value);
		return 1;
	}

	if(strcmp(name,"defaultscript")==0){
		SAFE_DELETE_ARRAY(defaultscript);
		defaultscript=newstrdup(value);
		return 1;
	}

	if(strcmp(name,"lang")==0){
		if(_stricmp(value,"cs")==0) {
			SAFE_DELETE_ARRAY(langStr); langStr = newstrdup("cs");
			lang=langCS; return 1;
		}
		if(_stricmp(value,"en")==0) {
			SAFE_DELETE_ARRAY(langStr); langStr = newstrdup("en");
			lang=langEN; return 1;
		}
		return 0;
	}

	if(strcmp(name,"lang2")==0){
		if(_stricmp(value,"cs")==0) {
			SAFE_DELETE_ARRAY(langStr2); langStr2 = newstrdup("cs");
			lang2=langCS; return 1;
		}
		if(_stricmp(value,"en")==0) {
			SAFE_DELETE_ARRAY(langStr2); langStr2 = newstrdup("en");
			lang2=langEN; return 1;
		}
		return 0;
	}

	if(strcmp(name,"triplebuffer")==0){
		int tb;
		tb=atoi(value);
		if(tb) 
			DXnumbackbuff=2;
		else
			DXnumbackbuff=1;
		return 1;
	}


	return 0;
}
