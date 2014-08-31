/////////////////////////////////////////////////////////////////////////////
///
///		K E R N E L
///
///		Hlavni soubor Kernelu. Load levlu a skriptu. Rizeni
///		A: Honza M.D. Krcek
///
/////////////////////////////////////////////////////////////////////////////


#ifndef KERKERNEL_H
#define KERKERNEL_H

#include "types.h"
#include "KerConstants.h"
#include "objects.h"


class CKerMain;
struct CKerMessage;
struct CKerContext;
class CKerErrors;
class CKerServices;
class CKerMapInfo;
class CKerAutosMain;
class CKerName;
class CKerNamesMain;
class CFSRegister;
class CFSRegKey;
struct CKerObject;
class CKerArrInt;
class CKerArrDouble;
class CKerArrChar;
class CKerArrPointer;
class CKerArrObject;
class CKerArrName;


// Promenna, kde je ulozen ukazetel na hlavni a jediny objekt kernelu CKerMain
extern CKerMain *KerMain;

// Kontext:
// kontext aktualne volane funkce. Nove volani = novy kontext. Kontexty tvori zasobnik
extern CKerContext *KerContext;

// Vytvoreni a naloadovani kernelu. Provadi se pripadne rekompilace skriptu
// Vyvola vyjimku CKernelPanic, kdyz se to nepodari. Log se ulozi.
// Kernel bude umisten do promennych KerMain, KerInterpret a KerContext
// runmode muze byt EDITOR_RUN nebo NORMAL_RUN (hra)
// DobugMode urcuje zpusob a mnozstvi logovani errorovych hlasek
// kdyz je scriptonly 0 loaduje se level, 1 - loaduji se pouze skripty
void KerCreateKernel(int runmode, int debugmode, int scriptonly, char *load);

// Kdyz se nahrava ulozena hra
// Vytvoreni a naloadovani kernelu. Provadi se pripadne rekompilace skriptu
// vraci 1 (uspech), 0 - neuspech, level se nepodarilo nahrat, kernel nevytvoren, 
// nebo vyhodi vyjimku KernelPanic, kernel vytvoren
int KerLoadGameAndKernel(char *load, CFSRegister *SavedGame=0);


// Maximalni pocet vnorenych volani. A zaroven maximalni pocet soucasne vytvorenych zprav:
#define KER_CALL_STACK_MAX 500
#define KER_CALL_STACK_WARNING (KER_CALL_STACK_MAX/2)
#define KER_MESSAGES_MAX 5000
#define KER_MESSAGES_WARNING (KER_MESSAGES_MAX/2)

// Errory pri startu. Je treba kontrolovat. Kernel se nemusi podarit nainicializovat.
enum eKerOLErrors {
	eKerOLENoError=0,
	eKerOLEOpeningLevel,  // error pri loadovani levelu
	eKerOLEOpeningSCode,  // error pri nacitani skriptu
	eKerOLENeedToRecompile  // verze kernelu, kdyz byly skripty kompilovany nesouhlasi s aktualni verzi kernelu. Je treba rekompilovat skripty.
};



// Mokro ulozi aktualni radku do kontextu (pro ladeni, pro logovani chyb)
#define SET_CODE_LINE(cline) {if (KerContext) KerContext->line=(cline);}
// Mokro zjisti aktualni radku z kontextu (pro ladeni, pro logovani chyb)
#define GET_CODE_LINE (KerContext?KerContext->line:0)
// Mokro Reinicializuje kontext
#define REINIT_CONTEXT {if (KerContext) KerContext->ReInitContext();}


struct CKerGarbageCollector {
	friend CKerArrInt;
	friend CKerArrDouble;
	friend CKerArrChar;
	friend CKerArrPointer;
	friend CKerArrObject;
	friend CKerArrName;
	CKerGarbageCollector() {
		GCArrInt=0; GCArrDouble=0; GCArrChar=0; GCArrPointer=0; GCArrName=0; GCArrObject=0;
	}
	void EmptyMe();
private:
	CKerArrInt *GCArrInt;
	CKerArrDouble *GCArrDouble;
	CKerArrChar *GCArrChar;
	CKerArrPointer *GCArrPointer;
	CKerArrObject *GCArrObject;
	CKerArrName *GCArrName;
};


