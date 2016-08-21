/////////////////////////////////////////////////////////////////////////////
///
///		K E R N E L
///
///		Hlavni soubor Kernelu. Load levlu a skriptu. Rizeni
///		A: Honza M.D. Krcek
///
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "kernel.h"
#include "interpret.h"
#include "KerServices.h"
#include "LevelMap.h"
#include "ks.h"
#include "dx.h"
#include "ComCompiler.h"
#include "autos.h"
#include "register.h"
#include "editor.h"
#include "music.h"
#include "levels.h"




// Promena pro praci s hlavnim objektem kernelu CKerMain:
CKerMain *KerMain=0;

// Kontext:
CKerContext *KerContext =0;






////////////////////////////////////////////////////////////////////////////////
///
///		C K e r M a i n
///
////////////////////////////////////////////////////////////////////////////////


// Vrati velikost zakladnich typu (definovanych v eKerTypes) Priznak eKTret se ignoruje.
int CKerMain::GetTypeSize(int Type) {
	Type &= (eKTret-1);
	switch (Type) {
		case eKTchar : return 1;
		case eKTdouble : return 8;
		case eKTint:
		case eKTpointer:
		case eKTobject:
		case eKTname: 
		case eKTarrChar:
		case eKTarrDouble:
		case eKTarrInt:
		case eKTarrPointer:
		case eKTarrObject:
		case eKTarrName: return 4;
		case eKTvoid: 
		case eKTstruct: return 0;
		default: return Type-100+1;
	}
}


//////////////////////////////////////////////////
// vraci pointr na promennou objektu
void *CKerMain::GetAccessToVar(OPointer obj, CKerOVar* Var) { 
	CKerObject *o = Objs->GetObject(obj);
	if (!o) return 0;
	if (!Var->KSVar) {
		assert(o->ISData);
		return o->ISData + Var->Offset;
	} else {
		assert(o->Type->SetObjectVar);
		o->Type->SetObjectVar(o->KSVG);
		return *(Var->KSVar);
	}
}


//////////////////////////////////////////////////
// vraci pointr na promennou objektu
void *CKerMain::GetAccessToVar(OPointer obj, int VarNum) { 
	CKerObject *o = Objs->GetObject(obj);
	if (!o) return 0;
	if (!o->Type->SpecificKnownVars) return 0;
	CKerOVar *Var = o->Type->SpecificKnownVars[VarNum];
	if (!Var) return 0;
	if (!Var->KSVar) {
		assert(o->ISData);
		return o->ISData + Var->Offset;
	} else {
		assert(o->Type->SetObjectVar);
		o->Type->SetObjectVar(o->KSVG);
		return *(Var->KSVar);
	}
}


//////////////////////////////////////////////////
// vraci pointr na promennou objektu
void *CKerMain::GetAccessToVar(CKerObject *o, CKerOVar* Var) { 
	if (!Var->KSVar) {
		assert(o->ISData);
		return o->ISData + Var->Offset;
	} else {
		assert(o->Type->SetObjectVar);
		o->Type->SetObjectVar(o->KSVG);
		return *(Var->KSVar);
	}
}


//////////////////////////////////////////////////
// vraci pointr na promennou objektu
void *CKerMain::GetAccessToVar(CKerObject *o, int VarNum) { 
	if (!o->Type->SpecificKnownVars) return 0;
	CKerOVar *Var = o->Type->SpecificKnownVars[VarNum];
	if (!Var) return 0;
	if (!Var->KSVar) {
		assert(o->ISData);
		return o->ISData + Var->Offset;
	} else {
		assert(o->Type->SetObjectVar);
		o->Type->SetObjectVar(o->KSVG);
		return *(Var->KSVar);
	}
}



///////////////////////////////////////////////////////////////
// Nastavi pristup k promennym objektu (pozor nici kontext!) Kdyz objekt neexistuje, vraci null
CKerObject * CKerMain::GetAccessToObject(OPointer obj) {
	CKerObject *o = Objs->GetObject(obj);
	if (o && o->Type->SetObjectVar) o->Type->SetObjectVar(o->KSVG);
	return o;
}




//////////////////////////////////////////////////////////////////////////////////////
///
///		V o l a n i   a   Z p r a v y
///
//////////////////////////////////////////////////////////////////////////////////////




// Zkonvertuje Parametr Source typu SType na parametr Dest typu DType.
// Hodnota je prekopirovana. Konverze probehne pro vsechny typy. V nekterych pripadech je ohlasena chyba eKerEGParamConversionError
void CKerMain::ConvertParam(void *Source, int SType, void *Dest, int DType) {
	SType &= (eKTret-1);
	DType &= (eKTret-1); // priznak eKTret je ignorovan
	int max,f;
	switch (DType) {
		case eKTchar: 
			switch (SType) {
				case eKTarrChar:
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName:
					*(char*)Dest = 0;
					Errors->LogError(eKRTEarrayconv);
					return;
				case eKTdouble: 
					*(char*)Dest = (char)floor(*(double*)Source + 0.5); 
					return;
				case eKTpointer:
				case eKTobject:
				case eKTname: Errors->LogError(eKRTEptrtonum);
				default: 
					*(char*)Dest = *(char*)Source; 
					return;
			}
		case eKTdouble:
			switch (SType) {
				case eKTarrChar:
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName:
					*(double*)Dest = 0;
					Errors->LogError(eKRTEarrayconv);
					return;
				case eKTdouble: *(double*)Dest = *(double*)Source; return;
				case eKTpointer:
				case eKTobject:
				case eKTname: Errors->LogError(eKRTEptrtonum);
				case eKTint: *(double*)Dest = *(int*)Source; return;
				case eKTchar: *(double*)Dest = *(char*)Source; return;
				default: *(double*)Dest = *(char*)Source; Errors->LogError(eKRTEstrtonum); return;
			}
		case eKTpointer:
		case eKTobject:
		case eKTname:
			switch (SType) {
				case eKTarrChar:
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName:
					*(int*)Dest = 0;
					Errors->LogError(eKRTEarrayconv);
					return;
				case eKTdouble: *(int*)Dest = (int)floor(*(double*)Source + 0.5); Errors->LogError(eKRTEnumtoptr); return;
				case eKTpointer:
				case eKTobject: 
				case eKTname: *(int*)Dest = *(int*)Source; if (SType!=DType) Errors->LogError(eKRTEptrconv); return;
				case eKTint: *(int*)Dest = *(int*)Source; Errors->LogError(eKRTEnumtoptr); return;
				case eKTchar: *(int*)Dest = *(char*)Source; Errors->LogError(eKRTEnumtoptr); return;
				default: *(int*)Dest = *(char*)Source; Errors->LogError(eKRTEstrtonum); return;
			}
		case eKTint:
			switch (SType) {
				case eKTarrChar:
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName:
					*(int*)Dest = 0;
					Errors->LogError(eKRTEarrayconv);
					return;
				case eKTdouble: *(int*)Dest = (int)floor(*(double*)Source + 0.5); return;
				case eKTpointer:
				case eKTobject:
				case eKTname: Errors->LogError(eKRTEptrtonum);
				case eKTint: *(int*)Dest = *(int*)Source; return;
				case eKTchar: *(int*)Dest = *(char*)Source; return;
				default: *(int*)Dest = *(char*)Source; Errors->LogError(eKRTEstrtonum); return;
			}
		case eKTarrChar:			
		case eKTarrDouble:
		case eKTarrInt:
		case eKTarrPointer:
		case eKTarrObject:
		case eKTarrName:
			if (DType==SType) {
				*(int*)Dest = *(int*)Source;
			} else {
				*(int*)Dest = 0;
				Errors->LogError(eKRTEarrayconv);
			}
			return;
		default:
			if (SType>=eKTstring) {
				max = (DType>SType?SType:DType) - eKTstring;
				for (f=0;f<max;f++) if (!( ((char*)Dest)[f] = ((char*)Source)[f] )) return;
				((char*)Dest)[max] = 0;
				return;
			}
			if (SType == eKTchar && DType > eKTstring) {
				((char*)Dest)[0] = *(char*)Source;
				((char*)Dest)[1] = 0;
			} else {
				((char*)Dest)[0] = 0;
				Errors->LogError(eKRTEnumtostr); 
			}
	}
}





//////////////////////////////////////////////////////////////////
// Funkce nahraje do neprirazenych argumentu defaultni hodnoty
// Neprerazeny argument je oznacen nulou v KerContext->ParamsInfo[f]
void CKerMain::SetDefaultParams(CKerMethod *method) {
	int f;
	for (f=0;f<method->NumP;f++) if(KerContext->ParamsInfo[f] == 0) {
		switch (method->Params[f].Type & (eKTret-1)) {
			case eKTchar: *(char*)(KerContext->Params+method->Params[f].Offset) = method->Params[f].DefaultValue.Dchar; break;
			case eKTdouble: *(double*)(KerContext->Params+method->Params[f].Offset) = method->Params[f].DefaultValue.Ddouble; break;
			case eKTint:
			case eKTpointer:
			case eKTobject:
			case eKTarrChar:			
			case eKTarrDouble:
			case eKTarrInt:
			case eKTarrPointer:
			case eKTarrObject:
			case eKTarrName:
			case eKTname: *(int*)(KerContext->Params+method->Params[f].Offset) = method->Params[f].DefaultValue.Dint; break;
			default: if (method->Params[f].DefaultValue.Dstring) strcpy((char*)(KerContext->Params+method->Params[f].Offset),method->Params[f].DefaultValue.Dstring);
					 else *(char*)(KerContext->Params+method->Params[f].Offset) = 0;
		}
	}
}





///////////////////////////////////////////////////////////////////
 // Automaticka konverze zakladniho typu. Vystup se prida k vystupu pomoci returnovaci funkce
void CKerMain::ConvertParamRet(void *Source, int SType, void *Dest, int DType, int RetFce) {
	assert(RetFce);
	if (DType==eKTchar) {
		UC a;
		ConvertParam(Source,SType,&a,eKTchar);
		if (RetFce==eKTretOR) *(UC*)Dest |= a;
		else if (RetFce==eKTretAND) *(UC*)Dest &= a;
		else /*ADD*/ *(UC*)Dest += a;
	} else if (DType==eKTint) {
		int a;
		ConvertParam(Source,SType,&a,eKTchar);
		if (RetFce==eKTretOR) *(int*)Dest |= a;
		else if (RetFce==eKTretAND) *(int*)Dest &= a;
		else /*ADD*/ *(int*)Dest += a;
	} else assert(false);
}


//////////////////////////////////////////////////////////////////////////
// vola se v pripade ze nebylo nic predano. Pokud je tam korektni retFce, preda se default, jinak nahlasi chybu ErrorNum
void CKerMain::FillRetFceDefault(void *Dest, int DType, int RetFce, int ErrorNum) {
	DType &= (eKTret-1);
	switch (RetFce) {
		case eKTretNormal: Errors->LogError(ErrorNum); break;
		case eKTretOR:
		case eKTretADD:
			if (DType == eKTint) *(int*)Dest = 0;
			else if (DType == eKTchar) *(UC*)Dest = 0;
			else Errors->LogError(ErrorNum);
			break;
		case eKTretAND:
			if (DType == eKTint) *(int*)Dest = 0xFFFFFFFF;
			else if (DType == eKTchar) *(UC*)Dest = 0xFF;
			else Errors->LogError(ErrorNum);
			break;
	}
}


struct CKerICretInfo {
	int retType;	// to co jde na vystup. Zde si pamatuju RetFci a zda uz jsem vracel
	void *retPtr;	//		pointer do RetBuffu
	int fceType;	// to co mi vratila volana funkce
	void *fcePtr;	//		pointer do volane fce
};


