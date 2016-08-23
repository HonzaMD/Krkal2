//////////////////////////////////////////////////////////////////////////////
///
///		MAIN MENU - L e v e l s
///
///		Postup po levlech, odkryvani levlu, pristupova prava ...
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "levels.h"

#include "register.h"
#include "KerServices.h"
#include "krkal.h"
#include "ComFunctions.h"



CMMProfiles *MMProfiles = 0;
CMMLevelDirs *MMLevelDirs = 0;


CMMProfiles::~CMMProfiles() {
	SAFE_DELETE(profile);
}



// overi zda existuje profil a vrati jeho jmeno. Vrati null v pripade chyby
char *CMMProfiles::ReadProfileName(const char *filename){
	char *ret=0;
	CFSRegister *r = new CFSRegister(filename,"PROFILE");
	if (r->GetOpenError() != FSREGOK) {delete r; return 0;}
	CFSRegKey *k;
	if (k=r->FindKey("Name")) ret = newstrdup(k->GetDirectAccess());
	delete r;
	return ret;
}



// otevre profil, vraci regerror (FSREGOK, ..)
int CMMProfiles::Open(const char *filename) {
	Save();
	SAFE_DELETE(profile);
	ClearVars();
	profile = new CFSRegister(filename,"PROFILE");
	if (profile->GetOpenError() != FSREGOK) return profile->GetOpenError();

	CFSRegKey *k;
	if (!(k=profile->FindKey("Name"))) return FSREGCONTENTERROR;
	name = k;
	if (!(k=profile->FindKey("Dir"))) return FSREGCONTENTERROR;
	dir = k;
	if (!(k=profile->FindKey("Completed"))) return FSREGCONTENTERROR;
	completed = k->GetSubRegister();
	if (!(k=profile->FindKey("Opened"))) return FSREGCONTENTERROR;
	opened = k->GetSubRegister();
	if (!(k=profile->FindKey("Edited"))) return FSREGCONTENTERROR;
	edited = k->GetSubRegister();
	
	return FSREGOK;
}




// vytvori novy profil, 1 OK, 0 - spatne jmeno, obsahuje nepristupne znaky
int CMMProfiles::New(char *Name) {
	if (!FS->IsValidFilename(Name)) return 0;
	Save();
	SAFE_DELETE(profile);
	ClearVars();
	
	char *olddir;
	FS->GetCurDir(&olddir);
	FS->ChangeDir("$PROFILES$");
	if (FS->FileExist(Name)) {
		char *str;
		str = new char[strlen(Name)+10];
		int a=1;
		do {
			sprintf(str,"%s%i",Name,a);
			a++;
		} while (FS->FileExist(str));
		profile = new CFSRegister(str,"PROFILE",1);
		delete[] str;
	} else {
		profile = new CFSRegister(Name,"PROFILE",1);
	}

	name = profile->AddKey("Name",FSRTstring);
	name->stringwrite(Name);
	name->pos=0;
	dir = profile->AddKey("Dir",FSRTstring);
	dir->stringwrite("$GAMES$");
	dir->pos=0;
	completed = profile->AddKey("Completed",FSRTregister)->GetSubRegister();
	opened = profile->AddKey("Opened",FSRTregister)->GetSubRegister();
	edited = profile->AddKey("Edited",FSRTregister)->GetSubRegister();

	Save();
	FS->ChangeDir(olddir);
	return 1;
}




// Ulozi profil
int CMMProfiles::Save(){
	if (!profile) return 1;
	if (!profile->WriteFile()) {
		SaveError=1;
		return 0;
	}
	return 1;
}




// Zmeni aktualni adrsar. Je traba zadavat lokalne od $GAMES$
void CMMProfiles::ChangeDir(char *Dir) {
	dir->stringwrite(Dir);
	dir->top = dir->pos;
	dir->pos = 0;
	SetGame();
}