/////////////////////////////////////////////////////////////////////////////////

struct CKerLevelInfo {
	CKerLevelInfo() { LevelFile=0; ScriptsFile=0; SVersion=0; Author=0; Game=0; Comment=0; Music=0; Tags=0; Difficulty=0; Directory=0; Password=0; LocalNames=0; }
	~CKerLevelInfo();
	int LoadScript(CFSRegister *reg);
	int LoadLevel(CFSRegister *reg);
	void SaveLevel(CFSRegister *reg); // sejvne vse krome LevelFile do registru
	char *LevelFile;
	char *ScriptsFile;
	char *SVersion;
	_int64 SVersion64;
	char *Author;
	char *Game;
	char *Comment;
	char *Music;
	double Difficulty;
	int Tags;			// viz eMMLevelTags (zda je vdy pristupny, vzdy editovatelny, preskocitelny, zamceny)
	char *Directory;	// adresar ve kterem se level ma nachazet. zacina $GAMES$/..
	char *Password;		// heslo adresare
	CFSRegister *LocalNames;	// lokalizovana jmena luvlu. Bacha pozice muze bejt nastavena na konec
};




/////////////////////////////////////////////////////////////////////////////////
///
///		C K e r M a i n
///
///		Hlavni objekt Kernelu - Interface
///
/////////////////////////////////////////////////////////////////////////////////


class CKerMain {
friend CKerObject;
friend CKerMessage;
friend CKerContext;
friend CKerServices;
friend CKerLevelInfo;
public:
	CKerMain(int runmode, int debugmode); // Inituje kernel
						// runmode muze byt EDITOR_RUN nebo NORMAL_RUN (hra)
						// DobugMode urcuje zpusob a mnozstvi logovani errorovych hlasek
	void LoadLevel(char *level); // Naloaduje Kernel - volat po konstruktoru, uz muzou vznikat vyjimky CKernalPanic
	int LoadGame(CFSRegister *lev, CFSRegKey *LevelFile, CFSRegKey *ScriptsFile); // Nahraje ulozenou hru  - volat po konstruktoru, uz muzou vznikat vyjimky CKernalPanic
	void LoadScriptOnly(char *script); // Naloaduje a pust script bez levlu - volat po konstruktoru, uz muzou vznikat vyjimky CKernalPanic
	int SaveLevel(); // sejvne Level s vyuzitim LevelInfa. 1 - OK, 0 - chyba;
	~CKerMain();	// Destruktor
	int GetOpenError() {return OpenError;} // 0 - OK, 1 a vic Error, vraci eKerOLErrors
	CKerErrors *Errors;		// Interface pro Errory	(Nemenit, je zkonstruovano pri startu kernelu)
	CKerObjs *Objs;			// Interface pro Objekty (Nemenit, je zkonstruovano pri startu kernelu)
	CKerNamesMain *KerNamesMain;  // Interface pro Jmena (Nemenit, je zkonstruovano pri startu kernelu)
	CKerMapInfo *MapInfo;	// Info o velikosti mapy a tvaru mrizky
	CKerAutosMain *AutosMain;  // Interface na automatismy
	CKerLevelInfo LevelInfo;	// Informace o levlu

	// Parametr CodeLine je misto(radka), odkud je sluzba volana. Je pocitana relativne k dane metode. 
	// Nasledujicim funkcim se predava pro ladici ucely

	int GetTypeSize(int Type); // Vrati velikost zakladnich typu (definovanych v eKerTypes) Priznak eKTret se ignoruje.
	CKerName *GetObjType(OPointer ObjPtr) // Pro zjisteni typu objektu (jeho KSID jmena):
		{ CKerObject *obj=Objs->GetObject(ObjPtr); if (obj) return obj->Type->Name; else return 0;}
	OPointer NewObject(int CodeLine, CKerName *ObjType); // vytvori novy objekt, zavola constructor
	OPointer CopyObject(int CodeLine, OPointer object);  // vytvori objekt, zkopiruje do nej data, zavola copyconstructor, ktery by mel sparvne nastavit, zkopirovat pointrove retezce
	void DeleteObject(int CodeLine, OPointer Object) // Zruseni objektu (pozivaji se zpravy. K zruseni dojde az pote, co se objekt prestane pouzivat) Vola se destruktor.
		{ if (!Objs->GetObject(Object)) return; if (KerContext) message(CodeLine, Object,Destructor,eKerCTcallend,Object,0); else call(CodeLine, Object,Destructor,0); }