//////////////////////////////////////////////////////////
// PRIME SAFE VOLANI (Je mozne vracet hodnoty: navratova hodnota fce, parametry type eKTret)
// Funkce nalezne prislusne metody volaneho objektu a zavola je:
// Inicializuje se kontext, predaji se argumenty, volani, argumenty se predaji zpatky, deinit kontextu
// Navratova hodnota fce je ponechana na zasobniku
void CKerMain::IScall(OPointer Object, CKerName *Method, int NumArgs, int *ArgTypes, CKerName **ArgNames, void **ArgPointers, int RetType) {
	CKerMethod **ms; // volane fce
	CKerContext *ctx;
	int destruct=-1; // Nasatavim na 1 kdyz zjistim, ze se vola destruktor. Po zkonceni volani provedu dealokaci objektu
	int returned=0; // zda uz bylo neco vraceno
	int RetRetFce;
	void *retplace;  // kam mam na zasobnik hodit vracenou hodnotu.
	int StackPushSize=0;
	int f,a;
	CKerParam **ps;
	CKerObject *obj = Objs->GetObject(Object);
	CKerICretInfo *ArgInfos=0; // hledam parametry volane fce odpovidajici parametrum pri volani a zapamatuju si to. Pouziti: Vraceni hodnotou
	UC *ArgBuff=0;
	ScriptActivity++;

	// Vyhradim misto na zasobniku pro navratovou hodnotu fce
	RetRetFce = RetType & eKTretMask;
	RetType &= (eKTret-1);
	assert(RetType < eKTstring);
	retplace = KerInterpret->StackTop;
	KerInterpret->StackTop += GetTypeSize(RetType);
	if (KerInterpret->StackTop>=KerInterpret->HeapBottom) Errors->LogError(eKRTENoMemory);
	
	if (NumArgs) { // Vytvorym pole ArgTypes2, ArgPointers2:
		int size=0;
		for (f=0;f<NumArgs;f++) if (ArgTypes[f] >= eKTret) size+= GetTypeSize(ArgTypes[f]);
		if (size>0) {
			// pomocne pole si dam na zasobnik
			ArgBuff = KerInterpret->StackTop; 
			StackPushSize += size;
			KerInterpret->StackTop += size;
			ArgInfos = (CKerICretInfo*)KerInterpret->StackTop;
			StackPushSize += sizeof(CKerICretInfo)*NumArgs;
			KerInterpret->StackTop += sizeof(CKerICretInfo)*NumArgs;
			if (KerInterpret->StackTop>=KerInterpret->HeapBottom) Errors->LogError(eKRTENoMemory);
			size = 0;
			for (f=0;f<NumArgs;f++) {
//				ArgInfos[f].retType = eKTvoid | (ArgTypes[f] & (eKTretMask|eKTret));
				ArgInfos[f].retType = eKTvoid | (ArgTypes[f] & eKTretMask);
				ArgInfos[f].retPtr = ArgBuff + size;
				if (ArgTypes[f] >= eKTret) size+= GetTypeSize(ArgTypes[f]);
			}
		}
	}


	if (!obj || !Method) {
		if (!obj) Errors->LogError(eKRTESCnoObj); // neexistuje objekt!
		if (!Method) Errors->LogError(eKRTEBadMethod); 
	} else {

		if ((a = Method->KerPHTpos - obj->Type->MCLstart) < obj->Type->MCLstop) {
			if (a>=0 && obj->Type->MCLnames[a]==Method) {
				// Objekt ma volanou metodu(y) muzu pokracovat:
				destruct = 0;

				ms = obj->Type->MethodCL[a]; // metody, ktere budu volat
				ctx = new CKerContext; // inicializace kontextu:
				if (KerContext->parent) ctx->InitContext(obj,KerContext->parent->KCthis,Method); else ctx->InitContext(obj,0,Method);
				while (*ms) {			
					assert((**ms).Safe);
					if ((**ms).Name == Destructor) {
						if (destruct!=1) MapInfo->RemoveObjFromMap(0,Object);
						destruct = 1;
					}
					ctx->MethodName = (**ms).MethodName;
					ctx->Params = KerInterpret->StackTop; // Vyhradim pro argumenty misto na zasobniku 
					KerInterpret->StackTop += (**ms).ParamSize;
					ctx->ParamsInfo = KerInterpret->StackTop;
					KerInterpret->StackTop += (**ms).NumP;
					if (KerInterpret->StackTop>=KerInterpret->HeapBottom) Errors->LogError(eKRTENoMemory);
					ctx->SetArgumentPointers(*ms);

					// Predani Argumentu:
					for (f=0;f<(**ms).NumP;f++) KerContext->ParamsInfo[f] = 0;
					for (f=0;f<NumArgs;f++) {
						if (ArgInfos) ArgInfos[f].fceType = eKTvoid;
						if ((a = ArgNames[f]->KerPHTpos - (**ms).PCLstart) < (**ms).PCLstop) {
							if (a>=0 && (**ms).PCLnames[a] == ArgNames[f]) {
								ps = (**ms).ParamCL[a]; // <- Pole argumentu kam argument predavam
								while (*ps) {
									if (KerContext->ParamsInfo[*ps - (**ms).Params] != 0) Errors->LogError(eKRTEmenyToOne); 
									KerContext->ParamsInfo[*ps - (**ms).Params] = 1;
									if ((**ps).Type>=eKTret && ArgTypes[f]>=eKTret) { // bude se vracet hodnotou:
										int a = (**ps).Type & eKTretMask;
										if (!a) a = ArgInfos[f].retType & eKTretMask;
										else if (a != (ArgInfos[f].retType & eKTretMask)) Errors->LogError(eKRTEretTypeChanged);
										ArgInfos[f].retType = a | (ArgInfos[f].retType & eKTsubRetMask);
										if (ArgInfos[f].fceType!=eKTvoid) Errors->LogError(eKRTEretMenyToOne);
										ArgInfos[f].fceType = (**ps).Type;
										ArgInfos[f].fcePtr = KerContext->Params+(**ps).Offset;
									}
									ConvertParam(ArgPointers[f],ArgTypes[f],ctx->Params + (**ps).Offset,(**ps).Type);
									ps+=1;
								}
							}
						}
					}
					SetDefaultParams(*ms);

					// Volani:
					if (SaveReg) {
						CFSRegKey *oldk = KerSaveLoad.Open(SaveReg,SaveState,*ms);
						if ((**ms).Compiled) {
							(**ms).Function();
						} else  // predani rizeni interpretu
							KerInterpret->StartMethod(obj,(*ms));
						KerSaveLoad.Open(oldk,SaveState);
					} else {
						if ((**ms).Compiled) {
							(**ms).Function();
						} else  // predani rizeni interpretu
							KerInterpret->StartMethod(obj,(*ms));
					}

					// Vraceni argumentu:
					for (f=0;f<NumArgs;f++) if (ArgInfos && ArgInfos[f].fceType>=eKTret) {
						if ((ArgInfos[f].retType& (eKTret-1)) == eKTvoid) {
							// predavam poprve
							ConvertParam(ArgInfos[f].fcePtr,ArgInfos[f].fceType,ArgInfos[f].retPtr,ArgTypes[f]);
							ArgInfos[f].retType = (ArgInfos[f].retType & eKTretMask) | (ArgTypes[f] & (eKTret-1));
						} else {
							// predavam vicekreat
							int typ = ArgTypes[f] & (eKTret-1);
							int retFce = ArgInfos[f].retType & eKTretMask;
							if (retFce && ( typ==eKTchar || typ==eKTint)) {
								ConvertParamRet(ArgInfos[f].fcePtr,ArgInfos[f].fceType,ArgInfos[f].retPtr,typ,retFce);
							} else Errors->LogError(eKRTEretMenyToOne);
						}
					}
					// vraceni navratove hodnoty
					if (RetType!=eKTvoid && (**ms).ReturnType!=eKTvoid) {
						returned++;
						if (returned==1) {
							ConvertParam(ctx->Params,(**ms).ReturnType,retplace,RetType);
						} else {
							if (RetRetFce && (RetType==eKTchar || RetType==eKTint)) {
								ConvertParamRet(ctx->Params,(**ms).ReturnType,retplace,RetType,RetRetFce);
							} else if (returned==2) Errors->LogError(eKRTEretMenyToOne);
						}
					}
					// Uvolneni zasobniku:
					KerInterpret->StackTop -= (**ms).ParamSize + (**ms).NumP;
					ms+=1;
				}

				// Deinicializace kontextu:
				ctx->KCthis = 0;
				if (destruct) Destruct(Object, obj); // dealokace objektu, kdyz se volal destruktor
				if (ctx->startmq) RunMessages(&(ctx->startmq), &(ctx->endmq));
				delete ctx;
			}
		}

		if (destruct==-1&&(Method==Destructor||Method->Compare(Destructor)>=2)) {
			MapInfo->RemoveObjFromMap(GET_CODE_LINE,Object);
			Destruct(Object, obj); // dealokace objektu, kdyz se volal destruktor
		}
	}


	// zkontroluju zda vsechny argumenty vratily a hodnoty prekopiruju
	for (f=0;f<NumArgs;f++) if (ArgTypes[f] >= eKTret) {
		if ((ArgInfos[f].retType & (eKTret-1)) == eKTvoid) {
			FillRetFceDefault(ArgPointers[f],ArgTypes[f],ArgTypes[f] & eKTretMask,eKRTEnothingRetInArg);
		} else memcpy(ArgPointers[f],ArgInfos[f].retPtr,GetTypeSize(ArgTypes[f]));
	}

	// zkontroluju zda vratila fce
	if (!returned && RetType!=eKTvoid) {
		if (RetType == eKTchar) *(char*)retplace = (char)0;
		else if (RetType == eKTdouble) *(double*)retplace = (double)0;
		else *(int*)retplace = (int)0;
		FillRetFceDefault(retplace,RetType,RetRetFce,eKRTEfceNotReturning);
	}
	KerInterpret->StackTop -= StackPushSize;
}




/////////////////////////////////////////////////////////////////
//	Prime volani, kdyz chci vratit char.
//	fce prevede promenny pocet argumentu do formy jakou chce IScall a zavola ji
unsigned char CKerMain::callchar(int CodeLine, OPointer Object,CKerName *Method,int NumArgs, ...) { // int ArgType, CKerName *ArgName, Arg, ... 
	va_list list;
	int f;
	int *ArgTypes=0;
	CKerName **ArgNames=0;
	void **ArgPointers=0;
	int StackPushSize=0;

	SET_CODE_LINE(CodeLine);
	if (NumArgs) {
		ArgTypes = (int*)KerInterpret->StackTop;
		ArgNames = (CKerName**)(KerInterpret->StackTop + 4*NumArgs);
		ArgPointers = (void**)(KerInterpret->StackTop + 2*4*NumArgs);
		KerInterpret->StackTop += 3*4*NumArgs; StackPushSize += 3*4*NumArgs;
		if (KerInterpret->StackTop>=KerInterpret->HeapBottom) Errors->LogError(eKRTENoMemory);

		va_start( list, NumArgs ); 
		for (f=0;f<NumArgs;f++) {
			ArgTypes[f] = va_arg(list,int);
			ArgNames[f] = va_arg(list,CKerName*);
			switch (ArgTypes[f]) {
				case eKTchar: ArgPointers[f] = &va_arg(list, char); break;
				case eKTdouble: ArgPointers[f] = &va_arg(list, double); break;
				case eKTint:
				case eKTpointer:
				case eKTobject:
				case eKTname: 
				case eKTarrChar:			
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName:	ArgPointers[f] = &va_arg(list, int); break;
				default: ArgPointers[f] = va_arg(list, void*); 
			}
		}
		va_end(list);
	}
	
	IScall(Object,Method,NumArgs,ArgTypes,ArgNames,ArgPointers,eKTchar);

	KerInterpret->StackTop -= 1 + StackPushSize;
	return *(char*)(KerInterpret->StackTop+StackPushSize);
}




/////////////////////////////////////////////////////////////////
//	Prime volani, kdyz chci vratit int.
//	fce prevede promenny pocet argumentu do formy jakou chce IScall a zavola ji
int CKerMain::callint(int CodeLine, OPointer Object,CKerName *Method, int RetType, int NumArgs, ...) { // int ArgType, CKerName *ArgName, Arg, ... 
	va_list list;
	int f;
	int *ArgTypes=0;
	CKerName **ArgNames=0;
	void **ArgPointers=0;
	int StackPushSize=0;

///	assert(RetType==eKTint||RetType==eKTpointer||RetType==eKTobject||RetType==eKTname||RetType>=eKTarrChar&&RetType<eKTstring);

	SET_CODE_LINE(CodeLine);
	if (NumArgs) {
		ArgTypes = (int*)KerInterpret->StackTop;
		ArgNames = (CKerName**)(KerInterpret->StackTop + 4*NumArgs);
		ArgPointers = (void**)(KerInterpret->StackTop + 2*4*NumArgs);
		KerInterpret->StackTop += 3*4*NumArgs; StackPushSize += 3*4*NumArgs;
		if (KerInterpret->StackTop>=KerInterpret->HeapBottom) Errors->LogError(eKRTENoMemory);

		va_start( list, NumArgs ); 
		for (f=0;f<NumArgs;f++) {
			ArgTypes[f] = va_arg(list,int);
			ArgNames[f] = va_arg(list,CKerName*);
			switch (ArgTypes[f]) {
				case eKTchar: ArgPointers[f] = &va_arg(list, char); break;
				case eKTdouble: ArgPointers[f] = &va_arg(list, double); break;
				case eKTint:
				case eKTpointer:
				case eKTobject:
				case eKTname: 		
				case eKTarrChar:			
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName:	ArgPointers[f] = &va_arg(list, int); break;
				default: ArgPointers[f] = va_arg(list, void*); 
			}
		}
		va_end(list);
	}

	IScall(Object,Method,NumArgs,ArgTypes,ArgNames,ArgPointers,RetType);

	KerInterpret->StackTop -= 4+StackPushSize;
	return *(int*)(KerInterpret->StackTop+StackPushSize);
}




/////////////////////////////////////////////////////////////////
//	Prime volani, kdyz chci vratit double.
//	fce prevede promenny pocet argumentu do formy jakou chce IScall a zavola ji
double CKerMain::calldouble(int CodeLine, OPointer Object,CKerName *Method,int NumArgs, ...) { // int ArgType, CKerName *ArgName, Arg, ... 
	va_list list;
	int f;
	int *ArgTypes=0;
	CKerName **ArgNames=0;
	void **ArgPointers=0;
	int StackPushSize=0;

	SET_CODE_LINE(CodeLine);
	if (NumArgs) {
		ArgTypes = (int*)KerInterpret->StackTop;
		ArgNames = (CKerName**)(KerInterpret->StackTop + 4*NumArgs);
		ArgPointers = (void**)(KerInterpret->StackTop + 2*4*NumArgs);
		KerInterpret->StackTop += 3*4*NumArgs; StackPushSize += 3*4*NumArgs;
		if (KerInterpret->StackTop>=KerInterpret->HeapBottom) Errors->LogError(eKRTENoMemory);

		va_start( list, NumArgs ); 
		for (f=0;f<NumArgs;f++) {
			ArgTypes[f] = va_arg(list,int);
			ArgNames[f] = va_arg(list,CKerName*);
			switch (ArgTypes[f]) {
				case eKTchar: ArgPointers[f] = &va_arg(list, char); break;
				case eKTdouble: ArgPointers[f] = &va_arg(list, double); break;
				case eKTint:
				case eKTpointer:
				case eKTobject:
				case eKTname: 		
				case eKTarrChar:			
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName:	ArgPointers[f] = &va_arg(list, int); break;
				default: ArgPointers[f] = va_arg(list, void*); 
			}
		}
		va_end(list);
	}

	IScall(Object,Method,NumArgs,ArgTypes,ArgNames,ArgPointers,eKTdouble);

	KerInterpret->StackTop -= 8+StackPushSize;
	return *(double*)(KerInterpret->StackTop+StackPushSize);
}




/////////////////////////////////////////////////////////////////
//	Prime volani, kdyz chci vratit void.
//	fce prevede promenny pocet argumentu do formy jakou chce IScall a zavola ji
void CKerMain::call(int CodeLine, OPointer Object,CKerName *Method,int NumArgs, ...) { // int ArgType, CKerName *ArgName, Arg, ... 
	va_list list;
	int f;
	int *ArgTypes=0;
	CKerName **ArgNames=0;
	void **ArgPointers=0;
	int StackPushSize=0;

	SET_CODE_LINE(CodeLine);
	if (NumArgs) {
		ArgTypes = (int*)KerInterpret->StackTop;
		ArgNames = (CKerName**)(KerInterpret->StackTop + 4*NumArgs);
		ArgPointers = (void**)(KerInterpret->StackTop + 2*4*NumArgs);
		KerInterpret->StackTop += 3*4*NumArgs; StackPushSize += 3*4*NumArgs;
		if (KerInterpret->StackTop>=KerInterpret->HeapBottom) Errors->LogError(eKRTENoMemory);

		va_start( list, NumArgs ); 
		for (f=0;f<NumArgs;f++) {
			ArgTypes[f] = va_arg(list,int);
			ArgNames[f] = va_arg(list,CKerName*);
			switch (ArgTypes[f]) {
				case eKTchar: ArgPointers[f] = &va_arg(list, char); break;
				case eKTdouble: ArgPointers[f] = &va_arg(list, double); break;
				case eKTint:
				case eKTpointer:
				case eKTobject:
				case eKTname: 		
				case eKTarrChar:			
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName:	ArgPointers[f] = &va_arg(list, int); break;
				default: ArgPointers[f] = va_arg(list, void*); 
			}
		}
		va_end(list);
	}

	IScall(Object,Method,NumArgs,ArgTypes,ArgNames,ArgPointers,eKTvoid);

	KerInterpret->StackTop -= StackPushSize;
}





