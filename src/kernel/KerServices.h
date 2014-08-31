//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - S E R V I C E S
///
///		Sluzby Kernelu, Interface Systemu Krkal pro skripty
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////

#ifndef KERSERVICES_H
#define KERSERVICES_H

#include "types.h"
#include "kernel.h"
#include "LevelMap.h"


class CKerName;
class CFSRegKey;
class CFSRegister;
class CKerMain;



#define KERNUMEXTENSIONS 9
enum eEXTensions {
	eEXTunknown		= -1,
	eEXTlevel		= 0,
	eEXTpackage		= 1,
	eEXTscript		= 2,
	eEXTauto		= 3,
	eEXTels			= 4,
	eEXTtex			= 5,
	eEXTani			= 6,
	eEXTmModule		= 7,
	eEXTmSample		= 8,
	eEXTdirectory	= 10, // pouzito az v CMMLevelDirs
	eEXTcommand		= 11,
};

extern char *Extensions_Strings[KERNUMEXTENSIONS];



enum eSaveLoadState {
	eSLSnothing,
	eSLSsaveLevel,			// save level z editoru
	eSLSloadConstructor,	// load level
	eSLSsaveGame,
	eSLSloadGame,
};


//////////////////////////////////////////////////////////////////
///
///		DALSI SLUZBY KERNELU
///		Objekt je vytvoren staticky a obsahuje jen fce.
///		Pouzivat jen, pokud je spravne skonstruovan Kernel !!!
///
//////////////////////////////////////////////////////////////////

class CKerServices {
public:
	CKerServices() {IntBuff = 0; ObjBuff = 0; QuickSave = 0;}
	CFSRegister *QuickSave;

	void LogUserError(int Line, DWORD group, int info1 = 0, char *str = 0) // Nahlasi uzivatelsky error z libovolne skupiny (eKerErrGroups) (0 az 15)
		{ if (group>15) group = eKEGError; KerMain->Errors->LogError2(Line,KERGETERRNUM(group,10),info1,str);}
	void LogDebugInfo(int Line, DWORD num , int info1 = 0, char *str = 0) // Nahlasi error ze skupiny Debug
		{ num = num & 0x0000FFFF; KerMain->Errors->LogError2(Line,KERGETERRNUM(eKEGDebug,num),info1,str);}
	
	int IsFMemberOfS(CKerName *first, CKerName *second); // Vrati, jedna, jestlize jmena existuji a prvni <= Druhemu; JInak nulu
	int IsFMemberOfS(OPointer first_obj, CKerName *second); // Vrati, jedna, jestlize jmena existuji a prvni <= Druhemu; JInak nulu.  Prvni jmeno si zjisti jako typ objektu
	int IsFMemberOfS(CKerName *first, OPointer second_obj); // Vrati, jedna, jestlize jmena existuji a prvni <= Druhemu; JInak nulu
	int IsFMemberOfS(OPointer first_obj, OPointer second_obj); // Vrati, jedna, jestlize jmena existuji a prvni <= Druhemu; JInak nulu. 
	int IsFMemberOfSExcl(CKerName *first, CKerName *second); // Vrati, jedna, jestlize jmena existuji a prvni < Druhemu; JInak nulu
	int IsFMemberOfSExcl(OPointer first_obj, CKerName *second); // Vrati, jedna, jestlize jmena existuji a prvni < Druhemu; JInak nulu.  Prvni jmeno si zjisti jako typ objektu
	int IsFMemberOfSExcl(CKerName *first, OPointer second_obj); // Vrati, jedna, jestlize jmena existuji a prvni < Druhemu; JInak nulu
	int IsFMemberOfSExcl(OPointer first_obj, OPointer second_obj); // Vrati, jedna, jestlize jmena existuji a prvni < Druhemu; JInak nulu. 

	void InitMoveTo(OPointer obj, UI time, int dx, int dy, int dz=0); // nastartuje plynuly pohyb
	void MConnectObjs(OPointer obj1, OPointer obj2); // propoji objekty, tak ze se budou hejbat spolu. Plynuly pohyb se kopiruje z druhe skupiny na prvni
	void MDisconnectObj(OPointer obj);	// odpoji objekt od skupiny

