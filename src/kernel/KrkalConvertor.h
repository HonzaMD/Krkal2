//////////////////////////////////////////////////////////////////////////////
///
///		K r k a l   C o n v e r t or
///
///		konvertor levlu, ze stareho Krkala do noveho
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////

#ifndef KRKALCONVERTOR_H
#define KRKALCONVEROTOR_H

#include "types.h"

class CFSRegister;
class CFSRegKey;

struct COldVeci {
	COldVeci(UC _x,UC _y,UC _vec, COldVeci *_next);
	int NajdiNVec(int vec);
	// info ze staryho krkala
	UC x,y,vec;
	UC a1,a2,a3; // pro veci
	UC zvp;
	struct{		 // pro prepinace
		UC x,y,vec;
		int nvec;
		COldVeci *vec2;
	}a[6];
	// pro novy krkal
	int InMap;
	int SaveIndex;
	int nvec,vecIndex;
	COldVeci *next;
};

struct COldNewVeci {
	char *object;	// KSID jmeno objektu
	int startpos, endpos; // cisla do starych veci
	int typ; // viz eOldTyp
	int PrepCreate; // Zda ma prepinac vec umistovat jako uz vytvoreny objekt
};

#define MAXOLDNEWVECI 47
#define MAXOLDMUSIC 15

extern COldNewVeci OldNewVeci[MAXOLDNEWVECI];
extern char * OldMusic[MAXOLDMUSIC];

enum eOldTyp {
	eOTunknown,
	eOTplace,
	eOTseSmerem,
	eOTsBarvou,
	eOTpas,
	eOTprisera,
	eOTteleport,
	eOThajzl,
	eOTprepinac,
	eOTnoConnect,
	eOTelektroda,
	eOTdvere
};

class COldMain {
public:
	COldMain() {Veci = 0; usek=0; hudba=0;}
	int ConvertLevel(int oldlev, char *newlev_name, char *newlev_dir = "$CONVERTOR_OUTPUT$"); // Prevede level ze staryho krkala do novyho.  0 chyba, 1 OK
private:
	COldVeci *Veci;
	UC usek, hudba;

	int LoadLevel(char *file);
	void SaveLevel(CFSRegister *reg);
	void DeleteVeci();
	void AddSmer(CFSRegister *r2,COldVeci *v);
	void AddOnOff(CFSRegister *r2,int OnOff);
	int SaveAkce(CFSRegKey *k,COldVeci *v);
};

extern COldMain OldMain;


#endif