////////////////////////////////////////////////////////////////////////////
// Pomocna fce, ktera preda argumenty ze zpravy do volane metody
void CKerMain::PassParams(CKerMethod *method, CKerMessage *msg) {
	int f,a;
	int Offset=0; // pozice v argumentech
	CKerParam **ps;

	for (f=0;f<method->NumP;f++) KerContext->ParamsInfo[f] = 0; 
	for (f=0;f<msg->NumArgs;f++) {
		if ((a = msg->ArgNames[f]->KerPHTpos - method->PCLstart) < method->PCLstop) {
			if (a>=0 && method->PCLnames[a] == msg->ArgNames[f]) {
				ps = method->ParamCL[a]; // <- Pole argumentu kam argument predavam
				while (*ps) {
					if (KerContext->ParamsInfo[*ps - method->Params] != 0) Errors->LogError(eKRTEmenyToOne);
					KerContext->ParamsInfo[*ps - method->Params] = 1;
					ConvertParam(msg->Args+Offset,msg->ArgTypes[f],KerContext->Params+(**ps).Offset,(**ps).Type);
					ps+=1;
				}
			}
		}
		Offset += GetTypeSize(msg->ArgTypes[f]);
	}
	SetDefaultParams(method);
}



//////////////////////////////////////////////////////////////////////////
//	 PROVEDENI ZPRAV
//	fce projde frontu zprav, vyzvedava zpravy a provadi je (vola prislusne metody, predava jim argumenty)
void CKerMain::RunMessages(CKerMessage **startmq, CKerMessage **endmq) {
	CKerMessage *msg;  // provadena zprava
	CKerContext *ctx;
	CKerObject *obj;
	CKerMethod **ms;	// volane fce
	int a;
	int destruct;		// Nasatavim na 1 kdyz zjistim, ze se vola destruktor. Po zkonceni volani provedu dealokaci objektu

	ctx = new CKerContext(0); // vytvorim kontext
	while (*startmq) {
		msg = *startmq;  // vyzvednu zpravu z fronty
		*startmq = msg->next;
		if (*startmq==0) *endmq=0;
		ScriptActivity++;

		try {
			if (obj=Objs->GetObject(msg->Receiver)) { // pokud existuje Receiver
				destruct = -1;
				if ((a = msg->Method->KerPHTpos - obj->Type->MCLstart) < obj->Type->MCLstop) {
					if (a>=0 && obj->Type->MCLnames[a]==msg->Method) {
						// Objekt ma volanou metodu(y) muzu pokracovat:
						destruct = 0;
						ms = obj->Type->MethodCL[a]; // <- metody, ktere budu volat
						// inicializace kontext:
						ctx->InitContext(obj,msg->Sender,msg->Method);
						ctx->MsgMethodName = msg->MethodName;
						ctx->msgline = msg->CodeLine;
						while (*ms) {			
							assert((**ms).Safe);
							if ((**ms).Name == Destructor) {
								if (destruct==0) MapInfo->RemoveObjFromMap(0,msg->Receiver);
								destruct = 1;
							}
							ctx->MethodName = (**ms).MethodName;
							// Vyhradim pro argumenty misto na zasobniku :
							ctx->Params = KerInterpret->StackTop;
							KerInterpret->StackTop += (**ms).ParamSize;
							ctx->ParamsInfo = KerInterpret->StackTop;
							KerInterpret->StackTop += (**ms).NumP;
							ctx->SetArgumentPointers(*ms);
							if (KerInterpret->StackTop>=KerInterpret->HeapBottom) Errors->LogError(eKRTENoMemory);
							// Predani argumentu:
							if ((**ms).NumP) PassParams(*ms,msg);
							
							// Volani:
							if (SaveReg) {
								CFSRegKey *oldk = KerSaveLoad.Open(SaveReg,SaveState,*ms);
								if ((**ms).Compiled) {
									(**ms).Function();
								} else  // predani rizeni interpretu
									KerInterpret->StartMethod(obj,(*ms));
								KerSaveLoad.Open(oldk,SaveState);
							} else {
								if ((**ms).Compiled) {
									(**ms).Function();
								} else  // predani rizeni interpretu
									KerInterpret->StartMethod(obj,(*ms));
							}

							// Uvolneni zasobniku:
							KerInterpret->StackTop -= (**ms).ParamSize + (**ms).NumP;
							ms+=1;
						}
						// Deinicializace kontextu:
						ctx->KCthis = 0;
						if (destruct) Destruct(msg->Receiver, obj); // dealokace objektu, kdyz se volal destruktor
						if (ctx->startmq) RunMessages(&(ctx->startmq), &(ctx->endmq));
					}
				} 
				if (destruct==-1&&(msg->Method==Destructor||msg->Method->Compare(Destructor)>=2)) {
					ctx->InitContext(obj,msg->Sender,msg->Method);
					ctx->MsgMethodName = msg->MethodName;
					ctx->msgline = msg->CodeLine;
					ctx->KCthis = 0;
					MapInfo->RemoveObjFromMap(0,msg->Receiver);
					Destruct(msg->Receiver, obj); // dealokace objektu, kdyz se volal destruktor
				}
			}
		} catch(CKernelPanic) {
			delete msg;
			throw;
		}

		delete msg;
	}
	delete ctx;
}




///////////////////////////////////////////////////////////////////
// Vytvoreni noveho objektu daneho typu (dojde k alokaci pameti a volani konstruktoru)
OPointer CKerMain::NewObject(int CodeLine, CKerName *ObjType) {
	SET_CODE_LINE(CodeLine); OPointer ptr = (new CKerObject(ObjType))->thisO; 
	call(CodeLine,ptr,Constructor,0); return ptr; 
}



///////////////////////////////////////////////////////////////////
// vytvori objekt, zkopiruje do nej data, zavola copyconstructor, ktery by mel sparvne nastavit, zkopirovat pointrove retezce
OPointer CKerMain::CopyObject(int CodeLine, OPointer object) {
	SET_CODE_LINE(CodeLine);
	CKerObject* obj1 = Objs->GetObject(object);
	CKerObject* obj2;
	if (!obj1) {
		Errors->LogError(eKRTECOnoObj);
		return 0;
	}
	CKerObjectT *typ = obj1->Type;
	obj2 = new CKerObject(typ->Name);
	// kopiruju data:
	if (obj1->ISData) memcpy(obj2->ISData,obj1->ISData,typ->ISDataSize);
	if (obj1->KSVG) memcpy(obj2->KSVG,obj1->KSVG,typ->KSVGsize);
	call(CodeLine,obj2->thisO,CopyConstructor,0); // zavolam copy constructor
	return obj2->thisO;
}


///////////////////////////////////////////////////////////////////
// Dealokace objektu
void CKerMain::Destruct(OPointer ObjPtr, CKerObject *obj) {
	CKerContext *ctx = KerContext;
	while (ctx) {
		if (ctx->KCthis == ObjPtr) Errors->LogError(eKRTEDelObjInUse);
		ctx = ctx->parent;
	}
	delete obj;
}





//////////////////////////////////////////////////////////////
// Smazani vsech zprav z fronty. Zpravy se neprovedou.
void CKerMain::DeleteMessages(CKerMessage **startmq, CKerMessage **endmq) {
	CKerMessage *msg = *startmq, *msg2;
	while(msg) {
		msg2 = msg;
		msg = msg->next;
		delete msg2;
	}
	*startmq=0; *endmq=0;
}




////////////////////////////////////////////////////////////////////
///  RUN TURN
///  Entry Point. Provede se jedno kolo. Volat opakovane kazde kolo.
void CKerMain::RunTurn(int time, int krize) {
	_RunTurn = 1;
	ScriptActivity = 0;
	if (Time > (UI)(Time+time)) Errors->LogError(eKRTEOutOfTime);
	Time += time;
	TimeCrisis = krize;
	PopTimedMessages();  // vyzvednu casovane zpravy
	MapInfo->MoveMovingObjs(); // provedu pohyby
	// Provedu zpravy pro toto kolo.
	do {
		if (startmq[0]) RunMessages(startmq,endmq); // Provedu normalni zpravy
		startmq[0]=startmq[1]; startmq[1]=0;  // Dam zpravy z konce kola do normalnich zprav
		endmq[0]=endmq[1]; endmq[1]=0;
	} while (startmq[0]); // dokud jsou v tomto kole nejake zpravy
	// dam zpravy pro dalsi kolo do tohoto kola:
	startmq[0]=startmq[2]; startmq[2]=0;
	endmq[0]=endmq[2]; endmq[2]=0;
	startmq[1]=startmq[3]; startmq[3]=0;
	endmq[1]=endmq[3]; endmq[3]=0;
	// Podivam se jestli nemam ukoncit Kernel
	if (_TerminateKernel) {
		Errors->LogError(eKRTEKernelEnds);
		DebugMessage(1,0xFFFF0000,"K:END");
		throw CKernelPanic();
	}
	if (_SaveGame) {
		_SaveGame = 0;
		GSaveGame();
	}
	DebugMessage(0,0xFFFF8800,"SA:%03i",ScriptActivity);
	_RunTurn = 0;
}




///////////////////////////////////////////////////////
//	Prida casovane zpravy na ktere uz prisel cas od aktualni fronty zprav.
void CKerMain::PopTimedMessages() {
	CKerMessage *msg1=timedmsgs, *msg2=0;
	while (msg1 && msg1->Time <= Time) { msg2 = msg1; msg1 = msg1->next; }
	if (msg2) {
		msg2->next = 0;
		if (endmq[0]) endmq[0]->next = timedmsgs; else startmq[0] = timedmsgs;
		endmq[0] = msg2;
		timedmsgs = msg1;
	}
}



/////////////////////////////////////////////////////////////////////
/// VYTVORENI ZPRAVY
/// Argumenty jsou zkopirovany do zpravy.
void CKerMain::message(int CodeLine, OPointer Object, CKerName *Method, int when, OPointer callendObject, int NumArgs, ...) { // int ArgType, ..., CKerName *ArgName, ..., void Arg, ...
	int f,g,a=0;
	char *str;

	SET_CODE_LINE(CodeLine);
	if (!Method) return;
	CKerMessage *msg = new CKerMessage(when, callendObject); // Vytvorim zpravu. Pridam ji do patricne fronty zprav (podle when)
	// redirekce nekterych zprav:
	if (Method == KnownNames[eKKNtriggerOn].Name || Method == KnownNames[eKKNtriggerOff].Name) {
		void *var = GetAccessToVar(Object,eKVUmsgRedirect);
		if (var && *(OPointer*)var) Object = *(OPointer*)var;
		REINIT_CONTEXT;
	}
	msg->Receiver = Object;
	msg->Method = Method;
	if (NumArgs) {
		// Okopirovani Argumentu:
		msg->NumArgs = NumArgs;
		msg->ArgTypes = new int[NumArgs];
		msg->ArgNames = new CKerName*[NumArgs];
		va_list list;
		va_start( list, NumArgs ); 
		for (f=0;f<NumArgs;f++) { // prectu typy argumentu
			msg->ArgTypes[f] = va_arg( list, int);
			assert (msg->ArgTypes[f]<eKTret);
			a+=GetTypeSize(msg->ArgTypes[f]); // pocitam celkovou velikost argumentu
		}
		for (f=0;f<NumArgs;f++) {
			msg->ArgNames[f] = va_arg( list, CKerName*); // prectu jmena argumentu
		}
		msg->Args = new char[a]; // vytvorim pole pro argumenty
		a=0;
		// okopiruju argumenty:
		for (f=0;f<NumArgs;f++) {
			switch (msg->ArgTypes[f]) {
				case eKTchar : *(char*)(msg->Args+a) = va_arg(list, char); a+=1; break;
				case eKTdouble : *(double*)(msg->Args+a) = va_arg(list, double); a+=8; break;
				case eKTint:
				case eKTpointer:
				case eKTobject:
				case eKTname: *(int*)(msg->Args+a) = va_arg(list, int); a+=4; break;
				case eKTarrChar:			
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName: 
					assert(false);
					va_arg(list, int);
					*(int*)(msg->Args+a) = 0; a+=4;
					break;
				default: str = va_arg(list, char*); for (g=a;g<a+msg->ArgTypes[f]-100+1;g++) msg->Args[g] = str[g-a]; a=g;
			}
		}
		va_end( list );
	}
}




/////////////////////////////////////////////////////////////////////
/// VYTVORENI ZPRAVY
/// Argumenty jsou zkopirovany do zpravy.
void CKerMain::ISmessage(OPointer Object, CKerName *Method, int when, OPointer callendObject, int NumArgs, int *ArgTypes, CKerName **ArgNames, void **ArgPointers) {
	int f,g,a=0;
	if (!Method) return;
	CKerMessage *msg = new CKerMessage(when, callendObject); // Vytvorim zpravu. Pridam ji do patricne fronty zprav (podle when)
	// redirekce nekterych zprav:
	if (Method == KnownNames[eKKNtriggerOn].Name || Method == KnownNames[eKKNtriggerOff].Name) {
		void *var = GetAccessToVar(Object,eKVUmsgRedirect);
		if (var && *(OPointer*)var) Object = *(OPointer*)var;
		REINIT_CONTEXT;
	}
	msg->Receiver = Object;
	msg->Method = Method;
	if (NumArgs) {
		// Okopiruju argumenty:
		msg->NumArgs = NumArgs;
		msg->ArgTypes = new int[NumArgs];
		msg->ArgNames = new CKerName*[NumArgs];
		for (f=0;f<NumArgs;f++) {
			msg->ArgTypes[f] = ArgTypes[f]; // nactu typy argumentu
			msg->ArgNames[f] = ArgNames[f]; // nactu jmena argumentu
			assert (ArgTypes[f]<eKTret);
			a+=GetTypeSize(ArgTypes[f]);	// spocitam celkovou velikost argumentu
		}
		msg->Args = new char[a];  // Vytvorim pole pro argumenty
		a=0;
		for (f=0;f<NumArgs;f++) {
			// a nakopiruju do nej samotne argumenty:
			switch (ArgTypes[f]) {
				case eKTchar : *(char*)(msg->Args+a) = *(char*)(ArgPointers[f]); a+=1; break;
				case eKTdouble : *(double*)(msg->Args+a) = *(double*)(ArgPointers[f]); a+=8; break;
				case eKTint:
				case eKTpointer:
				case eKTobject:
				case eKTname: *(int*)(msg->Args+a) = *(int*)(ArgPointers[f]); a+=4; break;
				case eKTarrChar:			
				case eKTarrDouble:
				case eKTarrInt:
				case eKTarrPointer:
				case eKTarrObject:
				case eKTarrName: assert(false); *(int*)(msg->Args+a) = 0; a+=4; break;
				default: for (g=a;g<a+ArgTypes[f]-100+1;g++) msg->Args[g] = ((char*)(ArgPointers[f]))[g-a]; a=g;
			}
		}
	}
}



