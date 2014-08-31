//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - K e r C o n s t a n t s
///
///		Definica zakladnich konstant
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////

#ifndef KERCONSTANTS_H
#define KERCONSTANTS_H

#include "types.h"


class CKerName;
class CKerArrChar;
class CKerArrDouble;
class CKerArrInt;
class CKerArrPointer;
class CKerArrObject;
class CKerArrName;

typedef DWORD OPointer;  // typ Pointer na Objekt


// Typy jmen
enum eKerNameType {
	eKerNTvoid=0,	// neurceno
	eKerNTobject,	// objekt nebo skupina objektu
	eKerNTmethod,
	eKerNTparam,
	eKerNTauto,
	eKerNTobjectShadow,
	eKerNTkey,
	eKerNTsound,
	eKerNTobjectVoid,	// voidy propojene s objekty
	eKerNTautoVoid,		// voidy vznikle pri nahravani automatismu
};

enum eEdNameType {
	eEdNTvoid = 1,
	eEdNTobject = 2,
	eEdNTmethod = 4,
	eEdNTparam = 8,
	eEdNTeverything = 0xFFFFFFFF,
};

// typy zakladnich promennych
enum eKerTypes {
	eKTchar=0,		// unsigned char, 1B
	eKTdouble,		// 8B
	eKTint,			// signed int, 4B
	eKTpointer,		// pointer, 4B, u SAFE volani zakazano
	eKTobject,		// OPointer, 4B
	eKTname,		// ID jmena, CKerName* , 4B
	eKTvoid,		// pouziti jen u navratoveho typu funkce, 0B
	eKTarrChar,		// dynamicke pole zakladnich typu
	eKTarrDouble,
	eKTarrInt,
	eKTarrPointer,
	eKTarrObject,
	eKTarrName,
	eKTstruct,
	eKTstring=100,  // 100 az 350 jsou stringy velikosti 0 az 250
	eKTret=512,		// pricte se k beznemu typu, kdyz jde o vraceni hodnotou
	eKTretNormal	= 0,		// popis navratove funkce:
	eKTretOR		= 1*1024,
	eKTretAND		= 2*1024,
	eKTretADD		= 3*1024,
	eKTretMask		= 3*1024,	// maska, pomoci ktere z typu vyziskam navratovou funkci
	eKTsubRetMask   = 1024-1,	// maska pro vse co je pod ret fci
};

#define KERVARUSESSHIFT 10	// shift na odrolovani nepotrebnych bitu
#define KERVARUSESSIZE 21	// pocet promennych se specifickym vyznamem

// Vyznamy promennych pro kernel (promenna use)
enum eKerVarUses {
	// Jmeno promenne, pokud je znama (na USE je potreba napred provest shift KERVARUSESSHIFT)
	eKVUnone		= 0,
	eKVUcollizionCfg = 1,	// Konfigurace kolizi  -- UC
	eKVUcellz		= 2,	// Pocet bunek nad sebou, ktere obj zabira (0 az n) -- int
	eKVUcellx		= 3,	
	eKVUcelly		= 4,
	eKVUcellr		= 5,	// polomer u kruhovych oblasti na hexech	-- int
	eKVUcubeX1		= 6,	// relativni umisteni kolizni krycle		-- int
	eKVUcubeY1		= 7,	eKVUcubeZ1	= 8,
	eKVUcubeX2		= 9,	eKVUcubeY2	= 10, eKVUcubeZ2 = 11,
	eKVUx			= 12,	// --- int
	eKVUy			= 13,	// souradnice objektu
	eKVUz			= 14,	
	eKVUclzAddGr	= 15,	// Pricitaci mnozina objektu, se kterymi to koliduje  -- OPointer
	eKVUclzSubGr	= 16,	// Odcitaci mnozina objektu, se kterymi to koliduje
	eKVUaPicture	= 17,
	eKVUaNoConnect  = 18,
	eKVUclzFceGr	= 19,
	eKVUmsgRedirect = 20,

	// Vyznamy jednotlivych bitu promenne USE
	eKVUBlevelLoad	= 1,	
	eKVUBauto		= 2,
	eKVUBeditable	= 4,
	eKVUBspecialEdit= 8,
	eKVUBpozorPozor	= 16,
	eKVUBexclusive	= 32,
	eKVUBplannarNames=64,
	eKVUBincludeNull=128,
	eKVUBobjInMap	=256,
	eKVUBobjOutMap	=512,
};


struct CKerVarUsesInfo {
	UC PozorPozor, SpecialEdit;
	int EditType, NamesMask;
};

struct CKerVarGroupInfos {
	int typ;
	int count;
};

// typy znamych promennych
extern eKerTypes KerVarUsesTypes[KERVARUSESSIZE];
// jmena znamych promennych
extern char * KerVarUsesNames[KERVARUSESSIZE];
// dalsi info o znamych promennych
extern CKerVarUsesInfo KerVarUsesInfo[KERVARUSESSIZE];


enum eKerLimitsCfg {
	eKLCnone		= 0,
	eKLCinterval	= 1,
	eKLClist		= 2,
	eKLCup			= 3, // Od meze nahoru
	eKLCdown		= 4, // Od meze dolu
};

