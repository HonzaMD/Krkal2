//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - A u t o s
///
///		Prace s grafickymy automatismy, vyber grafiky pro objekty, graficke styly ..
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "autos.h"
#include "fs.h"
#include "register.h"
#include "kernel.h"
#include "GEnMain.h"
#include "ComConstants.h"
#include "gui.h"
#include "LevelMap.h"
#include "genmain.h"
#include "KerServices.h"
#include "names.h"
#include "dx.h"
#include "GameMenu.h"




/////////////////////////////////////////////////////////////
//		Popis sledovanych pozic pro navazovaci automatismus
//		Jsou to relativni pozice bunek, ktere odpovidaji prislusnym bitikum v navazovaci masce

struct CKerCACellPositions {
	int x,y,z;
} CACellPositions[27] = {
	{-1,0,0}, {0,1,0}, {1,0,0}, {0,-1,0}, 
	{-1,1,0}, {1,1,0}, {1,-1,0}, {-1,-1,0},
	{0,0,0}, {0,0,1}, {0,0,-1},
	{-1,0,1}, {0,1,1}, {1,0,1}, {0,-1,1}, 
	{-1,0,-1}, {0,1,-1}, {1,0,-1}, {0,-1,-1},
	{-1,1,1}, {1,1,1}, {1,-1,1}, {-1,-1,1},
	{-1,1,-1}, {1,1,-1}, {1,-1,-1}, {-1,-1,-1},
};




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///
///		PROCESSING AUTOMATISMU
///
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


///////////////////////////////
//		CKerAutoConnect
///////////////////////////////



// Konstruktor, initace poli
CKerAutoConnect::CKerAutoConnect(int sonsnum, int gridsize){
	int f;
	mask=0; GridCfg=0; Agresivity=0; GridSize=gridsize;  SonsNum = sonsnum;
	GX=0; GY=0; GZ=0; GFNames = new CKerName**[GridSize]; GFShift = new int[GridSize];
	FGoDown=0; NoErrorMask = 0;
	MaskLook = new int*[SonsNum];  GoDown = new CKerName*[SonsNum];
	for (f=0;f<SonsNum;f++) MaskLook[f] = new int[GridSize];
	for (f=0;f<GridSize;f++) GFNames[f] = 0;
}


/////////////////////////////////////////////////////////////////
/// destructor
CKerAutoConnect::~CKerAutoConnect() {
	int f;
	SAFE_DELETE_ARRAY(GX); SAFE_DELETE_ARRAY(GY); SAFE_DELETE_ARRAY(GZ);
	if (GFNames) {
		for(f=0;f<GridSize;f++) SAFE_DELETE_ARRAY(GFNames[f]);
		SAFE_DELETE_ARRAY(GFNames);
	}
	SAFE_DELETE_ARRAY(GFShift);
	SAFE_DELETE_ARRAY(FGoDown);
	SAFE_DELETE_ARRAY(NoErrorMask);
	if (MaskLook) {
		for(f=0;f<SonsNum;f++) SAFE_DELETE_ARRAY(MaskLook[f]);
		SAFE_DELETE_ARRAY(MaskLook);
	}
	SAFE_DELETE_ARRAY(GoDown);
}



////////////////////////////////////////////////////////////////////////////////////
// zjisti zda je hledany objekt v seznamu
CKerObject *CKerAutoConnect::IsObjInArray(CKerName* name, CKerArrObject *objs, CKerObject *Me, int relx, int rely, int relz) {
	int f;
	CKerObject *ko;
	int count = (*objs).GetCount();
	CKerName *n2;
	for (f=0; f<count; f++) {
		ko = KerMain->GetAccessToObject(objs->Read(f));
		if (ko && ko != Me) {
			if (ko->PlacedInfo->x==relx && ko->PlacedInfo->y==rely && ko->PlacedInfo->z == relz) {
				n2 = ko->PlacedInfo->APicture;
				if (n2 && (n2==name || n2->Compare(name))) return ko;
			}
		}
	}
	return 0;
}