///////////////////////////////////////////////////////////////////////////
// Nasledujici funkce najdou vsechny zpravy ( ve vsech frontach), ktere odpovidaji popisu. Pokud nektery parametr je zadan jako null, tak na tomto parametru nezalezi, vyhleda se vse
//////////////////////////////////////////////////////////////////////////

// Funkce vyhleda a smaze zpravy ze vsech front. Vrati pocet smazanych zprav.
int CKerMain::FDeleteMessages(OPointer Reciever, CKerName *msg, OPointer Sender) {
	CKerContext *ctx = KerContext;
	int Count = 0;
	int f;
	while (ctx) { // callend
		Count += FindCountDeleteMessages(&ctx->startmq,&ctx->endmq,Reciever,msg,Sender,1);
		ctx = ctx->parent;
	}
	for (f=0; f<4; f++) { // zakladni fronty
		Count += FindCountDeleteMessages(startmq+f,endmq+f,Reciever,msg,Sender,1);
	}
	Count += FindCountDeleteMessages(&timedmsgs,0,Reciever,msg,Sender,1);  // timed
	return Count;
}


// Funkce spocita zpravy ve vsech frontach (vraci pocet).
int CKerMain::FCountMessages(OPointer Reciever, CKerName *msg, OPointer Sender) {
	CKerContext *ctx = KerContext;
	int Count = 0;
	int f;
	while (ctx) { // callend
		Count += FindCountDeleteMessages(&ctx->startmq,&ctx->endmq,Reciever,msg,Sender);
		ctx = ctx->parent;
	}
	for (f=0; f<4; f++) { // zakladni fronty
		Count += FindCountDeleteMessages(startmq+f,endmq+f,Reciever,msg,Sender);
	}
	Count += FindCountDeleteMessages(&timedmsgs,0,Reciever,msg,Sender);  // timed
	return Count;
}


// Vyhleda zadane zpravy ve fronte, spocita je a pripadne smaze
int CKerMain::FindCountDeleteMessages(CKerMessage **startmq, CKerMessage **endmq,OPointer Reciever, CKerName *msg, OPointer Sender, int Delete ) {
	CKerMessage *m, *m2=0;
	int count = 0;
	while (*startmq) {
		m = *startmq;
		if ((!Reciever || m->Receiver == Reciever) && (!msg || m->Method == msg) && (!Sender || m->Sender == Sender)) {
			count ++;
			if (Delete) {
				*startmq = m->next;
				if (endmq && !m->next) *endmq = m2;
				delete m;
			} else {
				m2 = m; // v m2 je zprava navstivena v predchozim kroku
				startmq = &m->next;
			}
		} else {
			m2 = m; // v m2 je zprava navstivena v predchozim kroku
			startmq = &m->next;
		}
	}
	return count;
}




//////////////////////////////////////////////////////////////////////////////////
///
///		L o a d   a   I n i t   K e r n e l u
///
///////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////
// Zjisti jmeno zdrojoveho souboru a provede rekompilace. Vraci 0 v pripade chyby
int KerRecompileScripts(int scriptonly, char *load) {
	CFSRegister *reg;
	CFSRegKey *k;
	int ret;
	
	if (!scriptonly) {
		FS->ChangeDir("$GAME$");
		reg = new CFSRegister(load,"KRKAL LEVEL");
		if (!(k=reg->FindKey("Scripts File"))) {FS->ChangeDir("$KRKAL$"); delete reg; return 0;}
		FS->ChangeDirByFile(load);
		ret = compiler->TestComp(k->GetDirectAccess());
		delete reg;
	} else {
		ret = compiler->TestComp(load);
	}
	FS->ChangeDir("$KRKAL$");
	if (ret==0) return 1; else return 0;
}


///////////////////////////////////////////
// Vytvoreni a naloadovani kernelu. Provadi se pripadne rekompilace skriptu
void KerCreateKernel(int runmode, int debugmode, int scriptonly, char *load) {
	int err,comp;

	DebugMessage(0,0xFFFFFFFF,"XXX");
	new CKerMain(runmode, debugmode);
	if (scriptonly) KerMain->LoadScriptOnly(load);
	else KerMain->LoadLevel(load);
	err = KerMain->GetOpenError();
	DebugMessage(1,0xFFFFFFFF,"K:LE:%i",err);
	if (err) {
		if (err==eKerOLEOpeningLevel) KerMain->Errors->LogError(eKRTEPELoadingLevel,1); else {
			delete KerMain;
			comp = KerRecompileScripts(scriptonly,load);
			new CKerMain(runmode, debugmode);
//			if (err==eKerOLEOpeningSCode) KerMain->Errors->LogError(eKRTEELoadingScripts,1);
			KerMain->Errors->LogError(eKRTECompilating);
			if (!comp) KerMain->Errors->LogError(eKRTECompilationFailed);
			if (scriptonly) KerMain->LoadScriptOnly(load);
			else KerMain->LoadLevel(load);
			err = KerMain->GetOpenError();
			DebugMessage(1,0xFFFFFFFF,"K:LE:%i",err);
			if (err) {
				if (err == eKerOLEOpeningLevel) KerMain->Errors->LogError(eKRTEPELoadingLevel,2);
				else if (err == eKerOLEOpeningSCode) KerMain->Errors->LogError(eKRTEPELoadingScripts,2);
				else KerMain->Errors->LogError(eKRTEPanic);
			}
		}
	}
}



///////////////////////////////////////////
// Kdyz se nahrava ulozena hra
// Vytvoreni a naloadovani kernelu. Provadi se pripadne rekompilace skriptu
// vraci 1 (uspech), 0 - neuspech, level se nepodarilo nahrat, kernel nevytvoren, 
// nebo vyhodi vyjimku KernelPanic, kernel vytvoren
int KerLoadGameAndKernel(char *load, CFSRegister *SavedGame) {
	int err=0,comp;
	CFSRegister *lev;
	CFSRegKey *k, *sf, *lf;
	int rmode, dmode;

	DebugMessage(0,0xFFFFFFFF,"XXX");
	FS->ChangeDir("$GAME$");
	if (!SavedGame)	lev = new CFSRegister(load,"KRKAL SaveGame");
	else lev = SavedGame;
	lev->SeekAllTo0();
	if (lev->GetOpenError()==FSREGOK) {
		if (k = lev->FindKey("Run Mode")) rmode = k->readi(); else err = 1;
		if (k = lev->FindKey("Debug Mode")) dmode = k->readi(); else err = 1;
		if (!(sf=lev->FindKey("Scripts File"))) err = 1;
		if (!(lf=lev->FindKey("Level File"))) err = 1;
	} else err=1;
	if (err) {
		FS->ChangeDir("$KRKAL$");
		DebugMessage(1,0xFFFF0000,"K:FileE");
		if (!SavedGame)	delete lev;
		return 0;
	}

	try {
		new CKerMain(rmode, dmode);
		KerMain->Errors->LogError(eKRTEloadingGame,0,load);
		KerMain->LoadGame(lev,lf,sf);
		err = KerMain->GetOpenError();
		DebugMessage(1,0xFFFFFFFF,"K:LE:%i",err);
		if (err) {
			if (err==eKerOLEOpeningLevel) KerMain->Errors->LogError(eKRTEPELoadingLevel,1); else {
				delete KerMain;
				FS->ChangeDir("$GAME$");
				lev->SeekAllTo0();
				FS->ChangeDirByFile(lf->GetDirectAccess());
				comp = compiler->TestComp(sf->GetDirectAccess());
				new CKerMain(rmode, dmode);
				KerMain->Errors->LogError(eKRTECompilating);
				if (comp) KerMain->Errors->LogError(eKRTECompilationFailed);
				KerMain->Errors->LogError(eKRTEloadingGame,0,load);
				KerMain->LoadGame(lev,lf,sf);
				err = KerMain->GetOpenError();
				DebugMessage(1,0xFFFFFFFF,"K:LE:%i",err);
				if (err) {
					if (err == eKerOLEOpeningLevel) KerMain->Errors->LogError(eKRTEPELoadingLevel,2);
					else if (err == eKerOLEOpeningSCode) KerMain->Errors->LogError(eKRTEPELoadingScripts,2);
					else KerMain->Errors->LogError(eKRTEPanic);
				}
			}
		}
	} catch (CKernelPanic) {
		FS->ChangeDir("$KRKAL$");
		if (!SavedGame)	delete lev;
		throw;
	}

	FS->ChangeDir("$KRKAL$");
	if (!SavedGame)	delete lev;
	return 1;

}



CKerMain::CKerMain(int runmode, int debugmode) {
	// Inicalizace glob. promenne KerMain
	if (KerMain) throw  CExc(eKernel,0,"Error - Pokus Vytvorit Objekt KerMain dvakrat!");
	KerMain=this;

	startmq[0] = 0; startmq[1] = 0; startmq[2] = 0; startmq[3] = 0;
	endmq[0] = 0; endmq[1] = 0; endmq[2] = 0; endmq[3] = 0;
	timedmsgs = 0;
	GlobalVar = 0;
	NumGV = 0;
	SpecificKnownVars = 0;
	RunMode = runmode;
	DebugMode = debugmode;
	Time = 0;
	TimeCrisis = 0;
	OpenError=0; 
	SaveState = eSLSnothing;
	SaveReg = 0;
	_TerminateKernel = 0;
	_SaveGame = 0;
	_FileToSaveGame = 0;
	_RunTurn = 0;
	_message_counter = 0; _message_counter_warning = 0;
	_call_stack_counter = 0; _call_stack_counter_warning = 0;
	StaticObjectsCount = 0;
	Constructor=0; CopyConstructor=0; Destructor=0; LoadConstructor=0;

	Errors=0; KerNamesMain=0; KerInterpret=0;
	Objs = 0; MapInfo=0;

	// Inicializace dalsich casti kernelu:
	Errors = new CKerErrors(DebugMode);
	KerNamesMain = new CKerNamesMain();
	new CKerInterpret();
	Objs = new CKerObjs();
	MapInfo = new CKerMapInfo();
	AutosMain = new CKerAutosMain();
	KerServices.KernelStartInit();
}


// Nahraje ulozenou hru  - volat po konstruktoru, uz muzou vznikat vyjimky CKernalPanic
int CKerMain::LoadGame(CFSRegister *lev, CFSRegKey *LevelFile, CFSRegKey *ScriptsFile) {
	SaveState = eSLSloadGame;

	LevelInfo.LevelFile = newstrdup(LevelFile->GetDirectAccess());
	FS->ChangeDirByFile(LevelInfo.LevelFile);
	if (!LoadLevel2(lev)) OpenError=eKerOLEOpeningLevel; 

	SaveState = eSLSnothing;
	if (OpenError) return 0;	
	Errors->LogError(eKRTELoadComplete);
	Objs->LOShakeOffSize = Objs->LoadedObjects.GetCount();
	return 1;
}


// LOAD LEVLU
void CKerMain::LoadLevel(char *level) {
	CFSRegister *lev;
	SaveState = eSLSloadConstructor;

	Errors->LogError(eKRTELoadingLevel,0,level);
	LevelInfo.LevelFile = newstrdup(level);
	FS->ChangeDir("$GAME$");
	lev = new CFSRegister(level,"KRKAL LEVEL");
	if (lev->GetOpenError()==FSREGOK) FS->ChangeDirByFile(level);
	try {
		if (!LoadLevel2(lev)) OpenError=eKerOLEOpeningLevel; 
	} catch (CKernelPanic) {
		delete lev;
		FS->ChangeDir("$KRKAL$");
		throw;
	}
	FS->ChangeDir("$KRKAL$");
	SaveState = eSLSnothing;
	delete lev;
	if (OpenError) return;
	
//	level[34+20] = 0;
//	Objs->KernelDump(level+34);
	Errors->LogError(eKRTELoadComplete);
	Objs->LOShakeOffSize = Objs->LoadedObjects.GetCount();
}



// LOAD JEN SKRIPTU
void CKerMain::LoadScriptOnly(char *script) {
	CFSRegister *code;
	// Load scriptu:
	Errors->LogError(eKRTELoadingScript,0,script);
	LevelInfo.ScriptsFile = newstrdup(script);
	code = new CFSRegister(GetCodeName(script),"KRKAL SCRIPT CODE");
	if (code->GetOpenError()==FSREGOK) FS->ChangeDirByFile(script);
	try {
		if (!LoadScriptsCode(code)) OpenError=eKerOLEOpeningSCode;
	} catch (CKernelPanic) {
		delete code;
		throw;
	}
	FS->ChangeDir("$KRKAL$");
	delete code;
	if (OpenError) return;

	GetReadyToStart();

	InitGlobalsNoL(); // Inituje globalni promenne na 0, vytvori staticke objekty

	Errors->LogError(eKRTELoadComplete);
	Objs->LOShakeOffSize = Objs->LoadedObjects.GetCount();
}