	int GetTime() { return Time; }  // Vrati cas od startu kernelu
	void TerminateKernel() { _TerminateKernel = 1; } // ukonci kernel na konci kola
	void SaveGame(char *file=0) { SAFE_DELETE_ARRAY(_FileToSaveGame); _SaveGame = 1; if (file) _FileToSaveGame = newstrdup(file);} // sejvne hru na konci kola
	int GetRunMode() { return RunMode; }
	int IsGameMode() { return (RunMode==NORMAL_RUN); }
	int IsEditorMode() { return (RunMode==EDITOR_RUN); }

	// pro pristup k objektovym promennym:
	void *GetAccessToVar(OPointer obj, CKerOVar* Var); // vraci pointr na promennou objektu (pozor nici kontext!) kdyz promenna neni vraci null
	void *GetAccessToVar(OPointer obj, int VarNum); // vraci pointr na promennou objektu (pozor nici kontext!) kdyz promenna neni vraci null
	void *GetAccessToVar(CKerObject *kobj, CKerOVar* Var); // vraci pointr na promennou objektu (pozor nici kontext!) kdyz promenna neni vraci null
	void *GetAccessToVar(CKerObject *kobj, int VarNum); // vraci pointr na promennou objektu (pozor nici kontext!) kdyz promenna neni vraci null
	CKerObject * GetAccessToObject(OPointer obj); // Nastavi pristup k promennym objektu (pozor nici kontext!) Kdyz objekt neexistuje, vraci null
	CKerObject * GetAccessToObject(CKerObject *kobj) { // Nastavi pristup k promennym objektu (pozor nici kontext!)
		if (kobj->Type->SetObjectVar) kobj->Type->SetObjectVar(kobj->KSVG);	return kobj;
	}
	void *GetAccessToVar2(CKerObject *kobj, CKerOVar* Var) { // vraci pointr na promennou objektu, napred je treba volat fci GetAccesToObject
		if (!Var->KSVar) {	assert(kobj->ISData); return kobj->ISData + Var->Offset; } else return *(Var->KSVar);
	}
	void *GetAccessToVar3(CKerObject *kobj, int VarNum) { // vraci pointr na promennou objektu, napred je treba volat fci GetAccesToObject, vrati NULL, kdyz objekt promennou nema
		if (!kobj->Type->SpecificKnownVars) return 0;
		CKerOVar *Var = kobj->Type->SpecificKnownVars[VarNum]; if (!Var) return 0;
		if (!Var->KSVar) {	assert(kobj->ISData); return kobj->ISData + Var->Offset; } else return *(Var->KSVar);
	}

	
	// METODY PRO BEZPECNE VOLANI A POSILANI ZPRAV:
	// CodeLine - cislo radky, odkud se vola (pro ladeni), kdyz se CodeLine nezadava, predpokladam, ze je spravne nastavena v Kontextu
	// Object - volany objekt, Method - Volana metoda (jeji KSID jmeno), NumArgs - Pocet predavanych argumentu
	// when - cas kdy se ma zprava vyvolat viz. eKerCallTimes. pokud when==eKerCTcallend, tak callendObjekt urcuje
	//		objekt, po kterem se ma zprava vyvolat. Zprava se vyvola co nejdrive, ale az zkonci vsechny metody objektu callendObject
	//      Pokud when==eKerCTtimed, do CallendObject dej cas vyvolani
	// ArgType(s) - Typ argumentu - ten typ v jakem je argument pri volani (Kernel pak argument pripadne sam automaticky prevede na typ, jaky si konkretni metoda vyzaduje)
	// RetType - Typ, jaky chci, aby mi fce vratila (muzu nastavit na eKTvoid)
	// ArgName(s) - KSID Jmeno argumentu.
	// Arg - Argument (primo hodnota), u stringu a eKTret argumentu - pointr na argument)
	// ArgPointers - Pointery na argumenty
	void call(int CodeLine, OPointer Object,CKerName *Method,int NumArgs, ...); // int ArgType, CKerName *ArgName, Arg, ... 
	unsigned char callchar(int CodeLine, OPointer Object,CKerName *Method,int NumArgs, ...); // int ArgType, CKerName *ArgName, Arg, ... 
	int callint(int CodeLine, OPointer Object,CKerName *Method,int RetType, int NumArgs, ...); // int ArgType, CKerName *ArgName, Arg, ... 
	double calldouble(int CodeLine, OPointer Object,CKerName *Method,int NumArgs, ...); // int ArgType, CKerName *ArgName, Arg, ... 
	void message(int CodeLine, OPointer Object, CKerName *Method, int when, OPointer callendObject, int NumArgs, ...); // int ArgType, ..., CKerName *ArgName, ..., Arg, ...
	void ISmessage(OPointer Object, CKerName *Method, int when, OPointer callendObject, int NumArgs, int *ArgTypes, CKerName **ArgNames, void **ArgPointers);
	void IScall(OPointer Object, CKerName *Method, int NumArgs, int *ArgTypes, CKerName **ArgNames, void **ArgPointers, int RetType);  // navratova hodnota fce je ponechana na zasobniku

