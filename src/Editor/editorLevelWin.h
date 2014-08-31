///////////////////////////////////////////////
//
// editorLevelWin.h
//
//	interface pro propojeni GUI s game window enginu
//	obsahuje funkce pro praci editoru s mapou (umisotvani objektu do mapy, vyhledavani objektu v mape, ...)
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef EDITORLEVELWIN_H
#define EDITORLEVELWIN_H

#include "gameWindow.h"

typedef DWORD OPointer;  // typ Pointer na Objekt

class CGEnCursor;
class CEDObjectProperty;
class CKerName;

class CGUILine;


////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDLevelWindow
////////////////////////////////////////////////////////////////////////////////////////////////////

class CEDLevelWindow : public CGUIGameWindow
{
	friend class CEDEditor;
	friend class CEDEditorDesktop;
public:
	CEDLevelWindow(float _x, float _y, float _sx, float _sy);
		// vytvori okno kolem mapy, inicializuje pomocne struktury (kurzory, ...)
	virtual ~CEDLevelWindow();

	void DeleteObjectsFromCell(int cx, int cy, int cz);
		// smaze vsechny objekty na bunce (cx,cy,cz), 
		// ktere odpovidaji typu objektu "objDeleteType" nebo alepson vrstve objektu "layerDeleteType"
		// (urceno podle editor->GetSameTypeMod())
		// typ objektu a vrstva se urcuje prvnim smazanym objektem (delete mod)
		// nebo objektem, ktery se umistuje do mapy (insert mod)
	
	void CreateCellObjectsList(int cx, int cy, int cz);
		// vytvori strom se seznamem objektu na bunce (cx,cy,cz)

	void SelectObject(float ox, float oy);
		// vytvori ObjectProperty okno pro objekt, ktery najde na pozici (ox,oy) v mape
		// pokud je jich tam vice, vybere se prvni z hlediska poradi viditelnosti

	void SetObjNoConnect(float ox, float oy);
		// vytvori noConnect dialog pro objekt, ktery najde na pozici (ox,oy) v mape
		// pokud je jich tam vice, vybere se prvni z hlediska poradi viditelnosti

	void SelectObjectVar(float ox, float oy);
		// vybere objekt, ktery se ulozi do objektove promenne
		// objekt, ktery najde na pozici (ox,oy) v mape
		// pokud je jich tam vice, vybere se prvni z hlediska poradi viditelnosti

	void DeleteObject(float ox, float oy, bool first);
		// smaze objekt, ktery najde na pozici (ox,oy) v mape
		// pokud je jich tam vice, smze se prvni z hlediska poradi viditelnosti
		// "first" urcuje zda jde o pocatek mazani (true) nebo jiz prubezne mazani
		// na pocatku, prvni smazany objekt urcuje typ a vrstvu objektu, ktere potom pujdou mazat
		//	- objDeleteType, layerDeleteType

	void DeleteObjectLikeSelected(int cx, int cy, int cz, bool first);
		// smaze objekty na bunce (cx,cy,cz), odpovidajici mazanemu typu/vrstve
		// first == true - jde o prvni mazany objekt => urcuje se typ / vrstvu, ktera se bude dale mazat
		//	- objDeleteType, layerDeleteType
		// typ a vrstva se urcuje podle typu a vrstvy vybraneho objektu (ne objektu pod kurzorem)

	void DeleteObjectLikeSelectedFree(float ox, float oy, bool first);
		// smaze objekty na pozici (ox,oy), odpovidajici mazanemu typu/vrstve
		// first == true - jde o prvni mazany objekt => urcuje se typ / vrstvu, ktera se bude dale mazat
		//	- objDeleteType, layerDeleteType
		// typ a vrstva se urcuje podle typu a vrstvy vybraneho objektu (ne objektu pod kurzorem)

	void DeleteObjectsFrom3x3Cells(int cx, int cy, int cz);
		// jako DeleteObjectLikeSelected, ovsem maze objekty na vetsi plose (3x3 bunky)
		// (cx,cy,cz) je souradnice stredove bunky oblasti 3x3
	