// Inituje globalni promenne na 0, vytvori staticke objekty
void CKerMain::InitGlobalsNoL() {
	int f,g;
	for (f=0;f<NumGV;f++) for (g=0; g<GlobalVar[f].ArraySize; g++) {
		switch (GlobalVar[f].Type) {
			case eKTdouble: (*(double**)GlobalVar[f].KSVar)[g] = 0; break;
			case eKTint:
			case eKTname:
			case eKTpointer:
			case eKTobject: (*(int**)GlobalVar[f].KSVar)[g] = 0; break;
			case eKTarrChar: (*(CKerArrChar***)GlobalVar[f].KSVar)[g] = new CKerArrChar(); break;
			case eKTarrDouble: (*(CKerArrDouble***)GlobalVar[f].KSVar)[g] = new CKerArrDouble(); break;
			case eKTarrInt: (*(CKerArrInt***)GlobalVar[f].KSVar)[g] = new CKerArrInt(); break;
			case eKTarrPointer: (*(CKerArrPointer***)GlobalVar[f].KSVar)[g] = new CKerArrPointer(); break;
			case eKTarrObject: (*(CKerArrObject***)GlobalVar[f].KSVar)[g] = new CKerArrObject(); break;
			case eKTarrName: (*(CKerArrName***)GlobalVar[f].KSVar)[g] = new CKerArrName(); break;
			case eKTvoid: 
			case eKTstruct:	break;
			case eKTchar: (*(char**)GlobalVar[f].KSVar)[g] = 0; break;
			default: **((char**)GlobalVar[f].KSVar) = 0; if (GlobalVar[f].ArraySize>1) throw;
		}
	}
	for (f=0;f<NumGV;f++)  for (g=0; g<GlobalVar[f].ArraySize; g++) {
		if (GlobalVar[f].Type==eKTobject && GlobalVar[f].Name) {
			StaticObjectsCount++;
			OPointer ptr = (new CKerObject(GlobalVar[f].Name))->thisO;
			(*(OPointer**)GlobalVar[f].KSVar)[g] = ptr;
			Objs->LoadedObjects.Add(ptr);
			call(0,ptr,Constructor,0);
		}
	}
	if (!MapInfo->registered) Errors->LogError(eKRTEmapNotRegistered);
}



// Nahraje hodnoty globalnich promennych z levlu, 1 OK, 0 chyba 
int CKerMain::LLoadGlobals(CFSRegKey *r) {
	int f,g;
	for (f=0;f<NumGV;f++) {
		// ToDo zefektivnit (az se bude delat bezpecnostni politika), vznikaji memory leaky
		for (g=0; g<GlobalVar[f].ArraySize; g++) {
			switch (GlobalVar[f].Type) {
				case eKTdouble: (*(double**)GlobalVar[f].KSVar)[g] = 0; break;
				case eKTint:
				case eKTname:
				case eKTpointer:
				case eKTobject: (*(int**)GlobalVar[f].KSVar)[g] = 0; break;
				case eKTarrChar: (*(CKerArrChar***)GlobalVar[f].KSVar)[g] = new CKerArrChar(); break;
				case eKTarrDouble: (*(CKerArrDouble***)GlobalVar[f].KSVar)[g] = new CKerArrDouble(); break;
				case eKTarrInt: (*(CKerArrInt***)GlobalVar[f].KSVar)[g] = new CKerArrInt(); break;
				case eKTarrPointer: (*(CKerArrPointer***)GlobalVar[f].KSVar)[g] = new CKerArrPointer(); break;
				case eKTarrObject: (*(CKerArrObject***)GlobalVar[f].KSVar)[g] = new CKerArrObject(); break;
				case eKTarrName: (*(CKerArrName***)GlobalVar[f].KSVar)[g] = new CKerArrName(); break;
				case eKTvoid: 
				case eKTstruct:	break;
				case eKTchar: (*(char**)GlobalVar[f].KSVar)[g] = 0; break;
				default: **((char**)GlobalVar[f].KSVar) = 0; if (GlobalVar[f].ArraySize>1) throw;
			}
		}
		if (!r) LLoadVariable(0,GlobalVar+f,0);
		else LLoadVariable(r->GetSubRegister(),GlobalVar+f,0);
	}
	return 1;
}


// Paokud najde v registru promennou, tak ji nahraje
void CKerMain::LLoadVariable(CFSRegister *r, CKerOVar *OV, UC *offset) {
	CFSRegKey *l;
	int g,a,size;
	char ch;
	CKerName *name;
	void *var;

	if (!(OV->Use&eKVUBlevelLoad) && SaveState==eSLSloadConstructor) return;
	if (OV->Type == eKTvoid || OV->Type == eKTstruct) return;
	if (!r) {
		Errors->LogError(eKRTEVarLoad,0,OV->NameStr);
		return;
	}
	l = r->FindKey(OV->NameStr);
	if (!l) {
		Errors->LogError(eKRTEVarLoad,0,OV->NameStr);
		return;
	}
	r = l->GetSubRegister();
	l = r->FindKey("Type");
	if (!l || l->readi() != OV->Type) {
		Errors->LogError(eKRTEVarLoad,0,OV->NameStr);
		return;
	}

	if (OV->KSVar) var = *(OV->KSVar);  // najdu umisteni prom
	else var = offset + OV->Offset; 

	if (!(l=r->FindKey("Value"))) {
		if (OV->Type >= eKTarrChar && OV->Type <= eKTarrName && OV->ArraySize==1) {
			*((int*)var) = 0;
		} else {
			Errors->LogError(eKRTEVarLoad,0,OV->NameStr);
		}
		return;
	}


	int gg;
	if (l->CFSGetKeyType() == FSRTchar || l->CFSGetKeyType() == FSRTstring ) KerSaveLoad.Open(l,SaveState);
	for (gg=0; gg<OV->ArraySize; gg++) {

		switch (OV->Type) {					// prekopiruju obsah prom
			case eKTdouble: 
				((double*)var)[gg] = l->readd();
				break;
			case eKTint:
				(((int*)var))[gg] = l->readi(); 
				break;
			case eKTpointer:
				(((int*)var))[gg] = 0 ;
				break;
			case eKTname:
				if (OV->ArraySize>1) {
					((CKerName**)var)[gg] = KerSaveLoad.LoadName();
					break;
				}
				if (l->GetDirectAccessFromPos()[0]==0) ((CKerName**)var)[gg] = 0; else {
					name = KerNamesMain->GetNamePointer(l->GetDirectAccessFromPos());
					((CKerName**)var)[gg] = name;
					if (!name) Errors->LogError(eKRTEVarLoad,0,"Name n Exists"); //error
				}
				break;
			case eKTobject: 
				a = l->readi(); 
				if (a==0) ((OPointer*)var)[gg] = 0; 
				else if (SaveState == eSLSloadGame) ((OPointer*)var)[gg] = a;
				else if (a >= Objs->LoadedObjects.GetCount() || Objs->LoadedObjects[a]==0) {
					Errors->LogError(eKRTEVarLoad,0,"Object n Exists");
					((OPointer*)var)[gg] = 0;
				} else ((OPointer*)var)[gg] = Objs->LoadedObjects[a];
				break;
			case eKTchar: 
				((char*)var)[gg] = l->readc(); 
				break;
			case eKTarrChar:
				if (OV->ArraySize>1) {
					((CKerArrChar**)var)[gg] = KerSaveLoad.LoadCharA();
				} else {
					((CKerArrChar**)var)[gg] = new CKerArrChar();
					while (!l->eof()) ((CKerArrChar**)var)[gg]->Add(l->readc());
				}
				break;
			case eKTarrDouble:
				if (OV->ArraySize>1) {
					((CKerArrDouble**)var)[gg] = KerSaveLoad.loadDoubleA();
				} else {
					((CKerArrDouble**)var)[gg] = new CKerArrDouble();
					while (!l->eof()) ((CKerArrDouble**)var)[gg]->Add(l->readd());
				}
				break;
			case eKTarrInt:
				if (OV->ArraySize>1) {
					((CKerArrInt**)var)[gg] = KerSaveLoad.LoadIntA();
				} else {
					((CKerArrInt**)var)[gg] = new CKerArrInt();
					while (!l->eof()) ((CKerArrInt**)var)[gg]->Add(l->readi());
				}
				break;
			case eKTarrPointer:
				if (OV->ArraySize>1) {
					((CKerArrPointer**)var)[gg] = 0;
				} else {
					((CKerArrPointer**)var)[gg] = new CKerArrPointer();
					while (!l->eof()) ((CKerArrPointer**)var)[gg]->Add((void*)l->readi());
				}
				break;
			case eKTarrObject:
				if (OV->ArraySize>1) {
					((CKerArrObject**)var)[gg] = KerSaveLoad.LoadObjPtrA();
				} else {
					((CKerArrObject**)var)[gg] = new CKerArrObject();
					while (!l->eof()) {
						a = l->readi();
						if (a==0) ((CKerArrObject**)var)[gg]->Add(0);
						else if (SaveState == eSLSloadGame) ((CKerArrObject**)var)[gg]->Add(a);
						else if (a >= Objs->LoadedObjects.GetCount() || Objs->LoadedObjects[a]==0) {
							Errors->LogError(eKRTEVarLoad,0,"Object n Exists");
							((CKerArrObject**)var)[gg]->Add(0);
						} else ((CKerArrObject**)var)[gg]->Add(Objs->LoadedObjects[a]);
					}
				}
				break;
			case eKTarrName:
				if (OV->ArraySize>1) {
					((CKerArrName**)var)[gg] = KerSaveLoad.LoadNameA();
				} else {
					((CKerArrName**)var)[gg] = new CKerArrName();
					while (!l->eof()) {
						if (l->GetDirectAccessFromPos()[0] == 0) ((CKerArrName**)var)[gg]->Add(0);
						else {
							name = KerNamesMain->GetNamePointer(l->GetDirectAccessFromPos());
							if (!name) Errors->LogError(eKRTEVarLoad,0,"Name n Exists"); //error
							((CKerArrName**)var)[gg]->Add(name);
						}
						l->SetPosToNextString();
					}
				}
				break;
			default: 
				if (OV->ArraySize>1) throw;
				size = GetTypeSize(OV->Type)-1;
				g=0;
				while ((ch=l->readc())&&size) {
					((char*)var)[g] = ch;
					g++; size--; 
				}
				(((char*)var))[g] = 0;
				if(ch) Errors->LogError(eKRTEVarLoad,size,"String too big");
		}
	}
	KerSaveLoad.Close();
}



// sejvne promennou do registru
void CKerMain::LSaveVariable(CFSRegister *r, CKerOVar *OV, UC *offset, OPointer thisO) {
	CFSRegKey *l;
	int f;
	CKerName *name;
	void *var;

	if (!(OV->Use&eKVUBlevelLoad) && SaveState==eSLSsaveLevel) return;
	if (OV->Type == eKTvoid || OV->Type == eKTstruct) return;
	r = r->AddKey(OV->NameStr,FSRTregister)->GetSubRegister();
	r->AddKey("Type",FSRTint)->writei(OV->Type);

	if (OV->KSVar) var = *(OV->KSVar);  // najdu umisteni prom
	else var = offset + OV->Offset; 

	int gg;
	char *str = "Value";
	if (OV->Type >= eKTarrChar && OV->Type <= eKTarrName && (*(int*)var)==0 && OV->ArraySize == 1) return; // pole je NULL

	switch (OV->Type) {
		case eKTdouble: l = r->AddKey(str,FSRTdouble); break;
		case eKTint:
		case eKTobject:
		case eKTpointer: l = r->AddKey(str,FSRTint); break;
		case eKTarrName:
		case eKTname: l = r->AddKey(str,FSRTstring); break;
		case eKTchar: 
		case eKTarrChar: l = r->AddKey(str,FSRTchar); break;
		case eKTarrDouble: if (OV->ArraySize>1) l = r->AddKey(str,FSRTchar); else l = r->AddKey(str,FSRTdouble); break;
		case eKTarrInt:
		case eKTarrPointer:
		case eKTarrObject: if (OV->ArraySize>1) l = r->AddKey(str,FSRTchar); else l = r->AddKey(str,FSRTint); break;
		default: l = r->AddKey(str,FSRTstring); break;
	}
	if (l->CFSGetKeyType() == FSRTchar || l->CFSGetKeyType() == FSRTstring ) KerSaveLoad.Open(l,SaveState);

	for (gg=0; gg<OV->ArraySize; gg++) {

		switch (OV->Type) {					// prekopiruju obsah prom
			case eKTdouble: 
				l->writed(((double*)var)[gg]);
				break;
			case eKTint:
			case eKTpointer:
				l->writei(((int*)var)[gg]);
				break;
			case eKTname:
				if (OV->ArraySize>1) {
					KerSaveLoad.SaveName(((CKerName**)var)[gg]);
				} else {
					name = ((CKerName**)var)[gg];
					if(name == 0) l->writec(0);
					else {
						if (/*TODO v release runu to netestovat*/ !KerNamesMain->TestPointerValidity(name)) {
							l->writec(0);
							KerMain->Errors->LogError(eKRTEnotValidName,0,"SaveName");
						} else l->stringwrite( name->GetNameString());
					}
				}
				break;
			case eKTobject: 
				if (SaveState == eSLSsaveGame) l->writei(((OPointer*)var)[gg]); 
				else {
					l->writei(0);
					if (!thisO) Errors->LogError(eKRTEsavingGlobalObjPtr);
					else KerSaveLoad.AddEdge(thisO,((OPointer*)var)[gg],l,0);
				}
				break;
			case eKTchar: 
				l->writec(((char*)var)[gg]);
				break;
			case eKTarrChar:
				if (OV->ArraySize>1) {
					KerSaveLoad.SaveCharA(((CKerArrChar**)var)[gg]);
				} else {
					for (f=0; f<((CKerArrChar**)var)[gg]->GetCount(); f++) l->writec(((CKerArrChar**)var)[gg]->Read(f));
				}
				break;
			case eKTarrDouble:
				if (OV->ArraySize>1) {
					KerSaveLoad.SaveDoubleA(((CKerArrDouble**)var)[gg]);
				} else {
					for (f=0; f<((CKerArrDouble**)var)[gg]->GetCount(); f++) l->writed(((CKerArrDouble**)var)[gg]->Read(f));
				}
				break;
			case eKTarrInt:
				if (OV->ArraySize>1) {
					KerSaveLoad.SaveIntA(((CKerArrInt**)var)[gg]);
				} else {
					for (f=0; f<((CKerArrInt**)var)[gg]->GetCount(); f++) l->writei(((CKerArrInt**)var)[gg]->Read(f));
					break;
				}
			case eKTarrPointer:
				if (OV->ArraySize>1) {
					// nedelaj nic
				} else {
					for (f=0; f<((CKerArrPointer**)var)[gg]->GetCount(); f++) l->writei((int)(((CKerArrPointer**)var)[gg]->Read(f)));
				}
				break;
			case eKTarrObject:
				if (OV->ArraySize>1) {
					KerSaveLoad.SaveObjPtrA(((CKerArrObject**)var)[gg]);
				} else {
					if (SaveState == eSLSsaveGame) {
						for (f=0; f<((CKerArrObject**)var)[gg]->GetCount(); f++) {
							l->writei(((CKerArrObject**)var)[gg]->Read(f));
						}
					} else {
						if (!thisO) Errors->LogError(eKRTEsavingGlobalObjPtr);
						for (f=0; f<((CKerArrObject**)var)[gg]->GetCount(); f++) {
							if (thisO) KerSaveLoad.AddEdge(thisO,((CKerArrObject**)var)[gg]->Read(f),l,l->pos);
							l->writei(0);
						}
					}
				}
				break;
			case eKTarrName:
				if (OV->ArraySize>1) {
					KerSaveLoad.SaveNameA(((CKerArrName**)var)[gg]);
				} else {
					for (f=0; f<((CKerArrName**)var)[gg]->GetCount(); f++) {
						name = ((CKerArrName**)var)[gg]->Read(f);
						if(name == 0) l->writec(0);
						else {
							if (/*TODO v release runu to netestovat*/ !KerNamesMain->TestPointerValidity(name)) {
								l->writec(0);
								KerMain->Errors->LogError(eKRTEnotValidName,0,"SaveName");
							} else l->stringwrite( name->GetNameString());
						}
					}
				}
				break;
			default: 
				if (OV->ArraySize>1) throw;
				((char*)var)[OV->Type-eKTstring] = 0;
				l->stringwrite(((char*)var));
		}
	}
	KerSaveLoad.Close();
}



