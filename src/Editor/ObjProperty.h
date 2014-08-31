///////////////////////////////////////////////
//
// ObjProperty.h
//
//	interface pro stromove nastavovani vlastnosti hernich objektu
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef OBJPROPERTY_H
#define OBJPROPERTY_H

#include "tree.h"
#include "textedit.h"
#include "toolbar.h"

class CEDObjectTree;
class CEDObjectProperty;
class CEDObjectTreeHandler;
class CEDObjectPropertyHandler;
class CEDCellObjectsList;
class CGUIEditWindow;
class CGUIScriptButtonsHandler;


class CKerName;
class CKerNameList;
class CKerArrObject;
struct CKerObject;
struct CKerObjectT;

typedef DWORD OPointer;  // typ Pointer na Objekt

extern CEDObjectTree* objectList;

//////////////////////////////////////////////////////////////////////
// CEDObjectTree
//////////////////////////////////////////////////////////////////////

// seznam hernich objektu pro umisteni do levelu
// objekty jsou roztrideny do stromu podle kategorii (strom zavislosti)
// uzivatel si ve stromu vybere objekt, ktery chce vytvorit a pote ho muze editovat,
// umistit do mapy, atp.

class CEDObjectTree : public CGUITree
{
public:
	CEDObjectTree(float _x, float _y, float _sx, float _sy);
	virtual ~CEDObjectTree();

	void LoadObjects();

	void SetSelectedObject(typeID _selectedObjectPropertyID);	
		// nastavi ID vybraneho objektu
		// jen jeden objekt je vybran, se kterym se prave pracuje
		// tzn. edituji se jeho vlastnosti, umistuje do mapy, atp.
		// editovany objekt je reprezentovan pomoci CEDObjectProperty

	void AddShortCutObj(OPointer obj, char* scName);
		// prida do vyberoveho stromu shortcut na objekt "obj" pod jmenem "scName"
		// shortcut se zaradi do stromu za typ objektu
		// shortcuty jsou ve stromu od toho, aby si uzivatel mohl vybrat mezi stejnymi objekty,
		// ovsem ruzne nastavenymi (jejich promenne jsou jiz pripravene pro umisteni)
		// krome shortcutu ma kazdy typ objektu ve stromu prave jednoho zastupce
	
	void RefreshAutos(CKerName* kn);
		// aktualizuje automatismy u objektu ve stromu
		// nastaveni automatismu se muze zmenit, potom se musi patricne upravit i strom


	void PrepareAutoTexture(CKerName* aName, CTexture** objTex, bool& releaseTex);
		// pripravi z automatismu texturu, ktera ho bude reprezentovat v editoru
		// "aName" je jmeno automatismu
		// "objTex" je vysledna pripravena textura pro pouziti v GUI
		// "releaseTex" urcuje zda se ma textura po pouziti uvolnit (objTex->Release()) nebo ne


	void PrepareObjTexture(OPointer obj, CTexture** objTex, bool& releaseTex);

	CEDObjectTreeHandler* objHandler;	// spravce udalosti tykajici se vyberu objektu ze stromu atp.

protected:
	static int FindNameObject(CKerName* kn);	// tridici fce pro kerneli hledani vrstev jmen (KerMain->KerNamesMain->FindLayer)
	void AddLayer(CGUITreeItem* ti, CKerNameList* nl);
		// funkce pridavajici objekty do stromu po vrstvach, ktere odpovidaji zavislostem mezi CKerName
		// prida k polozkce stromu "ti" vsechny objekty ze seznamu "nl"
		// vola se rekurzivne - tim se vytvari strom
	void AddLayerWithAutos(CGUITreeItem* objTI, CGUITreeItem* parrTI, CKerNameList* nl);
		// prida vrstvy s automatismama
		// prida k polozce "parrTI" vsechny automatismy ze seznamu "nl"
		// objTI je odkaz na objekt, jehoz automatismy se pridavaji

	int AddShortCutObjItem(CGUITreeItem* ti, CKerName* kn, OPointer obj, char* scName);
		// pomocna fce pro AddShortCutObj
		// vyuzije se pro rekurzivni hledani polozky stromu, ktera odpovida pridavanemu typu objektu "kn"
	void MakeShortCutObjItem(CGUITreeItem* ti, CKerName* kn, OPointer obj, char* scName);
		// pomocna fce pro AddShortCutObj
		// po nalezeni odpovidajici polozky "ti" zaridi pridani nove polozky jako shortcutu

