//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - E R R O R S
///
///		Logovani a hlaseni behovych chyb
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "KerErrors.h"
#include "names.h"
#include "register.h"
#include "dx.h"



// Textovy popis Skupin:
char * KerEGNames[16] = {
"KerRTE Panic Error",
"KerRTE Fatal Error",
"KerRTE Error",
"KerRTE Warning",
"KerRTE Info",
"KerRTE Prm Conversion E",
"KerRTE Prm Assignation E",
"KerRTE Calling Err",
"KerRTE Debug Msg",
"KerRTE Map Error",
"KerRTE Unknown Group",
"KerRTE Unknown Group",
"KerRTE Unknown Group",
"KerRTE Unknown Group",
"KerRTE Unknown Group",
"KerRTE Unknown Group"
};


char * KerEGShortNames[16] = {
"Panic E",
"Fatal E",
"Error",
"Warning",
"Info",
"PConv E",
"PAssigE",
"Call E",
"Debug",
"Map E",
"Unknown",
"Unknown",
"Unknown",
"Unknown",
"Unknown",
"Unknown"
};



// Objekt, ktery se stara o soubory s errory a udrzuje je otevrene
// Umoznuje zobrazit Log ze souboru
CKerErrorFiles *KerErrorFiles = 0;



///////////////////////////////////////////////////////////////////////////
///
///		C K e r E r r o r S t a c k
///
///////////////////////////////////////////////////////////////////////////

// Konstuktor vytvori spojak a okopiruje do nej informace z Kontextu
CKerErrorStack::CKerErrorStack(CKerContext *ctx, int pocet) {
	MethodName = ctx->MethodName;
	line = ctx->line;
	thisO = ctx->KCthis;
	Sender = ctx->Sender;
	MName = ctx->MName;
	ThisName = KerMain->GetObjType(thisO);
	SenderName = KerMain->GetObjType(Sender);
	MsgMethodName = ctx->MsgMethodName;
	msgline = ctx->msgline;
	if (ctx->parent&&pocet<KER_MAX_CALLSTACK_LOG) parent = new CKerErrorStack(ctx->parent,pocet+1); else parent=0;
}







//////////////////////////////////////////////////////////////////
///
///		Z o b r a z e n i   L o g u
///
//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
///  Prace s Oknem
///		LOG WINDOW

CKerLogWindow::CKerLogWindow(float _x, float _y, float _sx, float _sy, char* title )
		: CGUIStdWindow(_x,_y,_sx,_sy,title) 
{
	MsgAcceptConsume(MsgMouseL);
	SetBackWindowNormalSize(10,10);
	autogrowing = true;
}

// Obsluha kliknuti do okna
int CKerLogWindow::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{
	if(!mouseState)	{
		InnerCoords(x,y);
		if(TransformCoordsFrontToBack(x,y)) {

		}
	}
	return 1;				// wow I eat this message (and it was so good)
}



