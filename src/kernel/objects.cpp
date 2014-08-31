//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - O B J E C T S
///
///		Informace o Bezicich objektech. Pristup k jejich Metodam a Datum.
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"
#include "objects.h"
#include "kernel.h"
#include "interpret.h"
#include "ks.h"
#include "KerServices.h"
#include "autos.h"
#include "KerArray.h"
#include "LevelMap.h"
#include "names.h"
#include "register.h"





/////////////////////////////////////////////////////////////

CKerValue::~CKerValue() { 
	switch (typ) {
		case eKTarrChar: if (KerMain) SAFE_DELETE(Achar); break;
		case eKTarrDouble: if (KerMain) SAFE_DELETE(Adouble); break;
		case eKTarrInt:  if (KerMain) SAFE_DELETE(Aint); break;
		case eKTarrName:  if (KerMain) SAFE_DELETE(Aname); break;
		case eKTarrObject:  if (KerMain) SAFE_DELETE(Aobject); break;
		default: if (typ>=eKTstring) SAFE_DELETE_ARRAY(Dstring); 
	}
}



///////////////////////////////////////////////////////////////////
///
///		C K e r O b j e c t   - (instance objektu)
///
///////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
/// Vytvoreni nove instance objektu, alokace, prideleni OPointru, pridani do Hashovaci tabulky
CKerObject::CKerObject(CKerName *type, OPointer ForceObjPtr) {
	void *var;
	CKerObjs *HT = KerMain->Objs;
	if (ForceObjPtr) {
		if (HT->GetObject(ForceObjPtr)) KerMain->Errors->LogError(eKRTEobjPtrConflict);
		thisO = ForceObjPtr;
	} else thisO = HT->GetNewObjectPointer();
	elements = 0; PlacedInfo = 0;
	Tag = 0; SaveRec = 0;

	if (thisO==0x10000000) KerMain->Errors->LogError(eKRTEobjcount1);
	if (thisO==0x80000000) KerMain->Errors->LogError(eKRTEobjcount2);
	if (thisO==0xffffff00) KerMain->Errors->LogError(eKRTEobjcount3);
	if (thisO==0xffffffff) KerMain->Errors->LogError(eKRTEobjcountOVERFLOW);
	
	if (!type||type->Type!=eKerNTobject) { 
		Type=KerMain->Objs->DefaultObjectT; 
		KerMain->Errors->LogError(eKRTEInvalidObjType);
	} else Type=type->ObjectType;
	if (Type->ISDataSize) ISData=KerInterpret->ISMalloc(Type->ISDataSize); else ISData=0;
	// Od ted nesmi vzniknout vyjimka:
	if (Type->AllocKSVG) {
		KSVG=Type->AllocKSVG(); 
		memset(KSVG,0,Type->KSVGsize);
	} else KSVG=0;
	next = HT->HT[thisO & OBJECT_HT_AND];
	HT->HT[thisO & OBJECT_HT_AND] = this;

	// Nahraji defaultni hodnoty nekterych znamych promennych
	if (Type->SetObjectVar) Type->SetObjectVar(KSVG);
	if (var = KerMain->GetAccessToVar3(this,eKVUaPicture)) *(CKerName**)var = KnownNames[eKKNdefaultAuto].Name;
	if (var = KerMain->GetAccessToVar3(this,eKVUaNoConnect)) *(UI*)var = 0;
	if (var = KerMain->GetAccessToVar3(this,eKVUcollizionCfg)) *(UC*)var = eKCCdefault;
	REINIT_CONTEXT;
}


/// Odstraneni objektu (uvolneni pameti, odebrani s Hash tabulek)
CKerObject::~CKerObject() {
	DeleteFromHT();
	SAFE_DELETE(KSVG);
	if (ISData) KerInterpret->ISFree(ISData);
	SAFE_DELETE_ARRAY(elements);
	SAFE_DELETE(SaveRec);
	if (PlacedInfo) {
//		SetAutoDone();
		SAFE_DELETE(PlacedInfo);
	}
}



//////////////////////////////////////////////////////////////////////
CKerObjSaveRec::CKerObjSaveRec() { 
	reg=0; Tag=0; SaveGraph= new CKerArrObject();
	pointers = 0;
	GlobalVar = 0;
	shortcut=0;
}

CKerObjSaveRec::~CKerObjSaveRec() {
	CKerOSRPointers *p1 = pointers, *p2;
	SAFE_DELETE(reg);
	SAFE_DELETE(SaveGraph);
	while (p1) {
		p2 = p1;
		p1 = p1->next;
		delete p2;
	}
}



/// Odebere objekt z Hashovaci tabulky
void CKerObject::DeleteFromHT() {
	CKerObject **o = &(KerMain->Objs->HT[thisO & OBJECT_HT_AND]);
	while (*o) {
		if (*o==this) {*o=next;return;}
		o = &((**o).next);
	}
}





