////////////////////////////////////////////////////////////////////////////////
///
///		I N T E R P R E T   interpretovanych scriptu
///
///		A: Jiri Margaritov (Interpretovani), Honza M.D. Krcek (Loading)
///
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "interpret.h"
#include "ks.h"
#include "kernel.h"
#include "KerServices.h"
#include "register.h"
#include "names.h"
#include "KerArray.h"
#include "ComKerServices.h"
#include "ComPart.h"

#include "GameMenu.h"
#include "scriptInterface.h"
#include "genmain.h"
#include "krkal.h"
#include "music.h"

#define HEAPTAG1 0xCECC5AA6
#define HEAPTAG2 0x663CABC5


// Glob. promenna pro praci s interpretem
CKerInterpret *KerInterpret=0;


////////////////////////////////////////////////////////////////////////////
///
///		C K e r I n t e r p r e t
///
////////////////////////////////////////////////////////////////////////////


/// INICIALIZACE
CKerInterpret::CKerInterpret() {
	if (KerInterpret) throw  CExc(eKernel,0,"Error - Pokus Vytvorit Objekt KerInterpret dvakrat!");
	KerInterpret=this;
	// inicializace pameti a ukazatelu do ni:
	memory=0;
	memory = new unsigned char[KER_INTERPRET_MEMORY_SIZE];
	CodeStart=memory+0;
	KerNamesStart=memory+0;
	GlobalsStart=memory+0;
	StackStart=memory+0;
	StackTop=memory+0;
	HeapBottom=memory+KER_INTERPRET_MEMORY_SIZE;
	HFstart = 0; HFend = 0;
	new CKerHeapFree(StackTop,HeapBottom,0,0);

// J.M.:	 inicializace poli pro predavani argumentu kernelu (viz napr. HardJump, instrukce MSG)
	argTypes = new int[MAX_SAFE_CALL_PARAMS];		// typy
	assert(argTypes);
	argNames = new CKerName*[MAX_SAFE_CALL_PARAMS];	// jmena
	assert(argNames);
	argPointers = new void*[MAX_SAFE_CALL_PARAMS];	// hodnoty
	assert(argPointers);

	IL = KER_INTERPRET_DEFAULT_IL;	// nastavim defaultni hodnotu pro limit max. poctu zpracovanych instrukci (ochrana proti zacykleni)

#ifdef JIRKA_KING_OF_COMPILERS		// logovani volanych metod do souboru - pozor, vyrazne zpomaluje interpret
	FS->ChangeDir("$COMPFILES$");
	FILE* f = fopen("last.dbg", "w");	// vymazani logu pri startu interpretu
	fclose(f);
#endif
}




/////////////////////////////////////////////////////////////////////////////////
///		LOADING

// Load Kodu:
void CKerInterpret::LoadCode(CFSRegKey *code) {
	int size = code->top;
	if (StackTop+size>=HeapBottom) KerMain->Errors->LogError(eKRTENoMemory,0,"code");
	memcpy(StackTop,code->GetDirectAccess(),size);
	CodeStart=StackTop;
	StackTop+=size;
}


// Load jmen, Vytvoreni jmen, umisteni pointru na jmena na zasobnik  (CKerName*)
// Dodani znamych jmen (jmena konstruktoru...)
int CKerInterpret::LoadNames(int num, CFSRegKey *r) {
	int f;
	KerNamesStart = StackTop;
	r->pos=r->top-1; if (r->readc()!=0) return 0;
	r->pos=0;
	if (StackTop + num*4 + MAXKNOWNNAMES*4 >=HeapBottom) KerMain->Errors->LogError(eKRTENoMemory,0,"names");

	// Nacteni jmen
	for (f=0;f<num;f++) {
		if (r->eof()) return 0;
		CKerName* tmp = new CKerName(r->GetDirectAccessFromPos(),KerMain->KerNamesMain);
		*((CKerName**)(StackTop)) = tmp;
		StackTop+=4;
		r->SetPosToNextString();
	}

	// Vyhledani a doplneni preddefinovanych jmen
	for (f=0; f < MAXKNOWNNAMES; f++) {
		if (!(KnownNames[f].Name = KerMain->KerNamesMain->GetNamePointer(KnownNames[f].namestr))) {
			KnownNames[f].Name = *((CKerName**)(StackTop)) = new CKerName(KnownNames[f].namestr,KerMain->KerNamesMain);
			StackTop+=4;
		}
		KnownNames[f].Name->Type = KnownNames[f].type;
	}

	return 1;
}