	void RunTurn(int time, int krize);	// Spusti Skripty na jedno kolo, cas je zadavan relativne od posledniho kola. MinTime <= time <= MaxTime
	void RunMessages(CKerMessage **startmq, CKerMessage **endmq); // Provede vsechny zpravy ze zadane fronty

	// Nasledujici funkce najdou vsechny zpravy ( ve vsech frontach), ktere odpovidaji popisu. Pokud nektery parametr je zadan jako null, tak na tomto parametru nezalezi, vyhleda se vse
	int FDeleteMessages(OPointer Reciever, CKerName *msg=0, OPointer Sender=0); // Funkce vyhleda a smaze zpravy ze vsech front. Vrati pocet smazanych zprav.
	int FCountMessages(OPointer Reciever, CKerName *msg=0, OPointer Sender=0); // Funkce spocita zpravy ve vsech frontach (vraci pocet).

	CKerName *Constructor, *LoadConstructor, *CopyConstructor, *Destructor;  // Jmena zakladnich metod

	int NumGV;				// pocet globalnich prom.		
	CKerOVar *GlobalVar;	// Informace o typech a umistenich globalch promennych
	CKerOVar **SpecificKnownVars;	// pole pointru na OVars, jejichz vyznamy Kernel zna - pole muze byt prazdne nebo ma velikost KERVARUSESSIZE, undexuje se pomoci eKarVarUses
	int StaticObjectsCount; // pocet statickych objektu
	
	int LoadDependencies(int NamesNum, CFSRegKey *r, CKerName **names);
	CKerGarbageCollector GarbageCollector;

private:
	// Funkce pro Init a Load Kernelu:
	int LoadScriptsCode(CFSRegister *code);
	int LoadLevel2(CFSRegister *lev);
	void GetReadyToStart(); // Pripravi naloadovanej Kernel K Behu scriptu
	void SetConstructors(); // vola se pote, co jsou nactene jmena. Vyhleda jmena typu Constructor a ulozi je do promenych
	void InitGlobalsNoL();	// Inituje globalni promenne na 0, vytvori staticke objekty
	void LLoadVariable(CFSRegister *r, CKerOVar *OV, UC *offset); // Paokud najde v registru promennou, tak ji nahraje
	void LSaveVariable(CFSRegister *r, CKerOVar *OV, UC *offset, OPointer thisO); // Sejvne promennou do registru
	int LLoadGlobals(CFSRegKey *r); // Nahraje hodnoty globalnich promennych z levlu, 1 OK, 0 chyba 
	int LLoadObjects(CFSRegKey *r); // Nahraje objekty z levlu a vytvori je, 1 OK, 0 chyba 
	void LSaveObjects();	// sejvne objekty do nezarazenych registru.
	int LSSortGraph(OPointer obj, OPointer parent, CFSRegister *sreg, int &count); // trideni grafu DFS, objekty predavoji sve sejvovaci egistry na vystup.
	int LLoadObject(CFSRegister *r, CKerName *ObjType, OPointer *StaticVar=0); // Nahraje objekt z levlu a vytvori ho, ptr objektu OK, 0 chyba , -1 zahodil jsem shortcut
	int GLoadKernelState(CFSRegister *lev); // nahraje stav kenelu a zpravy - pro GameLoad
	int GLoadMessageQueue(CFSRegKey *k2, int typ); // nahraje frontu zprav daneho typu
	void GSaveKernelState(CFSRegister *lev); // ulozi stav kenelu a zpravy - pro GameLoad
	void GSaveMessageQueue(CFSRegKey *k2, CKerMessage *mq); // ulozi frontu zprav daneho typu
	void GSaveGame(); // ulozi hru do pripravenoho registru
	void GSaveObjects(CFSRegister *objs); //ulozi vsechny objekty
	char *GetCodeName(char *script);
	
