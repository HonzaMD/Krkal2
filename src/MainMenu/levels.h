//////////////////////////////////////////////////////////////////////////////
///
///		MAIN MENU - L e v e l s
///
///		Postup po levlech, odkryvani levlu, pristupova prava ...
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////

#ifndef LEVELS_H
#define LEVELS_H

#include "types.h"

class CFSRegKey;
class CFSRegister;
class CMMLevelDir;
struct CMMDirFile;



// vyznamy bitu u Level Tagu
enum eMMLevelTags {
	eMMLTcompleted	= 1,		// zda je level dohrany
	eMMLTaccessible = 2,		// zda je pristupny k hrani
	eMMLTskipAccess = 4,		// zda je level pristupny diky necemu jinymu, nez ze ses k nemu dohral (pristupny vzdy, editoval s ho)
	eMMLTeditable	= 8,		// editovatelny
	eMMLTlocked		= 16,		// zamceny
	eMMLTnewNoAccess= 32,		// nove nakopirovany level, zatim neni pristupny kvuli restrikcim v pravech
	eMMLTdeleted	= 64,		// smazany level, v postupu stale zabira pozici
	eMMLTedited		= 128,		// tys ho editoval
	eMMLTopened		= 256,		// odemceny level
	eMMLTskipable	= 512,		// zda je level preskocitelny (neblokuje pozici), zda je adresar preskocitelny nebo je zanorovaci
	eMMLTalwEditable= 1024,		// zda je vzdy editovatelny
	eMMLTalwAccess	= 2048,		// zda je vzdy pristupny
};




class CMMProfiles {
	friend CMMDirFile;
	friend CMMLevelDir;
public:
	CMMProfiles() { ClearVars(); }
	~CMMProfiles();

	int Open(const char *filename);	// otevre profil, vraci regerror (FSREGOK, ..)
	char *ReadProfileName(const char *filename);  // overi zda existuje profil a vrati jeho jmeno. Alokuje ho. Vrati null v pripade chyby
	int New(char *Name);			// vytvori novy profil, 1 OK, 0 - spatne jmeno, obsahuje nepristupne znaky
	int Save();						// Ulozi profil

	void ChangeDir(char *Dir);		// Zmeni aktualni adrsar. Je traba zadavat ve tvaru $GAMES$/...
	void ChangeDirUp();				// vynori se o uroven vys (operace ..)
	void ChangeDirDown(char *Dir);	// zanori se do adresare Dir
	char *GetDir();					// vrati aktualni adresar

	void CompleteLev(const char *filename);		// volat, kdyz dohrajes level, aby byl mezi dohranejma
	void EditSaveLev(const char *filename);		// volat, kdyz ulozis level v editoru
	void OpenLev(const char *filename);			// nastavi level jako otevreny

	int SaveError;					// 1 nastal, 0 nenastal
private:
	void ClearVars() { name=0; completed=0; opened=0; edited=0; profile=0; SaveError=0; dir=0;}
	void SetGame();				// nastavi promenou $GAME$ pro filesystem

	CFSRegKey *name;			// jmeno hrace
	CFSRegKey *dir;				// aktualni adresar - zacina na $GAMES$/
	CFSRegister *completed;		// registr s dohranymi levly
	CFSRegister *opened;		// otevrene levly
	CFSRegister *edited;		// mnou editovane levly
	CFSRegister *profile;		// otevreny profil
};


extern CMMProfiles *MMProfiles;