////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//		C O N N E C T   P R O C E S S
//		vypocet navaznosti
CKerName *CKerAutoConnect::Process(CKerObject *ko, UI NoConnectMask, CKerObject **QueueEnd, CKerObject **QueueStart, int Reset, CKerObject *Test) {
	int f=0,g=0,i,a;
	int cx,cy,cz;
	int relx,rely,relz;
	int cpx,cpy,cpz;
	int _mask = mask;  // maska standartnich checkovacich pozic
	CKerObject *ko2;
	CKerArrInt cell;
	CKerArrObject *objs;
	CKerArrObject objs2;
	CKerName *a2;
	CKerACState *GridState = 0;


	if (FGoDown) {
		if (!ko->PlacedInfo || ((ko->PlacedInfo->CollCfg & eKCCareaMask) == eKCCoutOfMap)) return GoDown[FGoDown[0]]; // objekt neni v mape
		if (Reset==0||Reset==2) ko->PlacedInfo->Tag = 1; 
		else ko->PlacedInfo->Tag = 2; 
		KerMain->MapInfo->FindCell(ko->PlacedInfo->x,ko->PlacedInfo->y,ko->PlacedInfo->z,cx,cy,cz);  // najdu svoji bunku
		KerMain->MapInfo->FindCellPosition(cx,cy,cz,cpx,cpy,cpz);
		if (Reset==1 || (Reset==0 && Agresivity==0)) GridState = new CKerACState[GridSize];

		a = 0;
		while (_mask) {  // podle masky prohledam okolni pozice
			if (_mask&1) {
				if ((NoConnectMask&1)==0) {
					cell[0] = cx + CACellPositions[f].x; // bunka kam se budu koukat
					cell[1] = cy + CACellPositions[f].y;
					cell[2] = cz + CACellPositions[f].z;
					KerMain->MapInfo->FindCellPosition(cell[0],cell[1],cell[2],relx,rely,relz);
					relx = relx - cpx + ko->PlacedInfo->x;
					rely = rely - cpy + ko->PlacedInfo->y;
					relz = relz - cpz + ko->PlacedInfo->z;
					i=0;
					// prectu si z pozice objekty
					try {
						objs = (CKerArrObject*)KerMain->callint(0,KerMain->MapInfo->OMapa,KnownNames[eKKNmGetObjects].Name,eKTarrObject,2, eKTarrInt, KnownNames[eKKNcellsArray].Name,&cell, eKTarrObject,KnownNames[eKKNobjectArray].Name,&objs2);
						if (!objs) KerMain->Errors->LogError(eKRTEmapNotWorking,0,"MGetObjs");
					} catch(CKernelPanic) {
						SAFE_DELETE_ARRAY(GridState);
						throw;
					}
					while (GFNames[g][i]) {
						if (ko2 = IsObjInArray(GFNames[g][i],objs,ko,relx,rely,relz)) {
							// nasel jsem tam objekt, ktery mne zajima
							if (Reset==0 && Agresivity==1) {
								a |= (i+1)<<GFShift[g];
								if (ko2->PlacedInfo->Tag == 0) {
									ko2->PlacedInfo->AutoNext = 0;
									ko2->PlacedInfo->AutoPrev = *QueueEnd;
									ko2->PlacedInfo->Tag = 3;
									(**QueueEnd).PlacedInfo->AutoNext = ko2;
									*QueueEnd = ko2;
								}
							} if (Reset==1) {
								GridState[g].ko = ko2;
								a2 = ko2->PlacedInfo->AktivAuto2;
								if (!a2) a2 = ko2->PlacedInfo->AktivAuto;
								if (a2==GFNames[g][i] || a2->Compare(GFNames[g][i])) {
									GridState[g].GLook = i+1; // navazuje
									a |= (i+1)<<GFShift[g];
								} 
							} else {
								a2 = ko2->PlacedInfo->AktivAuto2;
								if (!a2) a2 = ko2->PlacedInfo->AktivAuto;
								if (Test==ko2 || a2==GFNames[g][i] || a2->Compare(GFNames[g][i])) {
									a |= (i+1)<<GFShift[g];// navazuje
								} else if (Reset==0 && ko2->PlacedInfo->Tag==0) {
									a2 = KerMain->AutosMain->ProcessAuto(ko2,QueueEnd,QueueStart,2,ko);
									if (a2 && (a2==GFNames[g][i] || a2->Compare(GFNames[g][i]))) {
										a |= (i+1)<<GFShift[g];// navazuje
										GridState[g].GLook = i+1;
										GridState[g].ko = ko2;
									} else ko2->PlacedInfo->Tag=0, ko2->PlacedInfo->AktivAuto2=0;
								}
							}
							break;
						}
						i++;
					}
					if (objs!=&objs2) delete objs;
				}
				g++;
			}
			_mask>>=1;
			NoConnectMask>>=1;
			f++;
		}

		if (Reset==1) {
			for (i=0; i<GridSize; i++) if (GridState[i].ko) {
				if (MaskLook[FGoDown[a]][i]!=GridState[i].GLook) {
					ko2 = GridState[i].ko;
					if (ko2->PlacedInfo->Tag != 1) {
						if (ko2->PlacedInfo->Tag) {
							if (ko2->PlacedInfo->AutoPrev) ko2->PlacedInfo->AutoPrev->PlacedInfo->AutoNext = ko2->PlacedInfo->AutoNext;
							else *QueueStart = ko2->PlacedInfo->AutoNext;
							if (ko2->PlacedInfo->AutoNext) ko2->PlacedInfo->AutoNext->PlacedInfo->AutoPrev = ko2->PlacedInfo->AutoPrev;
							else *QueueEnd = ko2->PlacedInfo->AutoPrev;
						}
						ko2->PlacedInfo->AutoNext = 0;
						ko2->PlacedInfo->AutoPrev = *QueueEnd;
						ko2->PlacedInfo->Tag = 1;
						(**QueueEnd).PlacedInfo->AutoNext = ko2;
						*QueueEnd = ko2;
					}
				} else ko->PlacedInfo->ConnectedObjs.Add(GridState[i].ko->thisO); // zapamatuju si ze jsem navazal
			}
			SAFE_DELETE_ARRAY(GridState);
		}

		if (Reset==0 && Agresivity==0) { 
			for (i=0; i<GridSize; i++) if (GridState[i].ko) {
				if (MaskLook[FGoDown[a]][i]!=GridState[i].GLook) GridState[i].ko->PlacedInfo->Tag = 0, GridState[i].ko->PlacedInfo->AktivAuto2=0;
				else {
					ko2 = GridState[i].ko;
					ko2->PlacedInfo->AutoNext = 0;
					ko2->PlacedInfo->AutoPrev = *QueueEnd;
					(**QueueEnd).PlacedInfo->AutoNext = ko2;
					*QueueEnd = ko2;
				}
			}
			SAFE_DELETE_ARRAY(GridState);
		}

		if (Reset==2) if (!(NoErrorMask[a>>3]&(1<<(a&0x7)))) return 0;
		
		return GoDown[FGoDown[a]];

	} else {
		// TODO - dodelat
		return GoDown[0];
	}
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
///
///		P R O C E S S   A U T O

CKerName *CKerAutosMain::ProcessAuto(CKerObject *kobj, CKerObject **QueueEnd, CKerObject **QueueStart, int Reset, CKerObject *Test, int RandomReset) {
	CKerName *Auto = kobj->Type->DefAuto;
	void *var;
	UI NoConnectMask = 0;
	double rand;
	int f,aa;
	int ox,oy,oz;
	CKerAuto *A;
	CKerObject *OldQEnd = 0;
	CKerName *VarV;

	if (kobj->PlacedInfo) Auto = kobj->PlacedInfo->APicture;
	else if (var = KerMain->GetAccessToVar(kobj,eKVUaPicture)) Auto = *(CKerName**)var;
	if (Auto == KnownNames[eKKNdefaultAuto].Name) Auto = kobj->Type->DefAuto;

	if (var = KerMain->GetAccessToVar(kobj,eKVUaNoConnect)) NoConnectMask = *(UI*)var;

	if (!Auto || Auto->Type != eKerNTauto) {return 0;}

	if (QueueEnd && Reset==0) OldQEnd = *QueueEnd;
	if (Reset==1) kobj->PlacedInfo->ConnectedObjs.SetCount(0);

	while (Auto->Auto->Type != eKATpicture) {
		A = Auto->Auto;
		switch (A->Type) {
			case eKATrandom: 
				if (kobj->PlacedInfo && kobj->PlacedInfo->AktivAuto && !RandomReset) {  // zkusim se zachovat stejne jako predtim:
					aa=0;
					for (f=0;f<A->SonsNum;f++) {
						if (KerServices.IsFMemberOfS(kobj->PlacedInfo->AktivAuto,A->psti[f].GoDown)) {
							aa=1;
							Auto = A->psti[f].GoDown;
							break;
						}
					}
					if (aa) break;
				}
				rand = mtr.rand();		// rozhodnu se podle randomu:
				for (f=0;f<A->SonsNum;f++) {
					if (rand <= A->psti[f].pst) {
						Auto = A->psti[f].GoDown;
						break;
					}
				}
				break;
			case eKATvar:
				VarV = 0;
				// najdu ridici promennou u objektu:
				for (f=0;f<kobj->Type->NumAVars;f++) {
					if (kobj->Type->AutoVars[f]->Name == A->VarName) {
						if (var = KerMain->GetAccessToVar(kobj,kobj->Type->AutoVars[f])) VarV = *(CKerName**)var;
						break;
					}
				}
				Auto = A->Default;
				if (!VarV) break; 
				// najdu mnozinu do ktere padne a tak se rozhodnu
				for (f=0; f<A->SonsNum; f++) {
					if (KerServices.IsFMemberOfS(VarV,A->intervals[f].a)) {
						Auto = A->intervals[f].GoDown;
						break;
					}
				}
				break;
			case eKATpattern:
				KerMain->GetAccessToObject(kobj);
				KerMain->MapInfo->FindObjCell(kobj,ox,oy,oz);
				Auto = A->GoDown[(ox*A->patA + oy*A->patB + oz*A->patC) % A->SonsNum];
				break;
			case eKATconnect:
				Auto = A->Connect->Process(kobj,NoConnectMask,QueueEnd,QueueStart, Reset, Test);
				break;
		}
		if (!Auto || Auto->Type != eKerNTauto) {
			if (kobj->PlacedInfo) kobj->PlacedInfo->AktivAuto2=kobj->PlacedInfo->AktivAuto;
			if (OldQEnd) {
				StornoQueue(OldQEnd);
				*QueueEnd = OldQEnd;
			}
			return 0;
		}
	}
	if (kobj->PlacedInfo) kobj->PlacedInfo->AktivAuto2=Auto;
	if (OldQEnd && kobj->PlacedInfo->AktivAuto == Auto) {
		StornoQueue(OldQEnd);
		*QueueEnd = OldQEnd;
	}
	return Auto;
}


void CKerAutosMain::StornoQueue(CKerObject *OldQend) {
	CKerObject *ko = OldQend->PlacedInfo->AutoNext;
	while (ko) {
		ko->PlacedInfo->AktivAuto2 = 0;
		ko->PlacedInfo->Tag = 0;
		ko = ko->PlacedInfo->AutoNext;
	}
	OldQend->PlacedInfo->AutoNext = 0;
}



void CKerAutosMain::ProcessAuto2(CKerObject *ko, int Remove,int ConnectionReset,int RandomReset) {
	if (!ko->PlacedInfo || !ko->Type->DefAuto || ko->PlacedInfo->CollCfg&eKCCtriggerBit || ko->PlacedInfo->CollCfg&eKCCinvisible) return;
	CKerObject *QTop = 0, *TestQBase=0, *QPtr;
	int DAdd=0,DReset=0,DDraw=0;

	if (!Remove) {
		// pridam objekt k umisteni
		TestQBase = ko;
		QTop = ko;
		ko->PlacedInfo->AutoNext = 0;
		ko->PlacedInfo->AutoPrev = 0;
		if (ConnectionReset) ko->PlacedInfo->Tag = 3;
		else ko->PlacedInfo->Tag = 1;

		// navazu se na vsechno
		QPtr = TestQBase;
		if (QPtr->PlacedInfo->Tag == 3) ProcessAuto(QPtr,&QTop,&TestQBase,0,0,RandomReset),DAdd++; // pro reset randomu
		QPtr = QPtr->PlacedInfo->AutoNext;
		while (QPtr) {
			if (QPtr->PlacedInfo->Tag == 3) ProcessAuto(QPtr,&QTop,&TestQBase,0),DAdd++;
			QPtr = QPtr->PlacedInfo->AutoNext;
		}
	}	

	// pridam stare navaznosti ke zruseni
	int f;
	CKerObject *ko2;
	for (f=0; f<ko->PlacedInfo->ConnectedObjs.GetCount(); f++) {
		ko2 = KerMain->Objs->GetObject(ko->PlacedInfo->ConnectedObjs.Read(f));
		if (ko2 && ko2->PlacedInfo && ko2->PlacedInfo->Tag==0) {
			ko2->PlacedInfo->AutoNext = 0;
			ko2->PlacedInfo->AutoPrev = QTop;
			ko2->PlacedInfo->Tag = 1;
			if (QTop) QTop->PlacedInfo->AutoNext = ko2;
			else TestQBase = ko2;
			QTop = ko2;
		}
	}

	// zkoriguju navaznosti
	QPtr = TestQBase;
	if (QPtr && !Remove && !ConnectionReset) { // pro reset randomu
		if (QPtr->PlacedInfo->Tag == 1) ProcessAuto(QPtr,&QTop,&TestQBase,1,0,RandomReset),DReset++;
		QPtr = QPtr->PlacedInfo->AutoNext;
	}
	while (QPtr) {
		if (QPtr->PlacedInfo->Tag == 1) ProcessAuto(QPtr,&QTop,&TestQBase,1),DReset++;
		QPtr = QPtr->PlacedInfo->AutoNext;
	}

	// zobrazim to
	QPtr = TestQBase;
	while (QPtr) {
		if (QPtr->PlacedInfo->AktivAuto != QPtr->PlacedInfo->AktivAuto2) {
			DDraw++;
			ChangeAktivAuto(QPtr,QPtr->PlacedInfo->AktivAuto,QPtr->PlacedInfo->AktivAuto2);
		}
		QPtr->PlacedInfo->AktivAuto = QPtr->PlacedInfo->AktivAuto2;
		QPtr->PlacedInfo->AktivAuto2 = 0;
		QPtr->PlacedInfo->Tag = 0;
		QPtr = QPtr->PlacedInfo->AutoNext;
	}
	ko->PlacedInfo->Tag = 0;


	DebugMessage(2,0xFF00FFFF,"A%i R%i D%i",DAdd,DReset,DDraw);

	CGameMenu::RefreshManikPicture(ko);
}


/////////////////////////////////////////////////////////////////////////////
///		Preprocessuje vsechny automatismy v ToDo fronte, pripadne prekresli obrazky
void CKerAutosMain::ProcessToDoAutos() {
	//CKerObject *ko;
	//CKerAuto *A;
	//while (ToDo) {
	//	A = ToDo->PlacedInfo->AktivAuto->Auto;
	//	ko = ToDo;
	//	ko->PlacedInfo->AktivAuto = ProcessAuto(ToDo);
	//	ChangeAktivAuto(ko,A,ko->PlacedInfo->AktivAuto->Auto);
	//}
}




//////////////////////////////////////////////////////////////////////////
// zmeni grafiku podle toho, jak se zmenily automatismy
void CKerAutosMain::ChangeAktivAuto(CKerObject *ko, CKerName *_OldA, CKerName *_NewA) {
	CKerAuto *OldA, *NewA;
	float ox, oy, oz;
	int f;
	int count;
	CGEnElement **els;
	if (_OldA) OldA = _OldA->Auto; else OldA = 0;
	NewA = _NewA->Auto;
	if (OldA != NewA) {
		if (!OldA) { // budu tvorit nove obrazky
			SAFE_DELETE_ARRAY(ko->elements);
			ko->elements = new CGEnElement*[NewA->SonsNum];
			for (f=0;f<NewA->SonsNum;f++) {
				ko->elements[f] = GEnMain->DrawElement(ko->thisO, (float)ko->PlacedInfo->x,(float)ko->PlacedInfo->y,(float)ko->PlacedInfo->z,NewA->textures[f].texture);
			}
		} else	if (NewA->SonsNum!=OldA->SonsNum) { // slozitej pripad
			count = NewA->SonsNum < OldA->SonsNum ? NewA->SonsNum : OldA->SonsNum;
			els = new CGEnElement*[NewA->SonsNum];
			// spolecny zaklad - zmenim obrazky
			for (f=0;f<count;f++) {
				GEnMain->SetPicture(ko->elements[f],NewA->textures[f].texture);
				els[f] = ko->elements[f];
			}
			if (OldA->SonsNum>NewA->SonsNum) {
				// zbytek musim umazat
				for (f=count; f<OldA->SonsNum; f++) GEnMain->DeleteElement(ko->elements[f]);
			} else {
				// zbytek musim vytvorit
				ko->elements[0]->GetPos(ox,oy,oz);
				for (f=count; f<NewA->SonsNum; f++) {
					els[f] = GEnMain->DrawElement(ko->thisO, ox,oy,oz,NewA->textures[f].texture);
					GEnMain->MoveElem(els[f],ko->elements[0]); // zachovam pohyby
				}
			}
    		delete[] ko->elements;
			ko->elements = els;
		} else { // jednoduchej pripad
			// zmenim obrazky
			for (f=0;f<NewA->SonsNum;f++) {
				GEnMain->SetPicture(ko->elements[f],NewA->textures[f].texture);
			}
		} 
	}
}





//////////////////////////////////////////////////////////////////
///
///		C K e r A u t o
///		a jeho podstruktury
///
//////////////////////////////////////////////////////////////////

// KOnstructor
CKerAuto::CKerAuto(int type, int sonsnum) {
	next = KerMain->AutosMain->autos;
	KerMain->AutosMain->autos =  this;
	Type = type; SonsNum = sonsnum;
	EditTex = 0;
	switch (type) {
		case eKATpicture:
			textures = new CKerAutoPicture[SonsNum];
			break;
		case eKATpattern:
			patA=0; patB=0; patC=0;
			GoDown = new CKerName*[SonsNum];
			break;
		case eKATrandom:
			psti = new CKerAutoRandom[SonsNum];
			break;
		case eKATvar:
			VarName=0; Default=0;
			intervals = new CKerAutoVar[SonsNum];
			break;
		case eKATconnect:
			Connect = 0;
			break;
	}
}


/////////////////////////////////////////////////////////
/// Destructor
CKerAuto::~CKerAuto() {
	switch (Type) {
		case eKATpicture:
			delete[] textures;
			break;
		case eKATpattern:
			delete[] GoDown;
			break;
		case eKATrandom:
			delete[] psti;
			break;
		case eKATvar:
			delete[] intervals;
			break;
		case eKATconnect:
			SAFE_DELETE(Connect);
			break;
	}
}



////////////////////////////////////////////////////////////////
// Destructor
CKerAutoPicture::~CKerAutoPicture() {
	SAFE_RELEASE(texture);
}




/////////////////////////////////////////////////////////////////////////////////////////
///////////////
///////////////		L O A D
///////////////
////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// vypocita dabulku rychleho sestupu. Diky tomu se da automatismus vyresit jednim dotazem
// Projde vsechny moznosti a vysledek da do tabulky.
void CKerAutoConnect::CalculateFastGoDown(int SHIFT) {
	if (SHIFT>16) {
		KerMain->Errors->LogError(eKRTEerrorInAuto,0,"MnohoKombinaciProConnect");
		return;
	}
	int *mask = new int[GridSize];
	int *stav = new int[GridSize];
	int top = 1<<SHIFT;
	int top2 = (top-1)/8 +1;
	int f,g,i;
	int err;
	int errmin, optpos;

	// vypoctu andovaci masku pro jednolive pozice
	for (f=0; f<GridSize; f++) {
		if (f+1 < GridSize) g=GFShift[f+1]; else g=SHIFT;
		mask[f] = (1<<(g - GFShift[f]))-1;
	}
	FGoDown = new int[top];
	NoErrorMask = new UC[top2];
	for (f=0; f<top2; f++) NoErrorMask[f] = 0;

	for(f=0; f<top; f++) {  // projdu vsechny kombinace a hledam tu moznost s nejmensi chybou
		optpos=0;
		errmin=1000000;
		for (g=0; g<GridSize; g++) stav[g] = (f>>GFShift[g])&mask[g];  // prevedu kombinaci na stavy na jednotlivych pozicich
		for (i=0; i<SonsNum; i++) {  // projdu syny
			err=0;
			/// spocitam error:
			for (g=0; g<GridSize; g++) {
				if (MaskLook[i][g] != stav[g]) {
					if (MaskLook[i][g]==0) err++; else err+=50;
				}
			}
			if (err<errmin) { // nasel jsem dosavadni optimum
				errmin = err;
				optpos = i;
			}
		}
		FGoDown[f] = optpos;  // vysledek
		if (errmin==0) NoErrorMask[f>>3] |= (1<<(f&0x7));
	}
	delete[] mask;
	delete[] stav;
}










////////////////////////////////////////////////////////////////////
///
///		C K e r A u t o s M a i n
///
////////////////////////////////////////////////////////////////////


// nahraje konkretni soubor s automatismama
int CKerAutosMain::LoadAutos(const char *file, int ChangeDirToFile) {
	int len = strlen(file);
	char *oldpath = 0;
	CFSRegKey *k, *name;
	CFSRegKey *k1, *k2, *k3, *k4;
	CFSRegister *r2,*r3;
	CKerName **names=0;
	int ncount=0;
	int f,i,ii;
	int typ,snum,snum2;
	int SHIFT;
	CKerAuto *A;

	if ((file[len-1]!='a' && file[len-1]!='A') || file[len-2]!='.') return 0;
	CFSRegister *r = new CFSRegister(file,"KRKAL AUTOMATICS");
	if (r->GetOpenError() != FSREGOK) {delete r; return 0;}
	if (ChangeDirToFile) {
		FS->GetCurDir(&oldpath);
		FS->ChangeDirByFile(file);
	}
	
	try {

		if (k = r->FindKey("Names")) {
			r2 = k->GetSubRegister();
			ncount = r2->GetNumberOfKeys();
			names = new CKerName*[ncount];

			// predbezne nacteni vsech jmen
			name = r2->GetFirstKey();
			f=0;
			while(name) {
				names[f] = KerMain->KerNamesMain->GetNamePointer(name->GetName());
				if (!names[f]) {
					names[f] = new CKerName(name->GetName(),KerMain->KerNamesMain);
					names[f]->Type = eKerNTautoVoid;
				}
				name = name->GetNextKey();
				f++;
			}

			// nacteni automatismu:
			name = r2->GetFirstKey();
			f=0;
			while(name) {
				r3 = name->GetSubRegister();
				typ = r3->FindKey("Type")->readi();
				if (typ==eKATpattern||typ==eKATconnect||typ==eKATpicture||typ==eKATrandom||typ==eKATvar||typ==eKATvarAdd) {
					snum = r3->FindKey("Sons Num")->readi();
					snum2=0;
					if (names[f]->Type==eKerNTauto) {
						if ((typ==eKATvar||typ==eKATvarAdd) && names[f]->Auto->Type==eKATvar) {
							// rozsirim automatismus o nove polozky - Jen pro VAR
							int ff;
							A = names[f]->Auto;
							CKerAutoVar *I2 = A->intervals;
							snum2 = A->SonsNum;
							A->SonsNum = snum = snum+snum2;
							A->intervals = new CKerAutoVar[snum];
							for (ff=0; ff<snum2; ff++) {
								A->intervals[ff].a = I2[ff].a;
								A->intervals[ff].GoDown = I2[ff].GoDown;
							}
							delete[] I2;
						} else return 0;
					} else {
						if (typ==eKATvarAdd) A = new CKerAuto(eKATvar,snum);
						else A = new CKerAuto(typ,snum);
						names[f]->Type = eKerNTauto;
						names[f]->Auto = A;
					}

					switch (typ) {
						case eKATvar:
							k = r3->FindKey("Var Name");
							A->VarName = names[k->readi()];
							if (k = r3->FindKey("Default")) A->Default = names[k->readi()];
						case eKATvarAdd:
							k = r3->FindKey("GoDown");
							k1 = r3->FindKey("Int");
							for (i=snum2; i<snum; i++) {
								A->intervals[i].GoDown = names[k->readi()];
								A->intervals[i].a = names[k1->readi()];
							}
							if (typ==eKATvar && snum && !A->Default) A->Default = A->intervals[0].GoDown;
							break;
						case eKATrandom:
							k = r3->FindKey("GoDown");
							k1 = r3->FindKey("Psti");
							for (i=0; i<snum; i++) {
								A->psti[i].GoDown = names[k->readi()];
								A->psti[i].pst = k1->readd();
							}
							break;
						case eKATconnect:
							SHIFT = 0;
							k = r3->FindKey("Grid Size");
							A->Connect = new CKerAutoConnect(A->SonsNum,k->readi());
							if (k = r3->FindKey("Mask")) A->Connect->mask = k->readi();
							if (k = r3->FindKey("Grid Cfg")) A->Connect->GridCfg = k->readi();
							if (k = r3->FindKey("Aggressivity")) A->Connect->Agresivity = k->readc();
							k = r3->FindKey("GFNames")->GetSubRegister()->GetFirstKey();
							for (i=0; i<A->Connect->GridSize; i++) {
								A->Connect->GFNames[i] = new CKerName*[k->top+1];
								for (ii=0; ii<k->top; ii++) A->Connect->GFNames[i][ii] = names[k->readi()];
								A->Connect->GFNames[i][k->top] = 0;
								A->Connect->GFShift[i] = SHIFT;
								ii = k->top;
								while (ii) {
									SHIFT++;
									ii = ii>>1;
								}
								k = k->GetNextKey();
							}
							k = r3->FindKey("GoDown");
							for (i=0; i<snum; i++) {
								A->Connect->GoDown[i] = names[k->readi()];
							}
							k = r3->FindKey("MaskLook")->GetSubRegister()->GetFirstKey();
							for (i=0; i<snum; i++) {
								for (ii=0; ii<A->Connect->GridSize; ii++) A->Connect->MaskLook[i][ii] = k->readi();
								k = k->GetNextKey();
							}
							if (k = r3->FindKey("GX")) {
								A->Connect->GX = new int[A->Connect->GridSize];
								for (i=0; i<A->Connect->GridSize; i++) A->Connect->GX[i] = k->readi();
							}
							if (k = r3->FindKey("GY")) {
								A->Connect->GY = new int[A->Connect->GridSize];
								for (i=0; i<A->Connect->GridSize; i++) A->Connect->GY[i] = k->readi();
							}
							if (k = r3->FindKey("GZ")) {
								A->Connect->GZ = new int[A->Connect->GridSize];
								for (i=0; i<A->Connect->GridSize; i++) A->Connect->GZ[i] = k->readi();
							}
							A->Connect->CalculateFastGoDown(SHIFT);
							break;
						case eKATpattern:
							k = r3->FindKey("GoDown");
							k1 = r3->FindKey("Pattern Consts");
							for (i=0; i<snum; i++) {
								A->GoDown[i] = names[k->readi()];
							}
							if (!k1->eof()) A->patA = k1->readi();
							if (!k1->eof()) A->patB = k1->readi();
							if (!k1->eof()) A->patC = k1->readi();
							break;
						case eKATpicture:
							k = r3->FindKey("Textures");
							k1 = r3->FindKey("Start Time");
							k2 = r3->FindKey("End Action");
							k3 = r3->FindKey("End Timeout");
							k4 = r3->FindKey("Timeout Random");
							for (i=0; i<snum; i++) {
								A->textures[i].texture = GEnMain->LoadPicture(k->GetDirectAccessFromPos());
								if (!A->textures[i].texture) KerMain->Errors->LogError(eKRTEpictureNotLoaded,0,k->GetDirectAccessFromPos());
								k->SetPosToNextString();
								A->textures[i].texture->AddRef();
								if (k1) A->textures[i].StartTime = k1->readi();
								if (k2) A->textures[i].EndAction = k2->readi();
								if (k3) A->textures[i].EndTimeOut = k3->readi();
								if (k4) A->textures[i].TimeOutRandom = k4->readi();
							}
							break;
					}
				}

				name = name->GetNextKey();
				f++;
			}

		}
		// nacteni dependenci:
		if (k = r->FindKey("Dependencies")) {
			KerMain->LoadDependencies(ncount,k,names);
		}

	} catch(CKernelPanic) {
		SAFE_DELETE_ARRAY(oldpath);
		SAFE_DELETE_ARRAY(names);
		delete r;
		throw;
	}

	if (oldpath) {
		FS->ChangeDir(oldpath);
		delete[] oldpath;
	}
	SAFE_DELETE_ARRAY(names);
	delete r;
	return 1;
}



////////////////////////////////////////////////////////////////////////////////
// nahraje vsechny soubory s atomatismama po ceste do rootu
void CKerAutosMain::LoadAllAutos() {
	char *ptr=0;
	char *games=0;
	const char *file;
	int levels=0;
	int dir,ret;
	int f;
	FS->GetCurDir(&ptr);
	FS->GetFullPath("$GAMES$",&games);  // zjistim si o kolik levlu se mi hladiny lisi
	f=0;
	while(ptr[f]) {
		if (ptr[f]=='/'||ptr[f]=='\\') levels++;
		f++;
	}
	f=0;
	while(games[f]) {
		if (games[f]=='/'||games[f]=='\\') levels--;
		f++;
	}
	levels++;

	// prochazim adresare a loaduju vsechny soubory s automatismama:
	try {
		while(levels) {
			ret = FS->FindFirst(&file,dir);
			if (ret==1&&dir==0) LoadAutos(file,0);
			while (ret==1) {
				ret = FS->FindNext(&file,dir);
				if (ret==1&&dir==0) LoadAutos(file,0);
			}
			levels--;
			if (levels) FS->ChangeDir("..");
		}
	} catch (CKernelPanic) {
		FS->ChangeDir(ptr);
		SAFE_DELETE_ARRAY(ptr);
		SAFE_DELETE_ARRAY(games);
	}
	FS->ChangeDir(ptr);
	SAFE_DELETE_ARRAY(ptr);
	SAFE_DELETE_ARRAY(games);
}




///////////////////////////////////////////////////////////////////////
//		Zkontroluje pripadne chyby v automatismech
void CKerAutosMain::CheckAutosIntegrity() {
	CKerNameList *nl = KerMain->KerNamesMain->Names;
	CKerAuto *A;
	int f;
	while (nl) {
		if (nl->name->Type==eKerNTauto) {  // pro kazde jmeno typu automatismus
			// Hlavne kontroluju, jestli ma syny a jestli to jsou take platne automatismy
			// Protoze ve jmenech nejsou cykly, musi byt stromy korektni
			A = nl->name->Auto;
			if (A->SonsNum<1) KerMain->Errors->LogError(eKRTEerrorInAuto,0,nl->name->GetNameString());
			switch (A->Type) {
				case eKATvar:
					for (f=0;f<A->SonsNum;f++)
						if (!A->intervals[f].GoDown || A->intervals[f].GoDown->Type != eKerNTauto || nl->name->Compare(A->intervals[f].GoDown)!=1) KerMain->Errors->LogError(eKRTEerrorInAuto,0,nl->name->GetNameString());
					break;
					if (A->Default && ( A->Default->Type != eKerNTauto || nl->name->Compare(A->Default)!=1)) KerMain->Errors->LogError(eKRTEerrorInAuto,0,nl->name->GetNameString());
				case eKATconnect:
					for (f=0;f<A->SonsNum;f++)
						if (!A->Connect->GoDown[f] || A->Connect->GoDown[f]->Type != eKerNTauto || nl->name->Compare(A->Connect->GoDown[f])!=1) KerMain->Errors->LogError(eKRTEerrorInAuto,0,nl->name->GetNameString());
					break;
				case eKATpattern:
					for (f=0;f<A->SonsNum;f++)
						if (!A->GoDown[f] || A->GoDown[f]->Type != eKerNTauto || nl->name->Compare(A->GoDown[f])!=1) KerMain->Errors->LogError(eKRTEerrorInAuto,0,nl->name->GetNameString());
					break;
				case eKATrandom:
					for (f=0;f<A->SonsNum;f++)
						if (!A->psti[f].GoDown || A->psti[f].GoDown->Type != eKerNTauto || nl->name->Compare(A->psti[f].GoDown)!=1) KerMain->Errors->LogError(eKRTEerrorInAuto,0,nl->name->GetNameString());
					if (A->psti[A->SonsNum-1].pst<1) KerMain->Errors->LogError(eKRTEerrorInAuto,0,nl->name->GetNameString());
					break;
			}
		}
		nl = nl->next;
	}
}





///////////////////////////////////////////////////////////////////////
// prideli dafaultni automatismy k objektum
void CKerAutosMain::AssignAutosToObjs() {
	CKerObjectT *ko;
	CKerNameList *list;
	int f;
	for (f=0; f<KerMain->Objs->NumObjectT; f++) {
		ko = &(KerMain->Objs->ObjectTypes[f]);
		if (ko->EditTag & etbInMap || (ko->SpecificKnownVars && ko->SpecificKnownVars[eKVUaPicture])) {
			list = KerMain->KerNamesMain->FindLayer(ko->AName,0,eKerNTauto);
			if (!list) {
				if (ko->EditTag & etbInMap) KerMain->Errors->LogError(eKRTEautoNotFound,0,ko->Name->GetNameString());
			} else {
				ko->DefAuto = list->name;  // vezmu prvniho, TODO rozhodovani podle stylu
				list->DeleteAll();
			}
		}
	}
}



////////////////////////////////////////////////////////////////////////
///		Destructor
CKerAutosMain::~CKerAutosMain() {
	CKerAuto *a = autos, *a2;
	SAFE_DELETE(SaveReg);
	SAFE_DELETE_ARRAY(SaveVer);
	while(a) {
		a2 = a;
		a = a->next;
		delete a2;
	}
}




//////////////////////////////////////////////
void CKerAutosMain::GEnProcessEnded(OPointer obj, int ProcessID) {
}






////////////////////////////////////////////////////////////////////
///
///		Jednoduche zmeny automatismu za behu kernelu
///
////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////
// Je treba volat drive, nez se zacnou za behu pridavat nove automatismy. Otevreni existujiciho souboru - verzi nastav na NULL. vytvoreni noveho souboru - Verzi mi predej. Vrati 1 OK, 0 - chyba. Pokud je uz neco otevreny, zmeny budou zapomenuty!
int CKerAutosMain::OpenFileToSaveAutos(const char *file, const char *version) {
	SAFE_DELETE(SaveReg);
	SAFE_DELETE_ARRAY(SaveVer);
	SaveReg = new CFSRegister(file,"KRKAL AUTOMATICS");
	CFSRegKey *k=0;
	if ((!(k = SaveReg->FindKey("Version")) && !version) || SaveReg->GetOpenError()==FSREGCONTENTERROR) {
		SAFE_DELETE(SaveReg);
		return 0;
	}
	if (k) SaveVer = newstrdup(k->GetDirectAccess());
	else {
		SaveVer = newstrdup(version);
		SaveReg->AddKey("Version",FSRTstring)->stringwrite(SaveVer);
		SaveReg->AddKey("Names",FSRTregister);
		SaveReg->AddKey("Dependencies",FSRTint);
	}
	if ((k=SaveReg->FindKey("Closed File")) && k->readi()==1) {
		SAFE_DELETE(SaveReg);
		SAFE_DELETE_ARRAY(SaveVer);
		return 0;
	}

	return 1;
}

//////////////////////////////////////////////////////
// Prida do souboru zmeny. 1 OK, 0 chyba
int CKerAutosMain::SaveChangedAutos() {
	if (!SaveReg) return 0;
	return SaveReg->WriteFile();
}



//////////////////////////////////////////////////////////////////////
// prida k objektu existujici automatismus. Je treba mit otevreny soubor, kam se bude sejvovat
void CKerAutosMain::AddEasyAuto(CKerObjectT *object, CKerName *Auto) {
	if (!SaveReg) return;
	CFSRegister *r = SaveReg->FindKey("Names")->GetSubRegister();
	CFSRegKey *k;
	CFSRegister *r3;
	int OPos, APos;
	assert(object->AName && object->AName->GetNameString());
	assert(Auto->Type == eKerNTauto);

	// pridam nebo najdu jmena v registru
	if (k = r->FindKey(object->AName->GetNameString())) OPos = r->FindKeyPos(k);
	else {
		OPos = r->GetNumberOfKeys();
		r3 = r->AddKey(object->AName->GetNameString(),FSRTregister)->GetSubRegister();
		r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	}
	if (k = r->FindKey(Auto->GetNameString())) APos = r->FindKeyPos(k);
	else {
		APos = r->GetNumberOfKeys();
		r3 = r->AddKey(Auto->GetNameString(),FSRTregister)->GetSubRegister();
		r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	}

	// vytvorim zavislost
	k = SaveReg->FindKey("Dependencies");
	k->pos = k->top;
	k->writei(OPos);  k->writei(APos);

	// pridam automatismus k objektu
	object->AName->AddChild(Auto);
	object->DefAuto = Auto;
	KerMain->KerNamesMain->CreateMatrix();
}


///////////////////////////////////////////////////////
// prida automatismus, ktery vytvori ze zadaneho obrazku nebo animace (.tex, .ani) Je treba mit otevreny soubor, kam se bude sejvovat
CKerName *CKerAutosMain::AddEasyAuto(char *path) {
	if (!SaveReg || !path) return 0;
	int num = 1;
	int f,g, size;
	int OK=0;
	size = strlen(path);
	f = size-4; // ukazije na tecku .tex, .ani
	if (size < 5) return 0;
	while (f>0 && path[f-1] != '/' && path[f-1] != '\\') f--;
	char *str = new char[size+100];
	strcpy(str,"_KSAD_");
	for ( g=6; f < size-4; f++,g++) str[g] = path[f];
	// g zakonzervu. Ukazuje na konec pevne casti

	CGEnResource *tex = GEnMain->LoadPicture(path);
	if (!tex) return 0;

	do {
		sprintf(str+g,"%i_%s",num,SaveVer);
		OK = 1;
		num++;
		if (KerMain->KerNamesMain->GetNamePointer(str)) OK = 0;
		if (SaveReg->FindKey("Names")->GetSubRegister()->FindKey(str)) OK = 0;
	} while (!OK);

	// Vytvorim Automatismus
	CKerAuto *A;
	A = new CKerAuto(eKATpicture,1);
	CKerName *Auto = new CKerName(str,KerMain->KerNamesMain);
	Auto->Type = eKerNTauto;
	Auto->Auto = A;
	A->textures[0].texture = tex;
	A->textures[0].texture->AddRef();

	// sejvnu automatismus
	CFSRegister *r3;
	// 3
	r3 = SaveReg->FindKey("Names")->GetSubRegister()->AddKey(str,FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite(path);

	delete[] str;
	return Auto;
}


///////////////////////////////////////////////////////////////////////
// prida k objektu automatismus, ktery vytvori ze zadaneho obrazku nebo animace (.tex, .ani) Je treba mit otevreny soubor, kam se bude sejvovat
void CKerAutosMain::AddEasyAuto(CKerObjectT *object, char *path) {
	if (!SaveReg || !path) return;
	CKerName *Auto;
	Auto = AddEasyAuto(path);
	if (Auto) AddEasyAuto(object,Auto);
}


