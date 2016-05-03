//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - S E R V I C E S
///
///		Sluzby Kernelu, Interface Systemu Krkal pro skripty
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "KerServices.h"
#include "ComFunctions.h"
#include "names.h"
#include "LevelMap.h"
#include "objects.h"
#include "register.h"
#include "gui.h"
#include "autos.h"
#include "scriptInterface.h"
#include "MersenneTwister.h"
#include "Ident.h"
#include "krkal.h"
#include "MMDesktop.h"


// Objekty obsahujici sluzby Kernelu
CKerServices KerServices;
CKerSaveLoad KerSaveLoad;


UI CKerSaveLoad::KERSL_NAMEID = 0xABCDE224;
UI CKerSaveLoad::KERSL_STRINGID = 0x518FD596;
UI CKerSaveLoad::KERSL_ARRINTID = 0x47AA55F0;
UI CKerSaveLoad::KERSL_ARRCHARID = 0x47AA55F1;
UI CKerSaveLoad::KERSL_ARRDOUBLEID = 0x47AA55F2;
UI CKerSaveLoad::KERSL_ARRNAMEID = 0x47AA55F3;
UI CKerSaveLoad::KERSL_ARROBJID = 0x47AA55F4;
UI CKerSaveLoad::KERSL_ARRNULLID = 0x47AA55FF;


char *Extensions_Strings[KERNUMEXTENSIONS] = {
	"X.lv","X.pkg","X.kc","X.a",".els",".tex",".ani",".mm",".ms"
};




////////////////////////////////////////////////////////////////////
//
//		C K e r S e r v i c e s
//
////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////
// Vrati, jedna, jestlize jmena existuji a prvni <= Druhemu; JInak nulu
int CKerServices::IsFMemberOfS(CKerName *first, CKerName *second) {
	if (!first||!second) return 0;
	if (first==second) return 1;
	if (second == KnownNames[eKKNeverything].Name && first != KnownNames[eKKNeverything].Name) return 1;
	if (first->Compare(second)>=2) return 1;
	return 0;
}


///////////////////////////////////////////////////////////////////
// Vrati, jedna, jestlize jmena existuji a prvni <= Druhemu; JInak nulu
// Prvni jmeno si zjisti jako typ objektu
int CKerServices::IsFMemberOfS(OPointer first_obj, CKerName *second) {
	if (!second) return 0;
	CKerObject *kobj = KerMain->Objs->GetObject(first_obj);
	CKerName *first;
	if (!kobj||!(first = kobj->Type->Name)) return 0;
	if (first==second) return 1;
	if (second == KnownNames[eKKNeverything].Name && first != KnownNames[eKKNeverything].Name) return 1;
	if (first->Compare(second)>=2) return 1;
	return 0;
}


///////////////////////////////////////////////////////////////////
// Vrati, jedna, jestlize jmena existuji a prvni <= Druhemu; JInak nulu
int CKerServices::IsFMemberOfS(OPointer first_obj, OPointer second_obj) {
	CKerName *first;
	CKerName *second;
	CKerObject *kobj = KerMain->Objs->GetObject(first_obj);
	if (!kobj||!(first = kobj->Type->Name)) return 0;
	kobj = KerMain->Objs->GetObject(second_obj);
	if (!kobj||!(second = kobj->Type->Name)) return 0;
	if (first==second) return 1;
	if (second == KnownNames[eKKNeverything].Name && first != KnownNames[eKKNeverything].Name) return 1;
	if (first->Compare(second)>=2) return 1;
	return 0;
}



///////////////////////////////////////////////////////////////////
// Vrati, jedna, jestlize jmena existuji a prvni <= Druhemu; JInak nulu
int CKerServices::IsFMemberOfS(CKerName *first, OPointer second_obj) {
	if (!first) return 0;
	CKerName *second;
	CKerObject *kobj = KerMain->Objs->GetObject(second_obj);
	if (!kobj||!(second = kobj->Type->Name)) return 0;
	if (first==second) return 1;
	if (second == KnownNames[eKKNeverything].Name && first != KnownNames[eKKNeverything].Name) return 1;
	if (first->Compare(second)>=2) return 1;
	return 0;
}




///////////////////////////////////////////////////////////////////
// Vrati, jedna, jestlize jmena existuji a prvni < Druhemu; JInak nulu
int CKerServices::IsFMemberOfSExcl(CKerName *first, CKerName *second) {
	if (!first||!second) return 0;
	if (second == KnownNames[eKKNeverything].Name && first != KnownNames[eKKNeverything].Name) return 1;
	if (first->Compare(second)>=2) return 1;
	return 0;
}


///////////////////////////////////////////////////////////////////
// Vrati, jedna, jestlize jmena existuji a prvni < Druhemu; JInak nulu
// Prvni jmeno si zjisti jako typ objektu
int CKerServices::IsFMemberOfSExcl(OPointer first_obj, CKerName *second) {
	if (!second) return 0;
	CKerObject *kobj = KerMain->Objs->GetObject(first_obj);
	CKerName *first;
	if (!kobj||!(first = kobj->Type->Name)) return 0;
	if (second == KnownNames[eKKNeverything].Name && first != KnownNames[eKKNeverything].Name) return 1;
	if (first->Compare(second)>=2) return 1;
	return 0;
}


///////////////////////////////////////////////////////////////////
// Vrati, jedna, jestlize jmena existuji a prvni < Druhemu; JInak nulu
int CKerServices::IsFMemberOfSExcl(OPointer first_obj, OPointer second_obj) {
	CKerName *first;
	CKerName *second;
	CKerObject *kobj = KerMain->Objs->GetObject(first_obj);
	if (!kobj||!(first = kobj->Type->Name)) return 0;
	kobj = KerMain->Objs->GetObject(second_obj);
	if (!kobj||!(second = kobj->Type->Name)) return 0;
	if (second == KnownNames[eKKNeverything].Name && first != KnownNames[eKKNeverything].Name) return 1;
	if (first->Compare(second)>=2) return 1;
	return 0;
}



