///////////////////////////////////////////////
//
// editor.h
//
// Interface pro hlavni objekt editoru
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef ED_EDITOR_H
#define ED_EDITOR_H

#include "gui.h"

class CEDEditor;
class CEDEditorIconHandler;
struct CEDPosition;
struct CEDShortCut;
struct CKerObject;

typedef DWORD OPointer;  // typ Pointer na Objekt


extern CEDEditor* editor;	// globalni objekt editor


//////////////////////////////////////////////////////////////////////
// CEDPosition
//////////////////////////////////////////////////////////////////////

// struktura pro vymezeni mista na plose pro okna
// udava polohu a velikost vymezene oblasti

struct CEDPosition
{
	CEDPosition() { x=y=sx=sy=0;};
	CEDPosition(float _x, float _y, float _sx, float _sy) { x=_x; y=_y; sx=_sx; sy=_sy; };
	void SetPosition(float _x, float _y, float _sx, float _sy) { x=_x; y=_y; sx=_sx; sy=_sy; };
	
	float x,y;
	float sx,sy;
};

//////////////////////////////////////////////////////////////////////
// CEDEditor
//////////////////////////////////////////////////////////////////////

// hlavni editacni mody editoru
// kazdy mod muze mit jeste svuj podmod (upresneni chovani)

enum EEditModeType {
	eEMnormal = 0,
	eEMinsert = 1,		// vklada vybrany objekt do mapy / seznamu globalnich objektu
	eEMselect = 2,		// vybira objekt z mapy / seznamu globalnich objektu
	eEMdelete = 3,		// maze objekt z mapy / seznamu globalnich objektu
	eEMselectCell = 4,	// vybira objekty na bunce v mape
	eEMnoConnect = 5,	// zobrazuje dialog pro editaci noConnect atributu vybraneho objektu

	eEMselectObj = 6,	// vybira objekt objekt z mapy / seznamu globalnich objektu a uklada ho do promenne editovaneho objektu
	eEMmoveObj = 7,		// presouva vybrany objekt

	eEMselectLocation = 8	// vybira misto z mapy a uklada ho do skupinove polozky (nekolik promennych)
};

class CEDEditor
{
	friend class CEDEditorIconHandler;
	friend class CEDObjectProperty;
public:
	CEDEditor();
	~CEDEditor();

	void Init(char* level=0, char* script=0);
		// inicializuje editor
		// nastartuje zadany "level" nebo "script"
		// pokud zadny level neni zadany, spusti jen samotny prazdny editor
	void InitScript(char *script);	// inicializuje editor scriptem	(nastavi veskere okna, parametry atp. editoru)
	void InitLevel(char *level);	// inicializuje editor levelem (nastavi veskere okna, parametry atp. editoru)
	void ClearLevel();				// vyprazdni prave editovany level (restartuje kernel se spravnym skriptem a nastavi cestu k levelu)

	void LoadScript(char *script);	// nastartuje kernel se skriptem
	void LoadLevel(char *level);	// nastartuje kernel s levelem
	
	void LoadGlobalObjects();	// nahraje globalni objekty levelu z kerneliho pole do pole "globalObjects"
	void LoadShortCuts();		// vytvori shortcuty, ktere jsou umisteny v poli shortCutArray, umisti je do stromu objektu - objectList

	void CreateObjectTree();	
		// vytvori strom se seznamem objektu pro editaci do levelu - objectList
		// prida ho do desktopu na vymezenou pozici
	void CreateObjectBrowser();
		// vytvori ObjectBrowser, tj. prochazec objektu ve forme stromu
		// u kazdeho objektu je seznam jeho metod a atributu
		// jen informaticni prvek
		// std. se v editoru nepouziva

	void CreateGameWindow();	// vytvori herni okno pro mapu a nastavi ho podle paramteru levelu/skritu
		
	void DeleteGameWindow();	// zrusi herni okno

	void SaveLevel();	// uozi prave editovany level

	void CloseLevel();	// zavre vsechny nastroje tykajici se levelu a ukonci kernel