	void ResetAuto(OPointer obj, int ResetRandom=0, int ResetConnection=0);   // reset automatismu
	CKerName *GetActivAuto(OPointer obj) {
		CKerObject *ko = KerMain->Objs->GetObject(obj);
		if (!ko || !ko->PlacedInfo) return 0; else return ko->PlacedInfo->AktivAuto;
	}
	CKerName *GetDefaultAuto(OPointer obj) {
		CKerObject *ko = KerMain->Objs->GetObject(obj);
		if (!ko) return 0; else return ko->Type->DefAuto;
	}
	CKerName *GetDefaultAuto2(CKerName *ObjType); 


	void DeleteOutOfMapCalls(CKerArrInt *Buff) {  // z pole odstrani bunky, ktere jsou mimo mapu
		KerMain->MapInfo->DeleteOutOfMapCalls(*Buff);
	}
	int FindObjCell(OPointer obj, int *cx, int *cy, int *cz) { // ze souradnic zjisti primarni bunku objektu. , vrati nulu v pripade chyby out of map
		CKerObject *ko = KerMain->GetAccessToObject(obj);
		if (!ko) return 0;
		int ret = KerMain->MapInfo->FindObjCell(ko,*cx,*cy,*cz);
		REINIT_CONTEXT;
		return ret;
	}
	int FindCell(int x, int y, int z, int *cx, int *cy, int *cz) { // zjisti do ktere bunky bod x,y,z patri. Vrati 0, jestlize jsi mimo mapu, jinak 1
		return KerMain->MapInfo->FindCell(x,y,z,*cx,*cy,*cz);
	}
	void FindCellPosition(int cx, int cy, int cz, int *x, int *y, int *z) {  // Najde centralni pozici bunky
		KerMain->MapInfo->FindCellPosition(cx,cy,cz,*x,*y,*z);
	}
	void ReadObjCoords(OPointer obj, int *x, int *y, int *z); // precte z objektu souradnice ma-li je (jinak vraci nuly). 


	void SetTag(OPointer obj) { CKerObject *ko = KerMain->Objs->GetObject(obj); if(ko) ko->Tag = 1; }
	void ClearTag(OPointer obj) { CKerObject *ko = KerMain->Objs->GetObject(obj); if(ko) ko->Tag = 0; }
	int CheckTag(OPointer obj) { CKerObject *ko = KerMain->Objs->GetObject(obj); if(ko) return ko->Tag; else return -1;}
	int ExistsObj(OPointer obj) { if (KerMain->Objs->GetObject(obj)) return 1; else return 0;}