// Nahraje objekt z levlu a vytvori ho, ptr objektu OK, 0 chyba, -1 zahodil jsem shortcut
int CKerMain::LLoadObject(CFSRegister *r, CKerName *ObjType, OPointer *StaticVar) {
	CFSRegKey *l=0;
	int f;
	int order;
	CKerObject *Obj;
	int InMap = 0;

	if (l = r->FindKey("Order")) {
		order = l->readi();
	} else RETERR;
	if (l = r->FindKey("ShortCut")) {
		if (!editor /*|| RunMode==NORMAL_RUN*/) return -1; // objekt je shortcut, zahodim ho
	}
	
	if (SaveState == eSLSloadGame) Obj = new CKerObject(ObjType,order);
	else Obj = new CKerObject(ObjType);
	OPointer ptr = Obj->thisO;
	if (l) editor->shortCutArray.Add(new CEDShortCut(ptr,l->GetDirectAccess())); // pridam jmeno shortcutu

	if (StaticVar) *StaticVar = ptr;
	CKerObjectT *objt = ObjType->ObjectType;
	unsigned char *offset = Obj->ISData;

	if (SaveState == eSLSloadConstructor)	Objs->LoadedObjects[order] = ptr;

	if (l = r->FindKey("Tag")) Obj->Tag = l->readi();

	// umistim obj do mapy
	if ((l = r->FindKey("Is In Map")) && (l->readc() == 1)) InMap = 1;
	if (SaveState == eSLSloadGame && InMap) {
		if (!MapInfo->GPlaceObjToMap(Obj,r->FindKey("Placed Info"))) return 0;
		if (l = r->FindKey("Mover")) MapInfo->GLoadMover(l,Obj);
	}

	if ((l = r->FindKey("Edited")) && (l->readc() == 1)) { // loaduju promenne pro objekt z editoru
		GetAccessToObject(Obj);
		l = r->FindKey("Basic Data");
		for (f=0;f<objt->NumVars;f++) { // je tam loadovana prom.
			if (!l) LLoadVariable(0,objt->OVars+f,offset);
			else LLoadVariable(l->GetSubRegister(),objt->OVars+f,offset);
		}
		SaveReg = r;
		if (SaveState==eSLSloadConstructor) call(0,ptr,LoadConstructor,0);
		else call(0,ptr,KnownNames[eKKNloadGeme].Name,0);
		SaveReg = 0;
	} else call(0,ptr,Constructor,0);
	// umistim obj do mapy
	if (SaveState == eSLSloadConstructor && InMap) MapInfo->PlaceObjToMap(0,ptr);

	return ptr;
}


// Nahraje objekty z levlu a vytvori je, 1 OK, 0 chyba 
int	CKerMain::LLoadObjects(CFSRegKey *r) {
	CFSRegKey *r2;
	CFSRegister *reg;
	CKerName *name;
	OPointer ptr;
	int f,g;
	OPointer nula=0;
	if (!r) RETERR;
	reg = r->GetSubRegister();

	Objs->LoadedObjects.Add(nula); // nulta pozice - NULL
	for (f=0; f<reg->GetNumberOfKeys();f++) Objs->LoadedObjects.Add(nula);// vynuluju polozky na mistech objektu
	
	// load statickych objektu:
	for (f=0;f<NumGV;f++) if (GlobalVar[f].Type==eKTobject && GlobalVar[f].Name) {
		StaticObjectsCount++;
		if (SaveState == eSLSloadGame) continue; // globalni objekty nenahravam zde
		r2 = reg->FindKey(GlobalVar[f].NameStr);
		if (!r2) {  // objekt v levlu neni
			Errors->LogError(eKRTEstaticLoad,0,GlobalVar[f].NameStr);
			ptr = (new CKerObject(GlobalVar[f].Name))->thisO;
			**((OPointer**)GlobalVar[f].KSVar) = ptr;
			Objs->LoadedObjects.Add(ptr);
			call(0,ptr,Constructor,0);
		} else {
			if (!LLoadObject(r2->GetSubRegister(),GlobalVar[f].Name,*((OPointer**)GlobalVar[f].KSVar))) return 0;
		}
	}
	if (!MapInfo->registered) Errors->LogError(eKRTEmapNotRegistered);
	
	// loadovani objektu umistenych v editoru:
	r2 = reg->GetFirstKey();
	while (r2) {
		if (r2->GetName()[3]=='I') {
			name = KerNamesMain->GetNamePointer(r2->GetName());
			if (!name) {
				Errors->LogError(eKRTEobjectLoad,0,r2->GetName());
			} else {
				if (!LLoadObject(r2->GetSubRegister(),name)) return 0;
			}
		}
		r2 = r2->GetNextKey();
	}


	if (SaveState == eSLSloadConstructor) {
		// zahodim ze seznamu objektu shortcuty
		if (editor) for (f=0; f<editor->shortCutArray.GetSize(); f++) if (editor->shortCutArray.Get(f)) {
			OPointer a = editor->shortCutArray.Get(f)->obj;
			for (g=0; g<Objs->LoadedObjects.GetCount(); g++) if (Objs->LoadedObjects.Get(g)==a) Objs->LoadedObjects[g] = 0;
		}
	} 

	return 1;
}



// Funkce pripravi Kernel na Beh
void CKerMain::GetReadyToStart() {
	// Priprava na spusteni skriptu:
	KerInterpret->Init_SetStack();
	Objs->CalculatePHT();
	KerInterpret->ResetMachine();
	Errors->LogError(eKRTEKernelInit,RunMode); // Kernel byl uspesne initovan!
}


/// Prevede cestu k souboru se zdrojakem na soubor s codem
char *CKerMain::GetCodeName(char *script) {
	int len,f;
	len = strlen(script);
	len-=3+19; // velikost pripony + velikost stringu s verzi
	strcpy(GCNstrbuff,"$SCRIPTS$\\");
	for (f=0;f<19;f++) GCNstrbuff[f+10] = script[len+f];
	strcpy(GCNstrbuff+19+10,".code");
	return GCNstrbuff;
}


// nahraje frontu zprav daneho typu
int CKerMain::GLoadMessageQueue(CFSRegKey *k2, int typ) {
	CFSRegister *r;
	CFSRegKey *k;
	int ctime = 0;
	int f;
	
	r = k2->GetSubRegister();
	k2 = r->GetFirstKey();

	while (k2) {
		r = k2->GetSubRegister(); // projdu kazdou zpravu
		if (typ==eKerCTtimed) {
			if (k = r->FindKey("Time")) ctime = k->readi() - Time; else RETERR 
		}
		CKerMessage *msg = new CKerMessage(typ,ctime);
		if (k = r->FindKey("Reciever")) msg->Receiver = k->readi();  else RETERR 
		if (k = r->FindKey("Method")) msg->Method = KerSaveLoad.LoadName2(k);  else RETERR 
		if (k = r->FindKey("Sender")) msg->Sender = k->readi(); 
		if (k = r->FindKey("NumArgs")) msg->NumArgs = k->readi();  else msg->NumArgs = 0; 

		if (msg->NumArgs) {
			int NumArgs = msg->NumArgs;
			CFSRegKey *k1,*k2,*k3;
			int a=0;
			if (!(k1 = r->FindKey("ArgTypes"))) RETERR
			if (!(k2 = r->FindKey("ArgNames"))) RETERR
			if (!(k3 = r->FindKey("Args"))) RETERR
			msg->ArgTypes = new int[NumArgs];
			msg->ArgNames = new CKerName*[NumArgs];

			for (f=0;f<NumArgs;f++) {
				msg->ArgTypes[f] = k1->readi(); // nactu typy argumentu
				msg->ArgNames[f] = KerSaveLoad.LoadName2(k2); // nactu jmena argumentu
				a+=GetTypeSize(msg->ArgTypes[f]);	// spocitam celkovou velikost argumentu
			}
			
			msg->Args = new char[a];  // Vytvorim pole pro argumenty
			a=0;
			KerSaveLoad.Open(k3,eSLSloadGame);
			for (f=0;f<NumArgs;f++) {
			// a nakopiruju do nej samotne argumenty:
				switch (msg->ArgTypes[f]) {
					case eKTchar : *(char*)(msg->Args+a) = KerSaveLoad.LoadChar(); a+=1; break;
					case eKTdouble : *(double*)(msg->Args+a) = KerSaveLoad.LoadDouble(); a+=8; break;
					case eKTint: *(int*)(msg->Args+a) = KerSaveLoad.LoadInt(); a+=4; break;
					case eKTobject: *(OPointer*)(msg->Args+a) = KerSaveLoad.LoadObjPtr(); a+=4; break;
					case eKTpointer: assert(false); *(int*)(msg->Args+a) = 0; a+=4; break;
					case eKTname: *(CKerName**)(msg->Args+a) = KerSaveLoad.LoadName(); a+=4; break;
					case eKTarrChar:			
					case eKTarrDouble:
					case eKTarrInt:
					case eKTarrPointer:
					case eKTarrObject:
					case eKTarrName: assert(false); *(int*)(msg->Args+a) = 0; a+=4; break;
					default: KerSaveLoad.LoadString(msg->Args+a,msg->ArgTypes[f]); a+=GetTypeSize(msg->ArgTypes[f]);
				}
			}
			KerSaveLoad.Close();
		}

		k2 = k2->GetNextKey();
	}
	return 1;
}



// ulozi frontu zprav daneho typu
void CKerMain::GSaveMessageQueue(CFSRegKey *k2, CKerMessage *mq) {
	CFSRegister *r, *r2;
	int f;
	
	r2 = k2->GetSubRegister();

	while (mq) {
		r = r2->AddKey("msg",FSRTregister)->GetSubRegister(); // projdu kazdou zpravu
		r->AddKey("Time",FSRTint)->writei(mq->Time);
		r->AddKey("Reciever",FSRTint)->writei(mq->Receiver);
		r->AddKey("Sender",FSRTint)->writei(mq->Sender);
		r->AddKey("NumArgs",FSRTint)->writei(mq->NumArgs);
		KerSaveLoad.SaveName2(r->AddKey("Method",FSRTstring),mq->Method);

		if (mq->NumArgs) {
			int NumArgs = mq->NumArgs;
			CFSRegKey *k1,*k2,*k3;
			int a=0;
			k1 = r->AddKey("ArgTypes",FSRTint);
			k2 = r->AddKey("ArgNames",FSRTstring);
			k3 = r->AddKey("Args",FSRTchar);

			KerSaveLoad.Open(k3,eSLSsaveGame);
			for (f=0;f<NumArgs;f++) {
				k1->writei(mq->ArgTypes[f]);
				KerSaveLoad.SaveName2(k2,mq->ArgNames[f]);

				switch (mq->ArgTypes[f]) {
					case eKTchar :  KerSaveLoad.SaveChar(*(char*)(mq->Args+a)); a+=1; break;
					case eKTdouble : KerSaveLoad.SaveDouble(*(double*)(mq->Args+a)); a+=8; break;
					case eKTint:  KerSaveLoad.SaveInt(*(int*)(mq->Args+a)); a+=4; break;
					case eKTobject: KerSaveLoad.SaveObjPtr(*(OPointer*)(mq->Args+a)); a+=4; break;
					case eKTpointer: assert(false);  a+=4; break;
					case eKTname: KerSaveLoad.SaveName(*(CKerName**)(mq->Args+a)); a+=4; break;
					case eKTarrChar:			
					case eKTarrDouble:
					case eKTarrInt:
					case eKTarrPointer:
					case eKTarrObject:
					case eKTarrName: assert(false);  a+=4; break;
					default: KerSaveLoad.SaveString(mq->Args+a,mq->ArgTypes[f]); a+=GetTypeSize(mq->ArgTypes[f]);
				}
			}
			KerSaveLoad.Close();
		}

		mq = mq->next;
	}
}


// nahraje stav kenelu a zpravy - pro GameLoad
int CKerMain::GLoadKernelState(CFSRegister *lev) {
	CFSRegKey *k;
	if (k=lev->FindKey("Time")) Time = k->readi(); else RETERR
	if (k=lev->FindKey("ObjCounter")) Objs->Counter = k->readi(); else RETERR
	if ((k=lev->FindKey("Message Queue 1"))) if (!GLoadMessageQueue(k,eKerCTmsg)) return 0;
	if ((k=lev->FindKey("Message Queue 2"))) if (!GLoadMessageQueue(k,eKerCTend)) return 0;
	if ((k=lev->FindKey("Message Queue 3"))) if (!GLoadMessageQueue(k,eKerCTtimed)) return 0;
	if (!MapInfo->GLoadMapInfo(lev->FindKey("Map Info"))) return 0;

	if (k=lev->FindKey("Global Light")) {
		unsigned char r,g,b;
		r = k->readc(); g = k->readc(); b = k->readc(); 
		GEnMain->SetTopLightIntenzity(r,g,b);
	}

	return 1;
}