	int OpenError; // Podarilo se kernel nahrat?
	int SaveState; // zda sejvuju/loaduju. viz eSaveloadState
	CFSRegister *SaveReg;  // registr, ve kterem maji volane funkce otevirat streamy k save/load. Nastaven jenkdyz je to povoleno, jinak null!

	int RunMode; // 0 - Normal Run, 1 - Editor Run
	int DebugMode; //
	int _RunTurn; // Nastaveno na 1, jestlize je Kernel volan pres funkci RunTurn, jinak 0
	int _TerminateKernel; // Kdyz je 1 Kernel se na konci kola pomoci vyjimky CKernelPanic radne ukonci
	int _SaveGame; // zda mam na konci kola sejvnout hru.
	char *_FileToSaveGame; // jemno souboru, kam se bude sejvovat.
	unsigned int Time; // cas od zacatku levlu
	int TimeCrisis; // 1 - Pokud jsem v casove krizi (nestiham), 0 - OK
	int ScriptActivity; // Pro sledovani vykonu. Kolik volani se uskutecnilo v tomto kole

	CKerMessage *startmq[4];  // Fronty zprav. 0 - eKerCTmsg, 1 - eKerCTend, 2 - eKerCTnext, 3 - eKerCTnextend
	CKerMessage *endmq[4];
	CKerMessage *timedmsgs;   // Casovane zpravy (nepotrebuju konec)
	void DeleteMessages(CKerMessage **startmq, CKerMessage **endmq); // Vyprazdni frontu zprav, aniz zpravy provede.
	void PopTimedMessages(); // Prida casovane zpravy na ktere uz prisel cas od aktualni fronty zprav.
	int FindCountDeleteMessages(CKerMessage **startmq, CKerMessage **endmq,OPointer Reciever, CKerName *msg, OPointer Sender, int Delete = 0); // Vyhleda zadane zpravy ve fronte, spocita je a pripadne smaze

	void PassParams(CKerMethod *method, CKerMessage *msg); // Pomocna rutina - predani Argumentu
	void SetDefaultParams(CKerMethod *method);			   // Pomocna rutina - nazadane argumenty nastavi na jejich defaultni hodnotu
	void ConvertParam(void *Source, int SType, void *Dest, int DType); // Automaticka konverze zakladniho typu
	void ConvertParamRet(void *Source, int SType, void *Dest, int DType, int RetFce); // Automaticka konverze zakladniho typu. Vystup se prida k vystupu pomoci returnovaci funkce
	void FillRetFceDefault(void *Dest, int DType, int RetFce, int ErrorNum); // vola se v pripade ze nebylo nic predano. Pokud je tam korektni retFce, preda se default, jinak nahlasi chybu ErrorNum
	void Destruct(OPointer ObjPtr, CKerObject *obj); // Provede dealokaci objektu (volano pote, co probehly destruktory)

	// Citace pro kontrolu priliz mnoha zasobnikovych volani a priliz mnoha zprav:
	int _message_counter, _message_counter_warning;  // Pocet vnorenych volani. A zda uz byl hlasen warning
	int _call_stack_counter, _call_stack_counter_warning; // Pocet nevyrizenych zprav. A zda uz byl hlasen warning
	char GCNstrbuff[64]; // buffer pro GetCodeName

};






//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
///
///		K o n t e x t :
///		kontext aktualne volane funkce. Nove volani = novy kontext. Kontexty tvori zasobnik
///
//////////////////////////////////////////////////////////////////////////////////////////