///////////////////////////////////////////////////////////////////
// Vrati, jedna, jestlize jmena existuji a prvni < Druhemu; JInak nulu
int CKerServices::IsFMemberOfSExcl(CKerName *first, OPointer second_obj) {
	if (!first) return 0;
	CKerName *second;
	CKerObject *kobj = KerMain->Objs->GetObject(second_obj);
	if (!kobj||!(second = kobj->Type->Name)) return 0;
	if (second == KnownNames[eKKNeverything].Name && first != KnownNames[eKKNeverything].Name) return 1;
	if (first->Compare(second)>=2) return 1;
	return 0;
}



////////////////////////////////////////////////////////////////
// precte z objektu souradnice ma-li je (jinak vraci nuly). 
void CKerServices::ReadObjCoords(OPointer obj, int *x, int *y, int *z) {
	*x=0; *y=0; *z=0;
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	if (!ko) return;
	if (ko->PlacedInfo) {
		*x = ko->PlacedInfo->x;
		*y = ko->PlacedInfo->y;
		*z = ko->PlacedInfo->z;
	} else {
		KerMain->GetAccessToObject(ko);
		KerMain->MapInfo->ReadObjCoords(ko,*x,*y,*z);
		REINIT_CONTEXT;
	}
}


// funkce vytvareji kopie kernelich poli
CKerArrInt *CKerServices::CopyIntA(CKerArrInt *a) {
	CKerArrInt *b;
	int f;
	if (!a) return 0;
	b = new CKerArrInt();
	for (f=0; f<a->GetCount(); f++) b->Add(a->Read(f));
	return b;
}
CKerArrChar *CKerServices::CopyCharA(CKerArrChar *a) {
	CKerArrChar *b;
	int f;
	if (!a) return 0;
	b = new CKerArrChar();
	for (f=0; f<a->GetCount(); f++) b->Add(a->Read(f));
	return b;
}
CKerArrDouble *CKerServices::CopyDoubleA(CKerArrDouble *a) {
	CKerArrDouble *b;
	int f;
	if (!a) return 0;
	b = new CKerArrDouble();
	for (f=0; f<a->GetCount(); f++) b->Add(a->Read(f));
	return b;
}
CKerArrObject *CKerServices::CopyObjPtrA(CKerArrObject *a) {
	CKerArrObject *b;
	int f;
	if (!a) return 0;
	b = new CKerArrObject();
	for (f=0; f<a->GetCount(); f++) b->Add(a->Read(f));
	return b;
}
CKerArrName *CKerServices::CopyNameA(CKerArrName *a) {
	CKerArrName *b;
	int f;
	if (!a) return 0;
	b = new CKerArrName();
	for (f=0; f<a->GetCount(); f++) b->Add(a->Read(f));
	return b;
}


// alokuje kerneli pole
void CKerServices::KernelStartInit() {
	IntBuff = new CKerArrInt();
	ObjBuff = new CKerArrObject();
}


/////////////////////////////////////////////////////////////////////////
// najde objekty, ktere jsou z mnoziny set, v bunkach na dane oblasti. set nemusi byt zadan.
// pokud je zadan objekt, tak je oblast brana relativne k centralni bunce onjektu.
void CKerServices::FindObjsInArea(int CodeLine, CKerArrObject *output,OPointer obj, CKerName *set, int x1, int y1, int x2, int y2, int z) {
	output->SetCount(0);
	IntBuff->SetCount(0);
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	if (ko) { // najdu pozici objektu a upravim podle ni oblast:
		int x,y;
		KerMain->GetAccessToObject(ko);
		KerMain->MapInfo->FindObjCell(ko,x,y,z);
		REINIT_CONTEXT;
		x1+=x; x2+=x;
		y1+=y; y2+=y;
	}
	
	// vyplnim pole bunek:
	int f,g;
	for (f=x1; f<=x2; f++) for (g=y1; g<=y2; g++) {
		IntBuff->Add(f);
		IntBuff->Add(g);
		IntBuff->Add(z);
	}
	// najdu objekty:
	KerMain->MapInfo->GetObjsFromCellSet(CodeLine,IntBuff,output);

	// otestuju je proti mnozine:
	if (set) {
		int sub=0;
		for (f=0; f<output->GetCount(); f++) {
			if (IsFMemberOfS(output->Read(f),set)) output->Get(f-sub) = output->Read(f);
			else sub++;
		}
		output->SetCount(output->GetCount()-sub);
	}
}


///////////////////////////////////////////////////////////////////
// naleznou se objekty v oblasti a zavola se na ne metoda
void CKerServices::AreaCall(int CodeLine, CKerName *Method,OPointer obj, CKerName *set, int x1, int y1, int x2, int y2, int z) {
	CKerArrObject output;
	IntBuff->SetCount(0);
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	if (ko) { // najdu pozici objektu a upravim podle ni oblast:
		int x,y;
		KerMain->GetAccessToObject(ko);
		KerMain->MapInfo->FindObjCell(ko,x,y,z);
		REINIT_CONTEXT;
		x1+=x; x2+=x;
		y1+=y; y2+=y;
	}
	
	// vyplnim pole bunek:
	int f,g;
	for (f=x1; f<=x2; f++) for (g=y1; g<=y2; g++) {
		IntBuff->Add(f);
		IntBuff->Add(g);
		IntBuff->Add(z);
	}
	// najdu objekty:
	KerMain->MapInfo->GetObjsFromCellSet(CodeLine,IntBuff,&output);

	// otestuju je proti mnozine a provedu volani:
	for (f=0; f<output.GetCount(); f++) {
		if (!set || IsFMemberOfS(output.Read(f),set)) KerMain->call(CodeLine,output.Read(f),Method,0);
	}
}