///////////////////////////////////////////////////////////////////
///
///		C K e r O b j s   - HLAVNI OBJEKT PRO PRACI S OBJEKTY
///
///////////////////////////////////////////////////////////////////


// konstruktor
CKerObjs::CKerObjs() {
	int f;
	ObjectTypes = 0;
	DefaultObjectT = 0;
	LOShakeOffSize = 16;
	NumObjectT = 0;
	Counter = 1;  // Opointers zacinam pridelovat od jedne
	for (f=0;f<OBJECT_HT_SIZE;f++) HT[f]=0;
}


///////////////////////////////////////////////////////////////
//	Prevede OPointer na CKerObject* (kdyz OPointer na nic neukazuje nebo objekt uz byl zrusen , vrati 0)
CKerObject *CKerObjs::GetObject(OPointer optr) {
	if (!optr) return 0;
	CKerObject *o = HT[optr&OBJECT_HT_AND];
	while (o) {
		if (o->thisO==optr) return o;
		o=o->next;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////
// zavola funkci postupe na vsechny objekty. Objekty je mozno zabijet
void CKerObjs::WalkThrowAllObjs(void (*Fce)(CKerObject *ko)) {
	int f,g;
	CKerObject *ko1;
	CKerArrObject objs;
	for (f=0; f<OBJECT_HT_SIZE; f++) {
		objs.SetCount(0);
		ko1 = HT[f];
		while (ko1) {
			objs.Add(ko1->thisO);
			ko1 = ko1->next;
		}
		for (g=0; g<objs.GetCount(); g++ ) {
			if (ko1 = GetObject(objs.Read(g))) Fce(ko1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// vyradi z LoadedObjects nepozivane objekty, pole setrese
void CKerObjs::LOShakeOff() {
	int f, sub=0;
	for (f=0; f<LoadedObjects.GetCount(); f++) {
		if (GetObject(LoadedObjects[f])) LoadedObjects[f-sub] = LoadedObjects[f];
		else sub++;
	}
	LOShakeOffSize = LoadedObjects.GetCount()-sub;
	LoadedObjects.SetCount(LOShakeOffSize);
}



////////////////////////////////////////////////////////////////////////
// otestuje spravnost skupinovych promennych
int CKerObjs::CheckVarGroups(CKerOVar **OVars, int NumOV) {
	int f;
	int count = 0;
	int type;
	for (f=0; f<NumOV; f++) {
		if (count) {
			if ((*OVars)[f].Type != type || ((*OVars)[f].EditType&eKETgroupBit)) return 0;
			(*OVars)[f].Use |= eKVUBspecialEdit;
			count--;
		} else if ((*OVars)[f].EditType&eKETgroupBit) {
			type = KerVarGroupInfos[(*OVars)[f].EditType-eKETgroupBit].typ;
			count = KerVarGroupInfos[(*OVars)[f].EditType-eKETgroupBit].count;
		}
	}
	if (count) return 0;
	return 1;
}



//////////////////////////////////////////////////////////////////////////////////
/// Nahraje informace o jedne promenne
int CKerObjs::LoadOVar(CFSRegKey *_km, CKerOVar *OV, CKerOVar ***SpecKnownVars, int GLOBAL) {
	CFSRegister *km = _km->GetSubRegister();
	CFSRegKey *l;
	CKSKSOV *KSOV;
	OV->NameStr = newstrdup(_km->GetName());
	OV->Type = km->FindKey("Type")->readi();
	if (OV->Type != eKTvoid && OV->Type != eKTstruct) {
		if (!GLOBAL) {
			if (l=km->FindKey("Offset")) {
				OV->Offset = l->readi();
				OV->KSVar = 0; // promenna je interpretovana
			} else {
				// promenna je kompilovana -> Najdu promennou
				KSOV = (CKSKSOV*)(KSMain->KSOVs->Member(_km->GetName()));
				if (KSOV) OV->KSVar = KSOV->Variable; else RETERR;
			}
			if (l=km->FindKey("Auto Control Name")) OV->Name = KerInterpret->GetNamesArray()[l->readi()];
		} else if (OV->Type == eKTobject) {
			if (l=km->FindKey("Object")) OV->Name = KerInterpret->GetNamesArray()[l->readi()];
		}
	} 
	if (l=km->FindKey("Use")) OV->Use = l->readi();
	if (l=km->FindKey("Array Size")) OV->ArraySize = l->readi();

	int a = OV->Use >> KERVARUSESSHIFT;
	if (a) {
		if (!*SpecKnownVars) { // nasel jsem prvni znamou promennou -> vytvorim pole a vynuluju ho:
			int ii;
			*SpecKnownVars = new CKerOVar*[KERVARUSESSIZE];
			for (ii=0; ii<KERVARUSESSIZE; ii++) (*SpecKnownVars)[ii] = 0;
		}
		(*SpecKnownVars)[a ] = OV; // Na sparavnou pozici do pole dam pointr na znamou promennou
		if (KerVarUsesInfo[a].PozorPozor) OV->Use |= eKVUBpozorPozor;
		if (KerVarUsesInfo[a].SpecialEdit) OV->Use |= eKVUBspecialEdit;
		OV->EditType = KerVarUsesInfo[a].EditType;
		OV->NamesMask = KerVarUsesInfo[a].NamesMask;
	}

	// tagy pro editor
	if (l=km->FindKey("Edit Type")) OV->EditType = l->readi();
	if (l=km->FindKey("User Name")) OV->UserName = newstrdup(l->GetDirectAccess());
	if (l=km->FindKey("Comment")) OV->Comment = newstrdup(l->GetDirectAccess());

	if (OV->EditType == eKETscripted) {
		if (!(l=km->FindKey("Method")) || OV->Type != eKTvoid) RETERR
		OV->Name = KerInterpret->GetNamesArray()[l->readi()];
		OV->Use = eKVUBeditable;
		if (l=km->FindKey("Param")) OV->ItemID = l->readi();
		return 1;
	}

	if (l=km->FindKey("Names Mask")) OV->NamesMask = l->readi();
	if (l=km->FindKey("Default")) {
		OV->DefaultValue = new CKerValue;
		if (!LoadObjectsLD(l,OV->DefaultValue,OV->Type)) RETERR
	}
	if (OV->Type>=eKTarrChar && OV->Type<eKTstring && (l=km->FindKey("Default Member"))) {
		OV->DefaultMember = new CKerValue;
		if (!LoadObjectsLD(l,OV->DefaultMember,OV->Type-eKTarrChar)) RETERR
	}
	if (l=km->FindKey("Limits")) OV->LimintsCfg = l->readi();
	l=0;  // nacteni intervalu hodnot
	l=km->FindKey("Interval");
	if (!l) l=km->FindKey("List");
	if (!l) l=km->FindKey("Limit Name");
	if (l) {
		int f;
		if (OV->Type >= eKTstring) { // zjistim pocet stringu
			OV->LimitsListCount = 0;
			while (!l->eof()) { OV->LimitsListCount++; l->SetPosToNextString();}
			l->pos = 0;
		} else OV->LimitsListCount = l->top;
		OV->LimitsList = new CKerValue[OV->LimitsListCount];
		int typ2 = OV->Type;
		if (typ2>=eKTarrChar && typ2<eKTstring) typ2-=eKTarrChar;
		if (typ2==eKTobject) typ2=eKTname;
		for (f=0; f<OV->LimitsListCount; f++) {
			if (!LoadObjectsLD(l,OV->LimitsList+f,typ2)) RETERR
		}
	} else if (a = (OV->Use >> KERVARUSESSHIFT)) {
		// zadani mezi pro zname promenne
		if (a>=eKVUcellz && a<=eKVUcellr) {
			OV->LimintsCfg = eKLCup;
			OV->LimitsListCount = 1;
			OV->LimitsList = new CKerValue[OV->LimitsListCount];
			OV->LimitsList[0].typ = eKTchar;
			OV->LimitsList[0].Dchar = 0;
		}
	}
	return 1;
}


/// Vytvori Dvojnika objektoveho jmena pro automatismy
CKerName *CKerObjs::DuplicateObjName(CKerName *name) {
	char *dvojce = newstrdup(name->GetNameString());
	dvojce[3] = 'A';
	CKerName *ret = new CKerName(dvojce,KerMain->KerNamesMain);
	ret->Type = eKerNTobjectShadow;
	delete[] dvojce;
	return ret;
}


//////////////////////////////////
// Inicializuje AutoVars
void CKerObjectT::FindAutoVars() {
	int f,count;
	NumAVars=0;
	for (f=0;f<NumVars;f++) if (OVars[f].Use & eKVUBauto) NumAVars++;
	if (NumAVars) {
		AutoVars = new CKerOVar*[NumAVars];
		count=0;
		for (f=0;f<NumVars;f++) if (OVars[f].Use & eKVUBauto) {
			AutoVars[count] = OVars+f;
			count++;
		}
	}
}


/////////////////////////////////////////////////////////////////////
///		NAHRAJE POPIS OBJEKTU (typu) DO KERNELU
int CKerObjs::LoadObjects(CFSRegister *o) {
	int f, g, NumM, h, NumP, NumOV;
	int PrmOfs;
	CFSRegKey *_ko,*l, *_km, *_kp;
	CFSRegister *ko, *km, *kp;
	CKSKSVG *ksKSVG;
	CKerMethod *M;
	CKerParam *P;
	if (!o) RETERR
	NumObjectT = o->GetNumberOfKeys()+1;		// pocet objektu
	ObjectTypes = new CKerObjectT[NumObjectT];  // alokuju pole pro informaci o vsech typech o.
	
	DefaultObjectT = &ObjectTypes[0];			// sam vytvorim defaultni (prazdny objekt)
	DefaultObjectT->Name = KnownNames[eKKNdefaultObject].Name;
	DefaultObjectT->AName = DuplicateObjName(DefaultObjectT->Name);
	DefaultObjectT->Name->ObjectType = &ObjectTypes[0];
	_ko = o->GetFirstKey();
	for (f=1;f<NumObjectT;f++) {
		if(!(ko = _ko->GetSubRegister())) RETERR
		// nactu jmeno (z i te pozice)
		ObjectTypes[f].Name = KerInterpret->GetNamesArray()[ko->FindKey("Name")->readi()];
		ObjectTypes[f].AName = DuplicateObjName(ObjectTypes[f].Name);
		ObjectTypes[f].Name->Type=eKerNTobject;
		ObjectTypes[f].Name->ObjectType = &ObjectTypes[f];
		ObjectTypes[f].EditTag = ko->FindKey("Edit Tag")->readi(); // load edit tagu
		if (l=ko->FindKey("User Name")) ObjectTypes[f].Name->UserName = ObjectTypes[f].UserName = newstrdup(l->GetDirectAccess());
		if (l=ko->FindKey("Comment")) ObjectTypes[f].Name->Comment = ObjectTypes[f].Comment = newstrdup(l->GetDirectAccess());
		// nahraju informace o alokaci kompilovanych promennych
		if (l=ko->FindKey("KSVG")) {
			ksKSVG = (CKSKSVG*)(KSMain->KSVGs->Member(l->GetDirectAccess()));
			ObjectTypes[f].AllocKSVG = ksKSVG->AllocKSVG;
			ObjectTypes[f].SetObjectVar = ksKSVG->SetObjectVar;
			ObjectTypes[f].KSVGsize = ksKSVG->Size;
		}
		if (l=ko->FindKey("IS D Size")) ObjectTypes[f].ISDataSize=l->readi(); // interpretovanych

		// nacteni popsanych dat
		if (l=ko->FindKey("Data")) {
			_km = l->GetSubRegister()->GetFirstKey();
			NumOV = ObjectTypes[f].NumVars = l->GetSubRegister()->GetNumberOfKeys();
			ObjectTypes[f].OVars = new CKerOVar[NumOV];
			for (g=0;g<NumOV;g++) {
				if (!LoadOVar(_km,&(ObjectTypes[f].OVars[g]),&(ObjectTypes[f].SpecificKnownVars))) return 0;
				_km = _km->GetNextKey();
			}
			if (!CheckVarGroups(&ObjectTypes[f].OVars,NumOV)) RETERR
			ObjectTypes[f].FindAutoVars();
		}

		// metody
		if (l=ko->FindKey("Methods")) {
			_km = l->GetSubRegister()->GetFirstKey();
			NumM = ObjectTypes[f].NumM = l->GetSubRegister()->GetNumberOfKeys();
			ObjectTypes[f].Methods = new CKerMethod[NumM];
			for (g=0;g<NumM;g++) {
				PrmOfs = 0;
				km = _km->GetSubRegister();
				M = &(ObjectTypes[f].Methods[g]);
				M->Compiled = km->FindKey("Compiled")->readc();
				M->Safe = km->FindKey("Safe")->readc();
				M->MethodName = new char[strlen(_km->GetName())+1]; // nactu jmeno fce pro ladici ucely
				strcpy(M->MethodName,_km->GetName());
				if (M->Compiled) {
					M->Function = ((CKSKSM*)KSMain->KSMs->Member(_km->GetName()))->Fce;
				} else {
					M->Jump = km->FindKey("Jump")->readi();
				}
				if (l=km->FindKey("Parent Object")) M->ParentObj = KerMain->KerNamesMain->GetNamePointer(l->GetDirectAccess());
				if (M->Safe) {
					if (l=km->FindKey("Name")) {
						M->Name = KerInterpret->GetNamesArray()[l->readi()];
						M->Name->Type=eKerNTmethod;
					} else RETERR
					if (l=km->FindKey("ReturnType")) {
						M->ReturnType = l->readi();
						PrmOfs+=KerMain->GetTypeSize(M->ReturnType);
					}
					if (l=km->FindKey("Params")) {
						_kp = l->GetSubRegister()->GetFirstKey();
						NumP = M->NumP = l->GetSubRegister()->GetNumberOfKeys();
						M->Params = new CKerParam[NumP];
						for (h=0;h<NumP;h++) {
							kp = _kp->GetSubRegister();
							P = &(M->Params[h]);
							if (l=kp->FindKey("Name")) {
								P->Name = KerInterpret->GetNamesArray()[l->readi()];
								P->Name->Type=eKerNTparam;
							} else RETERR
							P->Type = kp->FindKey("Type")->readi();
							P->Offset = PrmOfs;
							PrmOfs += KerMain->GetTypeSize(P->Type);
							if (!LoadObjectsLD(kp->FindKey("Default"),&(P->DefaultValue),P->Type)) RETERR
							_kp = _kp->GetNextKey();
						}
					}
				} else { // direct:
					if (M->Compiled) if (l=km->FindKey("DirectName")) {
						*(((CKSKSDM*)KSMain->KSDMs->Member(l->GetDirectAccess()))->Fce) = M->Function;
					} else RETERR
				}
				M->ParamSize = PrmOfs;
				_km = _km->GetNextKey();
			}
		}
		
		_ko=_ko->GetNextKey();
	}
	return 1;
}


///////////////////////////////////////////////////////////////////////
/// Pomocna funkce pro nahrani defaultnich hodnot predavanych argumentu
int CKerObjs::LoadObjectsLD(CFSRegKey *l, CKerValue *P, int Type) {
	Type &= (eKTret-1);
	int a;
	if (l) {
		switch (Type) {
			case eKTchar:
				if (l->CFSGetKeyType() != FSRTchar) RETERR
				P->Dchar = l->readc();
				break;
			case eKTdouble:
				if (l->CFSGetKeyType() != FSRTdouble) RETERR
				P->Ddouble = l->readd();
				break;
			case eKTint:
			case eKTobject:
			case eKTpointer:
				if (l->CFSGetKeyType() != FSRTint) RETERR
				P->Dint = l->readi();
				break;
			case eKTname:
				if (l->CFSGetKeyType() != FSRTint) RETERR
				a = l->readi();
				if (a==-1) P->Dname = 0;
				else P->Dname = KerInterpret->GetNamesArray()[a];
				break;
			case eKTarrChar:
				if (l->CFSGetKeyType() != FSRTchar) RETERR
				if (l->eof()) P->Achar = 0; else {
					P->Achar = new CKerArrChar();
					l->pos++;
					while (!l->eof()) P->Achar->Add(l->readc());
				}
				break;
			case eKTarrDouble:
				if (l->CFSGetKeyType() != FSRTdouble) RETERR
				if (l->eof()) P->Adouble = 0; else {
					P->Adouble = new CKerArrDouble();
					l->pos++;
					while (!l->eof()) P->Adouble->Add(l->readd());
				}
				break;
			case eKTarrInt:
				if (l->CFSGetKeyType() != FSRTint) RETERR
				if (l->eof()) P->Aint = 0; else {
					P->Aint = new CKerArrInt();
					l->pos++;
					while (!l->eof()) P->Aint->Add(l->readi());
				}
				break;
			case eKTarrObject:
				if (l->CFSGetKeyType() != FSRTint) RETERR
				if (l->eof()) P->Aobject = 0; else {
					P->Aobject = new CKerArrObject();
					l->pos++;
					while (!l->eof()) P->Aobject->Add(l->readi());
				}
				break;
			case eKTarrPointer:
				assert(0);
				RETERR
			case eKTarrName:
				if (l->CFSGetKeyType() != FSRTint) RETERR;
				if (l->eof()) P->Aname = 0; else {
					P->Aname = new CKerArrName();
					l->pos++;
					while (!l->eof()) {
						a = l->readi();
						if (a==-1) P->Aname->Add(0); 
						else P->Aname->Add(KerInterpret->GetNamesArray()[a]);
					}
				}
				break;
			default:
				if (l->CFSGetKeyType() != FSRTstring) RETERR
				if (Type>=100&&Type<=350) {
					P->Dstring = new char[Type-100+2];
					strcpy(P->Dstring,l->GetDirectAccessFromPos());
					l->SetPosToNextString();
				} else RETERR
		}
		P->typ = Type;
	}
	return 1;
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
///
///			C K e r C a l c u l a t e P H T
///
/////////////////////////////////////////////////////////////////////////////

// Inicializace pomocnych promennych
CKerCalculatePHT::CKerCalculatePHT() {
	int f,g,pos;
	// metody Objektu
	NumO = KerMain->Objs->NumObjectT;
	os = KerMain->Objs->ObjectTypes;
	ostart = new int[NumO]; for (f=0;f<NumO;f++) ostart[f]=0;
	ostop = new int[NumO]; for (f=0;f<NumO;f++) ostop[f]=0;
	olist = new CKerMList*[NumO]; for (f=0;f<NumO;f++) olist[f]=0;
	omlist = new CKerMethod**[NumO];

	// parametry Metod
	NumM=0;
	for (f=0;f<NumO;f++) NumM += os[f].NumM; // spocitam metody
	ms = new CKerMethod*[NumM];
	pos=0;
	for (f=0;f<NumO;f++) {  // najdu vsechny metody, pointry na ne si dam k sobe do pole
		for (g=0;g<os[f].NumM;g++) ms[pos+g]= os[f].Methods+g;
		pos+=os[f].NumM;
	}

	mstart = new int[NumM]; for (f=0;f<NumM;f++) mstart[f]=0;
	mstop = new int[NumM]; for (f=0;f<NumM;f++) mstop[f]=0;
	mlist = new CKerPList*[NumM]; for (f=0;f<NumM;f++) mlist[f]=0;
	mplist = new CKerParam**[NumM];
}



/////////////////////////////////////////////////////////////////
// najdu na jake metody se jmeno pro dany objekt prevede. 
// Funkce vraci pole s pointry na metodu, zakoncene nulou
CKerMethod ** CKerCalculatePHT::CreateOMList(CKerName *name, CKerObjectT *o) {
	int f,g;
	CKerMethod **ret;
	int NumM=o->NumM;
	if (NumM<=0) return 0;
	int *stav = new int[o->NumM];
	for (f=0;f<NumM;f++) {
		if (o->Methods[f].Name&&(o->Methods[f].Name==name||name->Compare(o->Methods[f].Name)>=2)) stav[f]=1; else stav[f]=0;
	}
	for (f=0;f<NumM;f++) for (g=0;g<NumM;g++) {
		if (stav[f]&&stav[g]&&o->Methods[f].Name->Compare(o->Methods[g].Name)==1) stav[f]=2;
	}
	g=0;
	for (f=0;f<NumM;f++) if (stav[f]==1) g++;
	if (!g) {
		delete[] stav;
		return 0;
	}
	ret = new CKerMethod*[g+1];
	g=0;
	for (f=0;f<NumM;f++) if (stav[f]==1) {
		ret[g]=o->Methods+f;
		g++;
	}
	ret[g]=0;
	delete[] stav;
	return ret;
}



///////////////////////////////////////////////////////////////
// najdi kandidata na volnou pozici (zacina se od nuly (zadat), 
// kdyz se pozice po druhem volani nezmeni, je volna
void CKerCalculatePHT::find_pos_o(int &pos) {
	int f;
	CKerMList *ml;
	for (f=0;f<NumO;f++) { // objekty projdu jenom jednou, kdyz je nekde pozice obsazena, zvysim ji 
		ml = olist[f];
		if (omlist[f]&&ostart[f]<=pos&&ostart[f]+ostop[f]>pos) {
			while (ml&&ml->pos<pos) ml=ml->next;
			while (ml&&ml->pos==pos) { pos++; ml=ml->next;}
		}
	}
	// vysledna pozice nemusi sedet pro vsechny objekty, pokud se nekde zmenila
}



//////////////////////////////////////////////////////////////////////
// proda seznamy tohoto jmena na danou pozici (kazdy objekt siseznamy nyni pamatuje ve spojaku olist)
void CKerCalculatePHT::place_at_pos_o(int pos, CKerName *name) {
	int f;
	CKerMList **ml;
	for (f=0;f<NumO;f++) {
		ml = olist+f;
		if (omlist[f]) { // zda se musi neco pridavat
			while (*ml&&(*ml)->pos<pos) ml=&(*ml)->next; // najdu v seznamu misto, kam mam pridat
			*ml = new CKerMList(*ml,pos,omlist[f],name);
			if (ostop[f]==0) { // tvoreni noveho sezname
				ostart[f]=pos;
				ostop[f]=1;
			} else	if (pos<ostart[f]) { // pridani pred zacatek
				ostop[f]+=(ostart[f]-pos);
				ostart[f] = pos;
			} else if (pos>=ostart[f]+ostop[f]) ostop[f]=pos-ostart[f]+1; // pridani za konec
		}
	}
}



////////////////////////////////////////////////////
// provede vypocet PHT pro metody objektu
void CKerCalculatePHT::calc_o() {
	CKerNameList *name = KerMain->KerNamesMain->Names;
	int f;
	int pos, opos;
	while (name) {
		if(name->name->Type==eKerNTmethod) {  // pro kazde jmeno metody
			for (f=0;f<NumO;f++) omlist[f] = CreateOMList(name->name,os+f);  // spocitam k objektum seznamy
			pos = 0;  // hledam od pozice 0
			do {
				opos=pos;
				find_pos_o(pos);
			} while (pos!=opos);  // najdu prvni pozici, ktera je u vsech objektu volna (pozice se uz nemeni)
			place_at_pos_o(pos,name->name);  // aktualizuju spojaky
			name->name->KerPHTpos=pos;       // u jmena si zapamatuju pozici
		}
		name = name->next;
	}
}



////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////
// Nalezeni seznamu parametru, do kterych se dany parametr preda
// seznam je zakoncen Nulou
CKerParam ** CKerCalculatePHT::CreateMPList(CKerName *name, CKerMethod *m) {
	int f,g;
	CKerParam **ret;
	int NumP=m->NumP;
	if (NumP<=0) return 0;
	int *stav = new int[NumP];
	for (f=0;f<NumP;f++) {
		if (m->Params[f].Name==name||name->Compare(m->Params[f].Name)>=2) stav[f]=1; else stav[f]=0;
	}
	for (f=0;f<NumP;f++) for (g=0;g<NumP;g++) {
		if (stav[f]&&stav[g]&&m->Params[f].Name->Compare(m->Params[g].Name)==1) stav[f]=2;
	}
	g=0;
	for (f=0;f<NumP;f++) if (stav[f]==1) g++;
	if (!g) {
		delete[] stav;
		return 0;
	}
	ret = new CKerParam*[g+1];
	g=0;
	for (f=0;f<NumP;f++) if (stav[f]==1) {
		ret[g]=m->Params+f;
		g++;
	}
	ret[g]=0;
	delete[] stav;
	return ret;
}


////////////////////////////////////////////////////////////////////////
// pro parametry - na hledani pozice
void CKerCalculatePHT::find_pos_m(int &pos) {
	int f;
	CKerPList *ml;
	for (f=0;f<NumM;f++) {		// metody projdu jenom jednou, kdyz je nekde pozice obsazena, zvysim ji 
		ml = mlist[f];
		if (mplist[f]&&mstart[f]<=pos&&mstart[f]+mstop[f]>pos) {
			while (ml&&ml->pos<pos) ml=ml->next;
			while (ml&&ml->pos==pos) { pos++; ml=ml->next;}
		}
	}
	// vysledna pozice nemusi sedet pro vsechny objekty, pokud se nekde zmenila
}



///////////////////////////////////////////////////////////////////
// umisteni na pozici, zarazeni do spojaku
void CKerCalculatePHT::place_at_pos_m(int pos, CKerName *name) {
	int f;
	CKerPList **ml;
	for (f=0;f<NumM;f++) {
		ml = mlist+f;
		if (mplist[f]) {  // zda se musi neco pridavat
			while (*ml&&(*ml)->pos<pos) ml=&(*ml)->next;  // najdu pozici ve spojaku, kam mam pridavat
			*ml = new CKerPList(*ml,pos,mplist[f],name);
			if (mstop[f]==0) {		// spojak byl prazdny
				mstart[f]=pos;
				mstop[f]=1;
			} else	if (pos<mstart[f]) {  // pridavam pred zacatek
				mstop[f]+=(mstart[f]-pos);
				mstart[f] = pos;
			} else if (pos>=mstart[f]+mstop[f]) mstop[f]=pos-mstart[f]+1;  // pridavam za konec
		}
	}
}



///////////////////////////////////////////////////
// provede vypocet PHT pro parametry metod
void CKerCalculatePHT::calc_m() {
	CKerNameList *name = KerMain->KerNamesMain->Names;
	int f;
	int pos, opos;
	while (name) {
		if(name->name->Type==eKerNTparam) {  // pro kazde jmeno parametru
			for (f=0;f<NumM;f++) mplist[f] = CreateMPList(name->name,ms[f]);  // k metodam vytvorim prislusne seznamy
			pos = 0;		// hledam volnou pozici (od 0)
			do {
				opos=pos;
				find_pos_m(pos);
			} while (pos!=opos);  // najdu prvni pozici, ktera je u vsech metod volna (to je kdyz se pozice prestane menit)
			place_at_pos_m(pos,name->name);		// aktualizuju spojaky
			name->name->KerPHTpos=pos;
		}
		name = name->next;
	}
}


//////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
///		Prevedeni spojaku na pole. Ulozeni tecto poli, startu a endu 
///		k objektum a metodam, uvolneni pomocnych promennych
CKerCalculatePHT::~CKerCalculatePHT() {
	int f,g;
	CKerMList *ml, *ml2;
	CKerPList *pl, *pl2;
	// pro vsechny objekty
	for (f=0;f<NumO;f++) {
		if (ostop[f]>0) {
			os[f].MethodCL = new CKerMethod**[ostop[f]];	// tvorim pole se seznamy
			os[f].MCLnames = new CKerName*[ostop[f]];		// tvorim pole se jmeny prislusejicim seznamu
			ml = olist[f];
			for (g=0;g<ostop[f];g++) {				// okopiruju a znicim spojak
				if(ml&&ml->pos==ostart[f]+g) {
					os[f].MethodCL[g] = ml->methods;
					os[f].MCLnames[g] = ml->name;
					ml2=ml;
					ml=ml->next;
					delete ml2;
				} else {
					os[f].MethodCL[g]=0;
					os[f].MCLnames[g]=0;
				}
			}
			os[f].MCLstart = ostart[f];
			os[f].MCLstop = ostop[f];
		} else {  // spojak byl prazdny
			os[f].MCLstart = 0;
			os[f].MCLstop = 0;
		};
	}
	SAFE_DELETE_ARRAY(ostart);
	SAFE_DELETE_ARRAY(ostop);
	SAFE_DELETE_ARRAY(olist);
	SAFE_DELETE_ARRAY(omlist);

	// pro vsechny metody
	for (f=0;f<NumM;f++) {
		if (mstop[f]>0) {
			ms[f]->ParamCL = new CKerParam**[mstop[f]];		// tvorim pole se seznamy
			ms[f]->PCLnames = new CKerName*[mstop[f]];		// tvorim pole s prislusejicimi jmeny
			pl = mlist[f];
			for (g=0;g<mstop[f];g++) {			// okopiruju spojak do pole a znicim ho
				if(pl&&pl->pos==mstart[f]+g) {
					ms[f]->ParamCL[g] = pl->params;
					ms[f]->PCLnames[g] = pl->name;
					pl2=pl;
					pl=pl->next;
					delete pl2;
				} else {
					ms[f]->ParamCL[g]=0;
					ms[f]->PCLnames[g]=0;
				}
			}
			ms[f]->PCLstart = mstart[f];
			ms[f]->PCLstop = mstop[f];
		} else {	// spojak byl prazdny
			ms[f]->PCLstart = 0;
			ms[f]->PCLstop = 0;
		}
	}
	SAFE_DELETE_ARRAY(mstart);
	SAFE_DELETE_ARRAY(mstop);
	SAFE_DELETE_ARRAY(mlist);
	SAFE_DELETE_ARRAY(mplist);
	SAFE_DELETE_ARRAY(ms);
}



void CKerObjs::KernelDump(char *file) {
	int size = KER_INTERPRET_MEMORY_SIZE+4;
	int size2;
	int f,g;
	char buff3[50000];
	int pos3=0;
	CKerObject *obj;
	for (f=0; f<OBJECT_HT_SIZE; f++) {
		obj = HT[f];
		while (obj) {
			size+=obj->Type->KSVGsize;
			obj = obj->next;
		}
	}
	FS->ChangeDir("$DATA$");
	char buff4[1000];
	sprintf(buff4,"%s.dump",file);
	size2 = FS->GetFileSize(buff4);
	int diff;

	char *buff = new char[size];
	char *buff2;
	if (size==size2) FS->ReadFile(buff4,buff,size);
	diff = *(UC**)buff - KerInterpret->GetMemory();
	*(UC**)buff = KerInterpret->GetMemory();
	
	int pos = 4;
	buff2=(char*)KerInterpret->GetMemory();
	for (g=0; g<KER_INTERPRET_MEMORY_SIZE; g++,pos++) {
		if (g+3<KER_INTERPRET_MEMORY_SIZE && *(int*)(buff+pos) == diff + *(int*)(buff2+g)) {
			*(int*)(buff+pos) = *(int*)(buff2+g);
			g+=3; pos+=3;
		} else {
			if (size==size2 && buff[pos] != buff2[g]) {
				pos3 += sprintf(buff3+pos3,"Memory %i, Diff: %i\n",g,buff[pos]-buff2[g]);
				if (pos3>45000) size2=0;
			}
			buff[pos] = buff2[g];
		}
	}
	for (f=0; f<OBJECT_HT_SIZE; f++) {
		obj = HT[f];
		while (obj) {
			buff2 = (char*)obj->KSVG;
			for (g=0; g<obj->Type->KSVGsize; g++,pos++) {
				if (g+3<obj->Type->KSVGsize && *(int*)(buff+pos) == diff + *(int*)(buff2+g)) {
					*(int*)(buff+pos) = *(int*)(buff2+g);
					g+=3; pos+=3;
				} else {
					if (size==size2 && buff[pos] != buff2[g]) {
						pos3 += sprintf(buff3+pos3,"Object %i, T: %s Offs: %i, Diff: %i\n",obj->thisO,obj->Type->Name->GetNameString(),g,buff[pos]-buff2[g]);
						if (pos3>45000) size2=0;
					}
					buff[pos] = buff2[g];
				}
			}
			obj = obj->next;
		}
	}
	if (!pos3) {
		if (size==size2) pos3 += sprintf(buff3+pos3,"Not Changed!!");
		else pos3 += sprintf(buff3+pos3,"size changed (different level)");
	}

	FS->WriteFile(buff4,buff,size);

	sprintf(buff4,"%s.log",file);
	if (pos3) FS->WriteFile(buff4,buff3,pos3,0);

	delete[] buff;
}