	void RefresItemAutos(CGUITreeItem* ti);
		// pomocna fce pro RefreshAutos
		// zajisti vlastni aktualizaci stromu s automatismama pod objektem jehoz polozku jsme jiz nasli - "ti"
		// aktualizace se provede smazanim starych podpolozek a vytvorenim noveho podstromu s automatismama pres AddLayerWithAutos
	int RefresAutosItemRecursive(CGUITreeItem* ti, CKerName* kn);
		// pomocna fce pro RefreshAutos
		// zaridi rekurzivni prochazeni stromu a volani RefresItemAutos na nalezene automatismy odpovidajici "kn"

	void CreateContextMenu();
		// vytvori kontextove menu pro vytvareni a mazani shortcutu na objekty
	void ShowMenu(int state, typeID treeItem);
		// zaridi zobrazeni kontextoveho menu na polozce stromu

	virtual int TreeHandler(typeID treeItem, int index, int state);
		// obsluhuje vetsinu klientskych operaci nad stromem
		// vytvari CEDObjectProperty pro objekt, jez si uzivatel ve stromu vybral
		// vytvari podstrom automatismu u vybraneho objektu

	virtual void TreeUpdateHandler(typeID treeItem, int index);

};


//////////////////////////////////////////////////////////////////////
// CEDObjectProperty
//////////////////////////////////////////////////////////////////////

// objekt reprezentujici herni objekt v editoru
// umoznuje editovat jeho promenne uzivatelem
// vytvaret shortcuty
// vybirat pro objekt automatismus
// nastavit promenne objektu na defaultni hodnoty
// vyresetovat objekt, tzn. nastavit hodnoty prommenych na hodnoty zadane v konstruktoru herniho objektu
// okopirovat objekt, tzn. vytvorit novy objekt se stejnymi hodnotami promennych

// jednotlive promenne objektu jsou zobrazeny ve stromu
// edituji se ruznymi zpusoby, podle typu promenne
// neco se zadava v editboxu, neco se vybira z mapy atp.

class CEDObjectProperty : public CGUITree
{
	friend class CEDObjectPropertyHandler;
	friend class CEDEditor;
	friend class CEDObjPropertyList;
	friend class CGUIScriptVariableInterface;
	friend class CEDASelector;
public:
	CEDObjectProperty(float _x, float _y, float _sx, float _sy, OPointer _obj, typeID _objectListItemID = 0, bool _objAlreadyPlaced=false);
		// vytvori editacni strom pro objekt "obj"
		// objectListItemID je odkaz na polozku stromu se seznamem objektu - kvuli aktualizaci stromu pri resetu a deletu objektu
		// objAlreadyPlaced urcuje zda je jiz objekt umisten v mape nebo seznamu globalnich objektu
	CEDObjectProperty(float _x, float _y, float _sx, float _sy, bool _globalVars);
		// vytvori editacni strom pro globalni promenne levelu (_globalVars==true)
	virtual ~CEDObjectProperty();

	void CreateVarProperty();
		// vytvori strom skladajici se ze vsech editovatelnych promennych objektu
		// cerpa informace o promennych z OVars ulozenych u objektu v Kernelu


	int VarValueChanged(CGUITreeItem* ti, void* value=0);	
		// vola se kdyz uzivatel zmenil hodnotu promenne objektu (promenna odpovida polozce stromu  - "ti->ut.OVar")
		// fce zkontroluje platnost nove hodnoty, zapise ji do objektu
		// vrati 1 - hodnota platna a zapsana, 0 - hodnota neplatna a nezapsana

	void ShowObjValue(CGUITreeItem* ti);	
		// nastavi hodnotu ulozenou v promenne objektu do editacniho pole elementu pridruzeneho k polozce stromu "ti"
		// zobrazi obsah promenne uzivateli (forma je dle typu promenne)

	void SetToDefault();	
		// nastavi hodnoty atributu objektu na defaultni - definovane uzivatelem ve skriptu

	void MakeShortCutDlg();	
		// vytvori dialog pro tvorbu shortCutu na objekt
	void ProcessShortCutDlg(CGUIEvent* event);	
		// obslouzi dialog

	void UpdateAllVars(CGUITreeItem* ti=0);	
		// aktualizuje hodnoty vsech promennych 
		// (nacte z promennych objektu a zapise do polozek property okna)
		// ti==0 => zacne aktualizaci od korene, sestupuje rekurzivne k listum
		// ti!=0 => zacne od polozky ti (vcetne)