	void PlaceObjectLarge(CEDObjectProperty *op, int cx, int cy, int cz, bool check);
		// umisti objekt "op" do mapy na plochu 3x3 bunky (tj. umisti 9 objektu)
		// (cx,cy,cz) je souradnice stredove bunky oblasti 3x3
		// "check" urcuje zda se v replace modu kontroluje kolize nebo se natvrdo umisti
		// objekty se umistuji jen tam, kde je volno
		// pokud chci prepisovat i obsazene pozice, musim mit zapnut replace mod



	// ------------------------------------------------------------------------------------------
	// fce pro praci s kurzorem nad mapou
	void ShowCursorByEditMod(int cx, int cy, int cz);
		// nastavi typ kurzoru podle editacniho modu
		// cx,cy,cz je souradnice bunky, nad kterou je prave kurzor

	void ShowStdCursor();
		// skryje vsechny krychlove kurzory a nastavi normalni kurzor GUI
		// pouziva se zejmena pri vyjeti kurzoru mimo mapu a podobnych pripadech
	void HideAnyCursor();
		// skryje vsechny kurzory (krychlove i normalni)


	void ShowCursorInsert(bool show, int cx, int cy, int cz);
		// pomocna fce pro ShowCursorByEditMod
		// zobrazi insertovaci kurzor podle modu (free/cell)
		// podle toho zda objekt lze vlozit nebo nikoliv se meni barva kurzoru (cell)
		// (cx,cy,cz) je souradnice bunky, nad kterou je prave kurzor
		// "show" urcuje zda kurzor zobrazit nebo skryt - funguje jen zobrazovani (true), skryvani je pres HideAnyCursor

	void ShowCursorDelete(bool show, int cx, int cy, int cz);
		// pomocna fce pro ShowCursorByEditMod
		// zobrazi deletovaci kurzor podle modu (free/cell)
		// (cx,cy,cz) je souradnice bunky, nad kterou je prave kurzor
		// "show" urcuje zda kurzor zobrazit nebo skryt - funguje jen zobrazovani (true), skryvani je pres HideAnyCursor

	void ShowCursorSelectCell(int cx, int cy, int cz);
		// pomocna fce pro ShowCursorByEditMod
		// zobrazi kurzor pro vyber bunky 
		// (cx,cy,cz) je souradnice bunky, nad kterou je prave kurzor

	void ShowCursorSelectCellLocation(int cx, int cy, int cz);
		// pomocna fce pro ShowCursorByEditMod
		// zobrazi kurzor pro vyber bunkove oblasti
		// (cx,cy,cz) je souradnice bunky, nad kterou je prave kurzor

	void ShowLargeCursorsInsert(int cx, int cy, int cz);
		// pomocna fce pro ShowCursorByEditMod & ShowCursorInsert
		// zobrazi kurzor nad oblasti 3x3 bunky
		// (cx,cy,cz) je souradnice stredove bunky oblasti 3x3
		// barva krychli kurzoru je urcena podle toho, zda objekt lze vlozit do bunky nebo nelze

	void ShowLargeCursorsDelete(int cx, int cy, int cz);
		// pomocna fce pro ShowCursorByEditMod & ShowCursorDelete
		// zobrazi kurzor nad oblasti 3x3 bunky
		// (cx,cy,cz) je souradnice stredove bunky oblasti 3x3
	// ------------------------------------------------------------------------------------------


	//-------------------------------------------------------------------------------------------
	// fce pro praci s oznacenim skupinove polozky
	void ShowPointMarker(int px, int py, int pz);
		// zobrazi oznacovac bodu na souradnicich (px,py,pz)
		// ukazuje kde lezi vybrany bod
	void ShowCellMarker(int px, int py, int pz);
		// zobrazi oznacovac bunky na souradnicich (px,py,pz)
		// ukazuje kde lezi vybrana bunka
	void ShowAreaMarker(int x1, int y1, int z1, int x2, int y2, int z2);
		// zobrazi oznacovac oblasti na obdelniku (x1,y1,z1 - x2,y2,z2)
		// ukazuje kde lezi vybrana oblast
	void ShowCellAreaMarker(int cx1, int cy1, int cz1, int cx2, int cy2, int cz2);
		// zobrazi oznacovac bunkove oblasti na obdelniku (x1,y1,z1 - x2,y2,z2)
		// ukazuje kde lezi vybrana bunkova oblast