////////////////////////////////////////////////
/// najde umisteni pro globalni promenne a vyplni tabulku pointeru na ne
int CKerInterpret::LoadGlobals(CFSRegKey *r) {
	CKSKSG *ksg;
	CFSRegister *rr;
	GlobalsStart = StackTop;
	KerMain->NumGV = 0;
	unsigned char *st2;
	if (!r) return 1;
	if (!(rr=r->GetSubRegister())) return 0;
	int numk = rr->GetNumberOfKeys();
	int ff=0;
	st2 = StackTop+4*numk; // <- oblas, kam budu na zasobniku umistovat glob. prom.
	if (st2 + numk*256 >= HeapBottom) KerMain->Errors->LogError(eKRTENoMemory,0,"globals");
	KerMain->NumGV = numk;
	KerMain->GlobalVar = new CKerOVar[numk]; // inicializuju pole s promennyma u kernelu

	r=rr->GetFirstKey();
	while (r) {
		KerMain->Objs->LoadOVar(r,KerMain->GlobalVar+ff,&(KerMain->SpecificKnownVars),1);
		KerMain->GlobalVar[ff].KSVar = (void **)StackTop;
		*((void**)(StackTop)) = st2; // nastavim pointer
		if (ksg=(CKSKSG*)KSMain->KSGs->Member(r->GetName()))  *ksg->Variable = st2;
		st2 += KerMain->GetTypeSize(KerMain->GlobalVar[ff].Type)*KerMain->GlobalVar[ff].ArraySize;
		if (st2 >= HeapBottom) KerMain->Errors->LogError(eKRTENoMemory,0,"globals");
		StackTop+=4;
		r=r->GetNextKey();
		ff++;
	}
	StackTop=st2;

	// Nastavim pointery na navratove hodnoty z KerArrayu v pripade chyby:
	KRValueCKerArrChar = (char*)StackTop;  StackTop += 4;
	KRValueCKerArrDouble = (double*)StackTop;  StackTop += 8;
	KRValueCKerArrPointer = (void**)StackTop;  StackTop += 4;
	KRValueCKerArrInt = (int*)StackTop;  StackTop += 4;
	KRValueCKerArrObject = (OPointer*)StackTop;  StackTop += 4;
	KRValueCKerArrName = (CKerName**)StackTop;  StackTop += 4;
	if (StackTop >= HeapBottom) KerMain->Errors->LogError(eKRTENoMemory,0,"globals");


	return 1;
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////
// Inicializace interpretu. Inicializace zasobniku a haldy
void CKerInterpret::Init_SetStack() { 
	StackStart = StackTop; 
} 


///											/////	
///		P R A C E   S   H A L D O U
///											/////

// MALLOC allokuje pamet na halde
UC *CKerInterpret::ISMalloc(unsigned int size) {
	CKerHeapFree *h = HFstart;
	UI hsize;
	UC *addr;

	size = ((size+12-1)&0xFFFFFFF0)+16; // 12 je na pomocna data (3 inty), adresu dale zvetsim na nejblizsi nasobek 16
	HFend->loweraddr = StackTop;  // Aktualizuju velikost posledniho volneho mista
	while (h && size > (hsize = h->upperaddr-h->loweraddr)) h = h->next; // najdu prvni volne misto, do ktereho se vejdu
	if (!h) { KerMain->Errors->LogError(eKRTENoMemory,size,"malloc"); return 0;}
	addr = h->upperaddr-size;  // adresa, kam umistim vytvareny blok
	if (h->next==0) { // Zvetsuji oblast haldy
		if (size+128 > hsize) { KerMain->Errors->LogError(eKRTENoMemory,size,"malloc"); return 0;}
		HeapBottom = addr;
	}
	((UI*)addr)[0] = HEAPTAG1;  // ulozim tagy a velikost vytvarene oblasti
	((UI*)addr)[1] = size;
	((UI*)addr)[2] = HEAPTAG2;
	if (size == hsize) delete h;  // volne misto jsem zaplnic cele.
	else h->upperaddr -= size;
	return addr+12;
}


//// FREE
void CKerInterpret::ISFree(void *ptr) {
	UC *ptrUp;
	UC *pointer = (UC*)ptr;
	CKerHeapFree *h = HFstart;

	if (!pointer) { KerMain->Errors->LogError(eKRTEFreeToNull); return; } // pointer je NULL
	if (pointer<StackTop || pointer >= memory+KER_INTERPRET_MEMORY_SIZE) { KerMain->Errors->LogError(eKRTEIllegalFree,(int)pointer); return; } // Pointer je mimo pamet
	pointer -=12;
	if (((UI*)pointer)[0] != HEAPTAG1 || ((UI*)pointer)[2] != HEAPTAG2) { KerMain->Errors->LogError(eKRTEIllegalFree,(int)pointer+12); return; }  // Nejsou tam tagy
	while (h && h->upperaddr>pointer) h = h->next; // najdu prvni volne misto pod dealokovanou oblasti
	if (!h) { KerMain->Errors->LogError(eKRTEIllegalFree,(int)pointer+12,"? wierd.."); return; }
	// zjistim velikost a smazu tagy:
	((UI*)pointer)[0]=0; ptrUp = pointer+((UI*)pointer)[1]; ((UI*)pointer)[2]=0;
	// rozsirim volne misto:
	if (h->upperaddr==pointer) {
		if (h->prev && h->prev->loweraddr == ptrUp) {
			h->upperaddr = h->prev->upperaddr;
			delete h->prev;
		} else h->upperaddr = ptrUp;
		HeapBottom = HFend->upperaddr;
	} else {
		if (h->prev && h->prev->loweraddr == ptrUp) h->prev->loweraddr = pointer;
		else new CKerHeapFree(pointer,ptrUp,h,h->prev);
	}
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// DESTRUKTOR
CKerInterpret::~CKerInterpret() {
	while (HFstart) delete HFstart;
	SAFE_DELETE_ARRAY(memory);
	KerInterpret=0;

// J.M.: zruseni poli pro predavani argumentu
	SAFE_DELETE_ARRAY(argTypes);
	SAFE_DELETE_ARRAY(argNames);
	SAFE_DELETE_ARRAY(argPointers);
}

void CKerInterpret::ResetMachine() 
{
	// tohle budu predpokladat:
	assert(sizeof(unsigned char) == 1);		
	assert(sizeof(int) == 4);
	assert(sizeof(UIntInstruction) == 4);
	assert(sizeof(CIntInstructionRec) == 16);

	// nastavim nektere casto vyuzivane hodnoty do spec. atributu - kvuli rychlosti
	CB = (CIntInstructionRec*)CodeStart;
	CL = (CIntInstructionRec*)(KerNamesStart-cInstructionSize);
	ML = memory + KER_INTERPRET_MEMORY_SIZE;

	IC = 0;		// jeste jsem nic nezpracoval
	IP = -1;	// zadna instrukce neni aktualni
	instructionsNr = CL-CB;		

	// same nuly, jeste nic nezpracovavam...
	OB = PB = SB = 0;	
	instr = 0;
	tab = 0;
}

void CKerInterpret::StartMethod(CKerObject* object, CKerMethod* method)
{
	UC *oldPB, *oldOB, *oldSB, *oldST;
	int oldIC, oldIP;
	CIntInstructionRec* oldInstr;

	// parametry mam nastaveny od kernelu -> zaznam je pod nimi a nebo vubec ne (konstruktor)

	// Ulozim stary kontext:
	oldPB = PB;
	oldOB = OB;
	oldSB = SB;
	oldST = StackTop;
	oldIC = IC;
	oldIP = IP;
	oldInstr = instr;

	// Nastavim novy kontext:
	PB = KerContext->Params + KerMain->GetTypeSize(method->ReturnType);
	OB = object->ISData;
	SB = StackTop;
	IC = 0;

//	MyDebugTab("last.dbg", tab); MyDebug("last.dbg", method->MethodName, method->Jump, KerMain->GetTime());
//	tab++;
	HardJump(method->Jump);		// Zacnu samotnou interpretaci...
//	tab--;
//	MyDebugTab("last.dbg", tab); MyDebug("last.dbg", "OK", method->Jump);

	// Zapisu na zasobnik navratovou hodnotu:
	Load(0);
	switch(method->ReturnType)
	{
	case eKTchar:			*(KerContext->Params) = op[0].intData;				break;
	case eKTdouble:			*((double*)KerContext->Params) = op[0].doubleData;	break;
	case eKTvoid:			break;
	default:
		*((int*)KerContext->Params) = op[0].intData;
	}

	// Obnovim stary kontext:
	PB = oldPB;
	OB = oldOB;
	SB = oldSB;
	StackTop = oldST;
	IC = oldIC;
	IP = oldIP;
	instr = oldInstr;
}


void CKerInterpret::HardJump(int addr)
{
	int i, j, k;
	double d;
	void* vp;
	UC* cp;
	CKerName* n;
	UComValues val;
	CIntInstructionRec *instr2;

    DoJump(addr);		// skocim na prvni instrukci

	while(true)			// a ted porad dokola:
	{	
		if(IP*16 == 311696)
			j = j;
		switch(instr->instr.opCode)		// dekoduji instrukci
		{
		case instrDBG:		
			break;

		case instrJMP:  
			DoJump(instr->res.intValue);
			continue;

		case instrJMPZ:  
			Load(1);
			if(op[1].intData == 0)
			{
				if((i = instr->instr.byteValue[3]) != eOpTpMissing)		// i == typ pro ulozeni vysledku podminky
				{
					InitOp(i, instr->op2.intValue, 0);		
					Store(op[1].intData, i, instr->op2.intValue);
				}
				DoJump(instr->res.intValue);	// podminka uspela -> skacu
				continue;
			} else
				break;							// podminka neuspela -> dalsi instrukce

		case instrJMPNZ:  
			Load(1);
			if(op[1].intData != 0)
			{
				if((i = instr->instr.byteValue[3]) != eOpTpMissing)		// i == typ pro ulozeni vysledku podminky
				{
					InitOp(i, instr->op2.intValue, 0);		
					Store(op[1].intData, i, instr->op2.intValue);
				}
				DoJump(instr->res.intValue);	// podminka uspela -> skacu
				continue;
			} else
				break;							// podminka neuspela -> dalsi instrukce

		case instrRSS:  
			StackTop += instr->res.intValue;	// vyhradim misto na zasobniku pro lokalni promenne a mezivysledky
			CheckOverflow();
			SB += instr->op1.intValue;
			break;

		case instrDRET:		// navrat z direct volani
			Load(0);

			cp = PB - cMethodCallRecordSize;
			RestoreFromCallRec(cp);					// obnovim stav pred volanim

			i = ((int*)cp)[3];						// typ operandu pro navrat hodnoty
			j = ((int*)cp)[4];						// ofset operandu pro navrat hodnoty

			if(isDouble[0] = isDOUBLE(opTp[0]))		// zapisu navratovou hodnotu
				Store(op[0].doubleData, (enmOpType)i, j);
			else
				Store(op[0].intData, (enmOpType)i, j);
			
//			tab--;
//			MyDebugTab("last.dbg", tab); MyDebug("last.dbg", "DRET: ", instr2->op2.intValue, KerMain->GetTime());

			if((i = ((int*) cp)[5]) < 0)			// neni se kam vratit -> koncim
				return;

			DoJump(i);		// skok na navratovou adresu
			continue;

		case instrSRET:  
			return;			// navrat ze safe metody - koncim interpretaci

		case instrSWITCH: 
			Load(1);					// podminka
			i = op[1].intData;			// - zkopiruju si ji do lokalni promenne
			
			j = 0;
			k = (instr + instr->op2.intValue)->instr.intValue;		// adresa default navesti
			while(j < instr->res.intValue)							// cyklus podle poctu navesti
			{
				int x;
				if((x = (instr + 1 + j/2)->GetPart(j%2)) == i)				// testuju podminku navesti
				{ 
					k = (instr + 1 + j/2)->GetPart(j%2 + 2);			// pri shode mam adresu skoku
					break;
				}
				j++;
			}
			DoJump(k);		// skok na vybrane navesti
			continue;

		case instrMOV:  
			Load1();		
			if(isDouble[0])	
				if(isDouble[1])	Store(op[1].doubleData); else Store(op[1].intData);
			else
				Store(op[1].intData);
			break;

		case instrADD:  
			Load2();
			if(isDouble[0])
			{
				if(isDouble[1])	op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2])	op[0].doubleData += op[2].doubleData; else op[0].doubleData += op[2].intData;
				Store(op[0].doubleData);
			} else
				Store(op[1].intData + op[2].intData);
			break;

		case instrSUB:  
			Load2();
			if(isDouble[0])
			{
				if(isDouble[1])	op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2])	op[0].doubleData -= op[2].doubleData; else op[0].doubleData -= op[2].intData;
				Store(op[0].doubleData);
			} else
				Store(op[1].intData - op[2].intData);
			break;

		case instrMUL:  
			Load2();
			if(isDouble[0])
			{
				if(isDouble[1])	op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2])	op[0].doubleData *= op[2].doubleData; else op[0].doubleData *= op[2].intData;
				Store(op[0].doubleData);
			} else
				Store(op[1].intData * op[2].intData);
			break;

		case instrDIV:  
			Load2();
			if(isDouble[0])
			{
				if(isDouble[1])	op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2])	op[0].doubleData /= op[2].doubleData; else op[0].doubleData /= op[2].intData;
				Store(op[0].doubleData);
			} else
				Store(op[1].intData / op[2].intData);
			break;

		case instrMOD:  
			Load2();
			Store(op[1].intData % op[2].intData);
			break;

		case instrSHR:  
			Load2();
			Store(op[1].intData >> op[2].intData);
			break;

		case instrSHL:  
			Load2();
			Store(op[1].intData << op[2].intData);
			break;

		case instrAND:  
			Load2();
			Store(op[1].intData & op[2].intData);
			break;

		case instrOR:  
			Load2();
			Store(op[1].intData | op[2].intData);
			break;

		case instrXOR:  
			Load2();
			Store(op[1].intData ^ op[2].intData);
			break;

		case instrLAND:  
			Load2();
			if(isDouble[1])	op[1].intData = op[1].doubleData != 0; else op[1].intData = op[1].intData != 0;
			if(isDouble[2])	op[2].intData = op[2].doubleData != 0; else op[2].intData = op[2].intData != 0;
			Store(op[1].intData && op[2].intData);
			break;

		case instrLOR:  
			Load2();
			if(isDouble[1])	op[1].intData = op[1].doubleData != 0; else op[1].intData = op[1].intData != 0;
			if(isDouble[2])	op[2].intData = op[2].doubleData != 0; else op[2].intData = op[2].intData != 0;
			Store(op[1].intData || op[2].intData);
			break;

		case instrNEG:  
			Load1();
			Store(~(op[1].intData));
			break;

		case instrNOT:  
			Load1();
			if(isDouble[1])	op[0].intData = op[1].doubleData == 0; else op[0].intData = op[1].intData == 0;
			Store(op[0].intData);
			break;

		case instrINC:  
			Load1();
			if(isDouble[0])
			{
				if(isDouble[1])	op[0].doubleData = op[1].doubleData+1; else op[0].doubleData = op[1].intData+1;
				Store(op[0].doubleData);
			} else
				Store(op[1].intData + 1);
			break;

		case instrDEC:  
			Load1();
			if(isDouble[0])
			{
				if(isDouble[1])	op[0].doubleData = op[1].doubleData-1; else op[0].doubleData = op[1].intData-1;
				Store(op[0].doubleData);
			} else
				Store(op[1].intData - 1);
			break;

		case instrTESTG:  
			Load2();
			if(isDouble[1] || isDouble[2])
			{
				if(isDouble[1]) op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2]) op[0].doubleData -= op[2].doubleData; else op[0].doubleData -= op[2].intData;
				Store(op[0].doubleData > 0);
			} else
				Store(op[1].intData > op[2].intData);
			break;

		case instrTESTL:  
			Load2();
			if(isDouble[1] || isDouble[2])
			{
				if(isDouble[1]) op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2]) op[0].doubleData -= op[2].doubleData; else op[0].doubleData -= op[2].intData;
				Store(op[0].doubleData < 0);
			} else
				Store(op[1].intData < op[2].intData);
			break;

		case instrTESTGE:  
			Load2();
			if(isDouble[1] || isDouble[2])
			{
				if(isDouble[1]) op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2]) op[0].doubleData -= op[2].doubleData; else op[0].doubleData -= op[2].intData;
				Store(op[0].doubleData >= 0);
			} else
				Store(op[1].intData >= op[2].intData);
			break;

		case instrTESTLE:  
			Load2();
			if(isDouble[1] || isDouble[2])
			{
				if(isDouble[1]) op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2]) op[0].doubleData -= op[2].doubleData; else op[0].doubleData -= op[2].intData;
				Store(op[0].doubleData <= 0);
			} else
				Store(op[1].intData <= op[2].intData);
			break;

		case instrTESTE:  
			Load2();
			if(isDouble[1] || isDouble[2])
			{
				if(isDouble[1]) op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2]) op[0].doubleData -= op[2].doubleData; else op[0].doubleData -= op[2].intData;
				Store(op[0].doubleData == 0);
			} else
				Store(op[1].intData == op[2].intData);
			break;

		case instrTESTNE:  
			Load2();
			if(isDouble[1] || isDouble[2])
			{
				if(isDouble[1]) op[0].doubleData = op[1].doubleData; else op[0].doubleData = op[1].intData;
				if(isDouble[2]) op[0].doubleData -= op[2].doubleData; else op[0].doubleData -= op[2].intData;
				Store(op[0].doubleData != 0);
			} else
				Store(op[1].intData != op[2].intData);
			break;

		case instrNCMP:  
			Load2();
			Store(KerServices.IsFMemberOfS((CKerName*)op[1].intData, (CKerName*)op[2].intData));
			break;

		case instrNCMPEX:  
			Load2();
			Store(KerServices.IsFMemberOfSExcl((CKerName*)op[1].intData, (CKerName*)op[2].intData));
			break;

		case instrASGND:  
			Load1();
			Store(KerContext->ParamsInfo[op[1].intData]);
			break;

		case instrTYPEOF:  
			Load1();
			i = (int)KerMain->GetObjType(op[1].intData);
			Store(i);
			break;

		case instrADDR:  
			TouchRes();						// oba operandy jsou vzdy prime -> nenahravam
			switch(instr->op1.intValue)		// typ adresace
			{
			case eOpTpStackTmp:			j = (int)(SB + instr->op2.intValue);					break;
			case eOpTpLocal:			j = (int)(PB + instr->op2.intValue);					break;
			case eOpTpAttribute:		j = (int)(OB + instr->op2.intValue);					break;
			case eOpTpGlobal:			j = ((int*)GlobalsStart)[instr->op2.intValue];			break;
			default:
				KerMain->Errors->LogError(eKRTECorruptedInstr);
			}
			Store(j);
			break;

		case instrPEEK:  
			LoadPointer(0);					// cilovy pointer
			Load(1);						// zdrojovy pointer
			i = instr->op2.intValue;		// velikost cteneho bloku

			CheckPointerForRead((UC*)op[1].voidPtrData, i);
			CheckPointerForWrite((UC*)op[0].voidPtrData, i);
			memcpy(op[0].voidPtrData, op[1].voidPtrData, i);
			break;

		case instrAGETCNT:  
			Load1();						// op[2] vzdy primy -> nenahravam
			vp = op[1].voidPtrData;
			switch(instr->op2.intValue)
			{
			case kwIntArray:		i = ((CKerArrInt*)vp)->GetCount();		break;
			case kwCharArray:		i = ((CKerArrChar*)vp)->GetCount();		break;
			case kwDoubleArray:		i = ((CKerArrDouble*)vp)->GetCount();	break;
			case kwNameArray:		i = ((CKerArrName*)vp)->GetCount();		break;
			case kwObjptrArray:		i = ((CKerArrObject*)vp)->GetCount();	break;
			case kwVoidArray:		i = ((CKerArrPointer*)vp)->GetCount();	break;
			default:
				KerMain->Errors->LogError(eKRTECorruptedInstr);
			}
			Store(i);
			break;

		case instrNEW:			
			Load1();				// op[2] - vzdy primy -> nenahravam
			vp = ISMalloc(instr->op2.intValue, op[1].intData);
			Store((int)vp);
			break;

		case instrNEWO:  
			Load1();				// op[2] - vzdy primy -> nenahravam
			n = ((CKerName**)KerNamesStart)[instr->op1.intValue];
			i = KerMain->NewObject(instr->op2.intValue, n);
			Store(i, instr->instr.byteValue[1], instr->res.intValue);
			break;

		case instrNEWOV:  
			Load1();				// op[2] - vzdy primy -> nenahravam
			n = (CKerName*)op[1].voidPtrData;
			i = KerMain->NewObject(instr->op2.intValue, n);
			Store(i, instr->instr.byteValue[1], instr->res.intValue);
			break;

		case instrNEWA:  
			TouchRes();				// oba operandy vzdy prime -> nenahravam
			switch(instr->op1.intValue)
			{
			case kwIntArray:		vp = new CKerArrInt();		break;
			case kwCharArray:		vp = new CKerArrChar();		break;
			case kwDoubleArray:		vp = new CKerArrDouble();	break;
			case kwNameArray:		vp = new CKerArrName();		break;
			case kwObjptrArray:		vp = new CKerArrObject();	break;
			case kwVoidArray:		vp = new CKerArrPointer();	break;
			default:
				KerMain->Errors->LogError(eKRTECorruptedInstr);
			}
			Store((int)vp);
			break;

		case instrTHIS:  
			TouchRes();
			Store((int)KerContext->KCthis);
			break;

		case instrSENDER:  
			TouchRes();
			Store((int)KerContext->Sender);
			break;

		case instrNAME:  
			TouchRes();
			n = ((CKerName**)KerNamesStart)[instr->op1.intValue];
			Store((int)n);
			break;

		case instrPOKE: 
			if(instr->instr.byteValue[2] == eOpTpImmediate)
			{
				Load(0);
				Load(1);				// op[2] vzdy primy -> nenahravam
				cp = (UC*)op[0].voidPtrData;

				switch(instr->op2.intValue)
				{
				case 1:		MemoryWriteC(cp, i = op[1].intData);			Store(i);		break;
				case 4:		MemoryWriteI(cp, i = op[1].intData);			Store(i);		break;
				case 8:		MemoryWriteD(cp, d = op[1].doubleData);			Store(d);		break;
				default:
					i = i;
					assert(false);
				}
			} else
			{
				Load(0);					// cilovy pointer
				LoadPointer(1);					// zdrojovy pointer
				i = instr->op2.intValue;		// velikost cteneho bloku

				CheckPointerForRead((UC*)op[1].voidPtrData, i);
				CheckPointerForWrite((UC*)op[0].voidPtrData, i);
				memcpy(op[0].voidPtrData, op[1].voidPtrData, i);
				break;

			}
			break;

		case instrPUTDC:  
			TouchRes();			// oba operandy prime -> nenahravam je
			val.intArray[0] = instr->op1.intValue;
			val.intArray[1] = instr->op2.intValue;
			Store(val.doubleData);
			break;

		case instrPUTSC:  
			LoadPointer(0);		// oba operandy prime -> nenahravam je
			cp = (UC*)op[0].voidPtrData;
			i = instr->op1.intValue;
			strncpy((char*)cp, (char*)(instr+1), i);
			cp[i] = '\0';
			Skip(instr->op2.intValue);
			break;

		case instrDEL:  
			Load(0);
			vp = op[0].voidPtrData;
			ISFree(instr->op1.intValue, vp);
			break;

		case instrDELO:  
			Load(0);
			KerMain->DeleteObject(instr->op1.intValue, op[0].intData);
			break;

		case instrDELA: 
			Load(0);
			vp = op[0].voidPtrData;
			switch(instr->op1.intValue)
			{
			case kwIntArray:		delete (CKerArrInt*)	vp;		break;
			case kwCharArray:		delete (CKerArrChar*)	vp;		break;
			case kwDoubleArray:		delete (CKerArrDouble*)	vp;		break;
			case kwNameArray:		delete (CKerArrName*)	vp;		break;
			case kwObjptrArray:		delete (CKerArrObject*)	vp;		break;
			case kwVoidArray:		delete (CKerArrPointer*)vp;		break;
			default:
				KerMain->Errors->LogError(eKRTECorruptedInstr);
			}
			break;

		case instrAIGET:  
			Load2();
			vp = &(((CKerArrInt*)op[1].voidPtrData)->Get(op[2].intData));
			Store((int)vp);
			break;

		case instrACGET:  
			Load2();
			vp = &(((CKerArrChar*)op[1].voidPtrData)->Get(op[2].intData));
			Store((int)vp);
			break;

		case instrADGET:  
			Load2();
			vp = &(((CKerArrDouble*)op[1].voidPtrData)->Get(op[2].intData));
			Store((int)vp);
			break;

		case instrAOGET:  
			Load2();
			vp = &(((CKerArrObject*)op[1].voidPtrData)->Get(op[2].intData));
			Store((int)vp);
			break;

		case instrANGET:  
			Load2();
			vp = &(((CKerArrName*)op[1].voidPtrData)->Get(op[2].intData));
			Store((int)vp);
			break;

		case instrAPGET:  
			Load2();
			vp = &(((CKerArrPointer*)op[1].voidPtrData)->Get(op[2].intData));
			Store((int)vp);
			break;

		case instrAIREAD:  
			Load2();
			i = ((CKerArrInt*)op[1].voidPtrData)->Get(op[2].intData);
			Store(i);
			break;

		case instrACREAD:  
			Load2();
			i = ((CKerArrChar*)op[1].voidPtrData)->Read(op[2].intData);
			Store(i);
			break;

		case instrADREAD:  
			Load2();
			d = ((CKerArrDouble*)op[1].voidPtrData)->Read(op[2].intData);
			Store(d);
			break;

		case instrAOREAD:  
			Load2();
			i = ((CKerArrObject*)op[1].voidPtrData)->Read(op[2].intData);
			Store(i);
			break;

		case instrANREAD:  
			Load2();
			n = ((CKerArrName*)op[1].voidPtrData)->Read(op[2].intData);
			Store((int)n);
			break;

		case instrAPREAD:  
			Load2();
			vp = ((CKerArrPointer*)op[1].voidPtrData)->Read(op[2].intData);
			Store((int)vp);
			break;

		case instrASETCNT:  
			Load(0);						// op[0] - pointer pole
			Load(1);						// op[1] - pocet k nastaveni
			vp = op[0].voidPtrData;
			switch(instr->op2.intValue)
			{
			case kwIntArray:	((CKerArrInt*)	  vp)->SetCount(op[1].intData);	break;
			case kwCharArray:	((CKerArrChar*)	  vp)->SetCount(op[1].intData);	break;
			case kwDoubleArray:	((CKerArrDouble*) vp)->SetCount(op[1].intData);	break;
			case kwNameArray:	((CKerArrName*)	  vp)->SetCount(op[1].intData);	break;
			case kwObjptrArray:	((CKerArrObject*) vp)->SetCount(op[1].intData);	break;
			case kwVoidArray:	((CKerArrPointer*)vp)->SetCount(op[1].intData);	break;
			default:
				KerMain->Errors->LogError(eKRTECorruptedInstr);
			}
			break;

		case instrSTRCPY: 
			Load(0);
			Load(1);
			strncpy((char*)op[0].voidPtrData, (char*)op[1].voidPtrData, cMaxStringLn+1);
			break;

		case instrSTRCAT: 
			Load(1);
			Load(2);
			i = strlen((char*)op[1].voidPtrData);
			j = strlen((char*)op[2].voidPtrData);
			cp = StackTop;

			StackTop += i + j + 1;
			CheckOverflow();
			strcpy((char*)cp, (char*)op[1].voidPtrData);
			strcat((char*)cp, (char*)op[2].voidPtrData);
			break;

		case instrSERV:			// volani sluzby kernelu
			{
			CIntInstructionRec* instr2;
			TouchRes();			// oba operandy vzdy prime -> nenahravam
	
			instr2 = instr;
			Skip(1);

			KernelServiceCall(instr2->op1.intValue, instr2->op2.intValue);	// samotne volani
			instr = instr2 + 1;

			if(isDouble[0])		// zapis navratove hodnoty
				Store(op[0].doubleData, instr2->instr.byteValue[1], instr2->res.intValue);
			else
				Store(op[0].intData, instr2->instr.byteValue[1], instr2->res.intValue);

			Skip(instr->res.intValue);		// preskocim datove instrukce
			}
			continue;

		case instrDCALL:				// direct volani
			Load1();
			instr2 = instr;
			Skip(1);
			
			i = instr->instr.intValue;	// velikost bloku parametru
			cp = SB;
			CreateCallRec(instr2->instr.byteValue[1], instr2->res.intValue, i, KerMain->Objs->GetObject(op[1].intData));
			memcpy(PB, cp + instr->op1.intValue, i);