// Prace s kontextem:
// 1. New (Kontext se prida na zasobnik kontextu KerContext)
// Pro vsechny zpravy:
//    2. InitContext (kontext bude nastaven na konkretni objekt)
//    Pro vsechny Metody pro konkretni objekt:
//      3. Nastav promenne Params, ParamsInfo, MethodName
//      Volani 
//    4. Deinit : nastav prom. KCthis na 0, zavolej KerMain->RunMessages na zpravy v kontextu
// 5. delete (kontext bude odstranen ze zasobniku, stary kontext bude reinitovan)
struct CKerContext {
	CKerContext(int Line) { 
		ParamsArray = 0;
		KerMain->_call_stack_counter++;
		startmq = 0; endmq = 0; SetObjectVar = 0; MethodName = 0; line = 0;
		parent = KerContext;
		if (parent) parent->line = Line;
		if ((KerMain->_call_stack_counter >= KER_CALL_STACK_WARNING && !KerMain->_call_stack_counter_warning)||KerMain->_call_stack_counter >= KER_CALL_STACK_MAX) _log_error();
		KerContext = this; // kontext se automaticky prida na zasobnik
	}
	CKerContext() { // kdyz je line uz nastavena
		ParamsArray = 0;
		KerMain->_call_stack_counter++;
		startmq = 0; endmq = 0; SetObjectVar = 0; MethodName = 0; line = 0;
		parent = KerContext;
		if ((KerMain->_call_stack_counter >= KER_CALL_STACK_WARNING && !KerMain->_call_stack_counter_warning)||KerMain->_call_stack_counter >= KER_CALL_STACK_MAX) _log_error();
		KerContext = this; // kontext se automaticky prida na zasobnik
	}
	~CKerContext() { // Odebrani kontextu ze sasobniku, reinicializace stareho kontextu.
		SAFE_DELETE_ARRAY(ParamsArray);
		KerMain->_call_stack_counter--;
		KerMain->DeleteMessages(&startmq, &endmq);
		KerContext = parent;
		if (parent&&parent->SetObjectVar) parent->SetObjectVar(parent->KSVG);
	}
	void InitContext(CKerObject *obj, OPointer sender, CKerName *mName) {  // Inicializace casti kontextu
		// obj - Volany objekt, Sender, mName - KSID volane metody (pro ladeni)
		KCthis = obj->thisO; // Zpristupneni volaneho objektu ...
		ISData = obj->ISData;
		KSVG = obj->KSVG;
		Sender = sender;
		MName = mName;  // Info pro ladeni ...
		MsgMethodName = 0;
		msgline = 0;
		SetObjectVar = obj->Type->SetObjectVar;  // Nastavi adresy pro promenne objektu v KS
		if (SetObjectVar) SetObjectVar(KSVG);
	}
	void ReInitContext() { // znavu sparvne nastavi pointery na KS objektove promenne
		if (SetObjectVar) SetObjectVar(KSVG);
	}
	void SetArgumentPointers(CKerMethod *ms) {  // Nastavi pointery na jednotlive argumenty. Napred je treba nastavit Params!
		int f;
		if (ms->NumP) {
			SAFE_DELETE_ARRAY(ParamsArray);
			ParamsArray = new void*[ms->NumP];
			for (f=0;f<ms->NumP;f++) ParamsArray[f] = Params + ms->Params[f].Offset;
		}
	}
	char *MethodName; // jmeno provadene metody (ptr na string) - pro ladici ucely
	CKerName *MName; // KSID jmeno metody - pro ladici ucely
	int line; // aktualni cislo radky, za zivota kontextu je treba line aktualizovat
	char *MsgMethodName; // jmeno metody, odkud byla zavolana zprava (ptr na string) - pro ladici ucely
	int msgline; // cislo radky, odkud byla zavolana zprava
	unsigned char *Params;  // Pintr na parametry. 1. je navratova hodnota fce
	void **ParamsArray;		// pole pointru na jednotlive argumenty
	unsigned char *ParamsInfo; // Informace o parametrech. 0-neprirazeno(pouzit default), 1-prirazeno
	CKerMessage *startmq;   // fronta zprav pro aktualni kontext
	CKerMessage *endmq;
	OPointer KCthis;  // this
	OPointer Sender;  // volajici objekt, 0-volano z kernelu, objekt neni znam
	UC *ISData;		  // Misto dat v IS	
	void *KSVG;		  // Pointr na strukturu s daty v KS	
	void (*SetObjectVar)(void *KSVG);  // Funkce ktera inicializuje glob promenne s pointry na polozky v KSVG
	CKerContext *parent;  // Nadrazeny kontext (Kontexty jsou v zasobniku)
private:
	void _log_error();
};