struct CMMDirFile {
	friend class CMMLevelDir;
public:
	CMMDirFile() { Type = -1; Name = 0; Author = 0; UserName = 0; UserNameExt = 0; Tags = 0; next = 0; Difficulty = 0; Password = 0; LevelPath = 0; LevelDir = 0; }
	~CMMDirFile() { SAFE_DELETE_ARRAY(Name); SAFE_DELETE_ARRAY(Author); SAFE_DELETE_ARRAY(UserName); SAFE_DELETE_ARRAY(UserNameExt); SAFE_DELETE_ARRAY(Password); SAFE_DELETE_ARRAY(LevelPath); SAFE_DELETE_ARRAY(LevelDir); }
	int Type;		// viz eEXTensions z Kerservices
	char *Name;			// jmeno fajlu
	char *Author;
	char *LevelPath;	// absolutni cesta od Games k levlu (k !level). Jen pro levly
	char *LevelDir;		// Absolutni cesta do adresare s levlem. Jen pro levly
	char *UserName;		// zkracene jmeno fajlu, prelozene, bez pripony
	char *UserNameExt;  // zkracene jmeno fajlu, prelozene, s priponou
	double Difficulty;	// uroven obtiznosti
	int Tags;		// nastoveno u levlu a u adresaru. Bitova maska. viz eMMLevelTags
	char *Password;
	CMMDirFile *next;
private:
	int LoadLevelInfo(CMMLevelDir *dir); // Nahraje jazykove lokalizovane jmeno, nastavi tagy pro levly / adresare. Vrati 0, pokud se to nahrat nepodarilo
	int Compare(CMMDirFile *f2);	// vrati jedna, pokud f2 je null nebo pokud je this pred f2
};


class CMMLevelDir {
	friend class CMMLevelDirs;
	friend struct CMMDirFile;
public:
	CMMLevelDir(char *path);// otevre adresar
	~CMMLevelDir();
	void SetAccessTags();		// nastavi tagy pro pristupy
	char *Path;				// absolutni cesta s unifikovanyma lomitkama
	char *GamesPath;		// cesta zacinajici od Games, pokud je, unifikovana lomitka
	CMMDirFile *Files;		// spojak vsech entit v adresari
private:
	CFSRegister *Profile;   // profil adresare pokud existuje, jinak null
	int AccessCfg;			// olik levlu mam najednou pristupnych, 0 vsechny
	int EditRights;			// 0 - jen dohrane, 1 - vsechny, 2 - nic
	CFSRegKey *OrigPath;	// casta zacinajici od Games, kdyz adresar vzniknul
	CFSRegKey *Password;	// hlavni heslo
	CFSRegKey *PlayPass;	// heslo na docasne zpristupneni k hrani
	CFSRegKey *EditPass;	// heslo na docasne zpristupneni k editovani

	CMMLevelDir *next;		// dalsi nactenej adresar
};





class CMMLevelDirs {
	friend CMMLevelDir;
public:
	CMMLevelDirs() { dirs=0; GameDir=0;}
	~CMMLevelDirs();
	CMMLevelDir *LoadDir(char *Path);	// nahraje adresar. Pokud je string null nahraje aktualni adresar
	CMMLevelDir *ReLoadDir(char *Path);	// nahraje adresar. Nepouzije Cache, tu refreshne. Pokud je string null nahraje aktualni adresar
	void InvalidateDir(char *Path);		// prislusny adresar odstrani z cache. Pokud je string null nahraje aktualni adresar
	CMMLevelDir *LoadActualDir() { return LoadDir(MMProfiles->GetDir());}	// nahraje aktualni adresar nastaveny u profilu
	CMMLevelDir *LoadUpDir(CMMLevelDir *dir, int VerifyGames=1);		// nahraje adresar o uroven vys, nez je dir. Pokud je nastaven Verify Dir, tak vrati null v pripade, ze vysledek neni nekde v $GAMES$
	void UnifySlashes(char *path);		// sjednoti vsechny lomitka na /
	char *GetGameDir();					// vrati absolutni cestu do games, unifokovana lomitka
	char *GetGamePath(char *abspath);	// prevede absolutni cestu na cestu od Games. string alokuje
	char *GetVersionString(const char *FileName);	// vykucha string vezrze z mena fajlu. Funkco vraci pointr na svuj vnitrni bufffer. Vrati 0 pri neuspechu
private:
	CMMLevelDir *dirs;		// spojak nactenych adresaru
	char *GameDir;			// absolutni cesta do games, unifokovana lomitka
	char buff[30];
};


extern CMMLevelDirs *MMLevelDirs;


#endif