////////////////////////////////////////////////////////////////////
 // naleznou se objekty v oblasti a zavola se na ne metoda
void CKerServices::AreaMessage(int CodeLine, CKerName *Method,OPointer obj, CKerName *set, int x1, int y1, int x2, int y2, int z) {
	ObjBuff->SetCount(0);
	IntBuff->SetCount(0);
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	if (ko) { // najdu pozici objektu a upravim podle ni oblast:
		int x,y;
		KerMain->GetAccessToObject(ko);
		KerMain->MapInfo->FindObjCell(ko,x,y,z);
		REINIT_CONTEXT;
		x1+=x; x2+=x;
		y1+=y; y2+=y;
	}
	
	// vyplnim pole bunek:
	int f,g;
	for (f=x1; f<=x2; f++) for (g=y1; g<=y2; g++) {
		IntBuff->Add(f);
		IntBuff->Add(g);
		IntBuff->Add(z);
	}
	// najdu objekty:
	KerMain->MapInfo->GetObjsFromCellSet(CodeLine,IntBuff,ObjBuff);

	// otestuju je proti mnozine a provedu volani:
	for (f=0; f<ObjBuff->GetCount(); f++) {
		if (!set || IsFMemberOfS(ObjBuff->Read(f),set)) KerMain->message(CodeLine,ObjBuff->Read(f),Method,eKerCTmsg,0,0);
	}
}




////////////////////////////////////////////////////////////////
// ze stringu vyhodi uvodni _KS??_, cisla verzi, __M_ nahradi ctyrbotkou
// funkce string alokuje, je treba dealokovat!
char *CKerServices::ConverNameToShort(const char *ksid, int EatStart, char *LocalizeTo) {
	int len,f=0,sub=0;
	int aa;
	int NType = 0;
	assert(ksid); if (!ksid) return 0;
	len = strlen(ksid);
	if (len+1>=16000) throw CExc(eKernel,1,"Error - Pretekl pomocny buffer :(");
	char *str = strbuff;
	if (ksid[0]=='_' && ksid[1]=='K' && ksid[2]=='S') {
		if (ksid[3]=='I') NType=1;
		else if (ksid[3]=='D') NType=2;
		else if (ksid[3]=='O') NType=3;
		f=3; sub=3; while(ksid[f]!='_' && f<len) { f++; sub++; }
		if (ksid[f]=='_') { f++; sub++; }
	}
	
	int ltIndex=0;
	if (LocalizeTo && LocalizeTo[0]=='_' && LocalizeTo[1]=='K' && LocalizeTo[2]=='S') {
		ltIndex=3; while(LocalizeTo[ltIndex]!='_' && LocalizeTo[ltIndex]!=0) ltIndex++; 
		if (LocalizeTo[ltIndex]=='_') { ltIndex++; }
	}

	if (LocalizeTo) {
		int f2=f;
		while (LocalizeTo[ltIndex] == ksid[f2] && ksid[f2]!=0) {ltIndex++; f2++;}
		if (ksid[f2]=='_' && ksid[f2+1]=='_' && ksid[f2+2]=='M' && ksid[f2+3]=='_') {
			if (LocalizeTo[ltIndex]==0) {f=f2; sub=f2;}
			if (NType==2 || NType==3) {
				f+=4; sub+=4;
			} else {
				str[f-sub] = ':'; str[f-sub+1] = ':';
				sub+=2; f+=4;
			}
		}
	}

	do {
		if (ksid[f]=='_' && ksid[f+1]=='_' && ksid[f+2]=='M' && ksid[f+3]=='_') aa=1; else aa=0;
		if (ksid[f]=='_' && ksid[f+1]=='K' && ksid[f+2]=='N' && ksid[f+3]=='_') {
			str[f-sub] = '@';
			f+=4; sub+=3;
			if (ksid[f]=='_' && ksid[f+1]=='_' && ksid[f+2]=='M' && ksid[f+3]=='_') aa=1; else aa=0;
		}
		while (ksid[f] && !aa) {
			str[f-sub] = ksid[f];
			f++;
			if (ksid[f]=='_' && ksid[f+1]=='_' && ksid[f+2]=='M' && ksid[f+3]=='_') aa=1; else aa=0;
		}
		if (f-sub>=20 && ishexa(ksid[f-1]) && ishexa(ksid[f-2]) && ishexa(ksid[f-3]) && ishexa(ksid[f-4]) && ksid[f-5]=='_') sub+=20;
		if (aa) {
			str[f-sub] = ':'; str[f-sub+1] = ':';
			sub+=2; f+=4;
			if (EatStart) {
				sub = f;
				EatStart--;
			}
		}
	} while (ksid[f]);
	str[f-sub] = ksid[f];
	return str;
}




