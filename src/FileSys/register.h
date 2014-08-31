//////////////////////////////////////////////////////////////////////////
///
///		R E G I S T R Y  - Prace se strukturovanym souborem
///
///		A: M.D.
///
//////////////////////////////////////////////////////////////////////////

/*
	Co to umi?
	----------
	Umi pracovat se soubory se specialni strukturou. Soubor zacina hlavickou
	KrkalReg\0. Nasleduje hlavicka zadana uzivatelem
	(To je string ukonceny nulou) Podle hlavicky se kontroluje jestli je soubor OK
	Dale soubor obsahuje klice. Klic je tvoren jmenem (0-string) a 0 az n datovymi 
	polozkami. Ke kazdemu klici je prirazen typ, ktery urcuje jakeho typu budou
	polozky klice. Mozne typy jsou: Char 1B, Int 4B, Double 8B, _in64 8B, string 1B 
	(string se chova naprosto stejne jako typ char) a typ Register = vnoreny registr.
	Data klicu je mozne menit, pridavat a ubirat. Velikost klicu se podle toho 
	automaticky meni. Na data u klice se lze divat jako na samostatny stream (soubor)
	a vetsina operaci s daty je take velmi podobna klasickym operacim se souborem.

	Pouziti
	-------
	Registr se otevre nebo vytvori volanim:  new CFSRerister(<cesta>,<hlavicka>);
	Metodou GetOpenError() lze otestovat chyby pri otevirani
	Trida CFSRegister umoznuje vyhledavat, pridavat a rusit klice.
	Metoda WriteFile ulozi cely registr na disk (vcetne vnorenych registru)
	Trida CFSRegKey umoznuje praci se samotnym klicem.
	Pouzivejte stravne typy metod pro dany typ klice!!
	Pokud je klic typu registr, metodou GetSubRegister() ziskate pristup k vnorenemu 
		registru. Data samotneho klice v zadnem pripade nemente!!
	Dulezite promene u klice:
		pos - aktualni polozka - zde se bude zapisovat nebo cist. Zmena pos = seek
		top - pocet polozek - muzete menit (tim se daji umazavat polozky)
	Je mozny i primy pristup, Ale pozor, pointery na primy pristup muzou prestat platit!
		(Vhodne treba, kdyz nejaka funkce si chce precist (=okopirovat) string)

*/


#ifndef FSREGISTER_H
#define FSREGISTER_H

#include "types.h"
#include "fs.h"


class CFSRegister;
struct CFSRegHT;
class CFSRegKey;

#define FSRMAXREGKEYVEL (1024*1024*16) // Maximalni velikost klice. Omezeni je kvuli bezpecnosti.
#define FSMAXHEADSIZE 1024
#define KER_REGISTER_HTS 251

// Mozne hodnoty OPEN ERRORU
#define FSREGOK 1
#define FSREGFILEERROR 2
#define FSREGCONTENTERROR 0

// Zadaj, kdyz nechces registr loadovat, ale rovnou otevrit prazdny
#define FSREGCLEARIT 1

// Podporovane Typy Klicu:
#define FSRNumTypes 6
enum EFSRTypes {
	FSRTchar = 0,
	FSRTint,
	FSRTint64,
	FSRTdouble,
	FSRTregister,
	FSRTstring
};

extern int FSRTypeSizes[FSRNumTypes];
extern CFSRegHT *RegActualHT;




///////////////////////////////////////////////////////////////////////////////////
///
///			H A S H O V A C I   T A B U L K A   P R O   R E G I S T R Y
///
///////////////////////////////////////////////////////////////////////////////////


struct CFSRegHT {
	CFSRegHT() {
		int f;
		for (f=0; f<KER_REGISTER_HTS; f++) HT[f] = 0;
		NumKeys = 0;
		NumRefs = 0;
	}
	~CFSRegHT() {
		if (RegActualHT == this) RegActualHT = 0;
	}
	void Release() {
		NumRefs--;
		if (NumRefs<=0) delete this;
	}
	CFSRegKey *HT[KER_REGISTER_HTS];
	int NumKeys;
	int NumRefs;
	CFSRegKey *Member(const char *name, CFSRegister *r, CFSRegKey *PrevKey = 0); // Vyhleda nasledujici vyskyt, nebo prvni vyskyt, pokud je PrevKey ponechan na 0
	int HashFunction(const char *name, CFSRegister *r);
};



///////////////////////////////////////////////////////////////////////////////////
///
///			K L I C		- CFSRegKey
///
///////////////////////////////////////////////////////////////////////////////////

