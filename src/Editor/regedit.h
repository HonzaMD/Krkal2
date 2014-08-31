///////////////////////////////////////////////
//
// regedit.h
//
// Interface pro editor registru
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef ED_REGEDIT_H
#define ED_REGEDIT_H

#include "tree.h"
#include "register.h"

//////////////////////////////////////////////////////////////////////
// CEDRegEdit
//////////////////////////////////////////////////////////////////////

// zobrazuje strukturu registru ve forme stromu

class CEDRegEdit : public CGUITree
{
public:
	CEDRegEdit(float _x, float _y, float _sx, float _sy, char *_dir, char *_file);
		// otevre registr "file" v adresari "dir"
		// zobrazi jeho obsah po urovnich, ve forme stromu
	virtual ~CEDRegEdit();

	virtual int TreeHandler(typeID treeItem, int index, int state);	
		// state = (0/1/2/3) Clicked/MouseButtonLeft/MouseButtonRight/Keyboard focus (najeto na polozku klavesnici)
		// zajistuje postupne pridavani polozek registru
		// pro pripad, ze je v registru vice jak 10 polozek, pridavaji se postupne po 10
	virtual void TreeUpdateHandler(typeID treeItem, int index);
		// vytvareni registru po urovnich
		// registr v registru se vytvori, az kdyz ho uzivatel chce rozbalit, ne rovnou pri startu
		// kvuli rychlosti vytvareni velkych registru
		

	void AddRegister(CFSRegister *r, CGUITreeItem* parentItem);
		// prida jednu uroven registru "r" do polozky "parentItem"
	void AddKey(CFSRegKey* key, CGUITreeItem* parentItem, int val_index=0);
		// prida hodnotu klice "key" k polozce registru "parentItem"
		// "val_index" urcuje od kolikateho klice se maji polozky pridavat
	
protected:

	CFSRegister *reg;	// otevreny registr

	char *dir;	// adresar, kde se registr nachazi
	char *file;	// jmeno souboru registru
};

#endif