///////////////////////////////////////////////
//
// editorMenu.h
//
// Interface pro Menu editoru - New Level, Load/Save Level a souvisejici dialogy
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef ED_EDITORMENU_H
#define ED_EDITORMENU_H

#include "gui.h"
#include "tree.h"	// kvuli CEDFileSelector
#include "dialogs.h"

class CGUIEditWindow;

////////////////////////////////////////////////////////////////////////////////////////////////////
// EditorMenuHandler
////////////////////////////////////////////////////////////////////////////////////////////////////

// hlavni spravce udalosti menu editoru
// zarizuje funkcnost menu, tedy reaguje na stisknuti tlacitek menu
// vytvarenim dialogu atp.

class CEDEditorMenuHandler : public CGUIEventProgram
{
	virtual void EventHandler(CGUIEvent *event);
public:
	CEDEditorMenuHandler(int numIcons) { icons = new typeID[numIcons]; helpWindow = levelPropertiesDlgID = newLevelDlgID = saveLevelDlgID = msgBoxID = chBoxID = 0; msgBoxIndex = -1; };
	virtual ~CEDEditorMenuHandler() { SAFE_DELETE_ARRAY(icons); };

	void CreateNewDialog();
		// vytvori dialog pro vytvareni noveho levelu na zaklade skriptu

	typeID *icons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti
					// icony reprezentuji polozky menu (urcuji se v CEDEditorDesktop::AddEditorMenu())

	typeID newLevelDlgID, saveLevelDlgID, levelPropertiesDlgID;	// objID vytvorenych dialogu
	typeID helpWindow;
	
	typeID msgBoxID;	// ID messageBoxu (s ujistenim, zda opravdu chce uzivatel provest zvolenou akci)
	typeID chBoxID;		// ID CheckBoxu pro urceni, zda chce uzivatel pouzit GameMOD nebo nikoliv
	int msgBoxIndex;	// msgBox ktere polozky menu je pouzit
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// File Selector
////////////////////////////////////////////////////////////////////////////////////////////////////

// upraveny FileBrowser pro vyber skriptu levelu a automatismu
// co se bude vybirat, tj. jake soubory v adresarich se budou zobrazovat urcuje maska - filterExt

class CEDFileSelector : public CGUIFileBrowser
{
public:
	CEDFileSelector(float _x, float _y, float _sx, float _sy, char* path="$GAME$", char* label="GAME:");
	virtual ~CEDFileSelector();

	int filterExt;	// pouze jake pripony se budou zobrazovat	(viz. KerServices.h - eEXTensions)

	static int Compare(CGUITreeItem *aa, CGUITreeItem *bb);	// setridi: nejdriv adresare, potom levely
	static int CompareLevelDIR(CGUITreeItem *aa, CGUITreeItem *bb);	// setridi: nejdriv adresare, potom levely, potom ostatni soubory
protected:

	virtual int TreeHandler(typeID treeItem, int index, int state);	// nepouzito
	virtual int Filter(CGUITreeItem* parent, const char *name, int dir, int first);	// nepouzito
	virtual CGUIFileBrowserItem* AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir);
		// pridavani polozek podle jejich typu voli ikonu, atp.

	virtual int Keyboard(UINT character, UINT state);	// zarizuje zavreni okna po vyberu polozky
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDScriptSelectorDlg
////////////////////////////////////////////////////////////////////////////////////////////////////

// dialog pro vytvoreni noveho levelu
// pri vytvareni levelu se musi zadat skript na nemz bude level zalozen
// jmeno levelu a adresar, kde bude level ulozen
// lze vybrat jiz existujici level, ktery bude prepsat nebo napsat jmeno noveho

class CEDScriptSelectorDlg: public CGUIDlgOkCancel
{
	
public:
	CEDScriptSelectorDlg(float _x, float _y);
	virtual ~CEDScriptSelectorDlg();

protected:

	virtual int Ok();	// inicializuje editor s vybranym skriptem
	virtual void EventHandler(CGUIEvent *event);
		// ridi ovladaci prvky dialogu
		// po stisku tlacitek vytvari FileSelectory pro vyber skriptu ci levelu


	CGUIEditWindow *edScript, *edLevel;
	CGUIButton *bScriptSelector;
	char *scriptName;
	char *levelName;
	char *levelPath;

	typeID fsScriptSelector, fsLevelSelector;
	typeID fsLevelSelectorButton;
	typeID chBoxID;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDLevelSaveSelectorDlg
////////////////////////////////////////////////////////////////////////////////////////////////////

// dialog pro ulozeni levelu pod jinym jmenem (save as..)
// umoznuje vybrat jmeno levelu a adresar, kam se ulozi
// muzu napsat nove jmeno, nebo vybrat z exitujicich levelu

class CEDLevelSaveSelectorDlg: public CGUIDlgOkCancel
{
	
public:
	CEDLevelSaveSelectorDlg(float _x, float _y);
	virtual ~CEDLevelSaveSelectorDlg();

protected:

	virtual int Ok();	// po vybrani platneho jmena a adresare ulozi level
	virtual void EventHandler(CGUIEvent *event);
		// ridi ovladaci prvky dialogu

	CGUIEditWindow *edLevel;
	CGUIButton *bLevelSelector;
	char *levelName;
	char *levelPath;


	typeID fsLevelSelector;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDAutoSelectFileDlg
////////////////////////////////////////////////////////////////////////////////////////////////////

// dialog pro vyber souboru s automatismem a adresare kde je ulozen
// lze vytvorit i novy soubor pro automatismy

class CEDAutoSelectFileDlg: public CGUIDlgOkCancel
{
	
public:
	CEDAutoSelectFileDlg(float _x, float _y);
	virtual ~CEDAutoSelectFileDlg();

protected:

	virtual int Ok();	// po vybrani souboru s automatismem inicializuje Kernel,
						// aby vedel, kam bude ukladat vybrane automatismy k objektum
						// (KerMain->AutosMain->OpenFileToSaveAutos(file,version))
	virtual void EventHandler(CGUIEvent *event);	// ridi ovladaci prvky dialogu


	CGUIEditWindow *edA, *edAPath;
	CGUIButton *bASelector;
	char *AName;
	char *APath;


	typeID fsASelector;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDLevelPropertiesDlg
////////////////////////////////////////////////////////////////////////////////////////////////////

// dialog pro editaci level properties (Autor, Flags)

class CEDLevelPropertiesDlg : public CGUIDlgOkCancel
{

public:
	CEDLevelPropertiesDlg(float _x, float _y);
	virtual ~CEDLevelPropertiesDlg();

protected:

	virtual int Ok();	// po vybrani platneho jmena a adresare ulozi level
	// ridi ovladaci prvky dialogu

	CGUIEditWindow *edAutor;
	char *autor;
	CGUIEditWindow *edCsName;
	char *csName;
	CGUICheckBox *cbSkip, *cbEditable, *cbPLayable;
};


#endif