enum eKerEditTypes {
	eKETdefault		= 0, // default podle typu
	eKETnumeric		= 1,
	eKETcharacter	= 2,
	eKETboolean		= 3,
	eKETconnectionMask = 4,
	eKETautomaticGr	= 5,
	eKETscripted	= 6,
	eKETstring		= 7,

	eKETgroupBit	= 64,
	eKET2Dcell		= 64,
	eKET3Dcell		= 65,
	eKET2Dpoint		= 66,
	eKET3Dpoint		= 67,
	eKET2Darea		= 68,
	eKET3Darea		= 69,
	eKET2DcellArea	= 70,
	eKET3DcellArea	= 71,
};

#define KERNUMVARGROUPS 6
extern CKerVarGroupInfos KerVarGroupInfos[KERNUMVARGROUPS];


// Pocet znamych KSID jmen
#define MAXKNOWNNAMES 40
#define MAXKERKNOWNDEPENDENCIES 2

// Data ke kazdemu znamemu jmenu
struct CKnownNames {
	char *namestr;  // jmeno v textove podobe (zadano, konstanta)
	eKerNameType type;  // typ jmena (zadano, konstanta)
	int MethodType;		// navratovy typ metody.
	CKerName *Name;		// pointer. Spocita se pri startu Kernelu (S Kazdym startem je jiny!!)
};

enum eKerKnownNames {  // poradova cisla do znamych jmen:
	eKKNconstructor			= 0,
	eKKNloadConstructor		= 1,
	eKKNcopyConstructor		= 2,
	eKKNdestructor			= 3,
	eKKNdefaultObject		= 4,
	eKKNEsaveMe				= 5,
	eKKNmapPlaced			= 6,
	eKKNmapRemoved			= 7,
	eKKNplaceObjToMap		= 8,
	eKKNmGetObjects			= 9,
	eKKNcellsArray			= 10,
	eKKNobject				= 11,
	eKKNremoveObjFromMap	= 12,
	eKKNmoveObjInMap		= 13,	
	eKKNeverything			= 14,
	eKKNnothing				= 15,
	eKKNtestCollision		= 16,
	eKKNCollisionKill		= 17,
	eKKNkeepCellArray		= 18,
	eKKNremoveCellArray		= 19,
	eKKNplaceCellarray		= 20,
	eKKNmoveEnded			= 21,
	eKKNobjectArray			= 22,
	eKKNisMoveCorrect		= 23,
	eKKNcoordX				= 24,
	eKKNcoordY				= 25,
	eKKNcoordZ				= 26,
	eKKNdefaultAuto			= 27,
	eKKNtriggerOn			= 28,
	eKKNtriggerOff			= 29,
	eKKNobjType				= 30,
	eKKNresizeMap			= 31,
	eKKNscrollObj			= 32,
	eKKNitemID				= 33,
	eKKNbuttonID			= 34,
	eKKNbuttonUserID		= 35,
	eKKNgroupID				= 36,
	eKKNpublicMethods		= 37,
	eKKNloadGeme			= 38,
	eKKNsaveGame			= 39,
};

// Seznam vsech znamych jmen. Zname jmeno ma zname poradi v tomto seznamu. 
// Pres tento seznam lze tedy po staru Kernelu zjistit CKerName *
// Seznam je JEN pro cteni
extern CKnownNames KnownNames[MAXKNOWNNAMES];
extern int KerKnownDependencies[MAXKERKNOWNDEPENDENCIES*2];




enum eKerCollozionCfg {
	eKCCpoint		= 0,
	eKCConeCell		= 1,
	eKCCrect1		= 2,
	eKCCrect2		= 3,
	eKCChexCircle	= 4,
	eKCCcolCube		= 5,	
	eKCCoutOfMap	= 7,	 // nebude zadna kolize, ani s mapou
	eKCCtriggerBit	= 16,
	eKCCwall		= 32,
	eKCCfloor		= 64,
	eKCCcell		= 32+64,
	eKCCnothing		= 0, // kolize je s mapou, ale neni s zadnymi objekty
	eKCClevelMask   = 32+64, // bity urcijici kolizi vertikalne - urceni pater	
	eKCCareaMask	= 0xF,	 // bity urcujici kolizi horizontalne
	eKCCcenterColBit = 128,  // pro triggery, u druheho objektu se ignoruje kolizni oblast. Bere se jen base point
	eKCCinvisible	= 128,   // pro netrigery. Objekt je neviditelny
	eKCCdefault		= eKCConeCell|eKCCwall,
};



// Zpusoby behu kernelu 
#define NORMAL_RUN 0	// Hra
#define EDITOR_RUN 1	// Editor



enum eKerCallTimes {
	eKerCTnow=4, // okamzite volani - nepouzivat. nepripustne.
	eKerCTcallend=5, // volani po ukonceni vsech metod volanych do urciteho objektu (callendObject)
	eKerCTmsg=0, // Normalni zprava pro toto kolo
	eKerCTend=1, // zprava na konci kola
	eKerCTnext=2, // Zprava v dalsim kole
	eKerCTnextend=3, // Zprava na konci dalsiho kola
	eKerCTtimed=6	// Do CallendObject dej cas vyvolani
};


enum eKerCellTypes {
	eKCTctverce = 0,
	eKCTkosoctverce,
	eKCTplocheHexy,
	eKCTspicateHexy
};



#define RETERR {assert(false); return 0; }
// pro zvladani chyb pri nahravani kernelu


#endif