class CFSRegKey  {
friend CFSRegister;
friend CFSRegHT;
public:
	void writec(char a); // zapis na pos, pos se posune na dalsi polozku,
	void writei(int a);  // pocet dat se pripadne rozsiri (top)
	void writed(double a);  // pokud se data jiz nevejdou do stavajiciho pole, bude zvetseno
	void write64(_int64 a);  // pozivej spravnou funkci k danemu typu klice !!
	char readc();  // cteni dat, pokud ctete nezapsana data, nebo data za koncem (za top) je vysledek nedefinovany (vetsinou 0)
	int readi();	// cteni z pos, pos se posune na dalsi polozku	
	double readd();  // pozivej spravnou funkci k danemu typu klice !!
	_int64 read64();
	void blockwrite(const void *source, int Size); // size je v poctech polozek
	int blockread(void *dest, int Size); // size je v poctech polozek, muze precist i min, kdyz narazi na konec streamu. Vraci pocet prectenych polozek.
	char * stringread(); // vraci Null, kdyz se cte za koncem, jinak string ukonceny nulou. (vytvori ho) Je na tobe, abys ho zrusil. Pouzivej jen na typ char nebo string
	void stringwrite(const char *string) { assert(type==FSRTchar||type==FSRTstring); blockwrite(string,strlen(string)+1); } //zapise string pomoci blockwrite. Pouzivej jen na typ String nebo Char!
	void SetPosToNextString(); // Najde prvni nulu (testuje polozku, ne char)

	int pos; // pozice, kam se bude cist, nebo zapisovat. Nastav ji = seek
	int top; // vrchol dat (=pocet dat) nastav, na x, kdyz chces polozky od x dal smazat. (Kdyz top zvetsis, klic se zvetsi o nedefinovane polozky) Funkce write sami nastavuji top.
	int eof() {return (pos>=top); } // vrati 1 kdyz jsem za koncem (nejde cist)
	void seek(int position) {pos=position;} // nastaveni pozice
	
	void rename(char *Name); // prejmenovani klice (jmeno bude zkopirovano)
	char *GetName() {return name;} // jmeno nemenit! Pointer muze prestat platit!
	CFSRegKey *GetNextKey() {return next;} // vrati dalsi klic v registru
	CFSRegister *GetSubRegister() {return subregister;} // vrati vnoreny registr (0 kdyz neexistuje)
	int CFSGetKeyType() {return type;} // vrati typ tohoto klice

	int resize(int vel); // zvetsi pole s daty na dvojnasobek nebo na vel (podle toho, co je vic)	
	char * GetDirectAccess() { return data; } // primy pristup. Je treba si volat resize a settop
											  // po resize se musi GetDirectAccess zavolat znova!!
											  // Nerusit!
	char * GetDirectAccessFromPos() { return data+pos*FSRTypeSizes[type]; } // vrati ukazatel na aktualni pozici v datech (Po resize se musi volat znova)
	void SetAllData(char *Data, int Size); // Data v klici budou zahozena a nahrazena temito daty. Size je bajtech. o Data* se od teto chvile stara registr. Uz nemenit, nerusit, Data zrusi registr
	int GetDataArraySize() {return size; } // vrati velikost pole s daty v bajtech (vyuziti jen na primi pristup)
private:
	CFSRegKey(CFSRegister *reg,char Type, char *Name, CFSRegister *SubReg = 0); //Konstruktor pro tvorbu Noveho klice
	CFSRegKey(CFSRegister *reg,unsigned int *datasource,char *buffer, int &POS, int vel); //Konstruktor pro nacteni klice ze souboru
	virtual ~CFSRegKey(); 
	void zjistivelikostazmenu(int &vel,int &changed); // zjisti aktualni velikost
	void collectall(char *buff, int &pos, int &tablepos); // posbira vsechny data a sesype je do jednoho pole buff (pos-pozice v poli, tablepos - pozice tabulky klicu)

	void AddToHashTable();  // Pridej do hashTable
	void HFindAndDelete(CFSRegKey **p2,CFSRegKey *key); // najde jmeno key v seznamu p2 a odstrani ho

	char *data; // pointer na data
	char *name; // jmeno klice
	int size;   // velikost pole na data v Bajtech
	int size2;  // velikost pole na data v polozkach
	UC type;   // typ klice
	UC state;  //zda ma klic okopirovany data - bit0 (jmeno - bit1) k sobe a zda je ma smazat
	CFSRegKey *next;  // dalsi klic v registru
	CFSRegKey *HTnext; // dalsi klic v hashovacim retizku
	CFSRegister *subregister; // pointer na subregister
	CFSRegister *MyRegister; // registr, ve kterem je tento klic ulozen
};





/////////////////////////////////////////////////////////////////////////////////
///
///			R E G I S T E R		- CFSRegister
///
/////////////////////////////////////////////////////////////////////////////////