	//---------------------------------------------------------------------------------------------
	// fce pro skupinove promenne - edituji oblasti nebo body v mape
	void SaveLocationVar(int type, int lx, int ly, int lz);	
		// ulozi zadane hodnoty jako souradnice editovane oblasti
		// jake skupinove (oblastni) promenne se to tyka je ulozeno v "editedGroupItem" - tj. vybrana oblastni promenna
		// type urcuje u oblasti (area) zda se jedna o prvni bod (type==1) nebo o druhy bod (type==2)
		// (type==0) - ulozeni bodu/bunky
	int GetLocationVar(int &lx, int &ly, int &lz);	
		// ulozi do promennych hodnoty ze skupinovych promennych u polozky "editedGroupItem"
		// vraci typ - 0 (free point), 1 - (cell), 2 - (area), 3 - (cellArea)
		//	-1 chyba
	void GetLocationAreaVar(int &x1, int &y1, int &z1, int &x2, int &y2, int &z2);
			// ulozi do promennych hodnoty ze skupinovych promennych u polozky "editedGroupItem"
			// souradnice prvniho a druheho bodu oblasti (zda se jedna o bunky nebo pixely urcuje typ oblasti)
	void RefreshLocation(CGUITreeItem* groupItem);
		// nacte hodnoty promennych u skupiny "groupItem"
		// zobrazi oznacovac na mape, ktery ukazuje kde je vybrana oblast (bunka)
		// ulozi promenne ve spravnem poradi (nejdrive levy horni roh, potom pravy dolni roh oblasti)
	void ClearLocation(CGUITreeItem* groupItem);
		// vynuluje promenne u skupiny "groupItem"
	//---------------------------------------------------------------------------------------------

	
	int PlaceObjToMap(int ox, int oy, int oz, bool cell, bool check=false);	
		// umisti objekt do mapy na pozici ox,oy,oz
		// objekt musi mit automatismus, jinak nelze umistit
		// objekt musi byt umistitelny volne, nebo musi byt cell==true, tzn. zadane souradnice jsou zarovnany na bunky
		// check urcuje zda se ma kontrolovat moznost vlozeni (pri vkladani na policko, kde jiz neco je)
		//	nebo se vlozi kazdopadne (v replace modu)
		//	- pri kontrole (check==true) se prepisuji pouze starsi objekty, nez je prave vkladany
		// muze vyhodit vyjimku CKerPanic
		
	int IsPossiblePlaceObjToMap(int ox, int oy, int oz);
		// vraci:	0 - objekt nelze umistit do mapy na dane pozici (je v kolizi s jinym objektem)
		//			1 - lze umistit
		//			-1 - nelze umistit vubec do mapy

	int CreateGlobalObject();
		// 	umisti objekt do seznamu globalnich objektu

	void DeleteObject();	
		// muze vyhodit vyjimku CKerPanic
	void ResetObject();	
		// smaze objekt a vytvori novy
		// tim se hodnoty jeho promennych nastavi na hodnoty zadane v konstruktoru objektu
		// lze pouzit jen na jeste neumisteny objekt

	void RefreshObject();	
		// aktualizuje objekt 
		// informuje ostatni aktualizacni fce o zmene objektu, ty zaridi konkretni zviditelneni zmen v parametrech objektu
		// aktualizuje veci jako automatismy, polohu, velikost mapy, atp.

	void SetA();
		// vytvori strom pro vyber noveho automatismu objektu

	void UpdateEditedVar();
		// najde editBox, ktery ma focus a hodnotu v nem napsanou se pokusi ulozit do prislusne promenne objektu
		// kvuli aktualizaci hodnoty, ktera jeste nebyla potvrzena uzivatelem pri prechodu na jinou akci

	void CopyObjectProperty();
		// vytvori kopii objektu a zobrazi CEDObjectProperty pro novou kopii, 
		// puvodni CEDObjectProperty zavre
	
	void SetErrorMsg(char *msg);
		// nastavi chybovou hlasku, ktera se vypisuje u CEDObjectProperty
		// napr., ze objektu chybi automatismus atp.
		// msg muze byt 0

	void SetCursorSelectObj();
		// zobrazi kurzor pro vyber objektu do objektove promenne
	void SetNormalCursor();
		// zobrazi normalni kurzor (pro navraceni puvodniho stavu)