	void FindObjsInArea(int CodeLine, CKerArrObject *output,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // najde objekty, ktere jsou z mnoziny set, v bunkach na dane oblasti. set nemusi byt zadan.
		// pokud je zadan objekt, tak je oblast brana relativne k centralni bunce onjektu.
	void AreaCall(int CodeLine, CKerName *Method,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // naleznou se objekty v oblasti a zavola se na ne metoda
	void AreaMessage(int CodeLine, CKerName *Method,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // naleznou se objekty v oblasti a zavola se na ne metoda

	int round(double a) { return int(floor(a+0.5)); }

	// funkce vytvareji kopie kernelich poli
	CKerArrInt *CopyIntA(CKerArrInt *a);  
	CKerArrChar *CopyCharA(CKerArrChar *a);
	CKerArrDouble *CopyDoubleA(CKerArrDouble *a);
	CKerArrObject *CopyObjPtrA(CKerArrObject *a);
	CKerArrName *CopyNameA(CKerArrName *a);

	/////////////////////////////////////////////////////

	char *ConverNameToShort(const char *ksid, int EatStart=0, char *LocalizeTo=0); // ze stringu vyhodi uvodni _KS??_, cisla verzi, __M_ nahradi ctyrbotkou. Fce pro string pouziva svuj buffer, nedealokovat. EatStart urcuje kolik jmen ma uzrat ze zacatku. Vzdy tam ale alespon to posledni necha
	char *ConvertFileNameToShort(const char *file, int &ext, int AllwaysEatVersion=0, int EatKnExtension=1);  // Pokud zna priponu,vrati jmeno souboru bez pripony a bez pripadne verze. Jinak jmeno nemeni. Fce vraci pointer na svuj buffer, nedealokovat
	_int64 GenerateVersionNumber(char *DestStr = 0);	// vygeneruje nove cislo verze pomoci MersenneTwisteru. Vrati ho jako cislo. Pokud predas funkci pointer na string, kam se vejde 19 znaku (+1 na koncovou nulu), tak i jako string.

	void KernelStartInit(); // alokuje kerneli pole, vola konstruktor Kernelu
	void KernelEnd() {IntBuff = 0; ObjBuff = 0; } // pri destructoru kernelu

	void RandomKropic();
private:
	char strbuff[16000];
	CKerArrInt *IntBuff; // buffer pro funkce hledajici objekty na oblasti
	CKerArrObject *ObjBuff; // buffer pro funkce hledajici objekty na oblasti
};



extern CKerServices KerServices;






/////////////////////////////////////////////////////////////////////////////
///
///			C K e r S a v e L o a d
///			Sluzby pro saveovani, loadovani obsahu objektu
///
/////////////////////////////////////////////////////////////////////////////

class CKerSaveLoad {
	friend CKerMain;
public:
	CKerSaveLoad() { SaveState = eSLSnothing; buff=0;}
	CFSRegKey *Open(CFSRegKey *key, int state);		// volej pred tim, nez chces naloadovat/sejvnout objekt. Vraci klic, ktery byl otevren pred tim
	CFSRegKey *Open(CFSRegister *reg, int state, CKerMethod *m); // pro pouziti ve funkcich call. najde se a otevre se tan spravny stream
	void Close() { SaveState = eSLSnothing; buff=0;}	// volej pote, co se loadovani objektu dokonci (kdyz se ma hned loadovat jiny objekt, neni povinne)

	void Seek(int pos);		// prace se streamem
	void SeekToEnd();
	int GetPos();
	int Eof();

	void SaveInt(int a);
	void SaveChar(UC a);
	void SaveDouble(double a);
	void SaveObjPtr(OPointer obj);
	void SaveName(CKerName *name);
	void SaveName(char *name);  // specialni verze pro Krkal Convertor
	void SaveName2(CFSRegKey *buff, CKerName *name);
	void SaveString(char *str, int Type);

	void SaveIntA(CKerArrInt *a);
	void SaveCharA(CKerArrChar *a);
	void SaveDoubleA(CKerArrDouble *a);
	void SaveObjPtrA(CKerArrObject *a);
	void SaveObjPtrA(int poradi); // specialni verze pro Krkal Convertor. ulozi jednoprvkove pole. nebo nula prvkove (poradi==0)
	void SaveNameA(CKerArrName *a);

	int LoadInt();
	UC LoadChar();
	double LoadDouble();
	OPointer LoadObjPtr();
	CKerName *LoadName();
	CKerName *LoadName2(CFSRegKey *buff); // nacte jmeno, pro GameLoad
	void LoadString(char *str, int Type);

	CKerArrInt *LoadIntA();
	CKerArrChar *LoadCharA();
	CKerArrDouble *loadDoubleA();
	CKerArrObject *LoadObjPtrA();
	CKerArrName *LoadNameA();
private:
	void AddEdge(OPointer obj1, OPointer obj2, CFSRegKey *key, int pos); // prida hranu do sejvovaciho grafu. Volat tehdy, kdyz obj1 ma u sebe pointer na obj2
	int SaveState;	// zda sejvuju/loaduju. viz eSaveloadState
	CFSRegKey *buff; // klic kam, odkud sejvuju

	static UI KERSL_NAMEID;
	static UI KERSL_STRINGID;
	static UI KERSL_ARRINTID;
	static UI KERSL_ARRCHARID;
	static UI KERSL_ARRDOUBLEID;
	static UI KERSL_ARRNAMEID;
	static UI KERSL_ARROBJID;
	static UI KERSL_ARRNULLID;
};


extern CKerSaveLoad KerSaveLoad;





/////////////////////////////////////////////////////////////////////////////
///
///			C K e r E d i t I
///			Sluzby pro tvoreni skriptovanych editacnich polozek
///
/////////////////////////////////////////////////////////////////////////////


class CKerEditI {
public:
	CKerEditI() { OVar = 0;	}
	~CKerEditI();

	void ECreateInt(int *data, char *label, char *comment=0) {ECreateItem(data,eKTint,label,comment);}
	void ECreateChar(UC *data, char *label, char *comment=0) {ECreateItem(data,eKTchar,label,comment);}
	void ECreateDouble(double *data, char *label, char *comment=0) {ECreateItem(data,eKTdouble,label,comment);}
	void ECreateName(CKerName **data, char *label, char *comment=0) {ECreateItem(data,eKTname,label,comment);}
	void ECreateObjPtr(OPointer *data, char *label, char *comment=0) {ECreateItem(data,eKTobject,label,comment);}

	void ECreateIntA(CKerArrInt **data, char *label, char *comment=0) {ECreateItem(data,eKTarrInt,label,comment);}
	void ECreateCharA(CKerArrChar **data, char *label, char *comment=0) {ECreateItem(data,eKTarrChar,label,comment);}
	void ECreateDoubleA(CKerArrDouble **data, char *label, char *comment=0) {ECreateItem(data,eKTarrDouble,label,comment);}
	void ECreateNameA(CKerArrName **data, char *label, char *comment=0) {ECreateItem(data,eKTarrName,label,comment);}
	void ECreateObjPtrA(CKerArrObject **data, char *label, char *comment=0) {ECreateItem(data,eKTarrObject,label,comment);}

	void ESetIntLimits(int DV, int mezcfg=0, int* list=0, int excl=0);
	void ESetVarTags(int tags,int filter=-1); // nastavi NamesMask (filtr) a promennou Use (tags).
	void ESetEditType(int CodeLine, int tags); // nastavi EditType (eKET..).
	
	int EPlaceItem(int where=0, int before=0); // umisteni vytvorene polozky

	// Funkce vytvori editacni polozku s prislusnou skupinou
	int EAdd2DCell(int *x, int *y, char *label, char *comment=0,int where=0, int before=0) {return EAddGroup(label,comment,where,before,eKET2Dcell,2,x,y);}
	int EAdd3DCell(int *x, int *y, int *z, char *label, char *comment=0,int where=0, int before=0) {return EAddGroup(label,comment,where,before,eKET3Dcell,3,x,y,z);}
	int EAdd2DPoint(int *x, int *y, char *label, char *comment=0,int where=0, int before=0) {return EAddGroup(label,comment,where,before,eKET2Dpoint,2,x,y);}
	int EAdd3DPoint(int *x, int *y, int *z, char *label, char *comment=0,int where=0, int before=0) {return EAddGroup(label,comment,where,before,eKET3Dpoint,3,x,y,z);}
	int EAdd2DAreaP(int *x1, int *y1, int *x2, int *y2, char *label, char *comment=0,int where=0, int before=0) {return EAddGroup(label,comment,where,before,eKET2Darea,4,x1,y1,x2,y2);}
	int EAdd3DAreaP(int *x1, int *y1, int *z1, int *x2, int *y2, int *z2, char *label, char *comment=0,int where=0, int before=0) {return EAddGroup(label,comment,where,before,eKET3Darea,6,x1,y1,z1,x2,y2,z2);}
	int EAdd2DAreaC(int *x1, int *y1, int *x2, int *y2, char *label, char *comment=0,int where=0, int before=0) {return EAddGroup(label,comment,where,before,eKET2DcellArea,4,x1,y1,x2,y2);}
	int EAdd3DAreaC(int *x1, int *y1, int *z1, int *x2, int *y2, int *z2, char *label, char *comment=0,int where=0, int before=0) {return EAddGroup(label,comment,where,before,eKET3DcellArea,6,x1,y1,z1,x2,y2,z2);}


private:
	void ECreateItem(void *data, int typ, char *label, char *comment); // univerzalni funkce pro vytvareni polozky
	int EAddGroup(char *label, char *comment,int where, int before, int type, int PocetClenu,...); // univerzalni funkce pro vytvareni skupiny
	struct CKerOVar *OVar;

	static char *GroupMemberLabels[6];
};


extern CKerEditI KerEditI;


#endif