// vynori se o uroven vys (operace ..)
void CMMProfiles::ChangeDirUp() {
	char *p = dir->GetDirectAccess() + dir->top - 2;
	while (p >= dir->GetDirectAccess() && *p != '\\' && *p != '/') p--;
	if (p >= dir->GetDirectAccess()) {
		*p = 0;
		dir->top = p - dir->GetDirectAccess() + 1;
	}
}



// zanori se do adresare Dir
void CMMProfiles::ChangeDirDown(char *Dir){
	dir->pos = dir->top - 1;
	dir->writec('/');
	dir->stringwrite(Dir);
	dir->pos = 0;
	SetGame();
}



// nastavi promenou $GAME$ pro filesystem
void CMMProfiles::SetGame() {
	if (!dir || dir->top <= 9 || dir->GetDirectAccess()[8] == 0) return;
	int pos=8; // preskocim uvodni $GAMES$
	char *p = dir->GetDirectAccess()+8;
	while (*p != '\\' && *p != '/' && *p != 0) { p++; pos++;}
	p = new char[10+pos];
	int f;
	for (f=0; f<pos; f++) { p[f] = dir->GetDirectAccess()[f]; }
	p[pos] = 0;
	FS->AddFSDir("GAME",p,0);
	delete[] p;
}


// vrati aktualni adresar
char *CMMProfiles::GetDir() {
	return dir->GetDirectAccess();
}




// volat, kdyz dohrajes level, aby byl mezi dohranejma
void CMMProfiles::CompleteLev(const char *filename) {
	char *ver = MMLevelDirs->GetVersionString(filename);
	if (!ver) return;
	if (!completed->FindKey(ver)) completed->AddKey(ver,FSRTregister);
	Save();
}



// volat, kdyz ulozis level v editoru
void CMMProfiles::EditSaveLev(const char *filename) {
	char *ver = MMLevelDirs->GetVersionString(filename);
	if (!ver) return;
	if (!edited->FindKey(ver)) edited->AddKey(ver,FSRTchar);
	Save();
}