/////////////////////////////////////////////////////////////////
// Pokud zna priponu,vrati jmeno souboru bez pripony a bez pripadne verze. Jinak jmeno nemeni. Fce vraci pointer na svuj buffer, nedealokovat
char *CKerServices::ConvertFileNameToShort(const char *file, int &ext, int AllwaysEatVersion, int EatKnExtension) {
	int len,len2,f,g,sub=0;
	int aa;
	ext = eEXTunknown;
	if (!file) return 0;
	len = strlen(file);
	if (len+1>=16000) throw CExc(eKernel,1,"Error - Pretekl pomocny buffer :(");
	char *str = strbuff;
	const char *ksid = file;

	for(f=0;f<KERNUMEXTENSIONS;f++) {
		len2=strlen(Extensions_Strings[f]);
		g=1;
		while(Extensions_Strings[f][len2-g] != '.' && len-g>=0 && Extensions_Strings[f][len2-g]==file[len-g]) g++;
		if (len-g>=0 && Extensions_Strings[f][len2-g]==file[len-g]) {
			ext = f;
			break;
		}
	}

	if (ext==eEXTunknown || !EatKnExtension) g=0;
	f=0;
	do {
		if ((ksid[f]=='_' && ksid[f+1]=='_' && ksid[f+2]=='M' && ksid[f+3]=='_') || ksid[f]=='.' || ksid[f]=='\\' || ksid[f]=='/') aa=1; else aa=0;
		while (f < len-g && !aa) {
			str[f-sub] = ksid[f];
			f++;
			if ((ksid[f]=='_' && ksid[f+1]=='_' && ksid[f+2]=='M' && ksid[f+3]=='_') || ksid[f]=='.' || ksid[f]=='\\' || ksid[f]=='/') aa=1; else aa=0;
		}
		if (f>=20 && (AllwaysEatVersion || (ext!=eEXTunknown && Extensions_Strings[ext][0]=='X')) && ishexa(ksid[f-1]) && ishexa(ksid[f-2]) && ishexa(ksid[f-3]) && ishexa(ksid[f-4]) && ksid[f-5]=='_') sub+=20;
		if (aa && ksid[f]=='_') {
			str[f-sub] = ':'; str[f-sub+1] = ':';
			sub+=2; f+=4;
		} else if (aa && f<len-g) {
			str[f-sub] = ksid[f];
			f++;
		}
	} while (f < len-g);
	str[f-sub] = 0;
	return str;
}


///////////////////////////////////////////////////////
// nastartuje plynuly pohyb
void CKerServices::InitMoveTo(OPointer obj, UI time, int dx, int dy, int dz) {
	int f;
	CKerObject *ko2;
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	if (!ko || !ko->elements) return;
	for (f=0; f<ko->PlacedInfo->MConnectedObjs.GetCount(); f++) {
		ko2 = KerMain->Objs->GetObject(ko->PlacedInfo->MConnectedObjs.Read(f));
		new CKerMoveTimer(ko2,ko2->PlacedInfo->x,ko2->PlacedInfo->y,ko2->PlacedInfo->z, ko2->PlacedInfo->x+dx, ko2->PlacedInfo->y+dy, ko2->PlacedInfo->z+dz,time);
	}
}



////////////////////////////////////////////////////////////////////
// reset automatismu
void CKerServices::ResetAuto(OPointer obj, int ResetRandom, int ResetConnection) {  
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	if (ko) KerMain->AutosMain->ProcessAuto2(ko,0,ResetConnection,ResetRandom);
	REINIT_CONTEXT;
}


////////////////////////////////////////////////////////////////////
CKerName *CKerServices::GetDefaultAuto2(CKerName *ObjType) { 
	if (!ObjType || ObjType->Type != eKerNTobject) return 0; else return ObjType->ObjectType->DefAuto; 
}


////////////////////////////////////////////////////////////////
// propoji objekty, tak ze se budou hejbat spolu
void CKerServices::MConnectObjs(OPointer obj1, OPointer obj2) {
	CKerObject *ko1, *ko2, *ko;
	int f,c1,g,c2;
	ko1 = KerMain->Objs->GetObject(obj1);
	ko2 = KerMain->Objs->GetObject(obj2);
	if (!ko1 || !ko2) return;
	if (!ko1->PlacedInfo || !ko2->PlacedInfo) return;

	c1 = ko1->PlacedInfo->MConnectedObjs.GetCount();
	c2 = ko2->PlacedInfo->MConnectedObjs.GetCount();
	for (f=0; f<c1; f++) if (obj2 == ko1->PlacedInfo->MConnectedObjs.Read(f)) return; // uz jsou propojeny

	CKerMoveTimer *Mover = ko2->PlacedInfo->Mover;
	for (f=0; f<c1; f++) { // ke kazdemu z prvni skupiny pridam to co je ve druhe skupine
		ko = KerMain->Objs->GetObject(ko1->PlacedInfo->MConnectedObjs.Read(f));
		for (g=0; g<c2; g++) {
			ko->PlacedInfo->MConnectedObjs.Add(ko2->PlacedInfo->MConnectedObjs.Read(g));
		}
		SAFE_DELETE(ko->PlacedInfo->Mover);  // Necham at se objekty pohybuji stejne
		if (Mover) new CKerMoveTimer(ko,Mover);
	}
	for (f=0; f<c2; f++) { // ke kazdemu z druhe skupiny pridam to co je v prvni skupine
		ko = KerMain->Objs->GetObject(ko2->PlacedInfo->MConnectedObjs.Read(f));
		for (g=0; g<c1; g++) ko->PlacedInfo->MConnectedObjs.Add(ko1->PlacedInfo->MConnectedObjs.Read(g));
	}
}


////////////////////////////////////////////////////////////////
// odpoji objekt od skupiny
void CKerServices::MDisconnectObj(OPointer obj) {
	CKerObject *ko1, *ko;
	int f,c,g;
	int sub;
	ko1 = KerMain->Objs->GetObject(obj);
	if (!ko1 || !ko1->PlacedInfo) return;
	c = ko1->PlacedInfo->MConnectedObjs.GetCount();

	for (f=1; f<c; f++) {
		ko = KerMain->Objs->GetObject(ko1->PlacedInfo->MConnectedObjs.Read(f));
		sub = 0;
		for (g=1; g<c; g++) {
			if (ko->PlacedInfo->MConnectedObjs.Read(g) == obj) sub = 1;
			else ko->PlacedInfo->MConnectedObjs[g-sub] = ko->PlacedInfo->MConnectedObjs.Read(g);
		}
		ko->PlacedInfo->MConnectedObjs.SetCount(c-1);
	}
	ko1->PlacedInfo->MConnectedObjs.SetCount(1);
}