	typeID menuID;	// ID kontextoveho menu pro vytvareni a ruseni shortcutu

	OPointer obj;	// objekt, ktery CEDObjectProperty reprezentuje

	CGUITreeItem* aPictureTI;	// odkaz na polozku editujici APicture objektu
			// polozka stromu CEDObjectProperty, ktera odpovida promenne pro vyber automatismu objektu
			// kvuli aktualizaci pri vyberu automatismu v nabidkovem stromu objektu - CEDObjectTree

	CGUITreeItem *posXTI,*posYTI,*posZTI;	// odkaz na polozky editujici polohu objektu
		// kvuli aktualizaci polozek pri zmene polohy objektu umistenim do mapy atp.

	bool objAlreadyPlaced;	// zda je jiz objekt umisten v mape / seznamu globalnich objektu atp.
							// takovyto objekt se pri vytvareni kopii nesmi upravovat (napr. pri umistovani jeho kopie do mapy)
	bool globalVars;		// zda se jedna o strom (property okno) s globalnima promennyma levelu (globalVars==true)
							// nebo o property okno normalniho objektu

protected:
	void PrepareObjCursor();
		// pripravi kurzor odpovidajici texture objektu
		// kvuli umistovani do mapy (free) nebo seznamu globalnich objektu

	static int VarFilter(CKerOVar* OVar);	
		// filtr na OVary objektu
		// urcuje zda se jedna o promennou, kterou muze uzivatel editovat nebo nikoliv, tzn. nebude zobrazena
		// vrati zda zaradit promennou "OVar" mezi editovane polozky (1) nebo nikoliv (0)
	static int GroupVarFilter(CKerOVar* OVar);	
		// vrati zda se jedna o skupinovou polozku nebo nikoliv
		// tj. polozku urcujici oblast v mape, bunku, bod
	
	CGUITreeItem* AddEditItem(CGUITreeItem* groupItem, CKerOVar* OVar, typeID where=0, bool first=false, bool ownOVar=false);
		// prida editacni polozu do stromu
		// groupItem urcuje do jake vetve stromu bude prirazena (0==koren)
		// OVar se pouzije pro jmeno polozky, komentar, ulozi se do jejich promennych pro budouci praci
		// ownOVar urcuje zda se ma OVar pri ruseni polozky smazat nebo nikoliv
		// where, first je pro zarazeni nove polozky (kam, viz. strom)
	CGUITreeItem* AddEditArrayItem(CGUITreeItem* root, CKerOVar* OVar, int index, typeID where=0, bool first=false);
		// prida editacni polozku do stromu odpovidajici polozce kerneliho pole
		// root je korenova polozka kerneliho pole
		// index je poradi prvku pole, ktery se pridava

	CGUIEditWindow* CreateEditWindow();
		// vytvori a nastavi EditBox, ktery pote lze pridat k polozce
	CGUICheckBox* CreateCheckBox(char* text);
		// vytvori CheckBox a vrati ho
	CGUIEditWindow* GetEditWindow(CGUITreeItem* ti);
		// vrati EditBox umisteny u polozky "ti"
		// muze byt i v okne u polozky, ne jen primo
		// pokud zadny neni vrati 0


	void SelectLocationEditModByOVar(CGUITreeItem* groupItem, CKerOVar* OVar, bool on);
		// prepne editor do modu na editovani lokace
		// o jaky typ lokace jde se urci z OVaru - area, cell, point, ... a nastavi se to
		// groupItem urcuje jaka skupinova promenna se bude editovat
		// zobrazi se vybrana oblast na mape
		// "on" urcuje zda se tento mod zapina (true), tzn. udela se to co je napsano vise, nebo vypina (false),
		// tj. nastavi se puvodni editacni mod a zrusi se vybrana oblast na mape