////////////////////////////////////////////////////////////////////////////
///
///		C K e r M e s s a g e
///
////////////////////////////////////////////////////////////////////////////


struct CKerMessage {
	// Vytvoreni zpravy. When - cas vyvolani, pokud when==eKerCTcallend, tak callendObjekt urcuje
	// objekt, po kterem se ma zprava vyvolat. Zprava se vyvola co nejdrive, ale az zkonci vsechny metody objektu callendObject
	CKerMessage(int when, OPointer callendObject); // po zkonstruovani je jeste nutne vyplnit Receiver a Method
	~CKerMessage() {
		KerMain->_message_counter--;
		SAFE_DELETE_ARRAY(ArgTypes);
		SAFE_DELETE_ARRAY(ArgNames);
		SAFE_DELETE_ARRAY(Args);
	}
	OPointer Receiver;
	OPointer Sender;
	CKerName *Method;		// KSID metody
	int NumArgs;			// pocet argumentu	
	int *ArgTypes;			// pole s typy argumentu
	CKerName **ArgNames;	// Pole s KSID jmeny argumentu
	char *Args;				// Argumenty (jejich hodnota je do tohoto pole prekopirovana)
	int CodeLine;			// radek, odkud byla zprava volana (pro ladeni)
	char *MethodName;		// ptr na jmeno metody, odkud byla zprava volana (pro ladeni)
	CKerMessage *next;		// dalsi zprava ve fronte
	UI Time;				// cas vyvolani. Nastavuje se u timed messages
};






//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


#include "KerErrors.h"



// Rutiny, ktere je nutne volat na zacatku Direct call metody. (Inicializace kontextu)
// Predpoklad Metoda ma jako sve prvni dva parametry: int __CodeLine, OPointer __thisO
// CKerName * __ObjName (TypObjeku), CKerName *__MName (KSID Jmeno Metody), 
// char *__MethodName (Retezec KSM jmeno metody, __RetVal - Co vratit v pripade chyby, nastavit podle typu fce (Void - nastav na ; )
#define DIRECT_CALL_BEGIN(__ObjName, __MethodName, __RetVal) {\
	CKerObject *__obj = KerMain->Objs->GetObject(__thisO); \
	if (!__obj) { KerMain->Errors->LogError2(__CodeLine,eKRTEDCNoObj); return __RetVal; } \
	new CKerContext(__CodeLine); \
	if (KerContext->parent) KerContext->InitContext(__obj,KerContext->parent->KCthis,0); else KerContext->InitContext(__obj,0,0); \
	KerContext->MethodName = __MethodName; }

// test na spranvbnost objektu jsem zatim vyhodil
//	if (__ObjName!=__obj->Type->Name) { KerMain->Errors->LogError2(__CodeLine,eKRTEDCBadObj); return __RetVal; } \

// Rutinny, ktere je treba zavolat na konci Direct Call Metody (Deinicializace kontextu)
// Pozor, aby metoda nekde nevolala RETURN !!!!
#define DIRECT_CALL_END {\
	KerContext->KCthis = 0; \
	if (KerContext->startmq) KerMain->RunMessages(&(KerContext->startmq), &(KerContext->endmq)); \
	delete KerContext; }

#define KER_ARGUMENT(_typ, _num) (*(_typ*)KerContext->ParamsArray[_num])
#define KER_RETARG(_typ) (*(_typ*)KerContext->Params)
#define KER_NEW(_line,_typ,_size) ((_typ*)KerInterpret->ISMalloc(_line,sizeof(_typ)*(_size)))
#define KER_NEW2(_typ,_size) ((_typ*)KerInterpret->ISMalloc(sizeof(_typ)*(_size)))








#endif