/////////////////////////////////////////////////////////////////////
// vygeneruje nove cislo verze pomoci MersenneTwisteru. Vrati ho jako cislo. 
// Pokud predas funkci pointer na string, kam se vejde 19 znaku (+1 na koncovou nulu), tak i jako string. 
_int64 CKerServices::GenerateVersionNumber(char *DestStr ) {
	_int64 a,b,c,d;
	a = mtr.randInt(0xFFFF);
	b = mtr.randInt(0xFFFF);
	c = mtr.randInt(0xFFFF);
	d = mtr.randInt(0xFFFF);
	if (DestStr) {
		sprintf(DestStr,"%04X_%04X_%04X_%04X",(UI)a,(UI)b,(UI)c,(UI)d);
	}
	return (a<<48)|(b<<32)|(c<<16)|d;
}




/////////////////////////////////////////////////////////////////////////
//
//		C K e r S a v e L o a d
//
/////////////////////////////////////////////////////////////////////////


// volej pred tim, nez chces naloadovat/sejvnout objekt
CFSRegKey *CKerSaveLoad::Open(CFSRegKey *key, int state) { 
	CFSRegKey *oldb = buff;
	if (!key) { // close
		SaveState = eSLSnothing; buff=0;
		return oldb;
	}
	assert(key->CFSGetKeyType() == FSRTchar || key->CFSGetKeyType() == FSRTstring);
	buff = key; SaveState = state; 
	return oldb;
}  

// volej pred tim, nez chces naloadovat/sejvnout objekt
CFSRegKey *CKerSaveLoad::Open(CFSRegister *reg, int state, CKerMethod *m) { 
	CFSRegKey *oldb = buff;
	CFSRegKey *key;
	if (!reg) { // close
		SaveState = eSLSnothing; buff=0;
		return oldb;
	}
	char *str;
	if (m->ParentObj) str = m->ParentObj->GetNameString(); else str = "DS_UnknownObj";
	key = reg->FindKey(str);
	if (!key) {
		if (state == eSLSsaveGame || state == eSLSsaveLevel) key = reg->AddKey(str,FSRTchar);
		else (key = reg->FindKey("Data Stream"));
	}
	if (!key) { // close
		SaveState = eSLSnothing; buff=0;
		return oldb;
	}
	buff = key; SaveState = state; 
	return oldb;
}  



// prida hranu do sejvovaciho grafu. Volat tehdy, kdyz obj1 ma u sebe pointer na obj2
void CKerSaveLoad::AddEdge(OPointer obj1, OPointer obj2, CFSRegKey *key, int pos) {
	CKerObject *ko1, *ko2;
	ko1 = KerMain->Objs->GetObject(obj1);
	assert(ko1 && ko1->SaveRec);
	ko2 = KerMain->Objs->GetObject(obj2);
	if (!ko2) return;
	if (!ko2->SaveRec) {
		KerMain->Errors->LogError(eKRTEsavingOptrToNoSavO);
		return;
	}

	ko1->SaveRec->SaveGraph->Add(obj2);
	ko2->SaveRec->AddPointer(key,pos,obj1);
}



////////////////////////////////////////////////////
// prace se streamem

void CKerSaveLoad::Seek(int pos) {
	if (SaveState==eSLSnothing) KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"seek");
	else buff->pos = pos;
}

void CKerSaveLoad::SeekToEnd() {
	if (SaveState==eSLSnothing) KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SeekToEnd");
	else buff->pos = buff->top;
}

int CKerSaveLoad::GetPos() {
	if (SaveState==eSLSnothing) {
		KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"GetPos");
		return 0;
	} else return buff->pos;
}

int CKerSaveLoad::Eof() {
	if (SaveState==eSLSnothing) {
		KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"eof");
		return 1;
	} else return buff->eof();
}



///////////////////////////////////////////////////
// SAVE zakladnich typu

void CKerSaveLoad::SaveInt(int a) {
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveInt");	return; }
	buff->blockwrite(&a,4);
}

void CKerSaveLoad::SaveChar(UC a) {
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveChar");	return; }
	buff->writec(a);
}

void CKerSaveLoad::SaveDouble(double a) {
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveDouble");	return; }
	buff->blockwrite(&a,8);
}

void CKerSaveLoad::SaveObjPtr(OPointer obj) {
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveObjPtr");	return; }
	if (SaveState==eSLSsaveLevel) {
		AddEdge(KerContext->KCthis,obj,buff,buff->pos);
		obj = 0;
		buff->blockwrite(&obj,4);
	} else buff->blockwrite(&obj,4);
}

void CKerSaveLoad::SaveName(CKerName *name) {
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveName");	return; }
	if (!buff->eof()) { KerMain->Errors->LogError(eKRTESLwriteInMiddle,0,"SaveName");	return; }
	buff->blockwrite(&KERSL_NAMEID,4);
	if (!name) buff->writec(0);
	else {
		if (/*TODO v release runu to netestovat*/ !KerMain->KerNamesMain->TestPointerValidity(name)) {
			buff->writec(0);
			KerMain->Errors->LogError(eKRTEnotValidName,0,"SaveName");
		} else buff->stringwrite(name->GetNameString());
	}
}

void CKerSaveLoad::SaveName2(CFSRegKey *buff, CKerName *name) {
	if (!buff->eof()) { KerMain->Errors->LogError(eKRTESLwriteInMiddle,0,"SaveName");	return; }
	buff->blockwrite(&KERSL_NAMEID,4);
	if (!name) buff->writec(0);
	else {
		if (/*TODO v release runu to netestovat*/ !KerMain->KerNamesMain->TestPointerValidity(name)) {
			buff->writec(0);
			KerMain->Errors->LogError(eKRTEnotValidName,0,"SaveName");
		} else buff->stringwrite(name->GetNameString());
	}
}

// specialni verze pro Klrkal Convertor
void CKerSaveLoad::SaveName(char *name) {
	buff->blockwrite(&KERSL_NAMEID,4);
	if (!name) buff->writec(0);
	else buff->stringwrite(name);
}