struct CFSRootRegInfo {
	int deletebuff;  // zda vlastnim buffer a budu ho muset tedy smazat
	char *buffer;  // data
	char *Path;    //  absolutni cesta k souboru (u vnorenych registru je to NULL)
	char *Head;    // hlavicka (u vnorenych registru je to NULL)
	int CompressMode; // Hodnota nema smysl u SubRegistru. -1 Povodni nastaveni (viz WriteFile u FS), 0 - NoCompress, 1 - Compress
	CFSRootRegInfo() {deletebuff=0; buffer=0; Path=0; Head=0; CompressMode=-1; }
};


class CFSRegister {
friend CFSRegKey;
public:
	CFSRegister(const char *path, char *head, int ClearIt=0); //path - cesta k souboru s registrem. Head hlavicka souboru pr. "KRKAL LEVEL" - bude kontrolovana
															  // Nastav ClearIt na 1 (FSREGCLEARIT) pokud chces vytvorit prazdny registr. Pri operaci save bude pripadny stary registr prepsan (OpenErroe je v tomto pripade vzdy FSREGOK)
															  // Pokud Nastavis Head na NULL hlavicka se precte z fajlu (nepouzivat dohromady s flagem FSREGCLEARIT a pro tvoreni novych registru)
	CFSRegister(); // KONSTRUKTOR  pro vytvoreni prazdneho (nepropojeneho!) subregistru
	~CFSRegister(); // Registr se zrusi, zmeny se neulozi
	int WriteFile(); // Zapsani registru na disk, vrati 1 - OK, 0 - Chyba
	int ChangePath(const char *path); // zmeni cestu k souboru (save as .. nekam jinam)
	CFSRegKey *AddKey(char *name, int typ) {return new CFSRegKey(this,typ,name);} // Vytvori novy klic typu typ v registru, jmeno bude zkopirovano
	CFSRegKey *AddRegisterToKey(char *name, CFSRegister *SubReg) {return new CFSRegKey(this,FSRTregister,name,SubReg);} // Prida do registru registr. (propoji je) Pridavany registr musi byt samostatne vytvoreny (nepropojeny)
	void DeleteKey(CFSRegKey *key); // zrusi klic
	CFSRegKey *GetFirstKey() {return keys;} // vrati prvni klic ze seznamu vsech klicu registru
	CFSRegKey *FindKey(char *name) {if (!HashTable) CreateHT(); return HashTable->Member(name,this);} // vyhleda klic pogle jmena
	CFSRegKey *FindNextKey(char *name,CFSRegKey *PrevKey) {if (!HashTable) CreateHT(); return HashTable->Member(name,this,PrevKey);} // vyhleda nasledujici klic podle jmena
	int FindKeyPos(CFSRegKey *key); // postupnym prohledavanim zjisti pozici klice. vrati pozici nebo -1 v pripade neuspechu
	void DeleteAllKeys(); // zrusi vsechny klice
	void SeekAllTo0(); // Nastavi u vsech klicu v registru a podregistrech seek na 0
	int GetOpenError() { return OpenError;} // 1 - OK Register Loaded; 2 - Error in file (not exists); 0 - Error In Registr; viz makyrka FSREGOK, ..
										   // Pri chybe je vytvoren prazdny registr. Zapsan do souboru bude ale az prikazem WriteFile
	void SetRegisterToBeCompressed() { if (RootInfo) RootInfo->CompressMode = 1;}
	void SetRegisterToBeUnCompressed() {  if (RootInfo) RootInfo->CompressMode = 0;}
	int GetNumberOfKeys() {return NumKeys;} // Vrati pocet klicu v tomto registru
	static int VerifyRegisterFile(const char *path, char *head = 0); // precte ze souboru hlavicku a overi zda sedi. Vraci OpenError.
private:
	void zjistivelikostazmenu(int &vel,int &changed); // zjisti aktualni velikost
	void collectall(char *buff, int &pos, int &tablepos); // posbira vsechny data a sesype je do jednoho pole buff (pos-pozice v poli, tablepos - pozice tabulky klicu)
	CFSRegister(CFSRegKey *key); // Nahraje registr z klice
	void CreateHT(); // vytvori hashovaci tabulku
	UC RegChanged; // Kdyz 1 tak registr byl zmene, kdyz 0 tak se nevi
	UC OpenError;
	short NumKeys;  // pocet klicu
	CFSRegKey *keys; // seznam vsech klicu - prvni klic
	CFSRegKey *lastkey; //posledni klic
	CFSRegHT *HashTable;  // hashovaci tabulka jmen klicu tohoto registru
	CFSRootRegInfo *RootInfo; // informace pro korenovy registr
};






#endif