// ulozi stav kenelu a zpravy - pro GameLoad
void CKerMain::GSaveKernelState(CFSRegister *lev) {
	lev->AddKey("Run Mode",FSRTint)->writei(RunMode);
	lev->AddKey("Debug Mode",FSRTint)->writei(DebugMode);
	lev->AddKey("Time",FSRTint)->writei(Time);
	lev->AddKey("ObjCounter",FSRTint)->writei(Objs->Counter);
	GSaveMessageQueue(lev->AddKey("Message Queue 1",FSRTregister),startmq[0]);
	GSaveMessageQueue(lev->AddKey("Message Queue 2",FSRTregister),startmq[1]);
	GSaveMessageQueue(lev->AddKey("Message Queue 3",FSRTregister),timedmsgs);
	MapInfo->GSaveMapInfo(lev->AddKey("Map Info",FSRTregister));

	int f=0; 
	CFSRegKey *k = lev->AddKey("LoadedObjects",FSRTint);
	Objs->LOShakeOff();
	for(f=0; f<Objs->LoadedObjects.GetCount(); f++) k->writei(Objs->LoadedObjects[f]);

	k = lev->AddKey("Global Light",FSRTchar);
	unsigned char r,g,b;
	GEnMain->GetTopLightIntenzity(&r,&g,&b);
	k->writec(r); k->writec(g); k->writec(b); 
}


// Druha cast loadovani levlu - vlastni load
int CKerMain::LoadLevel2(CFSRegister *lev) {
	CFSRegister *code;
	CFSRegKey *k;
	if (lev->GetOpenError()!=FSREGOK) RETERR; 
	if (!LevelInfo.LoadLevel(lev)) RETERR;
	if (!(k=lev->FindKey("Scripts File"))) RETERR;
	// Load scriptu:
	LevelInfo.ScriptsFile = newstrdup(k->GetDirectAccess());
	Errors->LogError(eKRTELoadingScript,0,k->GetDirectAccess());
	code = new CFSRegister(GetCodeName(k->GetDirectAccess()),"KRKAL SCRIPT CODE");
	try {
		if (!LoadScriptsCode(code)) OpenError=eKerOLEOpeningSCode;
	} catch (CKernelPanic) {
		delete code;
		throw;
	}
	delete code;
	if (OpenError) return 1;
	
	GetReadyToStart();

	if (!LLoadGlobals(lev->FindKey("Globals"))) RETERR;
	if (SaveState == eSLSloadGame) 	if (!GLoadKernelState(lev)) RETERR;
	if (!LLoadObjects(lev->FindKey("Objects"))) RETERR;
	if (SaveState == eSLSloadGame) {
		int f=0;
		if (!(k=lev->FindKey("LoadedObjects"))) RETERR
		while (!k->eof()) { Objs->LoadedObjects[f] = k->readi(); f++;}
	}
	return 1;
}



// Druha cast loadovani scriptu - vlastni load
int CKerMain::LoadScriptsCode(CFSRegister *code) {
	CFSRegKey *k, *l;
	int f;
	
	if (code->GetOpenError()!=FSREGOK) RETERR;
	if (!(k=code->FindKey("Compiled Scripts Version"))) RETERR;
	if (k->readi()!=KS_VERSION) {
		// nesouhlasi mi verze kernelu!
		OpenError = eKerOLENeedToRecompile;
		return 1;
	}
	if (!LevelInfo.LoadScript(code)) RETERR;
	// Loaduju KOD:
	if (!(k=code->FindKey("Code"))) RETERR;
	KerInterpret->LoadCode(k);
	// Loaduju jmena a inicializuju vse kolem:
	if (!(k=code->FindKey("Number of Ker Names"))) RETERR;
	if (!(l=code->FindKey("Ker Names"))) RETERR;
	if (!KerInterpret->LoadNames(k->readi(),l)) RETERR;
	SetConstructors();
	for (f=0; f<MAXKERKNOWNDEPENDENCIES; f++) KnownNames[KerKnownDependencies[f*2]].Name->AddChild(KnownNames[KerKnownDependencies[f*2+1]].Name);  // pridani dependenci mezi znamymi jmeny
	if (!(l=code->FindKey("Dependencies"))) RETERR;
	k->pos=0;
	if (!LoadDependencies(k->readi(),l,KerInterpret->GetNamesArray())) RETERR;
	// Loaduju typy objektu:
	if (!(k=code->FindKey("Objects"))) RETERR;
	if (!Objs->LoadObjects(k->GetSubRegister())) RETERR;
	// load stromecky automatismu
	KerMain->AutosMain->LoadAllAutos();
	KSNamesAssign();
	KerNamesMain->CreateMatrix();
	AutosMain->CheckAutosIntegrity();
	AutosMain->AssignAutosToObjs();
	// Loaduju Globalni promenne:
	if (!KerInterpret->LoadGlobals(code->FindKey("Globals"))) RETERR;
	if (!Objs->CheckVarGroups(&GlobalVar,NumGV)) RETERR;
	Input->RegisterKernelKeys(); // Registrace Klaves ke Jmenum
	ME->RegisterKernelSound();
	return 1;
}




// Load hran mezi KSID jmeny
int CKerMain::LoadDependencies(int NamesNum, CFSRegKey *r, CKerName **names) {
	int f=r->top/2;
	int a,b;
	while (f) {
		if (r->eof()) RETERR; a=r->readi(); if (a<0||a>=NamesNum) RETERR;
		if (r->eof()) RETERR; b=r->readi(); if (b<0||b>=NamesNum) RETERR;
		names[a]->AddChild(names[b]);
		f--;
	}
	return 1;
}


// Nastavim, najdu jmena dulezitych metod:
void CKerMain::SetConstructors() {
	Constructor = KnownNames[eKKNconstructor].Name;
	LoadConstructor = KnownNames[eKKNloadConstructor].Name;
	CopyConstructor = KnownNames[eKKNcopyConstructor].Name;
	Destructor = KnownNames[eKKNdestructor].Name;
}





//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///
///		SAVE LEVEL



//ulozi vsechny objekty
void CKerMain::GSaveObjects(CFSRegister *objs) {
	int f,g;
	CKerObject *ko;
	CFSRegister *reg, *r2;

	for (f=0; f<OBJECT_HT_SIZE; f++) {
		ko = Objs->HT[f];
		while (ko) {
			reg = objs->AddKey(ko->Type->Name->GetNameString(),FSRTregister)->GetSubRegister();

			reg->AddKey("Edited",FSRTchar)->writec(1);
			reg->AddKey("Order",FSRTint)->writei(ko->thisO);
			if (ko->SaveRec) {
				reg->AddKey("ShortCut",FSRTstring)->stringwrite(ko->SaveRec->shortcut);
				SAFE_DELETE(ko->SaveRec);
			}
			// necham objekt at si sejvne sve pointery
			SaveReg = reg;
			call(0,ko->thisO,KnownNames[eKKNsaveGame].Name,0);
			SaveReg = 0;

			// sejvnu zbytek objektu
			reg->AddKey("Tag",FSRTint)->writei(ko->Tag);
			GetAccessToObject(ko);
			r2 = reg->AddKey("Basic Data", FSRTregister)->GetSubRegister();
			for (g=0; g<ko->Type->NumVars; g++) LSaveVariable(r2,ko->Type->OVars+g,ko->ISData,ko->thisO);

			if (MapInfo->IsObjInMap(ko->thisO)) {
				reg->AddKey("Is In Map",FSRTchar)->writec(1);
				ko->PlacedInfo->GSave(reg->AddKey("Placed Info",FSRTregister));
				if (ko->PlacedInfo->Mover) ko->PlacedInfo->Mover->GSaveMover(reg->AddKey("Mover",FSRTregister));
			}

			ko = ko->next;
		}
	}
	KerSaveLoad.Close();
}




void CKerMain::LSaveObjects() {
	int f,g;
	CKerObject *ko;
	CFSRegister *reg, *r2;

	// pripravim promenne kam se bude sejvovat:
	for (f=0; f<Objs->LoadedObjects.GetCount(); f++) {
		ko = Objs->GetObject(Objs->LoadedObjects[f]);
		if (ko) ko->SaveRec = new CKerObjSaveRec();
	}

	for (f=0; f<Objs->LoadedObjects.GetCount(); f++) {
		ko = Objs->GetObject(Objs->LoadedObjects[f]);
		if (ko && ko->SaveRec) {
			ko->SaveRec->reg = reg = new CFSRegister();
			reg->AddKey("Edited",FSRTchar)->writec(1);
			// necham objekt at si sejvne sve pointery
			SaveReg = reg;
			call(0,Objs->LoadedObjects[f],KnownNames[eKKNEsaveMe].Name,0);
			SaveReg = 0;

			// sejvnu zbytek objektu
			ko = GetAccessToObject(Objs->LoadedObjects[f]);
			if (ko) {
				r2 = reg->AddKey("Basic Data", FSRTregister)->GetSubRegister();
				for (g=0; g<ko->Type->NumVars; g++) LSaveVariable(r2,ko->Type->OVars+g,ko->ISData,ko->thisO);
			}
			if (MapInfo->IsObjInMap(Objs->LoadedObjects[f])) reg->AddKey("Is In Map",FSRTchar)->writec(1);
		}
	}
	KerSaveLoad.Close();
}




/////////////////////////////////////////////////////////////////////
// trideni grafu DFS, objekty predavoji sve sejvovaci egistry na vystup.
int CKerMain::LSSortGraph(OPointer obj, OPointer parent, CFSRegister *sreg, int &count) {
	if (!obj || obj==parent) return 1;
	CKerObject *ko = Objs->GetObject(obj);
	if (!ko) return 1;
	assert(ko->SaveRec);
	if (ko->SaveRec->Tag == 2) return 1; // dopredna hrana
	if (ko->SaveRec->Tag == 1) {
		Errors->LogError(eKRTEcycleInbjPtrs,obj);
		return 0; // zpetna hrana!!
	}

	// jdu po hranach na dalsi objekty:
	ko->SaveRec->Tag = 1;
	int ret=1;
	int f, _count = ko->SaveRec->SaveGraph->GetCount();
	for (f=0; f<_count; f++) {
		ret = LSSortGraph(ko->SaveRec->SaveGraph->Read(f),obj,sreg,count);
		if (!ret) return 0;
	}
	ko->SaveRec->Tag = 2;

	// sejvnu objekt.
	ko->SaveRec->reg->AddKey("Order",FSRTint)->writei(count);
	if (ko->SaveRec->GlobalVar) {
		sreg->AddRegisterToKey(ko->SaveRec->GlobalVar,ko->SaveRec->reg);
	} else {
		sreg->AddRegisterToKey(ko->Type->Name->GetNameString(),ko->SaveRec->reg);
	}
	ko->SaveRec->reg = 0;

	// spravne nastavim pointery na objekt
	CKerOSRPointers *p = ko->SaveRec->pointers;
	while (p) {
		if (Objs->GetObject(p->obj)) {
			p->key->pos = p->pos;
			if (p->key->CFSGetKeyType() == FSRTchar) p->key->blockwrite(&count,4);
			else p->key->writei(count);
		}
		p = p->next;
	}

	count ++;
	return 1;
}




// sejvne Level s vyuzitim LevelInfa. 1 - OK, 0 - chyba;
int CKerMain::SaveLevel() {
	int ObjCounter = 1;
	CFSRegister *reg, *objs;
	int ret = 1;
	int f;
	OPointer obj1,obj2;
	CKerObject *ko;
	int OldCount;

	Errors->LogError(eKRTEsavingLevel,0,LevelInfo.LevelFile);
	
	if (!LevelInfo.LevelFile || !LevelInfo.ScriptsFile || !LevelInfo.SVersion) {
		// error
		return 0;
	}
	SaveState = eSLSsaveLevel;

	// pridam ShortCuty k sejvovani
	Objs->LOShakeOff();
	OldCount = Objs->LoadedObjects.GetCount();
	if (editor) {
		for (f = 0; f<editor->shortCutArray.GetSize(); f++) {
			if (editor->shortCutArray.Get(f)) Objs->LoadedObjects.Add(editor->shortCutArray.Get(f)->obj);
		}
	}

	LSaveObjects();

	// sejvnu k shortCutum jejich jmena
	if (editor) {
		for (f = 0; f<editor->shortCutArray.GetSize(); f++) {
			if (editor->shortCutArray.Get(f)) {
				if (ko=Objs->GetObject(editor->shortCutArray.Get(f)->obj)) {
					assert(ko->SaveRec);
					ko->SaveRec->reg->AddKey("ShortCut",FSRTstring)->stringwrite(editor->shortCutArray.Get(f)->name);
				}
			}
		}
	}

	reg = new CFSRegister(LevelInfo.LevelFile,"KRKAL LEVEL",1);
	try {
		LevelInfo.SaveLevel(reg);

		objs = reg->AddKey("Objects",FSRTregister)->GetSubRegister();

		// Pridam hrany do grafu tak aby se Globalni promenne sejvly ve spravnem poradi na zacatku
		// Ke statickym objektum dale pridam jmeno jejich globalni promenne
		obj2 = 0;
		for (f=0; f<NumGV; f++) if (GlobalVar[f].Type==eKTobject && GlobalVar[f].Name) {
			obj1 = **(OPointer**)GlobalVar[f].KSVar;
			ko = Objs->GetObject(obj1);
			if (ko) {
				assert(ko->SaveRec);
				ko->SaveRec->GlobalVar = GlobalVar[f].NameStr;
				if (obj2) ko->SaveRec->SaveGraph->Add(obj2);
				obj2 = obj1;
			}
		}
		if (obj2) for (f=0; f<Objs->LoadedObjects.GetCount(); f++) {
			ko = Objs->GetObject(Objs->LoadedObjects[f]);
			if (ko && ko->SaveRec && !ko->SaveRec->GlobalVar) {
				ko->SaveRec->SaveGraph->Add(obj2);
			}
		}

		// sejvnu objekty ve spravnem poradi
		for (f=0; f<Objs->LoadedObjects.GetCount(); f++) {
			ret = LSSortGraph(Objs->LoadedObjects[f],0,objs,ObjCounter);
			if (!ret) break;
		}

		// sejvnu globalni promenne
		if (ret) {
			CFSRegister *r2 = reg->AddKey("Globals", FSRTregister)->GetSubRegister();
			for (f=0; f<NumGV; f++) LSaveVariable(r2,GlobalVar+f,0,0);
		}


		// smazi sejvovaci zaznamy
		for (f=0; f<Objs->LoadedObjects.GetCount(); f++) {
			ko = Objs->GetObject(Objs->LoadedObjects[f]);
			if (ko) SAFE_DELETE(ko->SaveRec);
		}

		Objs->LoadedObjects.SetCount(OldCount);

		if (ret) {
			MMProfiles->EditSaveLev(LevelInfo.LevelFile);
			ret = reg->WriteFile();
			if (!ret) Errors->LogError(eKRTEsavingLIOErr);
		}
	} catch (CKernelPanic) {
		delete reg;
		Objs->LoadedObjects.SetCount(OldCount);
		throw;
	}
	delete reg;
	SaveState = eSLSnothing;
	if (ret) Errors->LogError(eKRTEsaveLOK);
	return ret;
}




