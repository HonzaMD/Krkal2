/////////////////////////////////////////////////////////////////////////////
//
// ComKerServices.h
//
// Popis sluzeb kernelu a jejich parametru pro automaticke zpracovani volani
//	sluzeb v interpretu a generovani jejich volani v kompilovanych skriptech
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////


#ifndef COM_COMKERSERVICES_H__INCLUDED

#define COM_COMKERSERVICES_H__INCLUDED


#include "ComConstants.h"

class CKerMapInfo;

// zaznam typu
struct SComKerRetTypeRec
{
	int type;			// typ
	int pointer;		// pointer
};

// zaznam parametru sluzby
struct SComKerParam
{
	int type;			// typ
	int pointer;		// pointer
	bool isDefault;		// true, pokud je parametr defaultni
	int specUse;		// 0 normal, 1 CodeLine, 2 this, 3 typ stringu
};

// zaznam sluzby kernelu
struct SComKerServiceRec
{
	SComKerRetTypeRec returnValue;	// navratovy typ a pointer
	const char* name;				// jmeno sluzby
	const char *compileToStr;		// retezec, ktery se ma objevit pri volani sluzby v kompilovanych skriptech
	int paramsNr;					// pocet parametru sluzby
};

// zaznam kerneli konstanty
struct SComKerConstant
{
	const char* name;		// jmeno
	int	value;				// hodnota
};

struct SComKerUCParamRec
{
	int service;
	int param;
};