	//---------------------------------------------------------------------------------------------
	// fce pro pridani konkretni polozky pro promennou podle jejiho typu
	// ko - objekt obsahujici pridavanou promennou
	// OVar - popis pridavane promenne
	// ti - polozka stromu odpovidajici pridavane promenne (zatim jen jmeno promenne)
	// arrIndex - index do kerneliho pole v pripade, ze se jedna o pole (o jednicku vyssi, 0 - zadne pole to neni)
	void AddIntVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);
	void AddCharVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);
	void AddDoubleVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);
	void AddStringVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti);
	void AddKerNameVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);
	void AddObjectVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);


	//---------------------------------------------------------------------------------------------
	// fce pro ulozeni hodnoty do promenne podle jejiho typu
	// ko - objekt obsahujici promennou
	// OVar - popis promenne
	// ti - polozka stromu odpovidajici ukladane promenne
	// value - adresa, kde je ulozena hodnota, ktera se ma ulozit do promenne
	//	- pokud je 0, hodnota se urci z pridruzeneho editacniho pole u polozky (editboxu)
	// arrIndex - index do kerneliho pole v pripade, ze se jedna o pole (o jednicku vyssi, 0 - zadne pole to neni)
	// vraci 1 pri uspechnu, 0 jinak (hodnota neodpovida podminkam, atp.):
	int SaveIntVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value=0, int arrIndex=0);
	int SaveCharVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value=0, int arrIndex=0);
	int SaveDoubleVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value=0, int arrIndex=0);
	int SaveStringVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value=0);
	int SaveKerNameVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value=0, int arrIndex=0);
	int SaveObjectVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value=0, int arrIndex=0);

	
	//---------------------------------------------------------------------------------------------
	// fce pro zobrazeni hodnoty promenne v editacnim poli polozky (podle jejiho typu)
	// ko - objekt obsahujici promennou
	// OVar - popis promenne
	// ti - polozka stromu odpovidajici promenne
	// arrIndex - index do kerneliho pole v pripade, ze se jedna o pole (o jednicku vyssi, 0 - zadne pole to neni)
	void ShowIntVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);
	void ShowCharVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);
	void ShowDoubleVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);
	void ShowStringVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti);
	void ShowKerNameVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);
	void ShowObjectVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex=0);


	//---------------------------------------------------------------------------------------------
	// fce pro praci s kernelima polema:
	void CreateOrDeleteArrayVar(CGUITreeItem* root, CKerOVar* OVar, CGUITreeItem* treeItem, CGUICheckBox* chBox);
		// pokud kerneli pole pro promennou Ovar existuje, zrusi se vcetne vsech jeho polozek
		// pokud neexistuje, vytvori se nove prazdne
		// nakonec se updatuje strom
		// root - polozka stromu odpovidajici kernelskemu poli
		// treeItem - polozka stromu odpovidajici ovladacim prvkum pole - Add/Delete/Create array
		// chBox - checkbox na polozce treeItem, urcujici zda je pole vytvorene nebo ne
	void AddArrayVarProperty(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* root);
		// naplni polozku "root" odpovidajici kernelskemu poli typu "OVar" objektu "ko"
		// polozkami pole, nakonec prida ovladaci prvky Add/delete/Create Array
	void AddNewArrayVarItemProperty(CGUITreeItem* root, CKerOVar* OVar, typeID treeItemID);
		// prida novou polozku do kerneliho pole a rovnou ji prida i mezi editovane polozky stromu
		// root - korenova polozka odpovidajici kernelimu poli
		// treeItemID - ID polozky s ovladacima prvkama pole 
		//	(kvuli razeni pri pridavani nove polozky, nova polozka se zaradi za ostatni polozky, ale pred ovladaci prvky, ty zustanou na konci)
		// nakonec aktualizuje strom UpdateTree()
	void DeleteLastArrayVarItemProperty(CGUITreeItem* root, CKerOVar* OVar, CGUITreeItem* treeItem);
		// smaze posledni polozku kerneliho pole
		// root - korenova polozka odpovidajici kernelimu poli 
		// treeItemID - ID polozky s ovladacima prvkama pole 
		//	(kvuli hledani posledni polozky stromu - je to ta nad ovladacima prvkama)
		// nakonec aktualizuje strom UpdateTree()


	void AddVarProperty(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, CGUITreeItem* &groupItem, int &numGroupItems);
		// pomocna fce pro CreateVarProperty
		// prida jednu polozku (ti) pro jednu promennou (OVar) podle jejiho typu
		// "groupItem" je odkaz na koren skupinove polozky, pokud se zrovna vytvari jeji skupina (poloky spadajici do skupiny)
		// "numGroupItems" je pocet polozek, ktere patri do skupiny

	void *GetAccessToVar(CKerObject* ko, CKerOVar* Var);	
		// pristup k promenne objektu / globalni promenne (ko==0 && globalVars==true)
		// vraci adresu promenne

	int GetValue(int &value, CGUITreeItem* ti);	
		// hodnotu ulozenou v polozce "ti" zapise do "value" (urceno pro ziskani hodnoty ve skupinove polozce)
		// vraci 0 - OK, 1 - chyba
	int SaveValue(int value, CGUITreeItem* ti);
		// zapise hodnotu "value" do polozky "ti" (urceno pro zapis hodnoty ve skupinove polozce)
		// vraci 0 - OK, 1 - chyba


	void AddContextMenu();
		// prida kontextove menu s moznostmi nad objektem
	void AddHandlerAndPanel();	
		// prida handler a panel do okna

	virtual void UpdateTree();

	CGUIStaticText* objComment;


	bool ownObject;		// zda je obj vlastni objekt, nebo jen odkaz na objekt v seznamu objektu u CEDObjectTree (pripadne v jinem seznamu)
						// v pripade vlastniho objektu se v destruktoru nici
	
	CEDObjectPropertyHandler* handler;	// odkaz na spravce udalosti tykaji se menu, panelu a editace polozek stromu
										// zarizuje veskerou funkcnost editace promennych
	typeID objectListItemID;	// odkaz na polozku v seznamu objektu - kvuli aktualizaci obj pri resetu a mazani

	typeID parentObjPropertyID;	// ID objektu, ktery me ma ulozeneho ve sve promenne a zrovna me edituje

	CGUIStaticText* errorMsg;	// chybova hlaska
	char* shortCutName;			// jmeno shortCutu (v pripade, ze se jedna o shortcut na objekt)

	CGUITreeItem* editedGroupItem;	// odkaz na skupinovou polozku, ktera se zrovna edituje

	int beforeLocationEditMod;	// promenna urcuje, jaky byl editovaci mod predtim, nez se prepnul na editacni mod vyberu lokace

	CGUIScriptButtonsHandler* scriptHandler;	// spravce udalosti pro interface skriptu
};