	void DeleteMarker();
		// skryje vsechny oznacovace pro skupinove polozky (vyber bodu, bunky, oblasti)
	//-------------------------------------------------------------------------------------------

protected:
	// fce prevadejici prikazy od uzivatele (mys, klavesnice) na prikazy editoru
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID);
	virtual int MouseRight(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseRightFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int TimerImpulse(typeID timerID, float time);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int KeyboardUp(UINT character, UINT state);

	virtual void Normalize();		// predelani normalizace na umisteni mapy do pozice ji vyhrazene editorem
	//virtual void UnMaximize();		


	void DeleteObjectByType(OPointer obj);	
		// smaze objet "obj" pokud odpovida typu (objDeleteType,layerDeleteType)
	
	void SetInsertCursorColor(int curIndex, CEDObjectProperty *op, int cx, int cy, int cz);
		// nastavi barvu kurzoru, podle toho zda lze objekt "op" umistit na souradnice (cx,cy,cz)
		// "curIndex" je index do pole kurzoru (kvuli oblasti 3x3)
	void SetDeleteCursorColor(int curIndex, int cx, int cy, int cz);
		// nastavi barvu kurzoru, podle toho zda lze objekt "op" umistit na souradnice (cx,cy,cz)
		// "curIndex" je index do pole kurzoru (kvuli oblasti 3x3)

	int SomeObjsToDelete(int cx, int cy, int cz);
		// vrati zda se na bunce (cx,cy,cz) vyskytuje nejaky objekt na smazani, ktery vyhovuje
		// podminkam na typ objektu a vrstvu (objDeleteType/layerDeleteType)
	int DeleteTypeMatch(OPointer obj);
		// podle editacniho modu "editor->GetSameTypeMod()" urci zda objekt "obj"
		// je mozno smazat nebo nikoliv (zda vyhovuje podminkam - objDeleteType/layerDeleteType)
		// vraci 1 - lze smazat, 0 - nikoliv

	void SetPoint(int type, int px, int py, int pz, int cx, int cy, int cz);	
		// nastavi bod u skupinove promenne
		// zda jde o bod pri vyberu bodu, bunky, oblasti nebo bunecne oblasti se pozna z modu editoru "editor->GetSelectLocationMod()"
		// type==1 - prvni bod oblasti (zacatek vytvareni oblasti, pevny bod)
		// type==2 - druhy bod oblasti (volny bod)

	CGEnCursor* cur;	// bunkovy kurzor 
	CGEnCursor* curs[8];	// pole bunkovych kurzoru pro vkladani/mazani ve velkem (oblast 3x3) - "editor->GetLargeMod()"

	CGEnCursor* cellMarker;		// oznacovac pro bunku
	CGUIRectHost* pointMarker;	// oznacovat pro bod
	CGUILine* areaMarkerBorder[4];	// hranicni obdelnik kolem oznacovace oblasti
	CGUIRectHost* areaMarkerRect;	// vypln obdelniku oznacovace oblasti


	CKerName* objDeleteType;		// typ objektu, ktery je mozno mazat
	unsigned char layerDeleteType;	// vrstva objektu, ktera lze mazat
	
	bool cursorCellSet;	// zda je pouzit bunkovy kurzor
	int colX,colY,colZ;	// souradnice bunkoveho kurzoru

	bool mouseButtonDown;	// urcuje zda byla stisknuta mys v okrajove casti okna citlive na scrolling
	float sdx,sdy;			// o kolik se ma VP posunout (scrollovat)

	bool mouseClicked;		// urcuje zda bylo kliknuto do okna
							// kdyz uzivatel drzi tlacitko mysi nad oknem
							// se podle "mouseClicked" pozna zda ho nad oknem i stisknul nebo ne

	bool scrolling;		// zda je zapnut scrolling pres SPACE a posun mysi
	float scroll_x, scroll_y;	// pozice kurzoru pri zapoceti scrollingu (SPACEm)
								// v souradnicich materskeho okna

	int lx,ly,lz;	// pozice prvni bodu pri vyberu lokace (pevny bod)
};

#endif 