	void AddLevelObject(OPointer obj);	
		// prida objekt do seznamu vsech objektu levelu, ktere se budou ukladat
		// pokud objekt v seznamu neni nebude ulozen a tedy ani nebude vytvoren po naloudovani levelu
	int AddGlobalObject(OPointer obj);
		// prida objekt do seznamu globalnich objektu levelu
		// objekt se zobrazi jako nastroj v toolbaru
		// v pripade nutnosti zvetsi seznam
		
	int RemoveGlobalObject(OPointer obj);
		// odebere objekt ze seznamu globalnich objektu levelu
		// zrusi nastroj toolbaru reprzentujici objekt

	void CreateEditorPropertyWindow();
		// vytvori toolbar s nastroji na ovladani editacniho modu editoru

	void CreateGlobalObjectsToolBar();
		// vytvori toolbar pro seznam globalnich objektu levelu
		// umisti do toolbaru vsechny objekty z pole "globalObjects"
		

	void CreateDebugWindow();
		// vytvori kerneli debugovaci okno s prubeznymi vypisy o akcich provadenych nad kernelem

	void RefreshMapVP(int dx, int dy);
		// aktualizuje pozici VP (viewPort) do mapy (relativni scrolling)
		// kdyz kernel scrolluje s hernim oknem, musi informovat editor o zmene nascrollovani okna
		// synchronizace scrollovani oknem - kernel vs. GUI
	void RefreshMapVPabs(int vx, int vy);
		// aktualizuje pozici VP (viewPort) do mapy (absolutni scrolling)
		// kdyz kernel scrolluje s hernim oknem, musi informovat editor o zmene nascrollovani okna
		// synchronizace scrollovani oknem - kernel vs. GUI

	void RefreshMapPosition();
		// nastavi (aktualizuje) pozici a velikost mapy podle volne oblasti pro mapu => mapPosition
		// vola se po zmene velikosti mapy, pridavani oken do editoru, ktere zasahuji do oblasti pro mapu atp.

	void KernelShutDown();	
		// kernel vypnut => upravit podle toho editor
		// musi probehnout konsolidace editoru
		// vola se napr. po prichodu vyjimky z kernelu

	CEDEditorIconHandler* iconDefaultHandler;	// spravce pro ovladani editacnich modu editoru pomoci toolbaru - EditorPropertyWindow

	
	int GetEditMod() { return editMode; };	// vrati, ktery z hlavnich modu editace je nastaven
	void SetEditMod(int mod);	// nastavi editacni mod
	int GetLastEditMod() { return lastEditMode; };	// vrati, puvodni editacni mod, tj. ten ktery byl pred aktualnim modem

	//-----------------------------------------------------------------------------
	// fce pro pomocne mody editace: (popis vyznamu hodnot u promennych, viz. nize)
	int GetContinuosMod() { return continuousMod; };
	int GetReplaceMod() { return replaceMod; };
	int GetFreeMod() { return freeMod; };
	int GetSameTypeMod() { return sameTypeMod; };
	int GetLargeMod() { return largeMod; };
	int GetSelectLocationMod() { return selectLocationMod; };

	void SetContinuousMod(int mod);
	void SetReplaceMod(int mod);
	void SetFreeMod(int mod);
	void SetFreeModByObject();
	void SetSameTypeMod(int mod);
	void SetLargeMod(int mod);
	void SetSelectLocationMod(int mod);
	//-----------------------------------------------------------------------------


	int ResolveEditMod(UINT character);	
		// zmeni editacni mod podle stisknute klavesy a vrati mod, ktery nastavil
	void ModSwitch(int oldMod, int newMod);
		// provede zmeny, ktere ke zmene modu prislusi
		// editor prechazi z modu "oldMod" do modu "newMod"
		// vola se pri kazde zmene modu
		// (zavre nadbytecna okna, atp.)