void CKerSaveLoad::SaveString(char *str, int Type) {
	Type = (Type & (eKTret-1)) - eKTstring;
	assert (Type>=0 && Type <= 250);
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveString");	return; }
	if (!buff->eof()) { KerMain->Errors->LogError(eKRTESLwriteInMiddle,0,"SaveString");	return; }
	str[Type] = 0;
	buff->blockwrite(&KERSL_STRINGID,4);
	buff->stringwrite(str);
}




////////////////////////////////////////////////////
// SAVE poli

void CKerSaveLoad::SaveIntA(CKerArrInt *a) {
	int f;
	int count;
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveArray");	return; }
	if (!buff->eof()) { KerMain->Errors->LogError(eKRTESLwriteInMiddle,0,"SaveArray");	return; }
	if (!a) {
		buff->blockwrite(&KERSL_ARRNULLID,4);
		return;
	}
	buff->blockwrite(&KERSL_ARRINTID,4);
	count = a->GetCount();
	buff->blockwrite(&count,4);
	for (f=0; f<count; f++) {
		buff->blockwrite(&a->Read(f),4);
	}
}

void CKerSaveLoad::SaveCharA(CKerArrChar *a) {
	int f;
	int count;
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveArray");	return; }
	if (!buff->eof()) { KerMain->Errors->LogError(eKRTESLwriteInMiddle,0,"SaveArray");	return; }
	if (!a) {
		buff->blockwrite(&KERSL_ARRNULLID,4);
		return;
	}
	buff->blockwrite(&KERSL_ARRCHARID,4);
	count = a->GetCount();
	buff->blockwrite(&count,4);
	for (f=0; f<count; f++) {
		buff->blockwrite(&a->Read(f),1);
	}
}

void CKerSaveLoad::SaveDoubleA(CKerArrDouble *a){
	int f;
	int count;
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveArray");	return; }
	if (!buff->eof()) { KerMain->Errors->LogError(eKRTESLwriteInMiddle,0,"SaveArray");	return; }
	if (!a) {
		buff->blockwrite(&KERSL_ARRNULLID,4);
		return;
	}
	buff->blockwrite(&KERSL_ARRDOUBLEID,4);
	count = a->GetCount();
	buff->blockwrite(&count,4);
	for (f=0; f<count; f++) {
		buff->blockwrite(&a->Read(f),8);
	}
}

void CKerSaveLoad::SaveObjPtrA(CKerArrObject *a){
	int f;
	int count;
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveArray");	return; }
	if (!buff->eof()) { KerMain->Errors->LogError(eKRTESLwriteInMiddle,0,"SaveArray");	return; }
	if (!a) {
		buff->blockwrite(&KERSL_ARRNULLID,4);
		return;
	}
	buff->blockwrite(&KERSL_ARROBJID,4);
	count = a->GetCount();
	buff->blockwrite(&count,4);
	for (f=0; f<count; f++) {
		OPointer obj = a->Read(f);
		if (SaveState==eSLSsaveLevel) {
			AddEdge(KerContext->KCthis,obj,buff,buff->pos);
			obj = 0;
			buff->blockwrite(&obj,4);
		} else buff->blockwrite(&obj,4);
	}
}

// specialni verze pro Krkal Convertor. ulozi jednoprvkove pole. nebo nula prvkove (poradi==0)
void CKerSaveLoad::SaveObjPtrA(int poradi){
	buff->blockwrite(&KERSL_ARROBJID,4);
	int count;
	if (poradi) count=1; else count = 0;
	buff->blockwrite(&count,4);
	if (poradi)	buff->blockwrite(&poradi,4);
}


void CKerSaveLoad::SaveNameA(CKerArrName *a){
	int f;
	int count;
	if (SaveState!=eSLSsaveLevel && SaveState!=eSLSsaveGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"SaveArray");	return; }
	if (!buff->eof()) { KerMain->Errors->LogError(eKRTESLwriteInMiddle,0,"SaveArray");	return; }
	if (!a) {
		buff->blockwrite(&KERSL_ARRNULLID,4);
		return;
	}
	buff->blockwrite(&KERSL_ARRNAMEID,4);
	count = a->GetCount();
	buff->blockwrite(&count,4);
	for (f=0; f<count; f++) {
		if (!a->Read(f)) buff->writec(0);
		else {
			if (/*TODO v release runu to netestovat*/ !KerMain->KerNamesMain->TestPointerValidity(a->Read(f))) {
				buff->writec(0);
				KerMain->Errors->LogError(eKRTEnotValidName,0,"SaveNameA");
			} else	buff->stringwrite(a->Read(f)->GetNameString());
		}
	}
}



////////////////////////////////////////////////////////////////
// LOAD zakladnich typu

int CKerSaveLoad::LoadInt() {
	int a;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadInt");	return 0; }
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadInt Eof"); return 0; }
	buff->blockread(&a,4);
	return a;
}