// ulozi hru do pripravenoho registru
void CKerMain::GSaveGame() {
	CFSRegister *reg;
	int err=0;
	int f;
	Errors->LogError(eKRTEsavingGame,0,_FileToSaveGame);
	
	if (!LevelInfo.LevelFile || !LevelInfo.ScriptsFile || !LevelInfo.SVersion) {
		// error
		Errors->LogError(eKRTEgameNotSaved,0,0);
		return;
	}
	SaveState = eSLSsaveGame;

	if (_FileToSaveGame) reg = new CFSRegister(_FileToSaveGame,"KRKAL SaveGame",1);
	else {
		SAFE_DELETE(KerServices.QuickSave);
		reg = KerServices.QuickSave = new CFSRegister("$KRKAL$/Quick Save","KRKAL SaveGame",1);
	}

	try {
		LevelInfo.SaveLevel(reg);
		reg->AddKey("Level File",FSRTstring)->stringwrite(LevelInfo.LevelFile);

		// poznamenam si docasne u objektu jmena shortcutu
		if (editor) {
			CKerObject *ko;
			for (f = 0; f<editor->shortCutArray.GetSize(); f++) {
				if (editor->shortCutArray.Get(f)) {
					if (ko=Objs->GetObject(editor->shortCutArray.Get(f)->obj)) {
						ko->SaveRec = new CKerObjSaveRec();
						ko->SaveRec->shortcut = editor->shortCutArray.Get(f)->name;
					}
				}
			}
		}

		// sejv objektu
		GSaveObjects(reg->AddKey("Objects", FSRTregister)->GetSubRegister());

		CFSRegister *r2 = reg->AddKey("Globals", FSRTregister)->GetSubRegister();
		for (f=0; f<NumGV; f++) LSaveVariable(r2,GlobalVar+f,0,0);

		GSaveKernelState(reg);

	} catch (CKernelPanic) {
		if (_FileToSaveGame) delete reg; else SAFE_DELETE(KerServices.QuickSave);
		SaveState = eSLSnothing;
		throw;
	}

	if (!err) {
		if (_FileToSaveGame) err = !reg->WriteFile();
//		err = !reg->WriteFile();
	}
	if (_FileToSaveGame) delete reg;
	if (err) {
		if (!_FileToSaveGame) SAFE_DELETE(KerServices.QuickSave);
		Errors->LogError(eKRTEgameNotSaved,0,0);
	} else Errors->LogError(eKRTEsaveLOK);
	SaveState = eSLSnothing;
}


//////////////////////////////////////////////////////////////////////////////

// Destruktor
CKerMain::~CKerMain() {
	// Smazu zpravy
	Errors->SaveLogsToFile();
	DeleteMessages(startmq+0,endmq+0);
	DeleteMessages(startmq+1,endmq+1);
	DeleteMessages(startmq+2,endmq+2);
	DeleteMessages(startmq+3,endmq+3);
	DeleteMessages(&timedmsgs,&timedmsgs);
	while (KerContext) delete KerContext; // Smazu kontext(y)
	KerContext = 0;
	SAFE_DELETE_ARRAY(GlobalVar);
	SAFE_DELETE_ARRAY(SpecificKnownVars);
	SAFE_DELETE_ARRAY(_FileToSaveGame);
	// Zrusim ostatni soucasti Kernelu:
	KerServices.KernelEnd();
	SAFE_DELETE(MapInfo);
	SAFE_DELETE(Objs);
	SAFE_DELETE(AutosMain);
	delete KerNamesMain;
	SAFE_DELETE(KerInterpret);
	SAFE_DELETE(Errors);
	GarbageCollector.EmptyMe();
	KerMain=0;
}





/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
///
///		C K e r M e s s a g e
///
/////////////////////////////////////////////////////////////////////////////////

// Vytvoreni zpravy. When - cas vyvolani, pokud when==eKerCTcallend, tak callendObjekt urcuje
// objekt, po kterem se ma zprava vyvolat. Zprava se vyvola co nejdrive, ale az zkonci vsechny metody objektu callendObject
CKerMessage::CKerMessage(int when, OPointer callendObject) {
	CKerContext *ctx, *ctx2;
	CKerMessage **m;
	// Zjistim jestli zprav uz nebylo vytvoreno priliz moc:
	KerMain->_message_counter++;
	if (KerMain->_message_counter >= KER_MESSAGES_WARNING && !KerMain->_message_counter_warning && KerMain->_RunTurn) { KerMain->Errors->LogError(eKRTEmessagew); KerMain->_message_counter_warning = 1;}
	if (KerMain->_message_counter >= KER_MESSAGES_MAX && KerMain->_RunTurn) KerMain->Errors->LogError(eKRTEmessage);
	// Vyplnim nektere polozky zpravy:
	if (KerContext) {
		Sender = KerContext->KCthis;
		MethodName = KerContext->MethodName;
		CodeLine = KerContext->line;
	} else {
		Sender = 0;
		assert (when != eKerCTcallend);
		MethodName = 0;
		CodeLine = 0;
	}
	assert (when != eKerCTnow);
	NumArgs=0;
	ArgTypes=0;
	ArgNames=0;
	Args=0;
	if (when==eKerCTtimed) {
		callendObject = callendObject + KerMain->Time;
		if (callendObject <= KerMain->Time) when = eKerCTmsg; // casovane zprave uz vyprsel cas, udelam z ni normalni zpravu
	}
	if (when==eKerCTcallend) {
		// najdu prvni posledni kontext, kde je callendObject pouzivan
		ctx = KerContext; // je to aktualni kontext, pokud zadny takovy nenajdu.
		if (callendObject) {
			ctx2 = ctx->parent;
			while (ctx2) {
				if (ctx2->KCthis == callendObject) ctx = ctx2;
				ctx2 = ctx2->parent;
			}
		}
		// a tam pridam zpravu
		if (!ctx->startmq) ctx->startmq=this;
		else ctx->endmq->next = this;
		next=0;
		ctx->endmq = this;
	} else if (when==eKerCTtimed) {
		// zpravu zatridim mezi casovane zpravy
		m = &(KerMain->timedmsgs);
		while (*m && (**m).Time <= callendObject) m = &((**m).next);
		Time = callendObject;
		next = *m;
		*m = this;
	} else {
		// zpravu pridam do jedne ze 4 standardnich front
		if (!KerMain->startmq[when]) KerMain->startmq[when]=this;
		else KerMain->endmq[when]->next = this;
		next=0;
		KerMain->endmq[when] = this;
	}
}





///////////////////////////////////////////////////////////////////////////
///
///		C K e r C o n t e x t
///
///////////////////////////////////////////////////////////////////////////

// Pomocna funkce, ktara hlasi errory pri priliz mnoha vnorenych volani
void CKerContext::_log_error() {
	if (KerMain->_call_stack_counter >= KER_CALL_STACK_WARNING && !KerMain->_call_stack_counter_warning) { KerMain->Errors->LogError(eKRTEcallstackw); KerMain->_call_stack_counter_warning = 1;}
	if (KerMain->_call_stack_counter >= KER_CALL_STACK_MAX) KerMain->Errors->LogError(eKRTEcallstack);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CKerGarbageCollector::EmptyMe() {
	while (GCArrChar) delete GCArrChar;
	while (GCArrInt) delete GCArrInt;
	while (GCArrDouble) delete GCArrDouble;
	while (GCArrPointer) delete GCArrPointer;
	while (GCArrObject) delete GCArrObject;
	while (GCArrName) delete GCArrName;
	GCArrChar = 0;
	GCArrInt = 0;
	GCArrDouble = 0;
	GCArrPointer = 0;
	GCArrObject = 0;
	GCArrName = 0;

}


// Hodnoty ktere vrati dynamicka pole v pripade chyby
char* KRValueCKerArrChar = 0;
double* KRValueCKerArrDouble = 0;
int* KRValueCKerArrInt = 0;
void** KRValueCKerArrPointer = 0;
OPointer* KRValueCKerArrObject = 0;
CKerName** KRValueCKerArrName = 0;





//////////////////////////////////////////////////////////////////
///
///		C K e r L e v e l I n f o
///
//////////////////////////////////////////////////////////////////

CKerLevelInfo::~CKerLevelInfo() {
	SAFE_DELETE_ARRAY(LevelFile);
	SAFE_DELETE_ARRAY(ScriptsFile);
	SAFE_DELETE_ARRAY(SVersion);
	SAFE_DELETE_ARRAY(Author);
	SAFE_DELETE_ARRAY(Game);
	SAFE_DELETE_ARRAY(Comment);
	SAFE_DELETE_ARRAY(Music);
	SAFE_DELETE_ARRAY(Directory);
	SAFE_DELETE_ARRAY(Password);
	SAFE_DELETE(LocalNames);
}


int CKerLevelInfo::LoadScript(CFSRegister *reg) {
	CFSRegKey *k;
	if (k=reg->FindKey("Scripts Version")) SVersion = k->stringread();
	else return 0;
	if (k=reg->FindKey("Scripts Version 64")) SVersion64 = k->read64();
	else return 0;
	if (!Game && (k=reg->FindKey("Game"))) Game = k->stringread();

	return 1;
}


int CKerLevelInfo::LoadLevel(CFSRegister *reg) {
	CFSRegKey *k;
	if (k=reg->FindKey("Author")) Author = k->stringread();
	if (k=reg->FindKey("Game")) Game = k->stringread();
	if (k=reg->FindKey("Comment")) Comment = k->stringread();
	if (k=reg->FindKey("Music")) {
		Music = k->stringread();
		if(ME) ME->Play(Music,1,2);
	}
	if (k=reg->FindKey("Difficulty")) Difficulty = k->readd();
	if (k=reg->FindKey("Tags")) Tags = k->readi();
	if (k=reg->FindKey("LocalNames")) {
		CFSRegister *r = k->GetSubRegister();
		LocalNames = new CFSRegister();
		k = r->GetFirstKey();
		while (k) {
			if (k->CFSGetKeyType() == FSRTstring) LocalNames->AddKey(k->GetName(),FSRTstring)->stringwrite(k->GetDirectAccess());
			k = k->GetNextKey();
		}
	}
	if (k=reg->FindKey("Directory")) Directory = k->stringread();
	if (k=reg->FindKey("Password")) Password = k->stringread();
	if ((KerMain->SaveState == eSLSloadConstructor) && (k=reg->FindKey("LVersion"))) {
		// otestuju zda sedi verze levlu - kvuli bezpecnosti
		CFSRegister *r;
		CFSRegKey *k2;
		int err=0;
		if (strcmp(k->GetDirectAccess(),MMLevelDirs->GetVersionString(LevelFile)) != 0) KerMain->Errors->LogError(eKRTEinvLvlVersion);
		r = new CFSRegister("!level.info","KRKAL LEVEL I");
		if (r->GetOpenError() != FSREGOK) err=1; else {
			k2 = r->FindKey("LVersion");
			if (!k2 || strcmp (k->GetDirectAccess(), k2->GetDirectAccess()) != 0) err=1;
		}
		delete r;
		if (err) KerMain->Errors->LogError(eKRTEinvLvlVersion);
	}
	return 1;
}


void CKerLevelInfo::SaveLevel(CFSRegister *reg) {
	char *ver, *str;
	if (ScriptsFile) reg->AddKey("Scripts File",FSRTstring)->stringwrite(ScriptsFile);
	if (SVersion) reg->AddKey("Scripts Version",FSRTstring)->stringwrite(SVersion);
	reg->AddKey("Scripts Version 64",FSRTint64)->write64(SVersion64);
	if (Author) reg->AddKey("Author",FSRTstring)->stringwrite(Author);
	if (Game) reg->AddKey("Game",FSRTstring)->stringwrite(Game);
	if (Comment) reg->AddKey("Comment",FSRTstring)->stringwrite(Comment);
	if (Music) reg->AddKey("Music",FSRTstring)->stringwrite(Music);
	reg->AddKey("Difficulty",FSRTdouble)->writed(Difficulty);
	reg->AddKey("Tags",FSRTint)->writei(Tags);
	if (LocalNames) {
		CFSRegister *r = reg->AddKey("LocalNames",FSRTregister)->GetSubRegister();
		CFSRegKey *k = LocalNames->GetFirstKey();
		while (k) {
			if (k->CFSGetKeyType() == FSRTstring) r->AddKey(k->GetName(),FSRTstring)->stringwrite(k->GetDirectAccess());
			k = k->GetNextKey();
		}
	}
	if (Directory) reg->AddKey("Directory",FSRTstring)->stringwrite(Directory);
	if (Password) reg->AddKey("Password",FSRTstring)->stringwrite(Password);

	if (KerMain->SaveState == eSLSsaveLevel) { // sejvnu verzi levlu a level.info
		CFSRegister *reg2;
		ver = MMLevelDirs->GetVersionString(LevelFile);
		str = new char[strlen(LevelFile)+10];
		sprintf(str,"%s.info",LevelFile);
		reg->AddKey("LVersion",FSRTstring)->stringwrite(ver);

		reg2 = new CFSRegister(str,"KRKAL LEVEL I",1);
		SAFE_DELETE_ARRAY(str);
		if (Author) reg2->AddKey("Author",FSRTstring)->stringwrite(Author);
		if (Game) reg2->AddKey("Game",FSRTstring)->stringwrite(Game);
		if (Comment) reg2->AddKey("Comment",FSRTstring)->stringwrite(Comment);
		reg2->AddKey("Difficulty",FSRTdouble)->writed(Difficulty);
		reg2->AddKey("Tags",FSRTint)->writei(Tags);
		if (LocalNames) {
			CFSRegister *r = reg2->AddKey("LocalNames",FSRTregister)->GetSubRegister();
			CFSRegKey *k = LocalNames->GetFirstKey();
			while (k) {
				if (k->CFSGetKeyType() == FSRTstring) r->AddKey(k->GetName(),FSRTstring)->stringwrite(k->GetDirectAccess());
				k = k->GetNextKey();
			}
		}
		if (Directory) reg2->AddKey("Directory",FSRTstring)->stringwrite(Directory);
		if (Password) reg2->AddKey("Password",FSRTstring)->stringwrite(Password);
		reg2->AddKey("LVersion",FSRTstring)->stringwrite(ver);

		reg2->WriteFile();
		delete reg2;
	}
}