//////////////////////////////////////////////////////////////////////
// CEDVarArrayItemData
//////////////////////////////////////////////////////////////////////

// data ulozene u polozky stromu odpovidajici kernelimu poli (korenova polozka pole)
// obsahuje informace pro spravu pole
//  - OVar
//  - ID ovladacich prvku (add/delete button, Create Array Checkbox)
//		- kvuli zjistovani jaka akce se ma po stisknu tlacitka provest, u jakeho pole
// delOvar - urcuje zda se ma OVar pri ruseni polozky smazat (skriptovany OVar) nebo ne

class CEDVarArrayItemData : public CGUITreeItemUserData
{
public:
	CEDVarArrayItemData(CKerOVar* _OVar, typeID _addButton, typeID _delButton, typeID _createCheckBox, bool _delOvar) 
		{ OVar=_OVar;addButton=_addButton;delButton=_delButton; createCheckBox=_createCheckBox; delOvar = _delOvar;};
	virtual ~CEDVarArrayItemData();

	CKerOVar* OVar;
	typeID addButton, delButton, createCheckBox;
	bool delOvar;	// zda se ma Ovar v destruktoru mazat nebo ne (kvuli skriptovanym promenym)
};


//////////////////////////////////////////////////////////////////////
// CEDObjectTreeHandler
//////////////////////////////////////////////////////////////////////

// spravce udalosti tykajici se vyberu objektu ze stromu atp.
// spravuje:
//	- ruseni vybraneho objektu pri zavreni okna s CEDObjectProperty
//	- zmenu editacniho modu po focusaci okna s CEDObjectProperty
//	- veskera sprava ovladani nad toolbarem se seznamem globalnich objektu levelu
//		- vyber objektu, mazani, ...
//	- kontextove menu nad seznamem zajistujici vytvareni a ruseni shortcutu

class CEDObjectTreeHandler : public CGUIEventProgram
{
	virtual void EventHandler(CGUIEvent *event);
public:
	CEDObjectTreeHandler(int numIcons) { icons = new typeID[numIcons]; };
	virtual ~CEDObjectTreeHandler() { SAFE_DELETE_ARRAY(icons); };

	typeID *icons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti
};

//////////////////////////////////////////////////////////////////////
// CEDObjectPropertyHandler
//////////////////////////////////////////////////////////////////////

// spravce udalosti tykaji se menu, panelu a editace polozek stromu CEDObjectProperty
// zarizuje veskerou funkcnost editace promennych