// nastavi level jako otevreny
void CMMProfiles::OpenLev(const char *filename) {
	char *ver = MMLevelDirs->GetVersionString(filename);
	if (!ver) return;
	if (!opened->FindKey(ver)) opened->AddKey(ver,FSRTregister);
	Save();
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


CMMLevelDirs::~CMMLevelDirs() {
	CMMLevelDir *d = dirs, *d2;
	while (d) {
		d2 = d;
		d = d->next;
		delete d2;
	}
	SAFE_DELETE_ARRAY(GameDir);
}




// otevre adresar
CMMLevelDir::CMMLevelDir(char *path) {
	Path = newstrdup(path);
	GamesPath = MMLevelDirs->GetGamePath(path);
	next = MMLevelDirs->dirs;
	MMLevelDirs->dirs = this;
	Files=0;
	AccessCfg = 3;	EditRights = 0;
	OrigPath = 0; Password=0; PlayPass=0; EditPass=0;

	char *oldpath;
	FS->GetCurDir(&oldpath);
	FS->ChangeDir(path);

	// nactu profil adresare
	if (FS->FileExist("dir.info")) {
		CFSRegKey *k;
		Profile = new CFSRegister("dir.info","DIR PROFILE");
		if (Profile->GetOpenError() != FSREGOK) SAFE_DELETE(Profile);
		if (k=Profile->FindKey("AccessCfg")) AccessCfg = k->readi();
		if (k=Profile->FindKey("EditRights")) EditRights = k->readi();
		Password = Profile->FindKey("Password");
		OrigPath = Profile->FindKey("OrigPath");
		PlayPass = Profile->FindKey("PlayPass");
		EditPass = Profile->FindKey("EditPass");
	} else Profile = 0;

	// nactu obsah
	int dir,aa;
	const char *fn;
	char *buff;
	CMMDirFile *f1, **f2;
	if (FS->FindFirst(&fn,dir)==1) {
		do {
			// nactu informace o polozce
			if (fn && fn[0] != '.' && fn[0] != 0) {
				f1 = new CMMDirFile();
				f1->Name = newstrdup(fn);
				buff = KerServices.ConvertFileNameToShort(fn,f1->Type,1);
				if (f1->Type == eEXTunknown) if (dir==1 || dir==2) f1->Type = eEXTdirectory;
				if (!f1->LoadLevelInfo(this)) {
					f1->UserName = newstrdup(buff);
					f1->UserNameExt = newstrdup(KerServices.ConvertFileNameToShort(fn,aa,1,0));
				}
	
				// zatridim polozku
				f2 = &Files;
				while (!f1->Compare(*f2)) f2 = &(**f2).next;
				f1->next = *f2;
				*f2 = f1;
			}

		} while (FS->FindNext(&fn,dir)==1);
	}

	//zpristupnim levly
	SetAccessTags();

	FS->ChangeDir(oldpath);
	delete[] oldpath;
}


// nastavi tagy pro pristupy
void CMMLevelDir::SetAccessTags() {
	CMMDirFile *F = Files;
	int count;
	char *LevelVersion;
	while(F) {	
		// vykliruju to
		F->Tags = F->Tags & (~eMMLTaccessible);
		F->Tags = F->Tags & (~eMMLTskipAccess);
		F->Tags = F->Tags & (~eMMLTeditable);

		if (F->Type == eEXTlevel && !(F->Tags & eMMLTnewNoAccess) && !(F->Tags & eMMLTdeleted)) {
			
			LevelVersion = MMLevelDirs->GetVersionString(F->Name);
			if (MMProfiles->completed->FindKey(LevelVersion)) F->Tags |= eMMLTcompleted;
			if (MMProfiles->edited->FindKey(LevelVersion)) F->Tags |= eMMLTedited;
			if (MMProfiles->opened->FindKey(LevelVersion)) F->Tags |= eMMLTopened;
		
			// nastavim pravo na editaci
			if (EditRights == 0 && (F->Tags & eMMLTcompleted)) F->Tags |= eMMLTeditable;
			if (EditRights == 1) F->Tags |= eMMLTeditable;
			if (F->Tags & (eMMLTalwEditable|eMMLTedited)) F->Tags |= eMMLTeditable;
			// nastavim pravo na hrani
			if (F->Tags & (eMMLTcompleted|eMMLTopened)) F->Tags |= eMMLTaccessible;
			if (AccessCfg==0) F->Tags |= eMMLTaccessible;
		}
		F = F->next;
	}
	F = Files;
	count = AccessCfg;
	while(F) {
		if (F->Type == eEXTlevel && !(F->Tags & eMMLTnewNoAccess) && !(F->Tags & eMMLTdeleted)) {
			if (!(F->Tags & eMMLTaccessible) && count) {
				F->Tags |= eMMLTaccessible;
				if (!(F->Tags & eMMLTskipable)) count--;
			}
			if (F->Tags & (eMMLTalwAccess|eMMLTedited)) F->Tags |= eMMLTaccessible;
		}
		if (F->Name[0] == '(' && F->next && F->next->Name[0] != '(') // hack aby se daly hrat uzivatelske levly
			count = AccessCfg;
		F = F->next;
	}
}


// Nahraje jazykove lokalizovane jmeno, nastavi tagy pro levly / adresare. Vrati 0, pokud se to nahrat nepodarilo (tagy presto pripadne nastavi)
int CMMDirFile::LoadLevelInfo(CMMLevelDir *dir) {
	if (Type != eEXTdirectory && Type != eEXTlevel) return 0;
	if (Type == eEXTlevel && dir->GamesPath) {
		int len = strlen(dir->GamesPath)+strlen(Name)+10;
		LevelPath = new char[len];
		sprintf(LevelPath,"%s/%s/!level",dir->GamesPath,Name);
		LevelDir = new char[len];
		sprintf(LevelDir,"%s/%s",dir->GamesPath,Name);
	}
	FS->ChangeDir(Name);
	CFSRegister *r;
	if (Type == eEXTdirectory) r = new CFSRegister("dir.info","DIR PROFILE");
	else r = new CFSRegister("!level.info","KRKAL LEVEL I");
	FS->ChangeDir("..");
	if (r->GetOpenError() != FSREGOK) {
		delete r;
		return 0;
	}

	CFSRegKey *k;
	if (k=r->FindKey("Difficulty")) Difficulty = k->readd();
	if (k=r->FindKey("Tags")) Tags = k->readi();
	if (k=r->FindKey("Directory")) {
		if (dir->OrigPath) if (strcmp(k->GetDirectAccess(),dir->OrigPath->GetDirectAccess()) != 0) Tags |= eMMLTnewNoAccess;
	}
	if (k=r->FindKey("Author"))
		Author = k->stringread();
	if (k=r->FindKey("Password")) {
		Password = k->stringread();
		if (dir->Password) if (strcmp(Password,dir->Password->GetDirectAccess()) != 0) Tags |= eMMLTnewNoAccess;
	}
	if (!(k=r->FindKey("LVersion"))) Tags |= eMMLTnewNoAccess;
	else if (strcmp(k->GetDirectAccess(),MMLevelDirs->GetVersionString(Name)) != 0) Tags |= eMMLTnewNoAccess;

	if (k=r->FindKey("LocalNames")) {
		CFSRegister *r2 = k->GetSubRegister();
		if (!(k=r2->FindKey(KRKAL->cfg.langStr))) 
			if (!(k=r2->FindKey(KRKAL->cfg.langStr2))) {
				delete r;
				return 0;
			}
		UserName = newstrdup(k->GetDirectAccess());
		if (Type == eEXTlevel) {
			UserNameExt = new char[strlen(UserName)+5];
			sprintf(UserNameExt,"%s.lv",UserName);
		} else UserNameExt = newstrdup(UserName);
	} else {
		delete r;
		return 0;
	}

	delete r;
	return 1;
}



// vrati jedna, pokud f2 je null nebo pokud je this pred f2
int CMMDirFile::Compare(CMMDirFile *f2) {
	if (!f2) return 1;
	
	if (Difficulty < f2->Difficulty) return 1;
	if (Difficulty > f2->Difficulty) return 0;
	
	int thisCat = 0;
	if (Type == eEXTlevel)
		thisCat = 2;
	else if (Type != eEXTdirectory)
		thisCat = 1;

	int f2Cat = 0;
	if (f2->Type == eEXTlevel)
		f2Cat = 2;
	else if (f2->Type != eEXTdirectory)
		f2Cat = 1;

	if (thisCat < f2Cat) return 1;
	if (thisCat > f2Cat) return 0;

	if (_stricoll(UserNameExt,f2->UserNameExt)<0) return 1;

	return 0;
}



CMMLevelDir::~CMMLevelDir() {
	SAFE_DELETE_ARRAY(Path);
	SAFE_DELETE_ARRAY(GamesPath);
	CMMDirFile *p2, *p=Files;
	while(p) {
		p2 = p;
		p = p->next;
		delete (p2);
	}
	SAFE_DELETE(Profile);
}




// nahraje adresar. Pokud je string null nahraje aktualni adresar
CMMLevelDir *CMMLevelDirs::LoadDir(char *Path) {
	char *fpath;

	if (!Path || !*Path) {
		FS->GetCurDir(&fpath);
	} else if (!FS->GetFullPath(Path,&fpath)) return 0;
	UnifySlashes(fpath);

	CMMLevelDir *d = dirs;
	while (d) {		// zkusim najit, jestli uz adresar nemam otevrenej
		if (strcmp(d->Path,fpath) == 0) break;
		d = d->next;
	}

	if (!d) d = new CMMLevelDir(fpath);

	delete[] fpath;

	return d;

}




// nahraje adresar. Nepouzije Cache, tu refreshne. Pokud je string null nahraje aktualni adresar
CMMLevelDir *CMMLevelDirs::ReLoadDir(char *Path) {
	char *fpath;

	if (!Path || !*Path) {
		FS->GetCurDir(&fpath);
	} else if (!FS->GetFullPath(Path,&fpath)) return 0;
	UnifySlashes(fpath);

	CMMLevelDir **d = &dirs;
	while (*d) {		// zkusim najit, jestli uz adresar nemam otevrenej
		if (strcmp((**d).Path,fpath) == 0) {
			CMMLevelDir *d2 = *d;
			*d = d2->next;
			delete d2;
			break;
		}
		d = &(**d).next;
	}

	return new CMMLevelDir(fpath);
}



// prislusny adresar odstrani z cache. Pokud je string null nahraje aktualni adresar
void CMMLevelDirs::InvalidateDir(char *Path) {
	char *fpath;

	if (!Path || !*Path) {
		FS->GetCurDir(&fpath);
	} else if (!FS->GetFullPath(Path,&fpath)) return;
	UnifySlashes(fpath);

	CMMLevelDir **d = &dirs;
	while (*d) {		// zkusim najit, jestli uz adresar nemam otevrenej
		if (strcmp((**d).Path,fpath) == 0) {
			CMMLevelDir *d2 = *d;
			*d = d2->next;
			delete d2;
			break;
		}
		d = &(**d).next;
	}
}


// nahraje adresar o uroven vys, nez je dir. Pokud je nastaven Verify Dir, tak vrati null v pripade, ze vysledek neni nekde v $GAMES$
CMMLevelDir *CMMLevelDirs::LoadUpDir(CMMLevelDir *dir, int VerifyGames) {
	char *str;
	CMMLevelDir *ret=0;
	if (VerifyGames) {
		if (dir->GamesPath) str = newstrdup(dir->GamesPath); else return 0;
	} else str = newstrdup(dir->Path);
	int len = strlen(str);
	int f=len-1;
	while (f>0 && str[f] != '/') f--;
	if (f>0) {
		str[f] = 0;
		ret = LoadDir(str);
	}
	delete[] str;
	return ret;
}



// sjednoti vsechny lomitka na /
void CMMLevelDirs::UnifySlashes(char *path) {
	if (!path) return;
	while(*path) {
		if (*path == '\\') *path = '/';
		path++;
	}
}



// vrati absolutni cestu do games, unifokovana lomitka
char *CMMLevelDirs::GetGameDir() {
	if (GameDir) return GameDir;
	FS->GetFullPath("$GAMES$",&GameDir);
	UnifySlashes(GameDir);
	return GameDir;
}



// prevede absolutni cestu na cestu od Games 
char *CMMLevelDirs::GetGamePath(char *abspath) {
	int f=0;
	char *gd = GetGameDir();
	while (abspath[f] && gd[f] && abspath[f] == gd[f]) f++;
	if (gd[f] != 0) return 0;
	char *ret = new char[strlen(abspath)-f+3+8];
	sprintf(ret,"$GAMES$%s",abspath+f);
	return ret;
}




// vykucha string vezrze z mena fajlu. Funkco vraci pointr na svuj vnitrni bufffer
char *CMMLevelDirs::GetVersionString(const char *FileName) {
	if (!FileName) return 0;
	int f = strlen(FileName);
	int g;
	while (f>=20) {
		if (ishexa(FileName[f-1]) && FileName[f-5] == '_' && FileName[f-10] == '_' && ishexa(FileName[f-2]) && ishexa(FileName[f-3]) && ishexa(FileName[f-4])) {
			for (g=f-19; g<f; g++) buff[g-(f-19)] = FileName[g];
			buff[19] = 0;
			return buff;
		}
		f--;
	}
	return 0;
}