	CGrowingArray<OPointer> *globalObjects;	// pole globalnich objektu levelu, ktere jsou mimo mapu
											// index odpovida pozici v toolbaru
	typeID globalObjsTB;	// toolbar s globalnimi objekty levelu
	typeID gameWindowID;	// herni okno s mapou 
	typeID cellObjectsListID;	// okno se seznamem objektu na bunce
	typeID propertyTB;			// toolbar pro nastavovani editacniho modu
	typeID debugWindowID;		// okno s debug vypisy kernelu
	typeID noConnectDialogID;	// dialog pro nastavovani noConnect promenne objektu
	typeID aSelectorID;			// strom s automatismy pro vyber u objektu (pridavani automatismu k objektu, ne klasicke vybirani)

	typeID objPropertyID;		// ID Property okna s vlastnostmi objektu (odkaz na jedine povolene Property okno)
								// ObjProperty - prave editovany objekt
	typeID objVarSelectionID;	// ID CEDObjPropertyList 
								// strom s nabidkou hodnot pro ulozeni do promenne objektu
								// umi ulozit pres "opl->SetObject(o);" vybrany objekt napr. v mape

	CEDPosition listPosition, globalObjsTBPosition, objPropertyPosition, mapPosition, editorSettingsPosition, debugWinPosition;
								// vymezene oblasti v desktopu pro ovladaci prvky editoru

	OPointer replaceLimit;		// horni mez hodnot Opointeru, ktere objekty muzu replacovat
								// objekty s nizsim cislem budou pri replacu premazany

	int globalObjsNoDeleteIndex;	// index do seznamu globalnich objektu
									// urcuje, ktere objekty ze seznamu nelze smazat (staticke objekty)
									// <0..index)  nelze smazat

	typeID tbbID[8];	// cudlitka pro nastavovani modu editace (0-insert,1-select,2-delete,...)

	CGrowingArray<CEDShortCut*>  shortCutArray;	// pole s informacemi o shortcutech
												// do tohoto pole kernel nalouduje shortcuty z levelu
												// editor je podle tohoto pole vytvori a umisti do objectList
												// pri ukladani levelu je kernel z tohoto pole ulozi 

protected:

	int StaticObjectsFilter(CKerObject *ko);	// vraci 0 pokud ma byt vlozeny staticky objekt odfiltrovan (tzn. nevlozen do seznamu globalnich objektu), jinak 0

	int editMode;	// 0 - normal, 1 - insert object (object zvolen a pripraven pro vlozeni do mapy/seznamu)
	int continuousMod;	// 0 - normal, 1 - continuous
	int replaceMod;	// 0 - non replace, 1 - replace
	int freeMod;	// 0 - cellMod, 1 - freeMod (v cellModu se zarovnava na bunky, dela vyber z bunek a maze cela bunka, u freeModu se jede po samostatnych objektech)
	int sameTypeMod;	// 1 - sameType (maze pouze objekty stejneho typu jako je vybrany objekt)
	int largeMod;		// 1 - pracuje s vetsi oblasti (3x3)
	int selectLocationMod;	// 0 - select point, 1 - select cell, 2 - select area, 3 - select cellArea
	
	int lastEditMode;	// editMod pred stisknutim ESC (kvuli navratu puvodniho modu po dalsim ESC)
};


//////////////////////////////////////////////////////////////////////
// CEDEditorIconHandler
//////////////////////////////////////////////////////////////////////

// spravce pro ovladani editacnich modu editoru pomoci toolbaru - EditorPropertyWindow

class CEDEditorIconHandler : public CGUIEventProgram
{
	virtual void EventHandler(CGUIEvent *event);
public:
	CEDEditorIconHandler(int numIcons) { icons = new typeID[numIcons]; };
	virtual ~CEDEditorIconHandler() { SAFE_DELETE_ARRAY(icons); };

	typeID *icons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti
};



//////////////////////////////////////////////////////////////////////
// CEDShortCut
//////////////////////////////////////////////////////////////////////

// informace pro vytvareni shortcutu
// jmeno shortcutu a objekt, na ktery se odkazuje

struct CEDShortCut
{
	CEDShortCut(OPointer _obj, char* _name);
	~CEDShortCut();
	
	OPointer obj;
	char* name;
};


#endif