// popis sluzeb kernelu - vsechny jejich zaznamy; pocet sluzeb cKerSevicesNr je definovan v ComConstants.h
const static SComKerServiceRec cKerServices[cKerServicesNr] = 
{
	{ {kwVoid, 0},	"LogUserError", "KerServices.LogUserError", 4 },	// void LogUserError(int Line, DWORD group, int info1 = 0, char *str = 0)
	{ {kwVoid, 0},	"LogDebugInfo", "KerServices.LogDebugInfo", 4 },	// void LogDebugInfo(int Line, DWORD num , int info1 = 0, char *str = 0) 

	{ {kwInt, 0},	"IsFNMemberOfSN", "KerServices.IsFMemberOfS", 2 },	// int IsFMemberOfS(CKerName *first, CKerName *second); 
	{ {kwInt, 0},	"IsFOMemberOfSN", "KerServices.IsFMemberOfS", 2 },	// int IsFMemberOfS(OPointer first_obj, CKerName *second); 
	{ {kwInt, 0},	"IsFNMemberOfSO", "KerServices.IsFMemberOfS", 2 },	// int IsFMemberOfS(CKerName *first, OPointer second_obj); 
	{ {kwInt, 0},	"IsFOMemberOfSO", "KerServices.IsFMemberOfS", 2 },	// int IsFMemberOfS(OPointer first_obj, OPointer second_obj); 

	{ {kwVoid, 0},	"InitMoveTo", "KerServices.InitMoveTo",	5 },	// void InitMoveTo(OPointer obj, UI time, int dx, int dy, int dz=0); 

	{ {kwVoid, 0},	"SetTag",	"KerServices.SetTag",	1},		// void SetTag(OPointer obj);
	{ {kwVoid, 0},	"ClearTag",	"KerServices.ClearTag",	1},		// void ClearTag(OPointer obj);
	{ {kwInt,0},	"CheckTag",	"KerServices.CheckTag",	1},		// int CheckTag(OPointer obj);
	{ {kwInt, 0},	"ExistsObj", "KerServices.ExistsObj",	1},		// int ExistsObj(OPointer obj);

	{ {kwVoid, 0},	"TerminateKernel", "KerMain->TerminateKernel",	0},		// void TerminateKernel()		
	{ {kwInt, 0},	"GetKernelTime", "KerMain->GetTime",	0},					// unsigned int  { return Time; }  // Vrati cas od startu kernelu
	{ {kwInt, 0},	"GetRunMode", "KerMain->GetRunMode",	0},					// int GetRunMode()

	{ {kwVoid, 0},	"RegisterMap", "KerMain->MapInfo->RegisterMap",	10},				//void RegisterMap(int CellType, int leftx, int lefty, int rightx, int righty, int lowerlevel, int upperlevel, int cellsizeX, int cellsizeY, int cellSizeZ);	//  Vola objekt mapa ve svem konstruktoru.
	{ {kwVoid, 0},	"MapGetNumberOfCells", "KerMain->MapInfo->GetNumberOfCells",	4},	//void GetNumberOfCells(int *sizex, int *sizey, int *startx, int *starty) { *sizex = NumCellsx; *sizey = NumCellsy; *startx = FirstCellx; *starty = FirstCelly; }	// vrati  indexy prvni bunky a pocty bunek. vse jen v osach x a y

	{ {kwVoid, 0},	"FindCollidingCells", "KerMain->MapInfo->CellColisionB",	5},		//	CKerArrInt * CellColisionB(OPointer Obj, CKerArrInt *OutPutBuff = 0, int dx=0, int dy=0, int dz=0); // vrati pole trojic souradnic x y z bunek do kterych objekt zasahuje. Funkce bud vyuzije OutPutBuff, nebo sveje vzdy stejne pole v tompripade nedealokovat
	{ {kwVoid, 0},	"DeleteOutOfMapCells", "KerServices.DeleteOutOfMapCalls",	1},		//	void DeleteOutOfMapCalls(CKerArrInt &Buff); // z pole odstrani bunky, ktere jsou mimo mapu
	{ {kwInt, 0},	"IsCellInMap", "KerMain->MapInfo->IsCellInMap",	3},					//  int IsCellInMap(int cx, int cy, int cz); // 1 Ano, 0 Ne
	{ {kwInt, 0},	"IsPixelInMap", "KerMain->MapInfo->IsPixelInMap",	3},				//  int IsPixelInMap(int x, int y, int z) { return FindCell(x,y,z,x,y,z); } // 1 Ano, 0 Ne
	{ {kwInt, 0},	"IsObjInCollision", "KerMain->MapInfo->IsObjInCollision",	5},		//  int IsObjInCollision(int Codeline, OPointer obj, int dx=0, int dy=0, int dz=0);  // vrati 1 pokud objekt koliduje, jinak 0. dx dy dz je relativni posun souradnic objektu., vrati 1 take pokud je obj mimo mapu
	{ {kwObjptrArray, 1},	"FindCollidingObjs", "KerMain->MapInfo->FindCollidingObjs",	6},	//  CKerArrObject *FindCollidingObjs(int CodeLine, OPointer obj, CKerArrObject *ret=0, int dx=0, int dy=0, int dz=0); // Vrati seznam objektu se kterymi objekt koliduje. dx dy dz je relativni posun souradnic objektu. V promenne ret muzes urcit, kam se bude vracet, jinak fce pole alokuje

	{ {kwInt, 0},	"IsObjInMap", "KerMain->MapInfo->IsObjInMap",	1},					//int IsObjInMap(OPointer ObjPtr) // zjisti zda je objekt umisten v mape
	{ {kwVoid, 0},	"PlaceObjToMap", "KerMain->MapInfo->PlaceObjToMap",	2},				//void PlaceObjToMap(int CodeLine, OPointer obj);	// umisti objekt do mapy
	{ {kwVoid, 0},	"PlaceObjToMapKill", "KerMain->MapInfo->PlaceObjToMapWithKill",	2},	//void PlaceObjToMapWithKill(int CodeLine, OPointer obj);	// pred umistenim znici vsechny kolidujici objekty. umisti objekt do mapy
	{ {kwVoid, 0},	"RemoveObjFromMap", "KerMain->MapInfo->RemoveObjFromMap",	2},		//void RemoveObjFromMap(int CodeLine, OPointer obj); // odebere objekt z mapy
	{ {kwVoid, 0},	"MoveObjTo", "KerMain->MapInfo->MoveObjTo",	5},						//void MoveObjTo(int CodeLine, OPointer obj, int x, int y, int z, int MoveGEnElemToo=1);  // okamzite posune objekt na nove souradnice. MoveGEnElemToo vyuziva kontroler plynuleho pohybu, nenastavovat
	{ {kwVoid, 0},	"MoveObjRel", "KerMain->MapInfo->MoveObjRel",	5},						//void MoveObjRel

	{ {kwInt, 0},	"FindObjCell", "KerServices.FindObjCell",	4},		//int FindObjCell(OPointer obj, int *cx, int *cy, int *cz) { // ze souradnic zjisti primarni bunku objektu. , vrati nulu v pripade chyby out of map
	{ {kwInt, 0},	"FindCell", "KerServices.FindCell",	6},			//int FindCell(int x, int y, int z, int *cx, int *cy, int *cz) { // zjisti do ktere bunky bod x,y,z patri. Vrati 0, jestlize jsi mimo mapu, jinak 1
	{ {kwVoid, 0},	"FindCellPosition", "KerServices.FindCellPosition",	6},	//void FindCellPosition(int cx, int cy, int cz, int *x, int *y, int *z) {  // Najde centralni pozici bunky
	{ {kwVoid, 0},	"ReadObjCoords", "KerServices.ReadObjCoords",	4},		//void ReadObjCoords(OPointer obj, int *x, int *y, int *z); // precte z objektu souradnice ma-li je (jinak vraci nuly). 
	{ {kwVoid, 0},	"WriteObjCoords", "KerMain->MapInfo->ChangeObjCoords",	5},	//void ChangeObjCoords(int CodeLine, OPointer obj, int x, int y, int z);	  // zmeni souradnice objektu, objekt bude pripadne premisten.	
		

	{ {kwDouble, 0},	"rand", "mtr.rand",	1},				// real random
	{ {kwDouble, 0}, "randExc", "mtr.randExc",	1},		// exl real random
	{ {kwInt, 0},	"randInt", "mtr.randInt",	1},			// int random	

	{ {kwVoid, 0},	"SLSeek", "KerSaveLoad.Seek",	1},			//void Seek(int pos);		// prace se streamem
	{ {kwVoid, 0},	"SLSeekToEnd", "KerSaveLoad.SeekToEnd",	0},	//void SeekToEnd();
	{ {kwInt, 0},	"SLGetPos", "KerSaveLoad.GetPos",	0},		//int GetPos();
	{ {kwInt, 0},	"SLEof", "KerSaveLoad.Eof",	0},				//int Eof();

	{ {kwVoid, 0},	"SaveInt", "KerSaveLoad.SaveInt",	1},		//void SaveInt(int a);
	{ {kwVoid, 0},	"SaveChar", "KerSaveLoad.SaveChar",	1},		//void SaveChar(UC a);
	{ {kwVoid, 0},	"SaveDouble", "KerSaveLoad.SaveDouble",	1},	//void SaveDouble(double a);
	{ {kwVoid, 0},	"SaveObjPtr", "KerSaveLoad.SaveObjPtr",	1},	//void SaveObjPtr(OPointer obj);
	{ {kwVoid, 0},	"SaveName", "KerSaveLoad.SaveName",	1},		//void SaveName(CKerName *name);
	{ {kwVoid, 0},	"SaveString", "KerSaveLoad.SaveString",	2},	//void SaveString(char *str, int Type);

	{ {kwVoid, 0},	"SaveIntA", "KerSaveLoad.SaveIntA",	1},			//void SaveIntA(CKerArrInt *a);
	{ {kwVoid, 0},	"SaveCharA", "KerSaveLoad.SaveCharA",	1},		//void SaveCharA(CKerArrChar *a);
	{ {kwVoid, 0},	"SaveDoubleA", "KerSaveLoad.SaveDoubleA",	1},	//void SaveDoubleA(CKerArrDouble *a);
	{ {kwVoid, 0},	"SaveObjPtrA", "KerSaveLoad.SaveObjPtrA",	1},	//void SaveObjPtrA(CKerArrObject *a);
	{ {kwVoid, 0},	"SaveNameA", "KerSaveLoad.SaveNameA",	1},		//void SaveNameA(CKerArrName *a);

	{ {kwInt, 0},	"LoadInt", "KerSaveLoad.LoadInt",	0},			//int LoadInt();
	{ {kwChar, 0},	"LoadChar", "KerSaveLoad.LoadChar",	0},			//UC LoadChar();
	{ {kwDouble, 0},"LoadDouble", "KerSaveLoad.LoadDouble",	0},		//double LoadDouble();
	{ {kwObjptr, 1},"LoadObjPtr", "KerSaveLoad.LoadObjPtr",	0},		//OPointer LoadObjPtr();
	{ {kwName, 1},	"LoadName", "KerSaveLoad.LoadName",	0},			//CKerName *LoadName();
	{ {kwVoid, 0},	"LoadString", "KerSaveLoad.LoadString",	2},	//void LoadString(char *str, int Type);

	{ {kwIntArray, 1},	"LoadIntA", "KerSaveLoad.LoadIntA",	0},			//CKerArrInt *LoadIntA();
	{ {kwCharArray, 1},	"LoadCharA", "KerSaveLoad.LoadCharA",	0},		//CKerArrChar *LoadCharA();
	{ {kwDoubleArray, 1},"LoadDoubleA", "KerSaveLoad.loadDoubleA",	0},	//CKerArrDouble *loadDoubleA();
	{ {kwObjptrArray, 1},"LoadObjPtrA", "KerSaveLoad.LoadObjPtrA",	0},	//CKerArrObject *LoadObjPtrA();
	{ {kwNameArray, 1},	"LoadNameA", "KerSaveLoad.LoadNameA",	0},		//CKerArrName *LoadNameA();
	
	{ {kwVoid, 0},	"ResetAuto", "KerServices.ResetAuto", 3},			// ResetAuto(OPointer obj, int ResetRandom=0, int ResetConnection=0);   // reset automatismu

	{ {kwVoid, 0},	"MvConnectObjs", "KerServices.MConnectObjs", 2},	// void MConnectObjs(OPointer obj1, OPointer obj2); // propoji objekty, tak ze se budou hejbat spolu. Plynuly pohyb se kopiruje z druhe skupiny na prvni
	{ {kwVoid, 0},	"MvDisconnectObj", "KerServices.MDisconnectObj", 1},// void MDisconnectObj(OPointer obj);	// odpoji objekt od skupiny
	{ {kwName, 1},	"GetActivAuto", "KerServices.GetActivAuto", 1},		// CKerName *GetActivAuto(OPointer obj) 
	{ {kwName, 1},	"GetDefaultAuto", "KerServices.GetDefaultAuto", 1},	// CKerName *GetDefaultAuto(OPointer obj)
	{ {kwName, 1},	"GetDefaultAuto2", "KerServices.GetDefaultAuto2",1},// CKerName *GetDefaultAuto2(CKerName *ObjType) 

	{ {kwVoid, 0},	"FindObjsInArea", "KerServices.FindObjsInArea", 9},	//void FindObjsInArea(int CodeLine, CKerArrObject *output,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // najde objekty, ktere jsou z mnoziny set, v bunkach na dane oblasti. set nemusi byt zadan.
	{ {kwVoid, 0},	"AreaCall", "KerServices.AreaCall", 9},				//void AreaCall(int CodeLine, CKerName *Method,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // naleznou se objekty v oblasti a zavola se na ne metoda
	{ {kwVoid, 0},	"AreaMessage", "KerServices.AreaMessage", 9},		//void AreaMessage(int CodeLine, CKerName *Method,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // naleznou se objekty v oblasti a zavola se na ne metoda

	{ {kwInt, 0},	"IsKeyDown", "Input->IsKeyDown", 1},		//
	{ {kwVoid, 0},	"GameVictory", "KRKAL->GameVictory", 0},		//

	{ {kwInt, 0},	"IsGame", "KerMain->IsGameMode", 0},			//int IsGameMode() { return if (RunMode==NORMAL_RUN); }
	{ {kwInt, 0},	"IsEditor", "KerMain->IsEditorMode", 0},		//int IsEditorMode() { return if (RunMode==EDITOR_RUN); }

	{ {kwVoid, 0},	"SetScrollCenter", "KerMain->MapInfo->SetScrollCenter", 2},	//void SetScrollCenter(int x, int y); // nascrolluje tak, aby bod x,y bul ve stredu
	{ {kwVoid, 0},	"WindowScroll", "KerMain->MapInfo->WindowScroll", 3},	//void WindowScroll(int dx, int dy,UI time); // zaplne plynule scrollovani okna
	{ {kwVoid, 0},	"CalcScrollDistance", "KerMain->MapInfo->CalcScrollDistance", 4},	//void CalcScrollDistance(int x, int y, int *dx, int *dy, int FromCenter=1); // Funkce zmensi vzdalenost dx a dy, pokud uz neni treba scrollovat.(jsem zarazen o okraj)

	{ {kwInt, 0},	"MnuSetBar", "CGameMenu::SetProgressBar", 2},		//static int SetProgressBar(int index, int progress);
	{ {kwInt, 0},	"MnuSetItem", "CGameMenu::ObjectSetCount", 2},		//static int ObjectSetCount(int index, int count);
	{ {kwVoid, 0},	"MnuRefresh", "CGameMenu::Refresh", 0},				//static void Refresh();
	{ {kwInt, 0},	"MnuDeleteItem", "CGameMenu::DeleteItem", 1},		//static int DeleteItem(int index);
	{ {kwInt, 0},	"MnuAddItemN", "CGameMenu::AddItem", 2},			//static int AddItem(CKerName* item, CKerName* key);
	{ {kwInt, 0},	"MnuAddItemO", "CGameMenu::AddItem", 2},			//static int AddItem(OPointer obj, CKerName* key);
	{ {kwInt, 0},	"MnuAddBars", "CGameMenu::Add2ProgressBars", 2},	//static int Add2ProgressBars(int color1, int color2);

	{ {kwDouble,0}, "sqrt", "sqrt",1},
	{ {kwInt,0}, "round", "KerServices.round",1},	// int round(double a) { return int(floor(a+0.5)); }

	{ {kwInt,0}, "PlaySound", "ME->PlaySample",4},	//int PlaySample( class CKerName *soundname, int x, int y, double volume=1, float pitch=1); //prehraje zvuk, volume je 0..1, x,y souradnice, kde ma zvuk hrat, pitch=zrychleni 0.5-2

	{ {kwVoid,0}, "ECreateInt", "KerEditI.ECreateInt",3},	//void ECreateInt(int *data, char *label, char *comment=0) {ECreateItem(data,eKTint,label,comment);}
	{ {kwVoid,0}, "ECreateChar", "KerEditI.ECreateChar",3},	//void ECreateChar(char *data, char *label, char *comment=0) {ECreateItem(data,eKTchar,label,comment);}
	{ {kwVoid,0}, "ECreateDouble", "KerEditI.ECreateDouble",3},	//void ECreateDouble(double *data, char *label, char *comment=0) {ECreateItem(data,eKTdouble,label,comment);}
	{ {kwVoid,0}, "ECreateName", "KerEditI.ECreateName",3},	//void ECreateName(CKerName **data, char *label, char *comment=0) {ECreateItem(data,eKTname,label,comment);}
	{ {kwVoid,0}, "ECreateObjPtr", "KerEditI.ECreateObjPtr",3},	//void ECreateObjPtr(OPointer *data, char *label, char *comment=0) {ECreateItem(data,eKTobject,label,comment);}

	{ {kwVoid,0}, "ECreateIntA", "KerEditI.ECreateIntA",3},	//void ECreateIntA(CKerArrInt **data, char *label, char *comment=0) {ECreateItem(data,eKTarrInt,label,comment);}
	{ {kwVoid,0}, "ECreateCharA", "KerEditI.ECreateCharA",3},	//void ECreateCharA(CKerArrChar **data, char *label, char *comment=0) {ECreateItem(data,eKTarrChar,label,comment);}
	{ {kwVoid,0}, "ECreateDoubleA", "KerEditI.ECreateDoubleA",3},	//void ECreateDoubleA(CKerArrDouble **data, char *label, char *comment=0) {ECreateItem(data,eKTarrDouble,label,comment);}
	{ {kwVoid,0}, "ECreateNameA", "KerEditI.ECreateNameA",3},	//void ECreateNameA(CKerArrName **data, char *label, char *comment=0) {ECreateItem(data,eKTarrName,label,comment);}
	{ {kwVoid,0}, "ECreateObjPtrA", "KerEditI.ECreateObjPtrA",3},	//void ECreateObjPtrA(CKerArrObject **data, char *label, char *comment=0) {ECreateItem(data,eKTarrObject,label,comment);}

	{ {kwInt,0}, "EPlaceItem", "KerEditI.EPlaceItem",2},	//int EPlaceItem(int where=0, int before=0); // umisteni vytvorene polozky

	{ {kwInt,0}, "ERefresh", "CGUIScriptVariableInterface::ScriptedVariableFinish",0},	//static int ScriptedVariableFinish();
	{ {kwInt,0}, "EAddGap", "CGUIScriptVariableInterface::AddGap",2},	//static int AddGap(int where=0, int before=0);
	{ {kwInt,0}, "EDeleteItem", "CGUIScriptVariableInterface::DeleteItem",1},	//static int DeleteItem(int index);
	{ {kwInt,0}, "EAddButton", "CGUIScriptVariableInterface::AddButton",6},	//static int AddButton(int where=0, int shift=0, char* label=0, char* help=0, int userID=0, CKerName* scriptFunction=0);
	{ {kwInt,0}, "EDeleteButton", "CGUIScriptVariableInterface::DeleteButton",1},	//static int DeleteButton(int buttonID);

	{ {kwInt,0}, "EAddGroupItem", "CGUIScriptVariableInterface::AddGroupItem",4},	//static int AddGroupItem(char* label, char* help, int where=0, int before=0);
	{ {kwInt,0}, "EDeleteGroupItem", "CGUIScriptVariableInterface::DeleteGroupItem",1},	//static int DeleteGroupItem(int groupIndex);
	{ {kwInt,0}, "ESelectGroupItem", "CGUIScriptVariableInterface::SelectGroupItem",1},	//static int SelectGroupItem(int groupIndex);
	{ {kwInt,0}, "EDeleteAllGroupItems", "CGUIScriptVariableInterface::DeleteAllGroupItems",2},	//static int DeleteAllGroupItems(int groupIndex, int buttonsLet=0);

	{ {kwInt,0}, "EAdd2DCell", "KerEditI.EAdd2DCell",6},	//int EAdd2DCell(int *x, int *y, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET2Dcell,2,x,y);}
	{ {kwInt,0}, "EAdd3DCell", "KerEditI.EAdd3DCell",7},	//int EAdd3DCell(int *x, int *y, int *z, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET3Dcell,3,x,y,z);}
	{ {kwInt,0}, "EAdd2DPoint", "KerEditI.EAdd2DPoint",6},	//int EAdd2DPoint(int *x, int *y, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET2Dpoint,2,x,y);}
	{ {kwInt,0}, "EAdd3DPoint", "KerEditI.EAdd3DPoint",7},	//int EAdd3DPoint(int *x, int *y, int *z, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET3Dpoint,3,x,y,z);}
	{ {kwInt,0}, "EAdd2DAreaP", "KerEditI.EAdd2DAreaP",8},	//int EAdd2DAreaP(int *x1, int *y1, int *x2, int *y2, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET2Darea,4,x1,y1,x2,y2);}
	{ {kwInt,0}, "EAdd3DAreaP", "KerEditI.EAdd3DAreaP",10},	//int EAdd3DAreaP(int *x1, int *y1, int *z1, int *x2, int *y2, int *z2, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET3Darea,6,x1,y1,z1,x2,y2,z2);}
	{ {kwInt,0}, "EAdd2DAreaC", "KerEditI.EAdd2DAreaC",8},	//int EAdd2DAreaC(int *x1, int *y1, int *x2, int *y2, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET2DcellArea,4,x1,y1,x2,y2);}
	{ {kwInt,0}, "EAdd3DAreaC", "KerEditI.EAdd3DAreaC",10},	//int EAdd3DAreaC(int *x1, int *y1, int *z1, int *x2, int *y2, int *z2, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET3DcellArea,6,x1,y1,z1,x2,y2,z2);}

	{ {kwInt,0}, "EAddText", "CGUIScriptVariableInterface::AddText",4},	//static int AddText(char* text, char* help=0, int where=0, int before=0);

	{ {kwVoid,0}, "ESetVarTags", "KerEditI.ESetVarTags",2},	//void ESetVarTags(int tags,int filter=-1); // nastavi NamesMask (filtr) a promennou Use (tags).

	{ {kwInt, 0},	"IsAnyKeyDown", "Input->KeyPressed", 0},		//
	
	{ {kwInt, 0},	"AddLight", "GEnMain->AddLight", 7},	//int AddLight(int x, int y, int z, UC ir, UC ig, UC ib, int radius ); //(x,y,z) pozice, (ir,ig,ib) intenzita, radius polomer
	{ {kwVoid, 0},	"DeleteLight", "GEnMain->DeleteLight", 1},	//void DeleteLight(int lightPtr);
	{ {kwVoid, 0},	"SetTopLightIntenzity", "GEnMain->SetTopLightIntenzity", 3},	//void SetTopLightIntenzity(UC r, UC g, UC b);
	{ {kwVoid, 0},	"GetTopLightIntenzity", "GEnMain->GetTopLightIntenzity", 3},	//void GetTopLightIntenzity(UC &r, UC &g, UC &b);

	{ {kwIntArray, 1},	"CopyIntA", "KerServices.CopyIntA", 1},	//CKerArrInt *CopyIntA(CKerArrInt *a);  
	{ {kwCharArray, 1},	"CopyCharA", "KerServices.CopyCharA", 1},	//CKerArrChar *CopyCharA(CKerArrChar *a);
	{ {kwDoubleArray, 1},	"CopyDoubleA", "KerServices.CopyDoubleA", 1},	//CKerArrDouble *CopyDoubleA(CKerArrDouble *a);
	{ {kwObjptrArray, 1},	"CopyObjPtrA", "KerServices.CopyObjPtrA", 1},	//CKerArrObject *CopyObjPtrA(CKerArrObject *a);
	{ {kwNameArray, 1},	"CopyNameA", "KerServices.CopyNameA", 1},	//CKerArrName *CopyNameA(CKerArrName *a);

	{ {kwObjptr, 1},	"CopyObject", "KerMain->CopyObject", 2},	//OPointer CopyObject(int CodeLine, OPointer object);  // vytvori objekt, zkopiruje do nej data, zavola copyconstructor, ktery by mel sparvne nastavit, zkopirovat pointrove retezce

	{ {kwInt, 0},	"DeleteMessages", "KerMain->FDeleteMessages", 3},	//int FDeleteMessages(OPointer Reciever, CKerName *msg=0, OPointer Sender=0); // Funkce vyhleda a smaze zpravy ze vsech front. Vrati pocet smazanych zprav.
	{ {kwInt, 0},	"CountMessages", "KerMain->FCountMessages", 3},	//int FCountMessages(OPointer Reciever, CKerName *msg=0, OPointer Sender=0); // Funkce spocita zpravy ve vsech frontach (vraci pocet).
	{ {kwVoid, 0},	"ESetEditType", "KerEditI.ESetEditType", 2},	//void ESetEditType(int CodeLine, int tags); // nastavi EditType (eKET..).

	{ {kwVoid, 0},	"SaveTheGame", "KerMain->SaveGame",	1},		
	{ {kwVoid, 0 }, "ShowLevelIntro", "KerServices.ShowLevelIntro", 2 },

	{ { kwInt, 0 }, "MnuSetManikPicture", "CGameMenu::SetManikPicture", 2 },			//static int SetManikPicture(OPointer obj);
};
 