//			MyDebugTab("last.dbg", tab); MyDebug("last.dbg", "DCALL: ", instr2->op2.intValue, KerMain->GetTime());
//			tab++;
			DoJump(instr2->op2.intValue);	// skok na prvni instrukci volane metody
			continue;						// uz jsem na dalsi instrukci - nechci delat Step()

		case instrSCALL:				// safe volani
			Load2();
			instr2 = instr;
			Skip(1);

			i = instr->res.intValue;		// pocet predavanych parametru
			{
				// volani se mohou rekurzivne opakovat - nemuzu pro predavani parametru pouzivat globalne platne pole...
				UC* oldStackTop = StackTop;
				int* oldArgTypes = argTypes;	// stary pointer na typy parametru si schovam
				try {

					CKerName* argNames[MAX_SAFE_CALL_PARAMS];
					void* argPointers[MAX_SAFE_CALL_PARAMS];

					CKerName** argNamesPtr = argNames;
					void** argPointersPtr = argPointers;

					j = PrepareForSafeCall(i, argNamesPtr, argPointersPtr);		// pocet instrukci k preskoceni
					KerMain->IScall((OPointer)op[1].intData, (CKerName*)op[2].voidPtrData, i, argTypes, argNamesPtr, argPointersPtr, instr->instr.intValue);
				}
				catch (CKernelPanic) {
					StackTop = oldStackTop;		// obnoveni zasobniku
					argTypes = oldArgTypes;		// obnoveni argTypes
					throw;
				}

				StackTop = oldStackTop;		// obnoveni zasobniku
				argTypes = oldArgTypes;		// obnoveni argTypes
			}

			if(isDouble[0])			// zapisu navratovou hodnotu
				Store(*((double*)StackTop), instr2->instr.byteValue[1], instr2->res.intValue);
			else
				if(isChar[0])
					Store(*((UC*)StackTop), instr2->instr.byteValue[1], instr2->res.intValue);
				else
					Store(*((int*)StackTop), instr2->instr.byteValue[1], instr2->res.intValue);

			Skip(j+1);		// preskocim datove instrukce
			continue;		// uz jsem na dalsi instrukci - nechci delat Step()

		case instrMSG:  
			Load(0);		// time / callend
			Load(1);		// object
			Load(2);		// method
			instr2 = instr;
			Skip(1);

			{
				int* oldArgTypes = argTypes;	// stary pointer na typy parametru si schovam
				try {

					i = instr->res.intValue;		// pocet predavanych parametru
					j = PrepareForSafeCall(i, argNames, argPointers);		// pocet instrukci k preskoceni
					KerMain->ISmessage((OPointer)op[1].intData, (CKerName*)op[2].voidPtrData, instr->instr.intValue, (OPointer)op[0].intData, i, argTypes, argNames, argPointers);
				} 
				catch (CKernelPanic) {
					argTypes = oldArgTypes;		// obnoveni argTypes
					throw;
				}
				argTypes = oldArgTypes;		// obnoveni argTypes
			}
			Skip(j+1);
			continue;		// uz jsem na dalsi instrukci - nechci delat Step()

		default:
			KerMain->Errors->LogError(eKRTEUnknownInstr);	// to je spatne - neznama instrukce -> chyba
		}	// switch
		Step();				// OK, tak rychle dalsi
	}	// while
}