UC CKerSaveLoad::LoadChar() {
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadChar");	return 0; }
	if (buff->pos+1 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadChar Eof"); return 0; }
	return buff->readc();
}

double CKerSaveLoad::LoadDouble() {
	double a;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadDouble");	return 0; }
	if (buff->pos+8 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadDouble Eof"); return 0; }
	buff->blockread(&a,8);
	return a;
}

OPointer CKerSaveLoad::LoadObjPtr() {
	int a;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadObjPtr");	return 0; }
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadObjPtr Eof"); return 0; }
	buff->blockread(&a,4);
	if (SaveState == eSLSloadConstructor) {
		if (a==0) return 0;
		if (a >= KerMain->Objs->LoadedObjects.GetCount() || KerMain->Objs->LoadedObjects[a]==0) {
			KerMain->Errors->LogError(eKRTESLloadingVar,0,"Object n Exists");
			return 0;
		} else return KerMain->Objs->LoadedObjects[a];
	} else return (OPointer)a;
}

CKerName *CKerSaveLoad::LoadName() {
	int a;
	CKerName *name;
	char *B;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadName");	return 0; }
	if (buff->pos+5 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadName Eof"); return 0; }
	buff->blockread(&a,4);
	if (a != KERSL_NAMEID) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadName"); return 0; }
	B = buff->GetDirectAccessFromPos();
	buff->SetPosToNextString();
	buff->pos--;
	if (buff->readc() != 0) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadName"); return 0; }
	if (B[0]==0) return 0;
	name = KerMain->KerNamesMain->GetNamePointer(B);
	if (!name) KerMain->Errors->LogError(eKRTESLloadingVar,0,"Name n Exists"); //error
	return name;
}


 // nacte jmeno, pro GameLoad
CKerName *CKerSaveLoad::LoadName2(CFSRegKey *buff) {
	int a;
	CKerName *name;
	char *B;
	if (buff->pos+5 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadName Eof"); return 0; }
	buff->blockread(&a,4);
	if (a != KERSL_NAMEID) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadName"); return 0; }
	B = buff->GetDirectAccessFromPos();
	buff->SetPosToNextString();
	buff->pos--;
	if (buff->readc() != 0) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadName"); return 0; }
	if (B[0]==0) return 0;
	name = KerMain->KerNamesMain->GetNamePointer(B);
	if (!name) KerMain->Errors->LogError(eKRTESLloadingVar,0,"Name n Exists"); //error
	return name;
}

void CKerSaveLoad::LoadString(char *str, int Type) {
	Type = (Type & (eKTret-1)) - eKTstring;
	assert (Type>=0 && Type <= 250);
	int len,a;
	str[0] = 0;
	char *B;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadString");	return; }
	if (buff->pos+5 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadString Eof"); return; }
	buff->blockread(&a,4);
	if (a != KERSL_STRINGID) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadString"); return; }
	B = buff->GetDirectAccessFromPos();
	len = buff->pos;
	buff->SetPosToNextString();	
	len = buff->pos - len - 1;  // do zaporu se len dostat nemuze
	buff->pos--;
	if (buff->readc() != 0) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadString"); }
	if (len>Type) {
		KerMain->Errors->LogError(eKRTEstringError,0,"LoadString");
		memcpy(str,B,Type);
		str[Type]=0;
	} else {
		memcpy(str,B,len);
		str[len]=0;
	}
}





///////////////////////////////////////////////////////////////
// LOAD poli

CKerArrInt *CKerSaveLoad::LoadIntA() {
	int a;
	int count,f;
	CKerArrInt *ret;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadArray");	return 0; }
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	buff->blockread(&f,4);
	if (f==KERSL_ARRNULLID) return 0;
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	if (f != KERSL_ARRINTID) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray"); return 0; }
	buff->blockread(&count,4);
	ret = new CKerArrInt();
	if (buff->pos+ 4*count > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return ret; }
	for (f=0; f<count; f++) {
		buff->blockread(&a,4);
		ret->Add(a);
	}
	return ret;
}

CKerArrChar *CKerSaveLoad::LoadCharA() {
	int count,f;
	CKerArrChar *ret;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadArray");	return 0; }
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	buff->blockread(&f,4);
	if (f==KERSL_ARRNULLID) return 0;
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	if (f != KERSL_ARRCHARID) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray"); return 0; }
	buff->blockread(&count,4);
	ret = new CKerArrChar();
	if (buff->pos+ 1*count > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return ret; }
	for (f=0; f<count; f++) {
		ret->Add(buff->readc());
	}
	return ret;
}

CKerArrDouble *CKerSaveLoad::loadDoubleA() {
	double a;
	int count,f;
	CKerArrDouble *ret;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadArray");	return 0; }
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	buff->blockread(&f,4);
	if (f==KERSL_ARRNULLID) return 0;
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	if (f != KERSL_ARRDOUBLEID) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray"); return 0; }
	buff->blockread(&count,4);
	ret = new CKerArrDouble();
	if (buff->pos+ 8*count > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return ret; }
	for (f=0; f<count; f++) {
		buff->blockread(&a,8);
		ret->Add(a);
	}
	return ret;
}

CKerArrObject *CKerSaveLoad::LoadObjPtrA() {
	int a;
	int count,f;
	CKerArrObject *ret;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadArray");	return 0; }
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	buff->blockread(&f,4);
	if (f==KERSL_ARRNULLID) return 0;
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	if (f != KERSL_ARROBJID) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray"); return 0; }
	buff->blockread(&count,4);
	ret = new CKerArrObject();
	if (buff->pos+ 4*count > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return ret; }
	for (f=0; f<count; f++) {
		buff->blockread(&a,4);
		if (SaveState==eSLSloadConstructor) {
			if (a==0) ret->Add(0);
			else if (a >= KerMain->Objs->LoadedObjects.GetCount() || KerMain->Objs->LoadedObjects[a]==0) {
				KerMain->Errors->LogError(eKRTESLloadingVar,0,"Object n Exists");
				ret->Add(0);
			} else ret->Add(KerMain->Objs->LoadedObjects[a]);
		} else ret->Add(a);
	}
	return ret;
}

CKerArrName *CKerSaveLoad::LoadNameA() {
	char *B;
	CKerName *name;
	int count,f;
	CKerArrName *ret;
	if (SaveState!=eSLSloadConstructor && SaveState!=eSLSloadGame) { KerMain->Errors->LogError(eKRTEsaveLoadNotAllowed,0,"LoadArray");	return 0; }
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	buff->blockread(&f,4);
	if (f==KERSL_ARRNULLID) return 0;
	if (buff->pos+4 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return 0; }
	if (f != KERSL_ARRNAMEID) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray"); return 0; }
	buff->blockread(&count,4);
	ret = new CKerArrName();
	for (f=0; f<count; f++) {
		if (buff->pos+ 1 > buff->top) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray Eof"); return ret; }
		B = buff->GetDirectAccessFromPos();
		buff->SetPosToNextString();
		buff->pos--;
		if (buff->readc() != 0) { KerMain->Errors->LogError(eKRTESLloadingVar,0,"LoadArray"); return ret; }
		if (B[0]==0) ret->Add(0); else {
			name = KerMain->KerNamesMain->GetNamePointer(B);
			if (!name) KerMain->Errors->LogError(eKRTESLloadingVar,0,"Name n Exists"); //error
			ret->Add(name);
		}
	}
	return ret;
}



/////////////////////////////////////////////////////////////////////////////
///
///			C K e r E d i t I
///			Sluzby pro tvoreni skriptovanych editacnich polozek
///
/////////////////////////////////////////////////////////////////////////////

CKerEditI KerEditI;
//////////////////

/// Univerzalni funkce pro vytvareni polozky
void CKerEditI::ECreateItem(void *data, int typ, char *label, char *comment) {
	SAFE_DELETE(OVar);
	if (!label || !data) return;
	OVar = new CKerOVar();
	OVar->_KSVar = data;
	OVar->KSVar = &OVar->_KSVar;
	OVar->NameStr = KRKAL->CopyOutStrByLanguage(label);
	if (comment) OVar->Comment = KRKAL->CopyOutStrByLanguage(comment);
	OVar->Use = eKVUBeditable;
	OVar->Type = typ;
}

void CKerEditI::ESetIntLimits(int DV, int mezcfg, int* list, int excl) {
}

// nastavi NamesMask a promennou Use. pouziti pro objptra a pro name
void CKerEditI::ESetVarTags(int tags,int filter) {
	if (!OVar) return;
	OVar->NamesMask = filter;
	tags &= (eKVUBexclusive|eKVUBincludeNull|eKVUBobjInMap|eKVUBobjOutMap|eKVUBplannarNames);
	OVar->Use |= tags;
}

// nastavi EditType (eKET..).
void CKerEditI::ESetEditType(int CodeLine, int tags) {
	if (!OVar) return;
	if (tags!=eKETdefault && tags!=eKETnumeric && tags!=eKETcharacter && tags!=eKETboolean && tags!=eKETconnectionMask && tags!=eKETautomaticGr && tags!=eKETstring) {
		KerMain->Errors->LogError2(CodeLine,eKRTEinvalidEditType,tags);
		return;
	}
	OVar->EditType = tags;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
/// FUNKCE PRO VYTVARENI SKUPINOVYCH POLOZEK

char *CKerEditI::GroupMemberLabels[6] = {"x1","y1","z1","x2","y2","z2"};


int CKerEditI::EAddGroup(char *label, char *comment,int where, int before, int type, int PocetClenu,...) {
	va_list list;
	SAFE_DELETE(OVar);
	int ret = 1;
	int f;
	if (!label) return 0;
	va_start( list, PocetClenu ); 
	for (f=0; f<PocetClenu; f++) if (!va_arg(list,int*)) return 0;
	va_end(list);

	// vytvorim hlavicku skupiny
	OVar = new CKerOVar();
	OVar->NameStr = KRKAL->CopyOutStrByLanguage(label);
	if (comment) OVar->Comment = KRKAL->CopyOutStrByLanguage(comment);
	OVar->Use = eKVUBeditable;
	OVar->Type = eKTvoid;
	OVar->EditType = type;
	ret &= CGUIScriptVariableInterface::AddItem(OVar,where,before);
	if (where) where++;
	OVar = 0;

	// pridam cleny skupiny
	va_start( list, PocetClenu ); 
	for (f=0; f<PocetClenu; f++) {
		OVar = new CKerOVar();
		OVar->_KSVar = va_arg(list,int*);
		OVar->KSVar = &OVar->_KSVar;
		if (PocetClenu==4 && f>=2) OVar->NameStr = newstrdup(GroupMemberLabels[f+1]);
		else OVar->NameStr = newstrdup(GroupMemberLabels[f]);
		OVar->Use = eKVUBeditable;
		OVar->Type = eKTint;
		ret &= CGUIScriptVariableInterface::AddItem(OVar,where,before);
		if (where) where++;
		OVar = 0;
	}
	va_end(list);

	return ret;
}



////////////////////////////////////////////////////////////////
// umisteni vytvorene polozky
int CKerEditI::EPlaceItem(int where, int before) {
	if (!OVar) return 0;
	int ret = CGUIScriptVariableInterface::AddItem(OVar,where,before);
	OVar = 0;
	return ret;
}


//// DESTRUCTOR
CKerEditI::~CKerEditI() {
	SAFE_DELETE(OVar);
}

int rseed = 8855;

void CKerServices::RandomKropic() {
	#ifndef RKROPIC
	return;
	#else
	if (!RKROPIC) return;
	MTRand *rnd = new MTRand(rseed);
	rseed++;
	int f,g;
	int ft, gt;
	ft = 2+rnd->randInt(90);
	UC * P[100];
	for (f=0; f<ft; f++) {
		gt = 33+rnd->randInt(5000);
		P[f] = new UC[gt];
//		for (g=0; g<gt; g++) P[f][g] = (UC)rnd->randInt(255);
		for (g=0; g<gt; g++) P[f][g] = rnd->randInt(1)?0xFF:0;
	}
	for (f=0; f<ft; f++) {
		if (rnd->randInt(5)!=1) delete[] P[f];
	}
	delete rnd;
	#endif
}


void CKerServices::ShowLevelIntro(const char *textPath, const char *header) {
	if (MMDesktop)
		MMDesktop->ShowLevelIntro(textPath, KRKAL->CopyOutStrByLanguage(header));
}