class CEDObjectPropertyHandler : public CGUIEventProgram
{
	virtual void EventHandler(CGUIEvent *event);
public:
	CEDObjectPropertyHandler(int numIcons) { icons = new typeID[numIcons]; IDbadElem=0; badElFoc=0;};
	virtual ~CEDObjectPropertyHandler() { SAFE_DELETE_ARRAY(icons); };

	typeID *icons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti
	CEDObjectProperty* op;

	typeID IDbadElem;	// ID elementu, ktery hlasil spatne zadanou polozku
	int badElFoc;		//	zda je spatne zadany element focusovan
};


//////////////////////////////////////////////////////////////////////
// CEDObjectBrowser
//////////////////////////////////////////////////////////////////////

// strom s objekty
// reprezentuje kazdy objekt + jeho atributy a metody
// vytvoreni stromu se realizuje v editoru pres - CEDEditor::CreateObjectBrowser()

class CEDObjectBrowser : public CGUITree
{
public:
	CEDObjectBrowser(float _x, float _y, float _sx, float _sy);
	virtual ~CEDObjectBrowser();

protected:

};

//////////////////////////////////////////////////////////////////////
// CEDGlobalObjectsToolBar
//////////////////////////////////////////////////////////////////////

// toolbar se seznamem globalnich objektu v levelu
// toolbar zachazi s objekty presne podle editacniho modu
// tzn. pri kliknuti na objekt se rozhoduje podle modu, co s nim udela - select, insert, delete, ...
// nad toolbarem se zobrazuje kurzor dle editacniho modu

class CEDGlobalObjectsToolBar : public CGUIToolBar
{
public:
	CEDGlobalObjectsToolBar(float _x, float _y, float _sx, float _sy);
	virtual ~CEDGlobalObjectsToolBar();

protected:
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID);

	void ShowCursorByEditMod();
	void ShowStdCursor();
};



//////////////////////////////////////////////////////////////////////
// CEDCellObjectsList
//////////////////////////////////////////////////////////////////////

// objekt realizujici seznam hernich objektu na jedne bunce
// v seznamu lze vybirat objekt pro editaci
// v seznmu lze take vybirat objekt, ktery se ma ulozit do editovane objektove promenne

class CEDCellObjectsList : public CGUITree
{
public:
	CEDCellObjectsList(float _x, float _y, float _sx, float _sy, CKerArrObject* oa);
		// vytvori seznam objektu podle pole "oa"
	virtual ~CEDCellObjectsList();

	int RemoveObjectFromList(OPointer obj);
		// zrusi polozku seznamu odpovidajici objektu "obj"
		// vraci jak bylo uspesne mazani (0 - uspech, 1 - neuspech)

protected:
	virtual int TreeHandler(typeID treeItem, int index, int state);
	virtual int Keyboard(UINT ch, UINT state);
};


//////////////////////////////////////////////////////////////////////
// CEDObjPropertyList
//////////////////////////////////////////////////////////////////////

// objekt realizujici nabidku moznosti, ktere lze zvolit do editovane promenne
// vytvori se strom s moznostmi, urcenych pres OVar editovane promenne
// uzivatel si muze vybrat jednu z moznosti

class CEDObjPropertyList : public CGUITree
{
public:
	CEDObjPropertyList(float _x, float _y, float _sx, float _sy, CGUITreeItem* ti, CEDObjectProperty* op);
		// vytvori nabidku moznosti pro promennou polozky "ti" objektu "op"
	virtual ~CEDObjPropertyList();

	void SetObject(OPointer obj);
		// nastavi u editovane objektove polozky hodnotu na vybrany objekt "obj"
		// pokud je editovana polozka typu CKerName nastavi hodnotu typu objektu "obj" (tedy jeho CKerName)

	static int FindNameObject(CKerName* kn);	// fce pro filtrovani objektu mezi CKerName
	static int NameFilter(CKerName* kn);		// fce pro filtrovani jmen podle masky a typu

	static int nameMask;			// nastavena maska podle OVar
	static int eKVUBobjInMapPL;		// zda vybiram jen objekty umistitelne do mapy
	static int eKVUBobjOutMapPL;	// zda vybirem jen objekty umistitelne mimo mapu

protected:

	virtual int Keyboard(UINT ch, UINT state);
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);

	virtual int TreeHandler(typeID treeItem, int index, int state);
		// fce realizujici vyber hodnoty z moznosti (ulozi vybranou hodnotu a zavre nabidku)

	void AddLayer(CGUITreeItem* parrTI, CKerNameList* nl, int type, int direction, int (*FilterFce)(CKerName *name) = 0);
		// fce pridavajici moznosti po vrstvach (napr. pro automatismy atp.)
		// parrTI - kam se ma nova vrstva pridat (pod jakou polozku), 0==pod koren
		// nl - kerneli pole obsahujici pridavane polozky
		// type - typ objektu pro vyhledavani (eKerNTauto, eKerNTobject, ...)
		// direction - smer vyhledavani
		// FilterFce - filtrovaci fce
		// vyznamy jako u kerneli: KerMain->KerNamesMain->FindLayer(...)

	void DeletePreviousObject(CKerObject* ko, CKerOVar* OVar, int arrIndex);
		// smaze objekt ulozeny v objektove promenne OVar
		

	CGUITreeItem* propertyItem;		// odkaz na editovanou polozku (promennou, jejiz vyber moznosti se realizuje)
	CEDObjectProperty* objProperty;	// odkaz na editovany objekt (jeho CEDObjectProperty)

	bool mouseClick;	// zda se kliklo mysi do vyberu
	bool closeOnClick;	// zda se ma okno zavrit po kliknuti mysi
};


//////////////////////////////////////////////////////////////////////
// CEDNoConnectDialog
//////////////////////////////////////////////////////////////////////

// dialog pro editaci noConnect promenne u objektu
// noConnect rika, jakym smerem objekt nenavazuje
// urcuje se to ve 3D, na krychlich, tzn. mame 3x3x3 smery)
// pro kazdy smer ma dialog vlastni tlacitko, ktere urcuje navaznost ve svem smeru

class CEDNoConnectDialog : public CGUIToolBar
{
public:
	CEDNoConnectDialog(OPointer _obj);
		// vytvori noConnect dialog pro objekt "obj"
	virtual ~CEDNoConnectDialog();

protected:
	virtual void EventHandler(CGUIEvent *event);
		// spravuje udalosti od tlacitek dialogu - Ok, Cancel
		
	void ReMapButtonsIndex(typeID butts[27]);
		// priradi smery k tlacitkum 

	void LoadConfiguration();
		// podle nastaveni noConnect promenne nastavi stavy tlacitek
	void SaveConfiguration();
		// podle nastaveni tlacitek ulozi promennou noConnect

	OPointer obj;	// editovany objekt
	typeID buttons[27];	// pole se smerovymi tlacitky
	typeID buttOK, buttCancel;	// ovladaci tlacitka dialogu
};



//////////////////////////////////////////////////////////////////////
// CEDEditWindow
//////////////////////////////////////////////////////////////////////

// uprava std. editboxu pro potreby editoru
// predefinovani fce FocusChanged
// pri zmene focusu se automaticky uklada hodnota editboxu do editovane promenne

class CEDEditWindow : public CGUIEditWindow
{
public:
	CEDEditWindow(float _x, float _y, float _sx, int textColor=0xFF000000, int bgColor=0xFFFFFFFF, char *fontname=NULL); 
protected:
	virtual int FocusChanged(UINT focus, typeID dualID); //zmenil se focus
};



//////////////////////////////////////////////////////////////////////
// CEDASelector
//////////////////////////////////////////////////////////////////////

// strom s nabidkou automatismu pro vyber k objektu
// k objektovemu typu "objType" se priradi vybrany automatismus ze stromu

class CEDASelector : public CGUITree
{
public:
	CEDASelector(CKerObjectT* _objType, float _x, float _y, float _sx, float _sy);
	virtual ~CEDASelector();

protected:
	static int FindNameObject(CKerName* kn);	// tridici fce pro kerneli hledani vrstev jmen (KerMain->KerNamesMain->FindLayer)
	void AddLayerWithAutos(CGUITreeItem* parrTI, CKerNameList* nl);
		// prida vrstvu automatismu v poli "nl" do polozky "parrTI"

	void PrepareAutoTexture(CKerName* aName, CTexture** objTex, bool& releaseTex);
		// pripravi texturu automatismu pro pouziti jako obrazek u polozky

	virtual int Keyboard(UINT character, UINT state);
	virtual int TreeHandler(typeID treeItem, int index, int state);
		// realizuje vyber automatismu

	CKerObjectT* objType;	// typ objektu, ke kteremu se vybira automatismus
};




#endif 