myInline void CKerInterpret::KernelServiceCall(int kerServiceID, int paramCount)
{
	int i, param, lastString, line;
	CIntInstructionRec* instr2;

	// Nacteni parametru, vcetne tech se specialnim vyznamem, ktere uzivatel nezadava v programu, ale ktere kompilator doplnuje implicitne:
	lastString = cUnknown;
	line = instr->instr.intValue;
	instr2 = instr + 1;

	param = i = 0;
	while(i<paramCount)		// podle poctu parametru
	{
		// zpracovani specialnich parametru
		while(cKerServiceParams[kerServiceID][param].specUse && param < cKerServices[kerServiceID].paramsNr)
		{
			switch(cKerServiceParams[kerServiceID][param].specUse)		
			{
			// Cislo radky:
			case 1:		isChar[param] = isDouble[param] = false;    op[param].intData = line;				break;
			// Pointer this:
			case 2:		isChar[param] = isDouble[param] = false;    op[param].intData = KerContext->KCthis;	break;
			// Delka minuleho stringu:
			case 3:		isChar[param] = isDouble[param] = false;    op[param].intData = lastString;			break;	
			default:
				KerMain->Errors->LogError(eKRTECorruptedInstr);
			}
			param++;
		}

		// nacteni standardnich parametru
		if(param < cKerServices[kerServiceID].paramsNr)
		{
			LoadParam(i, param, instr2, cKerServiceParams[kerServiceID][param].type, cKerServiceParams[kerServiceID][param].pointer);
			param++;
			i++;
		}
	}

	////////////////////////////////////////////////////////////////////////////
	// zacatek automaticky generovaneho kodu - KernelServiceHandler.txt

	switch(kerServiceID)
	{
	case 0: 		 //LogUserError
		if(paramCount == 1)
			KerServices.LogUserError(op[0].intData, op[1].intData);
		else if(paramCount == 2)
			KerServices.LogUserError(op[0].intData, op[1].intData, op[2].intData);
		else if(paramCount == 3)
			KerServices.LogUserError(op[0].intData, op[1].intData, op[2].intData, (char*)op[3].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 1: 		 //LogDebugInfo
		if(paramCount == 1)
			KerServices.LogDebugInfo(op[0].intData, op[1].intData);
		else if(paramCount == 2)
			KerServices.LogDebugInfo(op[0].intData, op[1].intData, op[2].intData);
		else if(paramCount == 3)
			KerServices.LogDebugInfo(op[0].intData, op[1].intData, op[2].intData, (char*)op[3].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 2: 		 //IsFNMemberOfSN
		if(paramCount == 2)
			op[0].intData = KerServices.IsFMemberOfS((CKerName*)op[0].voidPtrData, (CKerName*)op[1].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 3: 		 //IsFOMemberOfSN
		if(paramCount == 2)
			op[0].intData = KerServices.IsFMemberOfS((OPointer)op[0].intData, (CKerName*)op[1].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 4: 		 //IsFNMemberOfSO
		if(paramCount == 2)
			op[0].intData = KerServices.IsFMemberOfS((CKerName*)op[0].voidPtrData, (OPointer)op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 5: 		 //IsFOMemberOfSO
		if(paramCount == 2)
			op[0].intData = KerServices.IsFMemberOfS((OPointer)op[0].intData, (OPointer)op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 6: 		 //InitMoveTo
		if(paramCount == 4)
			KerServices.InitMoveTo((OPointer)op[0].intData, op[1].intData, op[2].intData, op[3].intData);
		else if(paramCount == 5)
			KerServices.InitMoveTo((OPointer)op[0].intData, op[1].intData, op[2].intData, op[3].intData, op[4].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 7: 		 //SetTag
		if(paramCount == 1)
			KerServices.SetTag((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 8: 		 //ClearTag
		if(paramCount == 1)
			KerServices.ClearTag((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 9: 		 //CheckTag
		if(paramCount == 1)
			op[0].intData = KerServices.CheckTag((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 10: 		 //ExistsObj
		if(paramCount == 1)
			op[0].intData = KerServices.ExistsObj((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 11: 		 //TerminateKernel
		if(paramCount == 0)
			KerMain->TerminateKernel();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 12: 		 //GetKernelTime
		if(paramCount == 0)
			op[0].intData = KerMain->GetTime();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 13: 		 //GetRunMode
		if(paramCount == 0)
			op[0].intData = KerMain->GetRunMode();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 14: 		 //RegisterMap
		if(paramCount == 10)
			KerMain->MapInfo->RegisterMap(op[0].intData, op[1].intData, op[2].intData, op[3].intData, op[4].intData, op[5].intData, op[6].intData, op[7].intData, op[8].intData, op[9].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 15: 		 //MapGetNumberOfCells
		if(paramCount == 4)
			KerMain->MapInfo->GetNumberOfCells((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 16: 		 //FindCollidingCells
		if(paramCount == 2)
			KerMain->MapInfo->CellColisionB((OPointer)op[0].intData, (CKerArrInt*)op[1].voidPtrData);
		else if(paramCount == 3)
			KerMain->MapInfo->CellColisionB((OPointer)op[0].intData, (CKerArrInt*)op[1].voidPtrData, op[2].intData);
		else if(paramCount == 4)
			KerMain->MapInfo->CellColisionB((OPointer)op[0].intData, (CKerArrInt*)op[1].voidPtrData, op[2].intData, op[3].intData);
		else if(paramCount == 5)
			KerMain->MapInfo->CellColisionB((OPointer)op[0].intData, (CKerArrInt*)op[1].voidPtrData, op[2].intData, op[3].intData, op[4].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 17: 		 //DeleteOutOfMapCells
		if(paramCount == 1)
			KerServices.DeleteOutOfMapCalls((CKerArrInt*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 18: 		 //IsCellInMap
		if(paramCount == 3)
			op[0].intData = KerMain->MapInfo->IsCellInMap(op[0].intData, op[1].intData, op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 19: 		 //IsPixelInMap
		if(paramCount == 3)
			op[0].intData = KerMain->MapInfo->IsPixelInMap(op[0].intData, op[1].intData, op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 20: 		 //IsObjInCollision
		if(paramCount == 1)
			op[0].intData = KerMain->MapInfo->IsObjInCollision(op[0].intData, (OPointer)op[1].intData);
		else if(paramCount == 2)
			op[0].intData = KerMain->MapInfo->IsObjInCollision(op[0].intData, (OPointer)op[1].intData, op[2].intData);
		else if(paramCount == 3)
			op[0].intData = KerMain->MapInfo->IsObjInCollision(op[0].intData, (OPointer)op[1].intData, op[2].intData, op[3].intData);
		else if(paramCount == 4)
			op[0].intData = KerMain->MapInfo->IsObjInCollision(op[0].intData, (OPointer)op[1].intData, op[2].intData, op[3].intData, op[4].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 21: 		 //FindCollidingObjs
		if(paramCount == 1)
			op[0].voidPtrData = KerMain->MapInfo->FindCollidingObjs(op[0].intData, (OPointer)op[1].intData);
		else if(paramCount == 2)
			op[0].voidPtrData = KerMain->MapInfo->FindCollidingObjs(op[0].intData, (OPointer)op[1].intData, (CKerArrObject*)op[2].voidPtrData);
		else if(paramCount == 3)
			op[0].voidPtrData = KerMain->MapInfo->FindCollidingObjs(op[0].intData, (OPointer)op[1].intData, (CKerArrObject*)op[2].voidPtrData, op[3].intData);
		else if(paramCount == 4)
			op[0].voidPtrData = KerMain->MapInfo->FindCollidingObjs(op[0].intData, (OPointer)op[1].intData, (CKerArrObject*)op[2].voidPtrData, op[3].intData, op[4].intData);
		else if(paramCount == 5)
			op[0].voidPtrData = KerMain->MapInfo->FindCollidingObjs(op[0].intData, (OPointer)op[1].intData, (CKerArrObject*)op[2].voidPtrData, op[3].intData, op[4].intData, op[5].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 22: 		 //IsObjInMap
		if(paramCount == 1)
			op[0].intData = KerMain->MapInfo->IsObjInMap((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 23: 		 //PlaceObjToMap
		if(paramCount == 1)
			KerMain->MapInfo->PlaceObjToMap(op[0].intData, (OPointer)op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 24: 		 //PlaceObjToMapKill
		if(paramCount == 1)
			KerMain->MapInfo->PlaceObjToMapWithKill(op[0].intData, (OPointer)op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 25: 		 //RemoveObjFromMap
		if(paramCount == 1)
			KerMain->MapInfo->RemoveObjFromMap(op[0].intData, (OPointer)op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 26: 		 //MoveObjTo
		if(paramCount == 4)
			KerMain->MapInfo->MoveObjTo(op[0].intData, (OPointer)op[1].intData, op[2].intData, op[3].intData, op[4].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 27: 		 //MoveObjRel
		if(paramCount == 4)
			KerMain->MapInfo->MoveObjRel(op[0].intData, (OPointer)op[1].intData, op[2].intData, op[3].intData, op[4].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 28: 		 //FindObjCell
		if(paramCount == 4)
			op[0].intData = KerServices.FindObjCell((OPointer)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 29: 		 //FindCell
		if(paramCount == 6)
			op[0].intData = KerServices.FindCell(op[0].intData, op[1].intData, op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 30: 		 //FindCellPosition
		if(paramCount == 6)
			KerServices.FindCellPosition(op[0].intData, op[1].intData, op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 31: 		 //ReadObjCoords
		if(paramCount == 4)
			KerServices.ReadObjCoords((OPointer)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 32: 		 //WriteObjCoords
		if(paramCount == 3)
			KerMain->MapInfo->ChangeObjCoords(op[0].intData, (OPointer)op[1].intData, op[2].intData, op[3].intData);
		else if(paramCount == 4)
			KerMain->MapInfo->ChangeObjCoords(op[0].intData, (OPointer)op[1].intData, op[2].intData, op[3].intData, op[4].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 33: 		 //rand
		if(paramCount == 1)
			op[0].doubleData = mtr.rand(op[0].doubleData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 34: 		 //randExc
		if(paramCount == 1)
			op[0].doubleData = mtr.randExc(op[0].doubleData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 35: 		 //randInt
		if(paramCount == 1)
			op[0].intData = mtr.randInt(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 36: 		 //SLSeek
		if(paramCount == 1)
			KerSaveLoad.Seek(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 37: 		 //SLSeekToEnd
		if(paramCount == 0)
			KerSaveLoad.SeekToEnd();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 38: 		 //SLGetPos
		if(paramCount == 0)
			op[0].intData = KerSaveLoad.GetPos();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 39: 		 //SLEof
		if(paramCount == 0)
			op[0].intData = KerSaveLoad.Eof();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 40: 		 //SaveInt
		if(paramCount == 1)
			KerSaveLoad.SaveInt(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 41: 		 //SaveChar
		if(paramCount == 1)
			KerSaveLoad.SaveChar(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 42: 		 //SaveDouble
		if(paramCount == 1)
			KerSaveLoad.SaveDouble(op[0].doubleData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 43: 		 //SaveObjPtr
		if(paramCount == 1)
			KerSaveLoad.SaveObjPtr((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 44: 		 //SaveName
		if(paramCount == 1)
			KerSaveLoad.SaveName((CKerName*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 45: 		 //SaveString
		if(paramCount == 1)
			KerSaveLoad.SaveString((char*)op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 46: 		 //SaveIntA
		if(paramCount == 1)
			KerSaveLoad.SaveIntA((CKerArrInt*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 47: 		 //SaveCharA
		if(paramCount == 1)
			KerSaveLoad.SaveCharA((CKerArrChar*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 48: 		 //SaveDoubleA
		if(paramCount == 1)
			KerSaveLoad.SaveDoubleA((CKerArrDouble*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 49: 		 //SaveObjPtrA
		if(paramCount == 1)
			KerSaveLoad.SaveObjPtrA((CKerArrObject*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 50: 		 //SaveNameA
		if(paramCount == 1)
			KerSaveLoad.SaveNameA((CKerArrName*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 51: 		 //LoadInt
		if(paramCount == 0)
			op[0].intData = KerSaveLoad.LoadInt();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 52: 		 //LoadChar
		if(paramCount == 0)
			op[0].intData = KerSaveLoad.LoadChar();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 53: 		 //LoadDouble
		if(paramCount == 0)
			op[0].doubleData = KerSaveLoad.LoadDouble();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 54: 		 //LoadObjPtr
		if(paramCount == 0)
			op[0].intData = KerSaveLoad.LoadObjPtr();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 55: 		 //LoadName
		if(paramCount == 0)
			op[0].voidPtrData = KerSaveLoad.LoadName();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 56: 		 //LoadString
		if(paramCount == 1)
			KerSaveLoad.LoadString((char*)op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 57: 		 //LoadIntA
		if(paramCount == 0)
			op[0].voidPtrData = KerSaveLoad.LoadIntA();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 58: 		 //LoadCharA
		if(paramCount == 0)
			op[0].voidPtrData = KerSaveLoad.LoadCharA();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 59: 		 //LoadDoubleA
		if(paramCount == 0)
			op[0].voidPtrData = KerSaveLoad.loadDoubleA();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 60: 		 //LoadObjPtrA
		if(paramCount == 0)
			op[0].voidPtrData = KerSaveLoad.LoadObjPtrA();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 61: 		 //LoadNameA
		if(paramCount == 0)
			op[0].voidPtrData = KerSaveLoad.LoadNameA();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 62: 		 //ResetAuto
		if(paramCount == 1)
			KerServices.ResetAuto((OPointer)op[0].intData);
		else if(paramCount == 2)
			KerServices.ResetAuto((OPointer)op[0].intData, op[1].intData);
		else if(paramCount == 3)
			KerServices.ResetAuto((OPointer)op[0].intData, op[1].intData, op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 63: 		 //MvConnectObjs
		if(paramCount == 2)
			KerServices.MConnectObjs((OPointer)op[0].intData, (OPointer)op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 64: 		 //MvDisconnectObj
		if(paramCount == 1)
			KerServices.MDisconnectObj((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 65: 		 //GetActivAuto
		if(paramCount == 1)
			op[0].voidPtrData = KerServices.GetActivAuto((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 66: 		 //GetDefaultAuto
		if(paramCount == 1)
			op[0].voidPtrData = KerServices.GetDefaultAuto((OPointer)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 67: 		 //GetDefaultAuto2
		if(paramCount == 1)
			op[0].voidPtrData = KerServices.GetDefaultAuto2((CKerName*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 68: 		 //FindObjsInArea
		if(paramCount == 2)
			KerServices.FindObjsInArea(op[0].intData, (CKerArrObject*)op[1].voidPtrData, (OPointer)op[2].intData);
		else if(paramCount == 3)
			KerServices.FindObjsInArea(op[0].intData, (CKerArrObject*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData);
		else if(paramCount == 4)
			KerServices.FindObjsInArea(op[0].intData, (CKerArrObject*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData);
		else if(paramCount == 5)
			KerServices.FindObjsInArea(op[0].intData, (CKerArrObject*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData);
		else if(paramCount == 6)
			KerServices.FindObjsInArea(op[0].intData, (CKerArrObject*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData);
		else if(paramCount == 7)
			KerServices.FindObjsInArea(op[0].intData, (CKerArrObject*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData, op[7].intData);
		else if(paramCount == 8)
			KerServices.FindObjsInArea(op[0].intData, (CKerArrObject*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData, op[7].intData, op[8].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 69: 		 //AreaCall
		if(paramCount == 2)
			KerServices.AreaCall(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData);
		else if(paramCount == 3)
			KerServices.AreaCall(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData);
		else if(paramCount == 4)
			KerServices.AreaCall(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData);
		else if(paramCount == 5)
			KerServices.AreaCall(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData);
		else if(paramCount == 6)
			KerServices.AreaCall(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData);
		else if(paramCount == 7)
			KerServices.AreaCall(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData, op[7].intData);
		else if(paramCount == 8)
			KerServices.AreaCall(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData, op[7].intData, op[8].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 70: 		 //AreaMessage
		if(paramCount == 2)
			KerServices.AreaMessage(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData);
		else if(paramCount == 3)
			KerServices.AreaMessage(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData);
		else if(paramCount == 4)
			KerServices.AreaMessage(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData);
		else if(paramCount == 5)
			KerServices.AreaMessage(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData);
		else if(paramCount == 6)
			KerServices.AreaMessage(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData);
		else if(paramCount == 7)
			KerServices.AreaMessage(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData, op[7].intData);
		else if(paramCount == 8)
			KerServices.AreaMessage(op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData, (CKerName*)op[3].voidPtrData, op[4].intData, op[5].intData, op[6].intData, op[7].intData, op[8].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 71: 		 //IsKeyDown
		if(paramCount == 1)
			op[0].intData = Input->IsKeyDown((CKerName*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 72: 		 //GameVictory
		if(paramCount == 0)
			KRKAL->GameVictory();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 73: 		 //IsGame
		if(paramCount == 0)
			op[0].intData = KerMain->IsGameMode();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 74: 		 //IsEditor
		if(paramCount == 0)
			op[0].intData = KerMain->IsEditorMode();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 75: 		 //SetScrollCenter
		if(paramCount == 2)
			KerMain->MapInfo->SetScrollCenter(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 76: 		 //WindowScroll
		if(paramCount == 3)
			KerMain->MapInfo->WindowScroll(op[0].intData, op[1].intData, op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 77: 		 //CalcScrollDistance
		if(paramCount == 4)
			KerMain->MapInfo->CalcScrollDistance(op[0].intData, op[1].intData, (int*)op[2].intData, (int*)op[3].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 78: 		 //MnuSetBar
		if(paramCount == 2)
			op[0].intData = CGameMenu::SetProgressBar(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 79: 		 //MnuSetItem
		if(paramCount == 2)
			op[0].intData = CGameMenu::ObjectSetCount(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 80: 		 //MnuRefresh
		if(paramCount == 0)
			CGameMenu::Refresh();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 81: 		 //MnuDeleteItem
		if(paramCount == 1)
			op[0].intData = CGameMenu::DeleteItem(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 82: 		 //MnuAddItemN
		if (paramCount == 1)
			op[0].intData = CGameMenu::AddItem((CKerName*)op[0].voidPtrData);
		else if (paramCount == 2)
			op[0].intData = CGameMenu::AddItem((CKerName*)op[0].voidPtrData, (CKerName*)op[1].voidPtrData);
		else
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 83: 		 //MnuAddItemO
		if (paramCount == 1)
			op[0].intData = CGameMenu::AddItem((OPointer)op[0].intData);
		else if (paramCount == 2)
			op[0].intData = CGameMenu::AddItem((OPointer)op[0].intData, (CKerName*)op[1].voidPtrData);
		else
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 84: 		 //MnuAddBars
		if(paramCount == 2)
			op[0].intData = CGameMenu::Add2ProgressBars(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 85: 		 //sqrt
		if(paramCount == 1)
			op[0].doubleData = sqrt(op[0].doubleData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 86: 		 //round
		if(paramCount == 1)
			op[0].intData = KerServices.round(op[0].doubleData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 87: 		 //PlaySound
		if(paramCount == 3)
			op[0].intData = ME->PlaySample((CKerName*)op[0].voidPtrData, op[1].intData, op[2].intData);
		else if(paramCount == 4)
			op[0].intData = ME->PlaySample((CKerName*)op[0].voidPtrData, op[1].intData, op[2].intData, op[3].doubleData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 88: 		 //ECreateInt
		if(paramCount == 2)
			KerEditI.ECreateInt((int*)op[0].intData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateInt((int*)op[0].intData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 89: 		 //ECreateChar
		if(paramCount == 2)
			KerEditI.ECreateChar((unsigned char*)op[0].intData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateChar((unsigned char*)op[0].intData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 90: 		 //ECreateDouble
		if(paramCount == 2)
			KerEditI.ECreateDouble((double*)op[0].intData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateDouble((double*)op[0].intData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 91: 		 //ECreateName
		if(paramCount == 2)
			KerEditI.ECreateName((CKerName**)op[0].voidPtrData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateName((CKerName**)op[0].voidPtrData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 92: 		 //ECreateObjPtr
		if(paramCount == 2)
			KerEditI.ECreateObjPtr((OPointer*)op[0].voidPtrData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateObjPtr((OPointer*)op[0].voidPtrData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 93: 		 //ECreateIntA
		if(paramCount == 2)
			KerEditI.ECreateIntA((CKerArrInt**)op[0].voidPtrData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateIntA((CKerArrInt**)op[0].voidPtrData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 94: 		 //ECreateCharA
		if(paramCount == 2)
			KerEditI.ECreateCharA((CKerArrChar**)op[0].voidPtrData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateCharA((CKerArrChar**)op[0].voidPtrData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 95: 		 //ECreateDoubleA
		if(paramCount == 2)
			KerEditI.ECreateDoubleA((CKerArrDouble**)op[0].voidPtrData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateDoubleA((CKerArrDouble**)op[0].voidPtrData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 96: 		 //ECreateNameA
		if(paramCount == 2)
			KerEditI.ECreateNameA((CKerArrName**)op[0].voidPtrData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateNameA((CKerArrName**)op[0].voidPtrData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 97: 		 //ECreateObjPtrA
		if(paramCount == 2)
			KerEditI.ECreateObjPtrA((CKerArrObject**)op[0].voidPtrData, (char*)op[1].intData);
		else if(paramCount == 3)
			KerEditI.ECreateObjPtrA((CKerArrObject**)op[0].voidPtrData, (char*)op[1].intData, (char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 98: 		 //EPlaceItem
		if(paramCount == 0)
			op[0].intData = KerEditI.EPlaceItem();
		else if(paramCount == 1)
			op[0].intData = KerEditI.EPlaceItem(op[0].intData);
		else if(paramCount == 2)
			op[0].intData = KerEditI.EPlaceItem(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 99: 		 //ERefresh
		if(paramCount == 0)
			op[0].intData = CGUIScriptVariableInterface::ScriptedVariableFinish();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 100: 		 //EAddGap
		if(paramCount == 0)
			op[0].intData = CGUIScriptVariableInterface::AddGap();
		else if(paramCount == 1)
			op[0].intData = CGUIScriptVariableInterface::AddGap(op[0].intData);
		else if(paramCount == 2)
			op[0].intData = CGUIScriptVariableInterface::AddGap(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 101: 		 //EDeleteItem
		if(paramCount == 1)
			op[0].intData = CGUIScriptVariableInterface::DeleteItem(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 102: 		 //EAddButton
		if(paramCount == 0)
			op[0].intData = CGUIScriptVariableInterface::AddButton();
		else if(paramCount == 1)
			op[0].intData = CGUIScriptVariableInterface::AddButton(op[0].intData);
		else if(paramCount == 2)
			op[0].intData = CGUIScriptVariableInterface::AddButton(op[0].intData, op[1].intData);
		else if(paramCount == 3)
			op[0].intData = CGUIScriptVariableInterface::AddButton(op[0].intData, op[1].intData, (char*)op[2].intData);
		else if(paramCount == 4)
			op[0].intData = CGUIScriptVariableInterface::AddButton(op[0].intData, op[1].intData, (char*)op[2].intData, (char*)op[3].intData);
		else if(paramCount == 5)
			op[0].intData = CGUIScriptVariableInterface::AddButton(op[0].intData, op[1].intData, (char*)op[2].intData, (char*)op[3].intData, op[4].intData);
		else if(paramCount == 6)
			op[0].intData = CGUIScriptVariableInterface::AddButton(op[0].intData, op[1].intData, (char*)op[2].intData, (char*)op[3].intData, op[4].intData, (CKerName*)op[5].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 103: 		 //EDeleteButton
		if(paramCount == 1)
			op[0].intData = CGUIScriptVariableInterface::DeleteButton(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 104: 		 //EAddGroupItem
		if(paramCount == 2)
			op[0].intData = CGUIScriptVariableInterface::AddGroupItem((char*)op[0].intData, (char*)op[1].intData);
		else if(paramCount == 3)
			op[0].intData = CGUIScriptVariableInterface::AddGroupItem((char*)op[0].intData, (char*)op[1].intData, op[2].intData);
		else if(paramCount == 4)
			op[0].intData = CGUIScriptVariableInterface::AddGroupItem((char*)op[0].intData, (char*)op[1].intData, op[2].intData, op[3].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 105: 		 //EDeleteGroupItem
		if(paramCount == 1)
			op[0].intData = CGUIScriptVariableInterface::DeleteGroupItem(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 106: 		 //ESelectGroupItem
		if(paramCount == 1)
			op[0].intData = CGUIScriptVariableInterface::SelectGroupItem(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 107: 		 //EDeleteAllGroupItems
		if(paramCount == 1)
			op[0].intData = CGUIScriptVariableInterface::DeleteAllGroupItems(op[0].intData);
		else if(paramCount == 2)
			op[0].intData = CGUIScriptVariableInterface::DeleteAllGroupItems(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 108: 		 //EAdd2DCell
		if(paramCount == 3)
			op[0].intData = KerEditI.EAdd2DCell((int*)op[0].intData, (int*)op[1].intData, (char*)op[2].intData);
		else if(paramCount == 4)
			op[0].intData = KerEditI.EAdd2DCell((int*)op[0].intData, (int*)op[1].intData, (char*)op[2].intData, (char*)op[3].intData);
		else if(paramCount == 5)
			op[0].intData = KerEditI.EAdd2DCell((int*)op[0].intData, (int*)op[1].intData, (char*)op[2].intData, (char*)op[3].intData, op[4].intData);
		else if(paramCount == 6)
			op[0].intData = KerEditI.EAdd2DCell((int*)op[0].intData, (int*)op[1].intData, (char*)op[2].intData, (char*)op[3].intData, op[4].intData, op[5].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 109: 		 //EAdd3DCell
		if(paramCount == 4)
			op[0].intData = KerEditI.EAdd3DCell((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (char*)op[3].intData);
		else if(paramCount == 5)
			op[0].intData = KerEditI.EAdd3DCell((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (char*)op[3].intData, (char*)op[4].intData);
		else if(paramCount == 6)
			op[0].intData = KerEditI.EAdd3DCell((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (char*)op[3].intData, (char*)op[4].intData, op[5].intData);
		else if(paramCount == 7)
			op[0].intData = KerEditI.EAdd3DCell((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (char*)op[3].intData, (char*)op[4].intData, op[5].intData, op[6].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 110: 		 //EAdd2DPoint
		if(paramCount == 3)
			op[0].intData = KerEditI.EAdd2DPoint((int*)op[0].intData, (int*)op[1].intData, (char*)op[2].intData);
		else if(paramCount == 4)
			op[0].intData = KerEditI.EAdd2DPoint((int*)op[0].intData, (int*)op[1].intData, (char*)op[2].intData, (char*)op[3].intData);
		else if(paramCount == 5)
			op[0].intData = KerEditI.EAdd2DPoint((int*)op[0].intData, (int*)op[1].intData, (char*)op[2].intData, (char*)op[3].intData, op[4].intData);
		else if(paramCount == 6)
			op[0].intData = KerEditI.EAdd2DPoint((int*)op[0].intData, (int*)op[1].intData, (char*)op[2].intData, (char*)op[3].intData, op[4].intData, op[5].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 111: 		 //EAdd3DPoint
		if(paramCount == 4)
			op[0].intData = KerEditI.EAdd3DPoint((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (char*)op[3].intData);
		else if(paramCount == 5)
			op[0].intData = KerEditI.EAdd3DPoint((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (char*)op[3].intData, (char*)op[4].intData);
		else if(paramCount == 6)
			op[0].intData = KerEditI.EAdd3DPoint((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (char*)op[3].intData, (char*)op[4].intData, op[5].intData);
		else if(paramCount == 7)
			op[0].intData = KerEditI.EAdd3DPoint((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (char*)op[3].intData, (char*)op[4].intData, op[5].intData, op[6].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 112: 		 //EAdd2DAreaP
		if(paramCount == 5)
			op[0].intData = KerEditI.EAdd2DAreaP((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (char*)op[4].intData);
		else if(paramCount == 6)
			op[0].intData = KerEditI.EAdd2DAreaP((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (char*)op[4].intData, (char*)op[5].intData);
		else if(paramCount == 7)
			op[0].intData = KerEditI.EAdd2DAreaP((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (char*)op[4].intData, (char*)op[5].intData, op[6].intData);
		else if(paramCount == 8)
			op[0].intData = KerEditI.EAdd2DAreaP((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (char*)op[4].intData, (char*)op[5].intData, op[6].intData, op[7].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 113: 		 //EAdd3DAreaP
		if(paramCount == 7)
			op[0].intData = KerEditI.EAdd3DAreaP((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData, (char*)op[6].intData);
		else if(paramCount == 8)
			op[0].intData = KerEditI.EAdd3DAreaP((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData, (char*)op[6].intData, (char*)op[7].intData);
		else if(paramCount == 9)
			op[0].intData = KerEditI.EAdd3DAreaP((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData, (char*)op[6].intData, (char*)op[7].intData, op[8].intData);
		else if(paramCount == 10)
			op[0].intData = KerEditI.EAdd3DAreaP((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData, (char*)op[6].intData, (char*)op[7].intData, op[8].intData, op[9].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 114: 		 //EAdd2DAreaC
		if(paramCount == 5)
			op[0].intData = KerEditI.EAdd2DAreaC((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (char*)op[4].intData);
		else if(paramCount == 6)
			op[0].intData = KerEditI.EAdd2DAreaC((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (char*)op[4].intData, (char*)op[5].intData);
		else if(paramCount == 7)
			op[0].intData = KerEditI.EAdd2DAreaC((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (char*)op[4].intData, (char*)op[5].intData, op[6].intData);
		else if(paramCount == 8)
			op[0].intData = KerEditI.EAdd2DAreaC((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (char*)op[4].intData, (char*)op[5].intData, op[6].intData, op[7].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 115: 		 //EAdd3DAreaC
		if(paramCount == 7)
			op[0].intData = KerEditI.EAdd3DAreaC((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData, (char*)op[6].intData);
		else if(paramCount == 8)
			op[0].intData = KerEditI.EAdd3DAreaC((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData, (char*)op[6].intData, (char*)op[7].intData);
		else if(paramCount == 9)
			op[0].intData = KerEditI.EAdd3DAreaC((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData, (char*)op[6].intData, (char*)op[7].intData, op[8].intData);
		else if(paramCount == 10)
			op[0].intData = KerEditI.EAdd3DAreaC((int*)op[0].intData, (int*)op[1].intData, (int*)op[2].intData, (int*)op[3].intData, (int*)op[4].intData, (int*)op[5].intData, (char*)op[6].intData, (char*)op[7].intData, op[8].intData, op[9].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 116: 		 //EAddText
		if(paramCount == 1)
			op[0].intData = CGUIScriptVariableInterface::AddText((char*)op[0].intData);
		else if(paramCount == 2)
			op[0].intData = CGUIScriptVariableInterface::AddText((char*)op[0].intData, (char*)op[1].intData);
		else if(paramCount == 3)
			op[0].intData = CGUIScriptVariableInterface::AddText((char*)op[0].intData, (char*)op[1].intData, op[2].intData);
		else if(paramCount == 4)
			op[0].intData = CGUIScriptVariableInterface::AddText((char*)op[0].intData, (char*)op[1].intData, op[2].intData, op[3].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 117: 		 //ESetVarTags
		if(paramCount == 1)
			KerEditI.ESetVarTags(op[0].intData);
		else if(paramCount == 2)
			KerEditI.ESetVarTags(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 118: 		 //IsAnyKeyDown
		if(paramCount == 0)
			op[0].intData = Input->KeyPressed();
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 119: 		 //AddLight
		if(paramCount == 7)
			op[0].intData = GEnMain->AddLight(op[0].intData, op[1].intData, op[2].intData, op[3].intData, op[4].intData, op[5].intData, op[6].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 120: 		 //DeleteLight
		if(paramCount == 1)
			GEnMain->DeleteLight(op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 121: 		 //SetTopLightIntenzity
		if(paramCount == 3)
			GEnMain->SetTopLightIntenzity(op[0].intData, op[1].intData, op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 122: 		 //GetTopLightIntenzity
		if(paramCount == 3)
			GEnMain->GetTopLightIntenzity((unsigned char*)op[0].intData, (unsigned char*)op[1].intData, (unsigned char*)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 123: 		 //CopyIntA
		if(paramCount == 1)
			op[0].voidPtrData = KerServices.CopyIntA((CKerArrInt*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 124: 		 //CopyCharA
		if(paramCount == 1)
			op[0].voidPtrData = KerServices.CopyCharA((CKerArrChar*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 125: 		 //CopyDoubleA
		if(paramCount == 1)
			op[0].voidPtrData = KerServices.CopyDoubleA((CKerArrDouble*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 126: 		 //CopyObjPtrA
		if(paramCount == 1)
			op[0].voidPtrData = KerServices.CopyObjPtrA((CKerArrObject*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 127: 		 //CopyNameA
		if(paramCount == 1)
			op[0].voidPtrData = KerServices.CopyNameA((CKerArrName*)op[0].voidPtrData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 128: 		 //CopyObject
		if(paramCount == 1)
			op[0].intData = KerMain->CopyObject(op[0].intData, (OPointer)op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 129: 		 //DeleteMessages
		if(paramCount == 1)
			op[0].intData = KerMain->FDeleteMessages((OPointer)op[0].intData);
		else if(paramCount == 2)
			op[0].intData = KerMain->FDeleteMessages((OPointer)op[0].intData, (CKerName*)op[1].voidPtrData);
		else if(paramCount == 3)
			op[0].intData = KerMain->FDeleteMessages((OPointer)op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 130: 		 //CountMessages
		if(paramCount == 1)
			op[0].intData = KerMain->FCountMessages((OPointer)op[0].intData);
		else if(paramCount == 2)
			op[0].intData = KerMain->FCountMessages((OPointer)op[0].intData, (CKerName*)op[1].voidPtrData);
		else if(paramCount == 3)
			op[0].intData = KerMain->FCountMessages((OPointer)op[0].intData, (CKerName*)op[1].voidPtrData, (OPointer)op[2].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 131: 		 //ESetEditType
		if(paramCount == 1)
			KerEditI.ESetEditType(op[0].intData, op[1].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 132: 		 //SaveTheGame
		if(paramCount == 0)
			KerMain->SaveGame();
		else if(paramCount == 1)
			KerMain->SaveGame((char*)op[0].intData);
		else 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 133: 		 //ShowLevelIntro
		if (paramCount == 1)
			KerServices.ShowLevelIntro((char*)op[0].strData);
		else if (paramCount == 2)
			KerServices.ShowLevelIntro((char*)op[0].strData, (char*)op[1].strData);
		else
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	case 134: 		 //MnuSetManikPicture
		if (paramCount == 1)
			op[0].intData = CGameMenu::SetManikPicture((OPointer)op[0].intData);
		else if (paramCount == 2)
			op[0].intData = CGameMenu::SetManikPicture((OPointer)op[0].intData, (CKerName*)op[1].voidPtrData);
		else
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		break;

	default:
		KerMain->Errors->LogError(eKRTECorruptedInstr);
	}

	// konec automaticky generovaneho kodu - KernelServiceHandler.txt
	////////////////////////////////////////////////////////////////////////////
}

myInline int CKerInterpret::PrepareForSafeCall(int paramCount, CKerName** argNames, void** argPointers)
{
	int i, blkSize;
	CIntInstructionRec *instr2, *instrNames, *instrValues;

	argTypes = (int*)(instr+1);			// argTypes nastavim jednorazove - typy jdou vzdy v pameti za sebou

	blkSize = instr->op1.intValue;
	instrNames = instr + 1 + blkSize;
	instrValues = instr + 1 + 2*blkSize;

	for(i = 0; i < paramCount; i++)
	{
		instr2 = instrNames + i/4;
		argNames[i] = *((CKerName**)(SB + instr2->GetPart(i%4)));		// jmeno

		instr2 = instrValues + i/3;
		argPointers[i] = (void*)(SB + instr2->GetPart(i%3 + 1));		// hodnota
	}

	return 2 * blkSize + instr->op2.intValue;
}

myInline void CKerInterpret::CreateCallRec(int retDescriptor, int retIndex, int paramsSize, CKerObject* o)
{
	UC* oldStack = StackTop;
	
	StackTop += cMethodCallRecordSize + paramsSize;		// vyhradim misto na zasobniku
	CheckOverflow();

	CheckPointerForWrite(oldStack, 5*4 + 4);

	// Zapisu na zasobnik aktualni kontext pro pozdejsi obnoveni v DRET:
	((UC**)oldStack)[0] = OB;	
	((UC**)oldStack)[1] = PB;
	((UC**)oldStack)[2] = SB;
	((int*)oldStack)[3] = retDescriptor;
	((int*)oldStack)[4] = retIndex;
	((int*)oldStack)[5] = (IP+1) * cInstructionSize;

	// Nastavim novy kontext:
	PB = StackTop - paramsSize;
	SB = StackTop;
	if(!o)
		KerMain->Errors->LogError(eKRTECorruptedInstr);
	OB = o->ISData;
}


myInline void CKerInterpret::RestoreFromCallRec(UC* base)
{
	// Obnovim drive ulozeny kontext:
	CheckPointerForRead(base, 5*4 + 4);
	OB = ((UC**)base)[0];
	PB = ((UC**)base)[1];
	SB = ((UC**)base)[2];
	StackTop = base;
	CheckOverflow(true, false);
}

myInline void CKerInterpret::Load(int i, CIntInstructionRec* instr)
{
	int tmpInfo, offset;
	UC* p;

	if(!instr)
		instr = CKerInterpret::instr;	// defaultne budu nahravat operand aktualni instrukce

	offset = instr->GetPart(i + 1);
	tmpInfo = instr->instr.byteValue[i + 1];

	isDouble[i] = isDOUBLE(tmpInfo);
	isChar[i] = isCHAR(tmpInfo);

	switch(opTp[i] = (enmOpType)getARG(tmpInfo))
	{
	case eOpTpMissing:					// operand chybi -> neni co resit
		return;

	case eOpTpImmediate:				// primy operand -> int / char
		op[i].intData = offset;
		return;

	case eOpTpStackTmp:			p = SB + offset;		break;		// vyraz na zasobniku
	case eOpTpLocal:			p = PB + offset;		break;		// lokalni promenna 
	case eOpTpAttribute:		p = OB + offset;		break;		// atribut
	case eOpTpGlobal:												// globalni promenna
		p = ((UC**)GlobalsStart)[offset];		
		break;		
	
	default:
		KerMain->Errors->LogError(eKRTEUnknownInstr);
	}

	// Mam pointer, nyni z nej jen prectu data:
	if(isDouble[i])
		op[i].doubleData = MemoryReadD(p);
	else
		if(isChar[i])
			op[i].intData = MemoryReadC(p);
		else
			op[i].intData = MemoryReadI(p);
}

myInline void CKerInterpret::LoadPointer(int i, CIntInstructionRec* instr)
{
	int tmpInfo, offset;
	UC* p;

	if(!instr)
		instr = CKerInterpret::instr;	// defaultne budu nahravat operand aktualni instrukce

	offset = instr->GetPart(i+1);
	tmpInfo = instr->instr.byteValue[i+1];

	isDouble[i] = isDOUBLE(tmpInfo);
	isChar[i] = isCHAR(tmpInfo);

	switch(opTp[i] = (enmOpType)getARG(tmpInfo))
	{
	case eOpTpStackTmp:			p = SB + offset;		break;		// vyraz na zasobniku
	case eOpTpLocal:			p = PB + offset;		break;		// lokalni promenna 
	case eOpTpAttribute:		p = OB + offset;		break;		// atribut
	case eOpTpGlobal:												// globalni promenna
		p = ((UC**)GlobalsStart)[offset];		
		break;		
	default:
		KerMain->Errors->LogError(eKRTEUnknownInstr);
	}

	// Mam pointer, a to jsem chtel, zadnou hodnotu z nej tentokrat nectu.
	op[i].voidPtrData = p;	
}

myInline void CKerInterpret::LoadParam(int paramStackIndex, int paramTarget, CIntInstructionRec* base, int type, int pointer)
{
	int size, offset;
	UC* p;

	size = (base + paramStackIndex/3)->instr.byteValue[paramStackIndex % 3 + 1];
	offset = (base + paramStackIndex/3)->GetPart(paramStackIndex % 3 + 1);

	isDouble[paramTarget] = type == kwDouble && pointer == 0;
	isChar[paramTarget]	= type == kwChar && pointer == 0;

	p = SB + offset;		// parametr je vzdy na zasobniku

	if(isDouble[paramTarget])	// double nahravam do .doubleData:
	{
		switch(size)
		{
		case 8:			op[paramTarget].doubleData = *((double*)p);		break;
		case 4:			op[paramTarget].doubleData = *((int*)p);		break;
		case 1:			op[paramTarget].doubleData = *((char*)p);		break;
		default: 
			KerMain->Errors->LogError(eKRTECorruptedInstr);
		}
	} else			// ...ostatni nahravam do .intData:
		switch(size)
		{
		case 1:			op[paramTarget].intData = *((char*)p);		break;
		default: 
			if(type == kwChar && pointer == 1 && size != 4)
				op[paramTarget].voidPtrData = p;		// u stringu chci ale pointer (konverze na char*)
			else
				op[paramTarget].intData = *((int*)p);		
		}
}


myInline void CKerInterpret::TouchRes()
{
	int tmpInfo = instr->instr.byteValue[1];

	opTp[0] = (enmOpType)getARG(tmpInfo);
	isDouble[0] = isDOUBLE(tmpInfo);
	isChar[0] = isCHAR(tmpInfo);
}

myInline void CKerInterpret::InitOp(int tmpInfo, int data, int i)
{
	opTp[i] = (enmOpType)getARG(tmpInfo);
	assert(!isDOUBLE(tmpInfo));
	isDouble[i] = false;
	isChar[i] = isCHAR(tmpInfo);
	op[i].intData = data;
}

myInline void CKerInterpret::Store(int data, int type, int offset)
{
	UC* p;

	if(type == cUnknown)
	{
		type = opTp[0];
		offset = instr->res.intValue;
	} else
		isChar[0] = isCHAR(type);


	switch(type)
	{
	case eOpTpMissing:			return;
	case eOpTpStackTmp:			p = SB + offset;		break;		// vyraz na zasobniku
	case eOpTpLocal:			p = PB + offset;		break;		// lokalni promenna 
	case eOpTpAttribute:		p = OB + offset;		break;		// atribut
	case eOpTpGlobal:												// globalni promenna
		p = (UC*)MemoryReadI(GlobalsStart + offset * cPointerSize);
		break;		

	default:
			KerMain->Errors->LogError(eKRTECorruptedInstr);
	}

	// Mam pointer a nyni na nej zapisu celociselna data:
	if(isChar[0])
		MemoryWriteC(p, data);
	else
		MemoryWriteI(p, data);
}

myInline void CKerInterpret::Store(double data, int type, int offset)
{
	UC* p;
	if(type == cUnknown)
	{
		type = opTp[0];
		offset = instr->res.intValue;
	}

	switch(getARG(type))
	{
	case eOpTpMissing:		return;
	case eOpTpStackTmp:		p = SB + offset;		break;		// vyraz na zasobniku
	case eOpTpLocal:		p = PB + offset;		break;		// lokalni promenna 
	case eOpTpAttribute:	p = OB + offset;		break;		// atribut
	case eOpTpGlobal:															// globalni promenna
		p = (UC*)MemoryReadI(GlobalsStart + offset * cPointerSize);
		break;		
	default:
		KerMain->Errors->LogError(eKRTECorruptedInstr);
	}

	// Mam pointer a nyni na nej zapisu hodnotu typu double:
	MemoryWriteD(p, data);		
}

myInline void CKerInterpret::Step()
{ 
	IC++;			
	if(IC > IL)		// kontrola zacykleni (instruction count <= instruction limit)
		KerMain->Errors->LogError(eKRTETooLongExecution);
	IP++;
	if(IP > instructionsNr)  // kontrola vystoupeni z useku kodu
		KerMain->Errors->LogError(eKRTEInstrOutsideCode); 
	instr++;		// dalsi instrukce...
}

myInline void CKerInterpret::Skip(int numToSkip)
{ 
	IP += numToSkip;
	if(IP > instructionsNr)
		KerMain->Errors->LogError(eKRTEInstrOutsideCode); 
	instr += numToSkip; 
}

myInline int CKerInterpret::MemoryReadI(UC* addr)
{
	if(addr<CodeStart || addr+sizeof(int)>ML)	
		KerMain->Errors->LogError(eKRTEIllegalAddressRead); 
	return *((int*)addr);
}

myInline int CKerInterpret::MemoryReadC(UC* addr)
{
	if(addr<CodeStart || addr+sizeof(UC)>ML)
		KerMain->Errors->LogError(eKRTEIllegalAddressRead); 
	return *((UC*)addr);
}

myInline double CKerInterpret::MemoryReadD(UC* addr)
{
	if(addr<CodeStart || addr+sizeof(double)>ML)
		KerMain->Errors->LogError(eKRTEIllegalAddressRead); 
	return *((double*)addr);
}

myInline void CKerInterpret::MemoryWriteI(UC* addr, int value)
{
	if(addr<GlobalsStart || addr+sizeof(int)>ML)
		KerMain->Errors->LogError(eKRTEIllegalAddressWrite, IP*16, "wi"); 
	*((int*)addr) = value;
}

myInline void CKerInterpret::MemoryWriteC(UC* addr, int value)
{
	if(addr<StackStart || addr+sizeof(UC)>ML)
		KerMain->Errors->LogError(eKRTEIllegalAddressWrite, IP*16, "wc"); 
	*((UC*)addr) = value;
}

myInline void CKerInterpret::MemoryWriteD(UC* addr, double value)
{
	if(addr<StackStart || addr+sizeof(double)>ML)
		KerMain->Errors->LogError(eKRTEIllegalAddressWrite, IP*16, "wd"); 
	*((double*)addr) = value;
}

myInline void	CKerInterpret::DoJump(int addr)
{
	if(addr % cInstructionSize)		// kontrola zarovnani na 16B
		KerMain->Errors->LogError(eKRTEAddrNotAlligned); 

	instr = (CIntInstructionRec*)(CodeStart+addr);
	IP = addr / cInstructionSize;

	if(instr < CB || instr > CL)
		KerMain->Errors->LogError(eKRTEInstrOutsideCode); 
}

myInline void CKerInterpret::CheckDivisorI(int i)
{
	if(i == 0)
		KerMain->Errors->LogError(eKRTEDivideByZero);
}

myInline void CKerInterpret::CheckDivisorD(double d)
{
	if(d == 0)
		KerMain->Errors->LogError(eKRTEDivideByZero);
}

myInline void	CKerInterpret::CheckOverflow(bool stackAction, bool over)
{
	if(StackTop >= HeapBottom || StackTop < StackStart)
		KerMain->Errors->LogError(over ? eKRTEStackOverflow : eKRTEStackUnderflow); 
}

myInline void CKerInterpret::CheckPointerForRead(UC* addr, int size)
{
	if(addr < CodeStart || addr + size > ML)
		KerMain->Errors->LogError(eKRTEIllegalAddressRead); 

}

myInline void CKerInterpret::CheckPointerForWrite(UC* addr, int size)
{
	if(addr < StackStart || addr + size > ML)
		KerMain->Errors->LogError(eKRTEIllegalAddressWrite); 
}