// popis parametru sluzeb kernelu - vsechny jejich zaznamy; jejich pocet maximalni pocet (cKerSevicesMaxParams) je definovan v ComConstants.h
const static SComKerParam cKerServiceParams[cKerServicesNr][cKerServicesMaxParams] = 
{
	{	// void LogUserError(int Line, DWORD group, int info1 = 0, char *str = 0)
		{kwInt, 0, false, 1}, 
		{kwInt, 0, false, 0},
		{kwInt, 0, true, 0}, 
		{kwChar, 1, true, 0},
	}, 			

	{	// void LogDebugInfo(int Line, DWORD num, int info1 = 0, char *str = 0)
		{kwInt, 0, false, 1}, 
		{kwInt, 0, false, 0},
		{kwInt, 0, true, 0}, 
		{kwChar, 1, true, 0},
	}, 		

	{	// int IsFMemberOfS(CKerName *first, CKerName *second); 
		{kwName, 1, false, 0},
		{kwName, 1, false, 0},
	},

	{	// int IsFMemberOfS(OPointer first_obj, CKerName *second); 
		{kwObjptr, 1, false, 0},
		{kwName, 1, false, 0},
	}, 
	
	{	// int IsFMemberOfS(CKerName *first, OPointer second_obj); 
		{kwName, 1, false, 0},
		{kwObjptr, 1, false, 0},
	},

	{	// int IsFMemberOfS(OPointer first_obj, OPointer second_obj);
		{kwObjptr, 1, false, 0},
		{kwObjptr, 1, false, 0},
	}, 

	{	// void InitMoveTo(OPointer obj, UI time, int dx, int dy, int dz=0); 
		{kwObjptr, 1, false, 0},
		{kwInt, 0, false, 0},
		{kwInt, 0, false, 0},
		{kwInt, 0, false, 0},
		{kwInt, 0, true, 0},
	}, 

	{	// void SetTag(OPointer obj); 
		{kwObjptr, 1, false, 0},
	},

	{	// void ClearTag(OPointer obj); 
		{kwObjptr, 1, false, 0},
	},

	{	// int CheckTag(OPointer obj);
		{kwObjptr, 1, false, 0},
	},

	{	// int ExistsObj(OPointer obj); 
		{kwObjptr, 1, false, 0},
	},

	{
		{kwInt,0,false,0},
	},
	{
		{kwInt,0,false,0},
	},
	{
		{kwInt,0,false,0},
	},

	//void RegisterMap(int CellType, int leftx, int lefty, int rightx, int righty, int lowerlevel, int upperlevel, int cellsizeX, int cellsizeY, int cellSizeZ);	//  Vola objekt mapa ve svem konstruktoru.
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},

	//void GetNumberOfCells(int *sizex, int *sizey, int *startx, int *starty) { *sizex = NumCellsx; *sizey = NumCellsy; *startx = FirstCellx; *starty = FirstCelly; }	// vrati  indexy prvni bunky a pocty bunek. vse jen v osach x a y};
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
	},

	//	CKerArrInt * CellColisionB(OPointer Obj, CKerArrInt *OutPutBuff = 0, int dx=0, int dy=0, int dz=0); // vrati pole trojic souradnic x y z bunek do kterych objekt zasahuje. Funkce bud vyuzije OutPutBuff, nebo sveje vzdy stejne pole v tompripade nedealokovat
	{
		{kwObjptr,1,false,0},
		{kwIntArray,1,false,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},

	//	void DeleteOutOfMapCalls(CKerArrInt *Buff); // z pole odstrani bunky, ktere jsou mimo mapu
	{
		{kwIntArray,1,false,0},
	},

	//  int IsCellInMap(int cx, int cy, int cz); // 1 Ano, 0 Ne
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},

	//  int IsPixelInMap(int x, int y, int z) { return FindCell(x,y,z,x,y,z); } // 1 Ano, 0 Ne
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},

	//  int IsObjInCollision(int Codeline, OPointer obj, int dx=0, int dy=0, int dz=0);  // vrati 1 pokud objekt koliduje, jinak 0. dx dy dz je relativni posun souradnic objektu., vrati 1 take pokud je obj mimo mapu
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},

	//  CKerArrObject *FindCollidingObjs(int CodeLine, OPointer obj, CKerArrObject *ret=0, int dx=0, int dy=0, int dz=0); // Vrati seznam objektu se kterymi objekt koliduje. dx dy dz je relativni posun souradnic objektu. V promenne ret muzes urcit, kam se bude vracet, jinak fce pole alokuje
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
		{kwObjptrArray,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},

	//int IsObjInMap(OPointer ObjPtr) // zjisti zda je objekt umisten v mape
	{
		{kwObjptr,1,false,0},
	},

	//void PlaceObjToMap(int CodeLine, OPointer obj);	// umisti objekt do mapy
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
	},

	//void PlaceObjToMapWithKill(int CodeLine, OPointer obj);	// pred umistenim znici vsechny kolidujici objekty. umisti objekt do mapy
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
	},

	//void RemoveObjFromMap(int CodeLine, OPointer obj); // odebere objekt z mapy
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
	},

	//void MoveObjTo(int CodeLine, OPointer obj, int x, int y, int z, int MoveGEnElemToo=1);  // okamzite posune objekt na nove souradnice. MoveGEnElemToo vyuziva kontroler plynuleho pohybu, nenastavovat
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},

	//void MoveObjRel
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},

	//int FindObjCell(OPointer obj, int *cx, int *cy, int *cz) { // ze souradnic zjisti primarni bunku objektu. , vrati nulu v pripade chyby out of map
	{
		{kwObjptr,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
	},

	//int FindCell(int x, int y, int z, int *cx, int *cy, int *cz) { // zjisti do ktere bunky bod x,y,z patri. Vrati 0, jestlize jsi mimo mapu, jinak 1
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
	},

	//void FindCellPosition(int cx, int cy, int cz, int *x, int *y, int *z) {  // Najde centralni pozici bunky
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
	},

	//void ReadObjCoords(OPointer obj, int *x, int *y, int *z); // precte z objektu souradnice ma-li je (jinak vraci nuly). 
	{
		{kwObjptr,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
	},

	//{ {kwVoid, 0},	"WriteObjCoords", "KerMain->MapInfo->ChangeObjCoords",	5},	//void ChangeObjCoords(int CodeLine, OPointer obj, int x, int y, int z);	  // zmeni souradnice objektu, objekt bude pripadne premisten.	
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,true,0},
	},

	// real random
	{
		{kwDouble,0,false,0},
	},

	// excl real random
	{
		{kwDouble,0,false,0},
	},

	// int random	
	{
		{kwInt,0,false,0},
	},

	//void Seek(int pos);		// prace se streamem
	{
		{kwInt,0,false,0},
	},

	//void SeekToEnd();
	{
		{kwInt,0,false,0},
	},

	//int GetPos();
	{
		{kwInt,0,false,0},
	},

	//int Eof();
	{
		{kwInt,0,false,0},
	},


	//void SaveInt(int a);
	{
		{kwInt,0,false,0},
	},

	//void SaveChar(UC a);
	{
		{kwChar,0,false,0},
	},

	//void SaveDouble(double a);
	{
		{kwDouble,0,false,0},
	},

	//void SaveObjPtr(OPointer obj);
	{
		{kwObjptr,1,false,0},
	},

	//void SaveName(CKerName *name);
	{
		{kwName,1,false,0},
	},

	//void SaveString(char *str, int Type);
	{
		{kwString,1,false,0},
		{kwInt,0,false,3},
	},


	//void SaveIntA(CKerArrInt *a);
	{
		{kwIntArray,1,false,0},
	},

	//void SaveCharA(CKerArrChar *a);
	{
		{kwCharArray,1,false,0},
	},

	//void SaveDoubleA(CKerArrDouble *a);
	{
		{kwDoubleArray,1,false,0},
	},

	//void SaveObjPtrA(CKerArrObject *a);
	{
		{kwObjptrArray,1,false,0},
	},

	//void SaveNameA(CKerArrName *a);
	{
		{kwNameArray,1,false,0},
	},


	//int LoadInt();
	{
		{kwVoid,0,false,0},
	},

	//UC LoadChar();
	{
		{kwVoid,0,false,0},
	},

	//double LoadDouble();
	{
		{kwVoid,0,false,0},
	},

	//OPointer LoadObjPtr();
	{
		{kwVoid,0,false,0},
	},

	//CKerName *LoadName();
	{
		{kwVoid,0,false,0},
	},

	//void LoadString(char *str, int Type);
	{
		{kwString,1,false,0},
		{kwInt,0,false,3},
	},


	//CKerArrInt *LoadIntA();
	{
		{kwVoid,0,false,0},
	},

	//CKerArrChar *LoadCharA();
	{
		{kwVoid,0,false,0},
	},

	//CKerArrDouble *loadDoubleA();
	{
		{kwVoid,0,false,0},
	},

	//CKerArrObject *LoadObjPtrA();
	{
		{kwVoid,0,false,0},
	},

	//CKerArrName *LoadNameA();
	{
		{kwVoid,0,false,0},
	},
	
	//	void ResetAuto(OPointer obj, int ResetRandom=0, int ResetConnection=0);   // reset automatismu
	{
		{kwObjptr,1,false,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	
	//void MConnectObjs(OPointer obj1, OPointer obj2); // propoji objekty, tak ze se budou hejbat spolu. Plynuly pohyb se kopiruje z druhe skupiny na prvni
	{
		{kwObjptr,1,false,0},
		{kwObjptr,1,false,0},
	},
	//void MDisconnectObj(OPointer obj);	// odpoji objekt od skupiny
	{
		{kwObjptr,1,false,0},
	},
	//CKerName *GetActivAuto(OPointer obj) 
	{
		{kwObjptr,1,false,0},
	},
	//CKerName *GetDefaultAuto(OPointer obj)
	{
		{kwObjptr,1,false,0},
	},
	//CKerName *GetDefaultAuto2(CKerName *ObjType) 
	{
		{kwName,1,false,0},
	},

	//void FindObjsInArea(int CodeLine, CKerArrObject *output,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // najde objekty, ktere jsou z mnoziny set, v bunkach na dane oblasti. set nemusi byt zadan.
	{
		{kwInt,0,false,1},
		{kwObjptrArray,1,false,0},
		{kwObjptr,1,false,0},
		{kwName,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//void AreaCall(int CodeLine, CKerName *Method,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // naleznou se objekty v oblasti a zavola se na ne metoda
	{
		{kwInt,0,false,1},
		{kwName,1,false,0},
		{kwObjptr,1,false,0},
		{kwName,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//void AreaMessage(int CodeLine, CKerName *Method,OPointer obj, CKerName *set=0, int x1=0, int y1=0, int x2=0, int y2=0, int z=0); // naleznou se objekty v oblasti a zavola se na ne metoda
	{
		{kwInt,0,false,1},
		{kwName,1,false,0},
		{kwObjptr,1,false,0},
		{kwName,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	
	//	{ {kwInt, 0},	"IsKeyDown", "Input->IsKeyDown", 1},		//
	{
		{kwName,1,false,0},
	},

	//{ {kwVoid, 0},	"GameVictory", "KerMain->TerminateKernel", 1},		//
	{
		{kwVoid,0,false,0},
	},

	//{ {kwInt, 0},	"IsGameMode", "KerMain->IsGameMode", 0},			//int IsGameMode() { return if (RunMode==NORMAL_RUN); }
	{
		{kwVoid,0,false,0},
	},
	//{ {kwInt, 0},	"IsEditorMode", "KerMain->IsEditorMode", 0},		//int IsEditorMode() { return if (RunMode==EDITOR_RUN); }
	{
		{kwVoid,0,false,0},
	},

	//{ {kwVoid, 0},	"SetScrollCenter", "KerMain->MapInfo->SetScrollCenter", 2},	//void SetScrollCenter(int x, int y); // nascrolluje tak, aby bod x,y bul ve stredu
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},
	//{ {kwVoid, 0},	"WindowScroll", "KerMain->MapInfo->WindowScroll", 3},	//void WindowScroll(int dx, int dy,UI time); // zaplne plynule scrollovani okna
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},
	//{ {kwVoid, 0},	"CalcScrollDistance", "KerMain->MapInfo->CalcScrollDistance", 4},	//void CalcScrollDistance(int x, int y, int *dx, int *dy, int FromCenter=1); // Funkce zmensi vzdalenost dx a dy, pokud uz neni treba scrollovat.(jsem zarazen o okraj)
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
	},

	//{ {kwInt, 0},	"MnuSetBar", "CGameMenu::SetProgressBar", 2},		//static int SetProgressBar(int index, int progress);
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},
	//{ {kwInt, 0},	"MnuSetItem", "CGameMenu::ObjectSetCount", 2},		//static int ObjectSetCount(int index, int count);
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},
	//{ {kwVoid, 0},	"MnuRefresh", "CGameMenu::Refresh", 0},				//static void Refresh();
	{
		{kwVoid,0,false,0},
	},
	//{ {kwInt, 0},	"MnuDeleteItem", "CGameMenu::DeleteItem", 1},		//static int DeleteItem(int index);
	{
		{kwInt,0,false,0},
	},
	//{ {kwInt, 0},	"MnuAddItemN", "CGameMenu::AddItem", 1},			//static int AddItem(CKerName* item);
	{
		{kwName,1,false,0},
		{ kwName, 1, true, 0 },
	},
	//{ {kwInt, 0},	"MnuAddItemO", "CGameMenu::AddItem", 1},			//static int AddItem(OPointer obj);
	{
		{kwObjptr,1,false,0},
		{ kwName, 1, true, 0 },
	},
	//{ {kwInt, 0},	"MnuAddBars", "CGameMenu::Add2ProgressBars", 2},	//static int Add2ProgressBars(int color1, int color2);
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
	},

	// { {kwDouble,0}, "sqrt", "sqrt",1},
	{
		{kwDouble,0,false,0},
	},
	//{ {kwInt,0}, "round", "KerServices.round",1},	// int round(double a) { return int(floor(a+0.5)); }
	{
		{kwDouble,0,false,0},
	},

	//{ {kwInt,0}, "PlaySound", "ME->PlaySample",4},	int PlaySample( class CKerName *soundname, int x, int y, double volume=1, float pitch=1); //prehraje zvuk, volume je 0..1, x,y souradnice, kde ma zvuk hrat, pitch=zrychleni 0.5-2
	{
		{kwName,1,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwDouble,0,true,0},
	},


	//{ {kwVoid,0}, "ECreateInt", "KerEditI.ECreateInt",3},	//void ECreateInt(int *data, char *label, char *comment=0) {ECreateItem(data,eKTint,label,comment);}
	{
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},
	//{ {kwVoid,0}, "ECreateChar", "KerEditI.ECreateChar",3},	//void ECreateChar(char *data, char *label, char *comment=0) {ECreateItem(data,eKTchar,label,comment);}
	{
		{kwChar,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},
	//{ {kwVoid,0}, "ECreateDouble", "KerEditI.ECreateDouble",3},	//void ECreateDouble(double *data, char *label, char *comment=0) {ECreateItem(data,eKTdouble,label,comment);}
	{
		{kwDouble,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},
	//{ {kwVoid,0}, "ECreateName", "KerEditI.ECreateName",3},	//void ECreateName(CKerName **data, char *label, char *comment=0) {ECreateItem(data,eKTname,label,comment);}
	{
		{kwName,2,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},
	//{ {kwVoid,0}, "ECreateObjPtr", "KerEditI.ECreateObjPtr",3},	//void ECreateObjPtr(OPointer *data, char *label, char *comment=0) {ECreateItem(data,eKTobject,label,comment);}
	{
		{kwObjptr,2,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},

	//{ {kwVoid,0}, "ECreateIntA", "KerEditI.ECreateIntA",3},	//void ECreateIntA(CKerArrInt **data, char *label, char *comment=0) {ECreateItem(data,eKTarrInt,label,comment);}
	{
		{kwIntArray,2,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},
	//{ {kwVoid,0}, "ECreateCharA", "KerEditI.ECreateCharA",3},	//void ECreateCharA(CKerArrChar **data, char *label, char *comment=0) {ECreateItem(data,eKTarrChar,label,comment);}
	{
		{kwCharArray,2,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},
	//{ {kwVoid,0}, "ECreateDoubleA", "KerEditI.ECreateDoubleA",3},	//void ECreateDoubleA(CKerArrDouble **data, char *label, char *comment=0) {ECreateItem(data,eKTarrDouble,label,comment);}
	{
		{kwDoubleArray,2,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},
	//{ {kwVoid,0}, "ECreateNameA", "KerEditI.ECreateNameA",3},	//void ECreateNameA(CKerArrName **data, char *label, char *comment=0) {ECreateItem(data,eKTarrName,label,comment);}
	{
		{kwNameArray,2,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},
	//{ {kwVoid,0}, "ECreateObjPtrA", "KerEditI.ECreateObjPtrA",3},	//void ECreateObjPtrA(CKerArrObject **data, char *label, char *comment=0) {ECreateItem(data,eKTarrObject,label,comment);}
	{
		{kwObjptrArray,2,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
	},

	// { {kwInt,0}, "EPlaceItem", "KerEditI.EPlaceItem",2},	//int EPlaceItem(int where=0, int before=0); // umisteni vytvorene polozky
	{
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},

	//{ {kwInt,0}, "ERefresh", "CGUIScriptVariableInterface::ScriptedVariableFinish",0},	//static int ScriptedVariableFinish();
	{
		{kwVoid,0,false,0},
	},
	//{ {kwInt,0}, "EAddGap", "CGUIScriptVariableInterface::AddGap",2},	//static int AddGap(int where=0, int before=0);
	{
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EDeleteItem", "CGUIScriptVariableInterface::DeleteItem",1},	//static int DeleteItem(int index);
	{
		{kwInt,0,false,0},
	},
	//{ {kwInt,0}, "EAddButton", "CGUIScriptVariableInterface::AddButton",6},	//static int AddButton(int where=0, int shift=0, char* label=0, char* help=0, int userID=0, CKerName* scriptFunction=0);
	{
		{kwInt,0,true,0},
		{kwInt,0,true,0},
		{kwChar,1,true,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwName,1,true,0},
	},
	//{ {kwInt,0}, "EDeleteButton", "CGUIScriptVariableInterface::DeleteButton",1},	//static int DeleteButton(int buttonID);
	{
		{kwInt,0,false,0},
	},

	//{ {kwInt,0}, "EAddGroupItem", "CGUIScriptVariableInterface::AddGroupItem",4},	//static int AddGroupItem(char* label, char* help, int where=0, int before=0);
	{
		{kwChar,1,false,0},
		{kwChar,1,false,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EDeleteGroupItem", "CGUIScriptVariableInterface::DeleteGroupItem",1},	//static int DeleteGroupItem(int groupIndex);
	{
		{kwInt,0,false,0},
	},
	//{ {kwInt,0}, "ESelectGroupItem", "CGUIScriptVariableInterface::SelectGroupItem",1},	//static int SelectGroupItem(int groupIndex);
	{
		{kwInt,0,false,0},
	},
	//{ {kwInt,0}, "EDeleteAllGroupItems", "CGUIScriptVariableInterface::DeleteAllGroupItems",2},	//static int DeleteAllGroupItems(int groupIndex, int buttonsLet=0);
	{
		{kwInt,0,false,0},
		{kwInt,0,true,0},
	},

	//{ {kwInt,0}, "EAdd2DCell", "KerEditI.EAdd2DCell",6},	//int EAdd2DCell(int *x, int *y, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET2Dcell,2,x,y);}
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EAdd3DCell", "KerEditI.EAdd3DCell",7},	//int EAdd3DCell(int *x, int *y, int *z, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET3Dcell,3,x,y,z);}
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EAdd2DPoint", "KerEditI.EAdd2DPoint",6},	//int EAdd2DPoint(int *x, int *y, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET2Dpoint,2,x,y);}
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EAdd3DPoint", "KerEditI.EAdd3DPoint",7},	//int EAdd3DPoint(int *x, int *y, int *z, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET3Dpoint,3,x,y,z);}
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EAdd2DAreaP", "KerEditI.EAdd2DAreaP",8},	//int EAdd2DAreaP(int *x1, int *y1, int *x2, int *y2, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET2Darea,4,x1,y1,x2,y2);}
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EAdd3DAreaP", "KerEditI.EAdd3DAreaP",10},	//int EAdd3DAreaP(int *x1, int *y1, int *z1, int *x2, int *y2, int *z2, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET3Darea,6,x1,y1,z1,x2,y2,z2);}
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EAdd2DAreaC", "KerEditI.EAdd2DAreaC",8},	//int EAdd2DAreaC(int *x1, int *y1, int *x2, int *y2, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET2DcellArea,4,x1,y1,x2,y2);}
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},
	//{ {kwInt,0}, "EAdd3DAreaC", "KerEditI.EAdd3DAreaC",10},	//int EAdd3DAreaC(int *x1, int *y1, int *z1, int *x2, int *y2, int *z2, char *label, char *comment=0,int where=0, int before=0) {EAddGroup(label,comment,where,before,eKET3DcellArea,6,x1,y1,z1,x2,y2,z2);}
	{
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwInt,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},

	// { {kwInt,0}, "EAddText", "CGUIScriptVariableInterface::AddText",4},	//static int AddText(char* text, char* help=0, int where=0, int before=0);
	{
		{kwChar,1,false,0},
		{kwChar,1,true,0},
		{kwInt,0,true,0},
		{kwInt,0,true,0},
	},

	//{ {kwVoid,0}, "ESetVarTags", "KerEditI.ESetVarTags",2},	//void ESetVarTags(int tags,int filter=-1); // nastavi NamesMask (filtr) a promennou Use (tags).
	{
		{kwInt,0,false,0},
		{kwInt,0,true,0},
	},

	//{ {kwInt, 0},	"IsAnyKeyDown", "Input->KeyPressed", 0},		//
	{
		{kwVoid,0,false,0},
	},
	//{ {kwInt, 0},	"AddLight", "GEnMain->AddLight", 7},	//int AddLight(int x, int y, int z, UC ir, UC ig, UC ib, int radius ); //(x,y,z) pozice, (ir,ig,ib) intenzita, radius polomer
	{
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwInt,0,false,0},
		{kwChar,0,false,0},
		{kwChar,0,false,0},
		{kwChar,0,false,0},
		{kwInt,0,false,0},
	},
	//{ {kwVoid, 0},	"DeleteLight", "GEnMain->DeleteLight", 1},	//void DeleteLight(int lightPtr);
	{
		{kwInt,0,false,0},
	},

	//{ {kwVoid, 0},	"SetTopLightIntenzity", "GEnMain->SetTopLightIntenzity", 3},	//void SetTopLightIntenzity(UC r, UC g, UC b);
	{
		{kwChar,0,false,0},
		{kwChar,0,false,0},
		{kwChar,0,false,0},
	},
	//{ {kwVoid, 0},	"GetTopLightIntenzity", "GEnMain->GetTopLightIntenzity", 3},	//void GetTopLightIntenzity(UC &r, UC &g, UC &b);
	{
		{kwChar,1,false,0},
		{kwChar,1,false,0},
		{kwChar,1,false,0},
	},

	//{ {kwIntArray, 1},	"CopyIntA", "KerServices.CopyIntA", 1},	//CKerArrInt *CopyIntA(CKerArrInt *a);  
	{
		{kwIntArray,1,false,0},
	},
	//{ {kwCharArray, 1},	"CopyCharA", "KerServices.CopyCharA", 1},	//CKerArrChar *CopyCharA(CKerArrChar *a);
	{
		{kwCharArray,1,false,0},
	},
	//{ {kwDoubleArray, 1},	"CopyDoubleA", "KerServices.CopyDoubleA", 1},	//CKerArrDouble *CopyDoubleA(CKerArrDouble *a);
	{
		{kwDoubleArray,1,false,0},
	},
	//{ {kwObjptrArray, 1},	"CopyObjPtrA", "KerServices.CopyObjPtrA", 1},	//CKerArrObject *CopyObjPtrA(CKerArrObject *a);
	{
		{kwObjptrArray,1,false,0},
	},
	//{ {kwNameArray, 1},	"CopyNameA", "KerServices.CopyNameA", 1},	//CKerArrName *CopyNameA(CKerArrName *a);
	{
		{kwNameArray,1,false,0},
	},

	//{ {kwObjptr, 1},	"CopyObject", "KerMain->CopyObject", 2},	//OPointer CopyObject(int CodeLine, OPointer object);  // vytvori objekt, zkopiruje do nej data, zavola copyconstructor, ktery by mel sparvne nastavit, zkopirovat pointrove retezce
	{
		{kwInt,0,false,1},
		{kwObjptr,1,false,0},
	},

	//{ {kwInt, 0},	"DeleteMessages", "KerMain->FDeleteMessages", 3},	//int FDeleteMessages(OPointer Reciever, CKerName *msg=0, OPointer Sender=0); // Funkce vyhleda a smaze zpravy ze vsech front. Vrati pocet smazanych zprav.
	{
		{kwObjptr,1,false,0},
		{kwName,1,true,0},
		{kwObjptr,1,true,0},
	},
	//{ {kwInt, 0},	"CountMessages", "KerMain->FCountMessages", 3},	//int FCountMessages(OPointer Reciever, CKerName *msg=0, OPointer Sender=0); // Funkce spocita zpravy ve vsech frontach (vraci pocet).
	{
		{kwObjptr,1,false,0},
		{kwName,1,true,0},
		{kwObjptr,1,true,0},
	},

	//{ {kwVoid, 0},	"ESetEditType", "KerEditI.ESetEditType", 2},	//void ESetEditType(int CodeLine, int tags); // nastavi EditType (eKET..).
	{
		{kwInt,0,false,1},
		{kwInt,0,false,0},
	},

	// { {kwVoid, 0},	"SaveTheGame", "KerMain->SaveGame",	1},	
	{
		{kwChar,1,true,0},
	},

	//	{ { kwVoid, 0 }, "ShowLevelIntro", "MMDesktop->ShowLevelIntro", 2 },
	{
		{ kwChar, 1, false, 0 },
		{ kwChar, 1, true, 0 },
	},

	//	{ { kwInt, 0 }, "MnuSetManikPicture", "CGameMenu::SetManikPicture", 1 },			//static int SetManikPicture(OPointer obj);
	{
		{ kwObjptr, 1, false, 0 },
		{ kwName, 1, true, 0 },
	},

};


// popis konstant kernelu - vsechny jejich zaznamy; pocet konstant cKerConstantsNr je definovan v ComConstants.h
const static SComKerConstant cKerConstants[cKerConstantsNr] = 
{
	{"_KN_testConst", 777},
	
	{"_KN_eKCCpoint", 0 },
	{"_KN_eKCConeCell", 1},
	{"_KN_eKCCrect1", 2},
	{"_KN_eKCCrect2", 3},
	{"_KN_eKCChexCircle", 4},
	{"_KN_eKCCcolCube", 5},
	{"_KN_eKCCoutOfMap", 7},
	{"_KN_eKCCtriggerBit",16},
	{"_KN_eKCCwall",32},
	{"_KN_eKCCfloor",64},
	{"_KN_eKCCcell",32+64},
	{"_KN_eKCCnothing", 0}, // kolize je s mapou, ale neni s zadnymi objekty
	{"_KN_eKCCcenterColBit",128},
	{"_KN_eKCCinvisible",128},   // pro netrigery. Objekt je neviditelny
	

	{"_KN_NORMAL_RUN", 0},
	{"_KN_EDITOR_RUN", 1},
	
	{"_KN_eKCTctverce", 0},
	{"_KN_eKCTkosoctverce", 1},
	{"_KN_eKCTplocheHexy", 2},
	{"_KN_eKCTspicateHexy", 3},  

	{"_KN_eKEGPanicError",0},
	{"_KN_eKEGFatalError",1},
	{"_KN_eKEGError",2},
	{"_KN_eKEGWarning",3},
	{"_KN_eKEGInfo",4},
	{"_KN_eKEGParamConversionError",5},
	{"_KN_eKEGParamAssignationError",6},
	{"_KN_eKEGCallingError",7},
	{"_KN_eKEGDebug",8},
	{"_KN_eKEGMapError",9},

	{"_KN_eKVUBexclusive",32},
	{"_KN_eKVUBplannarNames",64},
	{"_KN_eKVUBincludeNull",128},
	{"_KN_eKVUBobjInMap",256},
	{"_KN_eKVUBobjOutMap",512},

	{"_KN_eEdNTvoid",1},
	{"_KN_eEdNTobject",2},
	{"_KN_eEdNTmethod",4},
	{"_KN_eEdNTparam",8},
	{"_KN_eEdNTauto",16},
	{"_KN_eEdNTobjectShadow",32},
	{"_KN_eEdNTkey",64},
	{"_KN_eEdNTsound",128},
	{"_KN_eEdNTobjVoid",256},
	{"_KN_eEdNTautoVoid",512},
	{"_KN_eEdNTeverything",eEdNTeverything },

	{"_KN_eKLCnone",0},
	{"_KN_eKLCinterval",1},
	{"_KN_eKLClist",2},
	{"_KN_eKLCup",3},
	{"_KN_eKLCdown",4},

	{"_KN_eKETdefault",0}, 
	{"_KN_eKETnumeric",1},
	{"_KN_eKETcharacter",2},
	{"_KN_eKETboolean",3},
	{"_KN_eKETconnectionMask",4},
	{"_KN_eKETautomaticGr",5},
	{"_KN_eKETstring",7},


};

const int cKerUCParams = 4;

const static SComKerUCParamRec cKerUCParamList[cKerUCParams] = 
{
	{122, 0},			// GetTopLightIntensity
	{122, 1},			
	{122, 2},			
	{89, 0},			// ECreateChar
};

bool static isUCParam(int service, int param)
{
	int i;
	for(i = 0; i < cKerUCParams; i++)
		if(cKerUCParamList[i].service == service && cKerUCParamList[i].param == param)
			return true;

	return false;
}

#endif