CKerLogWindow::~CKerLogWindow() {
	if (KerErrorFiles) KerErrorFiles->LogWindow=0;  // Informuju KerErrorFiles, ze okno bylo zruseno
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
/// RUN LOG WINDOW

CKerRunLogWindow::CKerRunLogWindow(int _NumLogs, float _x, float _y, float _sx, float _sy, char* title )
		: CGUIStdWindow(_x,_y,_sx,_sy,title) 
{
	MsgAcceptConsume(MsgMouseL|MsgTimer);
	SetBackWindowNormalSize(10,10);
	autogrowing = true;
	NumLogs = _NumLogs;
	// Otevru si soubor s popisy erroru (pokud uz neni otevren):
	if (!KerErrorFiles->err_file) KerErrorFiles->err_file = new CFSRegister("$ERRORS$","ERRORS");
	if (!KerErrorFiles->err_file->FindKey("Ker RTE")) KerErrorFiles->err_file->AddKey("Ker RTE",FSRTregister);
	edes = KerErrorFiles->err_file->FindKey("Ker RTE")->GetSubRegister();
	TimerImpulse(0,0); // Vykreslim logy
}

// Obsluha kliknuti do okna
int CKerRunLogWindow::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{
	if(!mouseState)	{
		InnerCoords(x,y);
		if(TransformCoordsFrontToBack(x,y)) {

		}
	}
	return 1;				// wow I eat this message (and it was so good)
}



CKerRunLogWindow::~CKerRunLogWindow() {
	timerGUI->DeleteAllRequests(this);
	KerMain->Errors->LogWindow=0;  // Informuju Errors, ze okno bylo zruseno
}



/// AKTALIZACE okne. Pokud doslo ke zmene, stary obsah se smaze a vypisi se nove logy (poslednich NumLogs logu)
/// Vola se 3 krat za sekundu
int CKerRunLogWindow::TimerImpulse(typeID timerID, float time){
	int count = 0;
	CKerErrorLog *log = KerMain->Errors->end;
	CFSRegKey *k;
	int Y;
	int color;
	char str[64];

	if (log && !log->showed) { // Kreslim, jen kdyz doslo ke zmene
		log->showed = 1; // Nastavim, ze tenhle log uz jsem videl
		while (log && count<NumLogs) { log = log->prev; count++; }  // spocitam pocet kreslenych radku
		Y = 5 + count*17-17; 
		log = KerMain->Errors->end; // pisu radky od posledniho

		DeleteAllBackElems();  // smazu vnitrek okna

		// Vypisu radek:
		while(log&&count) {
			switch (KERGETERRGROUP(log->ErrorNum)) {
				case eKEGPanicError:
				case eKEGFatalError: color=0xFFFF0000; break;
				case eKEGInfo: color=0xFFFFFF00; break;
				case eKEGDebug: color=0xFF00FFFF; break;
				case eKEGWarning: color=0xFFBBBBBB; break;
				default: color=0xFFFFFFFF;
			}

			AddBackElem(new CGUIStaticText((float)5,(float)Y,35,KerEGShortNames[KERGETERRGROUP(log->ErrorNum)],ARIAL,8,false,aLeft,color));
			sprintf(str,"%i",KERGETERRSNUM(log->ErrorNum));
			AddBackElem(new CGUIStaticText((float)40,(float)Y,20,str,ARIAL,8,false,aRight,color));
			sprintf(str,"%u",log->ErrorNum); k = edes->FindKey(str);
			if (k) AddBackElem(new CGUIStaticText((float)65,(float)Y,310,k->GetDirectAccess(),ARIAL,8,false,aLeft,color));
			sprintf(str,"%i",log->ErrorParam);
			AddBackElem(new CGUIStaticText((float)380,(float)Y,20,str,ARIAL,8,false,aRight,color));
			if (log->ErrorStr) AddBackElem(new CGUIStaticText((float)405,(float)Y,80,log->ErrorStr,ARIAL,8,false,aLeft,color));
			sprintf(str,"%i",log->time);
			AddBackElem(new CGUIStaticText((float)490,(float)Y,35,str,ARIAL,8,false,aRight,color));
			if (log->stack&&log->stack->MethodName) {
				if (log->stack->MethodName[0]=='_'&&log->stack->MethodName[1]=='K'&&log->stack->MethodName[2]=='S'&&log->stack->MethodName[3]=='M'&&log->stack->MethodName[0]=='_')
					AddBackElem(new CGUIStaticText((float)530,(float)Y,90,log->stack->MethodName+5,ARIAL,8,false,aLeft,color));
				else AddBackElem(new CGUIStaticText((float)530,(float)Y,90,log->stack->MethodName,ARIAL,8,false,aLeft,color));
			}

			Y-=17;
			log = log->prev;
			count--;
		}

	}
	timerGUI->AddRequest(this,(float)0.33);  // Nastavym casovac na dalsi zavolani
	return 0;
}


/// Vytvori RUN LOG WINDOW
CKerRunLogWindow* CKerErrors::ShowRunLogWindow(int _NumLogs, float _x, float _y, float _sx, float _sy) {
	if (LogWindow) {
		desktop->DeleteBackElem(LogWindow);
	}
	LogWindow = new CKerRunLogWindow(_NumLogs,_x,_y,_sx,_sy,"Error && Debug Log");
	desktop->AddBackElem(LogWindow);
	desktop->SetFocusEl(LogWindow,0);
	return LogWindow;
}



///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////
/// Zobrazi Log Ve Wokne
void CKerErrorFiles::ShowAllLogs(float _x, float _y, float _sx, float _sy) {
	char str[32];
	int f;
	int Y=110;
	int color;
	int i;
	CFSRegister *reg;
	CFSRegKey *key1,*key2;
	int LogCount=0;

	if (LogWindow) {  // prekreslim okno kdyz existovalo
		LogWindow->DeleteAllBackElems();  // smazu vnitrek okna
		LogWindow->SetBackWindowNormalSize(10,10);
	} else { // vytvorim okno
		LogWindow = new CKerLogWindow(_x,_y,_sx,_sy,"Error && Debug Log");
		desktop->AddBackElem(LogWindow);
		LogWindow->Center(true,true);
		desktop->SetFocusEl(LogWindow,0);
	}
	if (!err_log) {  // pokud nemam log, zkusim ho nahrad ze soboru
		err_log = new CFSRegister("$DEFERRLOG$","Kernel Run Time Errors Log");
		if (err_log->GetOpenError() != FSREGOK) {
			SAFE_DELETE(err_log);
			return;
		}
	}
	if (!(key1 = err_log->FindKey("Group Counts"))) return;
	if (!(key2 = err_log->FindKey("Group Maxs"))) return;
	err_log->SeekAllTo0();

	//  Zobrazim pocty erroru ve skupinach a max pocty pamatovanych logu
	for (f=0;f<10;f++) {
		LogWindow->AddBackElem(new CGUIStaticText((float)((f%2)*300+10),(float)((f/2)*17+6),(float)150,KerEGNames[f],ARIAL,8,false,aRight,0xFFFFFFFF));
		i = key1->readi();	sprintf(str,"%i",i);
		if (i) {
			LogWindow->AddBackElem(new CGUIStaticText((float)((f%2)*300+170),(float)((f/2)*17+6),(float)70,str,ARIAL,8,false,aLeft,0xFF00FF00));
		} else {
			LogWindow->AddBackElem(new CGUIStaticText((float)((f%2)*300+170),(float)((f/2)*17+6),(float)70,str,ARIAL,8,false,aLeft,0xFFFFFFFF));
		}
		if (i = key2->readi()) {
			sprintf(str,"%i",i);
			LogWindow->AddBackElem(new CGUIStaticText((float)((f%2)*300+250),(float)((f/2)*17+6),(float)50,str,ARIAL,8,false,aLeft,0xFFFFFFFF));
		} else {
			LogWindow->AddBackElem(new CGUIStaticText((float)((f%2)*300+250),(float)((f/2)*17+6),(float)50,"Off",ARIAL,8,false,aLeft,0xFFFF0000));
		}
	}

	// napisu nadpisy:
	LogWindow->AddBackElem(new CGUIStaticText((float)5,(float)Y,30,"group",ARIAL,8,false,aCenter,0xFF00FF00));
	LogWindow->AddBackElem(new CGUIStaticText((float)40,(float)Y,20,"num",ARIAL,8,false,aCenter,0xFF00FF00));
	LogWindow->AddBackElem(new CGUIStaticText((float)65,(float)Y,310,"description",ARIAL,8,false,aCenter,0xFF00FF00));
	LogWindow->AddBackElem(new CGUIStaticText((float)380,(float)Y,20,"info",ARIAL,8,false,aCenter,0xFF00FF00));
	LogWindow->AddBackElem(new CGUIStaticText((float)405,(float)Y,80,"info",ARIAL,8,false,aCenter,0xFF00FF00));
	LogWindow->AddBackElem(new CGUIStaticText((float)490,(float)Y,35,"time",ARIAL,8,false,aCenter,0xFF00FF00));
	LogWindow->AddBackElem(new CGUIStaticText((float)530,(float)Y,90,"source",ARIAL,8,false,aCenter,0xFF00FF00));

	// Vypisuju radky logu
	sprintf(str,"Log%i",LogCount);
	key1 = err_log->FindKey(str);
	while(key1) {
		reg = key1->GetSubRegister();
		Y+=17;
		i = reg->FindKey("ErrorNum")->readi();
		switch (KERGETERRGROUP(i)) {
			case eKEGPanicError:
			case eKEGFatalError: color=0xFFFF0000; break;
			case eKEGInfo: color=0xFFFFFF00; break;
			case eKEGDebug: color=0xFF00FFFF; break;
			case eKEGWarning: color=0xFFBBBBBB; break;
			default: color=0xFFFFFFFF;
		}

		LogWindow->AddBackElem(new CGUIStaticText((float)5,(float)Y,35,KerEGShortNames[KERGETERRGROUP(i)],ARIAL,8,false,aLeft,color));
		sprintf(str,"%i",KERGETERRSNUM(i));
		LogWindow->AddBackElem(new CGUIStaticText((float)40,(float)Y,20,str,ARIAL,8,false,aRight,color));
		if (key2 = reg->FindKey("Description")) LogWindow->AddBackElem(new CGUIStaticText((float)65,(float)Y,310,key2->GetDirectAccess(),ARIAL,8,false,aLeft,color));
		sprintf(str,"%i",reg->FindKey("Info1")->readi());
		LogWindow->AddBackElem(new CGUIStaticText((float)380,(float)Y,20,str,ARIAL,8,false,aRight,color));
		if (key2 = reg->FindKey("Info2")) LogWindow->AddBackElem(new CGUIStaticText((float)405,(float)Y,80,key2->GetDirectAccess(),ARIAL,8,false,aLeft,color));
		sprintf(str,"%i",reg->FindKey("Time")->readi());
		LogWindow->AddBackElem(new CGUIStaticText((float)490,(float)Y,35,str,ARIAL,8,false,aRight,color));
		if ((key2 = reg->FindKey("Stack0"))&&(key2 = key2->GetSubRegister()->FindKey("MethodName"))) {
			if (key2->GetDirectAccess()[0]=='_'&&key2->GetDirectAccess()[1]=='K'&&key2->GetDirectAccess()[2]=='S'&&key2->GetDirectAccess()[3]=='M'&&key2->GetDirectAccess()[0]=='_')
				LogWindow->AddBackElem(new CGUIStaticText((float)530,(float)Y,90,key2->GetDirectAccess()+5,ARIAL,8,false,aLeft,color));
			else LogWindow->AddBackElem(new CGUIStaticText((float)530,(float)Y,90,key2->GetDirectAccess(),ARIAL,8,false,aLeft,color));
		}

		LogCount++;
		sprintf(str,"Log%i",LogCount);
		key1 = err_log->FindKey(str);
	}

}



///////////////////////////////////////////////////////////////////
///
///		C K e r E r r o r s
///
///////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
/// Ulozu jeden zaznam do registru
void CKerErrors::SaveOneLog(CFSRegister *r, CKerErrorLog *log, CFSRegister *edes) {
	CFSRegKey *k;
	CFSRegister *rs;
	CKerErrorStack *st = log->stack;
    char str[32];
	int i;

	// Info k logu
	r->AddKey("ErrorNum",FSRTint)->writei(log->ErrorNum);
	sprintf(str,"%u",log->ErrorNum); k = edes->FindKey(str);
	if (k) r->AddKey("Description",FSRTstring)->stringwrite(k->GetDirectAccess());
	r->AddKey("Info1",FSRTint)->writei(log->ErrorParam);
	if (log->ErrorStr) r->AddKey("Info2",FSRTstring)->stringwrite(log->ErrorStr);
	r->AddKey("Time",FSRTint)->writei(log->time);

	// call stack:
	i =0;
	while (st) {
		sprintf(str,"Stack%i",i);
		rs = r->AddKey(str,FSRTregister)->GetSubRegister();

		if (st->MethodName) rs->AddKey("MethodName",FSRTstring)->stringwrite(st->MethodName);
		rs->AddKey("Line",FSRTint)->writei(st->line);
		if (st->MsgMethodName) rs->AddKey("MsgMethodName",FSRTstring)->stringwrite(st->MsgMethodName);
		rs->AddKey("MsgLine",FSRTint)->writei(st->msgline);
		rs->AddKey("This",FSRTint)->writei(st->thisO);
		rs->AddKey("Sender",FSRTint)->writei(st->Sender);
		if (st->MName) rs->AddKey("Method KSID",FSRTstring)->stringwrite(st->MName->GetNameString());
		if (st->ThisName) rs->AddKey("This Type",FSRTstring)->stringwrite(st->ThisName->GetNameString());
		if (st->SenderName) rs->AddKey("Sender Type",FSRTstring)->stringwrite(st->SenderName->GetNameString());

		st = st->parent;
		i++;
	}
}


// Ulozi aktualni stav logu do souboru. Logy tak budou pouzitelne, ikdyz dojde k ukonceni Kernelu
void CKerErrors::SaveLogsToFile(char *path) {
	CKerErrorLog *log = start;
	CFSRegister *edes;
	CFSRegKey *k;
	int f;
	int ReOpen = 0;
	char str[32];

	SAFE_DELETE(KerErrorFiles->err_log); // zavri registr
	if (KerErrorFiles->LogWindow) ReOpen = 1;
	// otevru a smazu registr
	KerErrorFiles->err_log = new CFSRegister(path,"Kernel Run Time Errors Log",FSREGCLEARIT);
	KerErrorFiles->err_log->SetRegisterToBeUnCompressed();
	if (!KerErrorFiles->err_file) KerErrorFiles->err_file = new CFSRegister("$ERRORS$","ERRORS");
	if (!KerErrorFiles->err_file->FindKey("Ker RTE")) KerErrorFiles->err_file->AddKey("Ker RTE",FSRTregister);
	edes = KerErrorFiles->err_file->FindKey("Ker RTE")->GetSubRegister();

	// Nastavim, ze se stal nejaky error
	KerErrorFiles->WasError = 0;
	for (f=0;f<16;f++) if (Logs[f].counter && Logs[f].error) KerErrorFiles->WasError = 1;

	// ulozim informace o poctech erroru a maximalnich poctech pamatovanych logu:
	k = KerErrorFiles->err_log->AddKey("Group Counts",FSRTint);
	for (f=0;f<16;f++) k->writei(Logs[f].counter);
	k = KerErrorFiles->err_log->AddKey("Group Maxs",FSRTint);
	for (f=0;f<16;f++) k->writei(Logs[f].max);
	f=0;
	while (log) {  // ukladam logy (jednotlive zaznamy)
		sprintf(str,"Log%i",f);
	    SaveOneLog(KerErrorFiles->err_log->AddKey(str,FSRTregister)->GetSubRegister(),log,edes);
		log = log->next;
		f++;
	}

	KerErrorFiles->err_log->WriteFile(); // zapisu to na disk.
	if (ReOpen) KerErrorFiles->ShowAllLogs(0,0,0,0);
}




// VYTVORENI LOGU
void CKerErrors::LogError( int errornum, int errorparam, char *errorstr) {
	int eg = errornum >> KERGROUPSHIFT; // <- Skupina, do ktere error patri
	CKerErrorLog *log;
	if (Logs[eg].counter < KERMAXERRORCOUNT) Logs[eg].counter++;
	if (Logs[eg].error) {
		eg = errornum >> KERGROUPSHIFT;
	}
	if (Logs[eg].max) { // loguju:
		if (Logs[eg].counter > Logs[eg].max) { // Log zaplnen
			// smazu jeden log ze skupiny
			log = Logs[eg].start;
			Logs[eg].start = log->nextig;
			if (!log->nextig) Logs[eg].end = 0;
			if (log->next) log->next->prev = log->prev; else end = log->prev;
			if (log->prev) log->prev->next = log->next; else start = log->next;
			delete log;
		}
		// pridam log
		log = new CKerErrorLog();
		if (Logs[eg].end) Logs[eg].end->nextig = log; else Logs[eg].start = log;
		Logs[eg].end = log;
		if (end) {
			end->next = log;
			log->prev = end;
		} else start = log;
		end = log;
    	log->ErrorNum = errornum;
		log->ErrorParam = errorparam;
		if (errorstr) {
			log->ErrorStr = new char[strlen(errorstr)+1];
			strcpy(log->ErrorStr,errorstr);
		}
		if (KerContext) log->stack = new CKerErrorStack(KerContext,0);
	}
	// Kontroluju, jestli nemam shodit Kernel:
	if (Logs[eg].terminateKer==1||(Logs[eg].terminateKer==2&&Logs[eg].counter>=Logs[eg].max)) {
		if (errornum!=eKRTEPaniCInfo) LogError(eKRTEPaniCInfo);
		DebugMessage(1,0xFFFF0000,"K:PANIC");
		throw CKernelPanic();
	}
}





///////////////////////////////////////////////////////////////////////////////////////

// INITACE podle DebugMode
CKerErrors::CKerErrors(int DebugMode) {
	int f;
	start = 0; end = 0;
	LogWindow = 0;
	Logs[eKEGPanicError].terminateKer = 1;
	Logs[eKEGFatalError].terminateKer = 2;
	Logs[eKEGInfo].error = 0;
	Logs[eKEGDebug].error = 0;
	Logs[eKEGWarning].error = 0;
	switch (DebugMode) {
		case eKerDBRelease: for (f=2;f<16;f++) Logs[f].max=0; break;
		case eKerDBDebug: Logs[eKEGFatalError].max = 10; break;
	}
}



// Destruktor - Smaze logy
CKerErrors::~CKerErrors() {
	CKerErrorLog *log = start, *log2;
	if (LogWindow) {
		desktop->DeleteBackElem(LogWindow);
	}
	while (log) {
		log2 = log;
		log = log->next;
		delete log2;
	}
}



CKerErrorFiles::~CKerErrorFiles() {
	SAFE_DELETE(err_file);
	SAFE_DELETE(err_log);
}



