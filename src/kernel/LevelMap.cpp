//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - L e v e l M a p
///
///		Sluzby Kernelu pro praci s Mapou a pro spolupraci sobjektem Mapa
///		Sluzby pro praci s umistitelnyma objektama, umistovani, pohyby, kolize
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "LevelMap.h"
#include "autos.h"
#include "objects.h"
#include "genmain.h"
#include "KerServices.h"
#include "names.h"
#include "editor.h"
#include "register.h"




//////////////////////////////////////////////////////////////////////////////
///
///		SCROLLING
///
//////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////
// nascrolluje tak, aby bod x,y bul ve stredu
void CKerMapInfo::CalcScrollWindowXY(int x, int y) {
	float fdx,fdy;
	int wdx, wdy;
	GEnMain->GetWindowSize(fdx,fdy);
	wdx = (int)fdx;
	wdy = (int)fdy;
	x-=wdx/2;	// posunu se do rohu
	y-=wdy/2;	
	if (x+wdx > rightx) x-=(x+wdx-rightx); // zarazky
	if (y+wdy > righty) y-=(y+wdy-righty);
	if (x < leftx) x+= (leftx - x);
	if (y < lefty) y+= (lefty - y);
	ScrollWX=x; ScrollWY=y;
}


////////////////////////////////////////////////////
// nascrolluje tak, aby bod x,y bul ve stredu
void CKerMapInfo::SetScrollCenter(int x, int y) {
	ScrollX=x; ScrollY=y;
	CalcScrollWindowXY(x,y);
	GEnMain->SetScroll((float)ScrollWX,(float)ScrollWY);
	if (editor) editor->RefreshMapVPabs(ScrollWX,ScrollWY);
}



////////////////////////////////////////////////////////
// Funkce zmensi vzdalenost dx a dy, pokud uz neni treba scrollovat.(jsem zarazen o okraj)
void CKerMapInfo::CalcScrollDistance(int x, int y, int *dx, int *dy) {
	float fdx,fdy;
	int wdx, wdy;
	GEnMain->GetWindowSize(fdx,fdy);
	wdx = (int)fdx;
	wdy = (int)fdy;
	x-=wdx/2;	// posunu se do rohu
	y-=wdy/2;	

	int x2=x+*dx,y2=y+*dy;
	if (x+wdx > rightx) x-=(x+wdx-rightx); // zarazky
	if (y+wdy > righty) y-=(y+wdy-righty);
	if (x < leftx) x+= (leftx - x);
	if (y < lefty) y+= (lefty - y);

	if (x2+wdx > rightx) x2-=(x2+wdx-rightx); // zarazky
	if (y2+wdy > righty) y2-=(y2+wdy-righty);
	if (x2 < leftx) x2+= (leftx - x2);
	if (y2 < lefty) y2+= (lefty - y2);

	*dx = x2-x;
	*dy = y2-y;
}

////////////////////////////////////////////////////////
// zaplne plynule scrollovani okna
void CKerMapInfo::WindowScroll(int dx, int dy,UI time) {
	int x = ScrollX;
	int y = ScrollY;
	CalcScrollWindowXY(x,y);
	ScrollX+=dx;
	ScrollY+=dy;
	CalcScrollDistance(x,y,&dx,&dy);

	GEnMain->Scroll((float)ScrollWX,(float)ScrollWY,(float)(ScrollWX+dx),(float)(ScrollWY+dy), time);
	ScrollWX+=dx; ScrollWY+=dy;
	if (editor) editor->RefreshMapVPabs(ScrollWX,ScrollWY);
}





//////////////////////////////////////////////////////////////////////////////
///
///		UMISTOVANI DO MAPY
///
//////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////
// umisti objekt do mapy
void CKerMapInfo::PlaceObjToMap(int CodeLine, OPointer obj) {
	CKerObject *ko = KerMain->GetAccessToObject(obj);
	SET_CODE_LINE(CodeLine);
	if (ko && !ko->elements) {
		// nactu promenne objektu:
		ko->PlacedInfo = new CKerPlacedInfo();
		ko->PlacedInfo->LoadObjectInfo(obj);
		ko->PlacedInfo->CalculateCube();
		ko->PlacedInfo->MConnectedObjs.Add(obj);
		if ((ko->PlacedInfo->CollCfg & eKCCareaMask) == eKCCoutOfMap) {
			ko->PlacedInfo->AktivAuto=0;
			ko->elements = new CGEnElement*[1];
			if (!(ko->PlacedInfo->CollCfg&eKCCinvisible)) {
				KerMain->AutosMain->ProcessAuto2(ko);
				if (!ko->PlacedInfo->AktivAuto) {
					// bez grafiky
					if (ko->Type->SpecificKnownVars[eKVUaPicture]) KerMain->Errors->LogError(eKRTEplacedOhNoGraphic);
				}
			}
			KerMain->message(CodeLine,obj,KnownNames[eKKNmapPlaced].Name,eKerCTmsg,0,0);
			REINIT_CONTEXT;
			return;
		}
		CKerArrInt *buff = CellColisionB(obj); // do jakych bunek se bude umistovat
	
		if (buff->GetCount()) {
			if (ko->PlacedInfo->CollCfg & eKCCtriggerBit) PlaceTriggerToMap(CodeLine,ko,buff);
			else {
				// vyprocessuju atomatismus:
	//			KerMain->AutosMain->ProcessToDoAutos();
				ko->PlacedInfo->AktivAuto=0;
				ko->elements = new CGEnElement*[1];
				KerMain->call(CodeLine,OMapa,KnownNames[eKKNplaceObjToMap].Name,2, eKTobject,KnownNames[eKKNobject].Name,obj, eKTarrInt,KnownNames[eKKNcellsArray].Name,buff);
				if (!(ko->PlacedInfo->CollCfg&eKCCinvisible)) {
					KerMain->AutosMain->ProcessAuto2(ko);
					if (!ko->PlacedInfo->AktivAuto) {
						// bez grafiky
						if (ko->Type->SpecificKnownVars[eKVUaPicture]) KerMain->Errors->LogError(eKRTEplacedOhNoGraphic);
					}
				}
				if (ko->Type->Name == KnownNames[eKKNscrollObj].Name) SetScrollCenter(ko->PlacedInfo->x,ko->PlacedInfo->y);
			}
			KerMain->message(CodeLine,obj,KnownNames[eKKNmapPlaced].Name,eKerCTmsg,0,0);
			
			// votestuju triggery:
			if (!(ko->PlacedInfo->CollCfg & eKCCtriggerBit)) {
				int f;
				CKerArrObject ts;
				GetTriggers(buff,&ts);
				for (f=0; f<ts.GetCount(); f++) {
					if (TestTrigger( KerMain->Objs->GetObject(ts[f]),ko)) {
						KerMain->message(CodeLine,ts[f],KnownNames[eKKNtriggerOn].Name,eKerCTmsg,0,2,eKTname,eKTobject,KnownNames[eKKNobjType].Name,KnownNames[eKKNobject].Name,ko->Type->Name ,obj);
					}
				}
			}
		} else KerMain->Errors->LogError(eKRTEplacingOutOfMap);
		if (!ko->elements) SAFE_DELETE(ko->PlacedInfo);
	}
	REINIT_CONTEXT;
}



////////////////////////////////////////////////////////////////////////
// pred umistenim znici vsechny kolidujici objekty. umisti objekt do mapy
void CKerMapInfo::PlaceObjToMapWithKill(int CodeLine, OPointer obj) {
	CKerArrObject objs;
	int f;
	if (!KerMain->Objs->GetObject(obj)) return;
	SET_CODE_LINE(CodeLine);
	// najdu kolidujici objekty a zavolam na ne KolisionKill:
	FindCollidingObjs(CodeLine,obj,&objs);
	for (f=0;f<objs.GetCount();f++) if (KerServices.ExistsObj(objs.Read(f))) KerMain->call(CodeLine,objs.Read(f),KnownNames[eKKNCollisionKill].Name,0);
	PlaceObjToMap(CodeLine,obj); // umistim objekt
}


////////////////////////////////////////////////////////
// umisti objekt do mapy pri nahravani hry
int CKerMapInfo::GPlaceObjToMap(CKerObject *ko, CFSRegKey *k) {
	ko->PlacedInfo = new CKerPlacedInfo();
	if (!ko->PlacedInfo->GLoad(k)) return 0;
	ko->elements = new CGEnElement*[1];
	if (ko->PlacedInfo->CollCfg & eKCCtriggerBit) {
		CKerArrInt *buff = CellColisionB(ko->thisO); // do jakych bunek se bude umistovat
		if (buff->GetCount()) PlaceTriggerToMap2(ko,buff);
	}
	if (!(ko->PlacedInfo->CollCfg&eKCCinvisible) && ko->PlacedInfo->AktivAuto) {
		KerMain->AutosMain->ChangeAktivAuto(ko,0,ko->PlacedInfo->AktivAuto);
	}
	if (ko->Type->Name == KnownNames[eKKNscrollObj].Name) SetScrollCenter(ko->PlacedInfo->x,ko->PlacedInfo->y);

	return 1;
}
			
	


////////////////////////////////////////////////////////
// odebere objekt z mapy
void CKerMapInfo::RemoveObjFromMap(int CodeLine, OPointer obj) {
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	SET_CODE_LINE(CodeLine);
	if (ko && ko->PlacedInfo) {
		KerMain->call(CodeLine,obj,KnownNames[eKKNmapRemoved].Name,0); // jeste zavolam objekt
		KerServices.MDisconnectObj(obj); // rozpojim se
		if (ko->PlacedInfo->CollCfg & eKCCtriggerBit) {
			RemoveTriggerFromMap(ko);
			return;
		}

		if ((ko->PlacedInfo->CollCfg & eKCCareaMask) == eKCCoutOfMap) {
			if (ko->PlacedInfo->AktivAuto) {
				// odeberu objekt z GEnu
				CKerAuto *A = ko->PlacedInfo->AktivAuto->Auto;
				int f;
				for (f=0;f<A->SonsNum;f++) {
					GEnMain->DeleteElement(ko->elements[f]);
				}
			} 
			SAFE_DELETE_ARRAY(ko->elements);
			SAFE_DELETE(ko->PlacedInfo);  // objekt uz neni umisten.
			REINIT_CONTEXT;
			return;
		}

		CKerArrInt *buff = CellColisionB(obj);	
		if (buff->GetCount()) {
			int f;
			CKerArrObject ts;
			GetTriggers(buff,&ts);
			for (f=0; f<ts.GetCount(); f++) {
				if (TestTrigger( KerMain->Objs->GetObject(ts[f]),ko)) {
					KerMain->message(CodeLine,ts[f],KnownNames[eKKNtriggerOff].Name,eKerCTmsg,0,2,eKTname,eKTobject,KnownNames[eKKNobjType].Name,KnownNames[eKKNobject].Name,ko->Type->Name ,obj);
				}
			}
			if (ko->PlacedInfo->AktivAuto) {
				// odeberu objekt z GEnu
				CKerAuto *A = ko->PlacedInfo->AktivAuto->Auto;
				for (f=0;f<A->SonsNum;f++) {
					GEnMain->DeleteElement(ko->elements[f]);
				}
				SAFE_DELETE_ARRAY(ko->elements);
				// odeberu objekt z mapy
				KerMain->call(CodeLine,OMapa,KnownNames[eKKNremoveObjFromMap].Name,2, eKTobject,KnownNames[eKKNobject].Name,obj, eKTarrInt,KnownNames[eKKNcellsArray].Name,buff);
				// odnavazu automatismus
				KerMain->AutosMain->ProcessAuto2(ko,1);
				SAFE_DELETE(ko->PlacedInfo);  // objekt uz neni umisten.
			} else {
				SAFE_DELETE_ARRAY(ko->elements);
				// odeberu objekt z mapy
				KerMain->call(CodeLine,OMapa,KnownNames[eKKNremoveObjFromMap].Name,2, eKTobject,KnownNames[eKKNobject].Name,obj, eKTarrInt,KnownNames[eKKNcellsArray].Name,buff);
				SAFE_DELETE(ko->PlacedInfo);  // objekt uz neni umisten.
			}

		}
		REINIT_CONTEXT;
	}
}



/////////////////////////////////////////////////////////////
// okamzite posune objekt na nove souradnice
void CKerMapInfo::MoveObjTo(int CodeLine, OPointer obj, int x, int y, int z) {
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	CKerObject *ko2;
	int f,c;
	if (!ko || !ko->elements) return;
	int relx = x - ko->PlacedInfo->x;
	int rely = y - ko->PlacedInfo->y;
	int relz = z - ko->PlacedInfo->z;

	c = ko->PlacedInfo->MConnectedObjs.GetCount();
	for (f=0; f<c; f++) {
		ko2 = KerMain->Objs->GetObject(ko->PlacedInfo->MConnectedObjs.Read(f));
		MoveObjTo2(CodeLine,ko2,ko2->thisO,ko2->PlacedInfo->x+relx,ko2->PlacedInfo->y+rely,ko2->PlacedInfo->z+relz);
	}
}


/////////////////////////////////////////////////////////////
// okamzite posune objekt relativne
void CKerMapInfo::MoveObjRel(int CodeLine, OPointer obj, int relx, int rely, int relz) {
	CKerObject *ko = KerMain->Objs->GetObject(obj);
	CKerObject *ko2;
	int f,c;
	if (!ko || !ko->elements) return;

	c = ko->PlacedInfo->MConnectedObjs.GetCount();
	for (f=0; f<c; f++) {
		ko2 = KerMain->Objs->GetObject(ko->PlacedInfo->MConnectedObjs.Read(f));
		MoveObjTo2(CodeLine,ko2,ko2->thisO,ko2->PlacedInfo->x+relx,ko2->PlacedInfo->y+rely,ko2->PlacedInfo->z+relz);
	}
}



/////////////////////////////////////////////////////////////
// okamzite posune objekt na nove souradnice
void CKerMapInfo::MoveObjTo2(int CodeLine, CKerObject *ko, OPointer obj, int x, int y, int z, int MoveGEnElemToo) {
	CKerArrInt Remove, Keep, Place;
	CKerAuto *A;
	int f,g,count;
	int sub1, sub2;
	CKerArrObject tr_1, tr_2;
	CKerArrObject *tr1=0, *tr2=0;

	// ToDo zjistit presne relativni posun a predat to mape
	if (MoveGEnElemToo && ko->PlacedInfo->Mover) delete ko->PlacedInfo->Mover;
	if (x==ko->PlacedInfo->x && y==ko->PlacedInfo->y && z==ko->PlacedInfo->z) return;

	// Pro OBjekty MIMO MAPU:
	if ((ko->PlacedInfo->CollCfg & eKCCareaMask) == eKCCoutOfMap) {
		// zmenim souradnoice
		KerMain->GetAccessToObject(ko);
		WriteObjCoords(ko,x,y,z);
		ko->PlacedInfo->x = x; ko->PlacedInfo->y = y; ko->PlacedInfo->z = z;
		if (MoveGEnElemToo && ko->Type->Name == KnownNames[eKKNscrollObj].Name) SetScrollCenter(ko->PlacedInfo->x,ko->PlacedInfo->y);

		if (ko->PlacedInfo->AktivAuto) {
//			KerMain->AutosMain->ProcessAuto2(ko);
			if (MoveGEnElemToo) {
				A = ko->PlacedInfo->AktivAuto->Auto;
				for (f=0; f<A->SonsNum; f++) ko->elements[f]->GEnSetPos((float)x, (float)y, (float)z);
			}
		}

		REINIT_CONTEXT;
		return;
	}

	CellColisionB(obj,&Place,x-ko->PlacedInfo->x,y-ko->PlacedInfo->y,z-ko->PlacedInfo->z);
	if (!(g=Place.GetCount()) /*|| 
	!KerMain->callint(CodeLine,obj,KnownNames[eKKNisMoveCorrect].Name,eKTint|eKTretAND,3, eKTint,KnownNames[eKKNcoordX].Name,x, eKTint,KnownNames[eKKNcoordY].Name,y, eKTint,KnownNames[eKKNcoordZ].Name,z) */) {
		if (!g) KerMain->Errors->LogError(eKRTEmovingOutOfMap);
		if (MoveGEnElemToo==0 && ko->PlacedInfo->Mover) {
			if (ko->PlacedInfo->AktivAuto) {
				count = ko->PlacedInfo->AktivAuto->Auto->SonsNum;
				for (f=0; f<count; f++) ko->elements[f]->GEnSetPos((float)ko->PlacedInfo->x, (float)ko->PlacedInfo->y, (float)ko->PlacedInfo->z);
			}
			ko->PlacedInfo->Mover->DeleteMe = 1;
		}
		return;
	}

	try {

		// odeberu obj ze stareho mista
		CellColisionB(obj,&Remove);
		if (ko->PlacedInfo->CollCfg&eKCCtriggerBit) {
			tr1 = GetObjsFromCellSet(CodeLine,&Remove,&tr_1);
			for (f=0; f<tr1->GetCount(); f++) {
				if (!TestTrigger(ko, KerMain->Objs->GetObject(tr1->Read(f)))) tr1->Get(f) = 0;
			}
		} else {
			tr1 = GetTriggers(&Remove,&tr_1);
			for (f=0; f<tr1->GetCount(); f++) {
				if (!TestTrigger( KerMain->Objs->GetObject(tr1->Read(f)),ko)) tr1->Get(f) = 0;
			}
		}

		// zmenim souradnoice
		KerMain->GetAccessToObject(ko);
		WriteObjCoords(ko,x,y,z);
		ko->PlacedInfo->x = x; ko->PlacedInfo->y = y; ko->PlacedInfo->z = z;
		if (MoveGEnElemToo && ko->Type->Name == KnownNames[eKKNscrollObj].Name) SetScrollCenter(ko->PlacedInfo->x,ko->PlacedInfo->y);

		// umistim obj
		if (ko->PlacedInfo->CollCfg&eKCCtriggerBit) {
			tr2 = GetObjsFromCellSet(CodeLine,&Place,&tr_2);
			for (f=0; f<tr2->GetCount(); f++) {
				if (!TestTrigger(ko, KerMain->Objs->GetObject(tr2->Read(f)))) tr2->Get(f) = 0;
			}
		} else {
			tr2 = GetTriggers(&Place,&tr_2);
			for (f=0; f<tr2->GetCount(); f++) {
				if (!TestTrigger( KerMain->Objs->GetObject(tr2->Read(f)),ko)) tr2->Get(f) = 0;
			}
		}

		// najdu prunik
		// TODO VYLEPSENI: udelat tag k bunkam a snizit slozitost z n*n na 3n
		sub1=0;
		for (f=0; f<Remove.GetCount()/3; f++) {
			sub2=0;
			Remove[(f-sub1)*3] = Remove[f*3];
			Remove[(f-sub1)*3+1] = Remove[f*3+1];
			Remove[(f-sub1)*3+2] = Remove[f*3+2];
			for (g=0; g<Place.GetCount()/3; g++) {
				Place[(g-sub2)*3] = Place[g*3];
				Place[(g-sub2)*3+1] = Place[g*3+1];
				Place[(g-sub2)*3+2] = Place[g*3+2];
				if (Place[g*3]==Remove[f*3] && Place[g*3+1]==Remove[f*3+1] && Place[g*3+2]==Remove[f*3+2]) {
					sub1++;
					sub2++;
					Keep.Add(Place[g*3]); Keep.Add(Place[g*3+1]); Keep.Add(Place[g*3+2]); 
				}
			}
			Place.SetCount(Place.GetCount()-sub2*3);
		}
		Remove.SetCount(Remove.GetCount()-sub1*3);
		if (ko->PlacedInfo->CollCfg&eKCCtriggerBit) {
			RemoveTriggerFromMap2(ko,&Remove);
			PlaceTriggerToMap2(ko,&Place);
		} else KerMain->call(CodeLine,OMapa,KnownNames[eKKNmoveObjInMap].Name,4, eKTobject,KnownNames[eKKNobject].Name,obj, eKTarrInt,KnownNames[eKKNremoveCellArray].Name,&Remove, eKTarrInt,KnownNames[eKKNkeepCellArray].Name,&Keep, eKTarrInt,KnownNames[eKKNplaceCellarray].Name,&Place);

		// zbytek umisteni objektu
		if (ko->PlacedInfo->AktivAuto) {
			KerMain->AutosMain->ProcessAuto2(ko);
			if (MoveGEnElemToo) {
				A = ko->PlacedInfo->AktivAuto->Auto;
				for (f=0; f<A->SonsNum; f++) ko->elements[f]->GEnSetPos((float)x, (float)y, (float)z);
			}
		}

		// aktivovani triggeru:
		CKerObject *ko2;
		for (f=0; f<tr1->GetCount(); f++) if (ko2 = KerMain->Objs->GetObject(tr1->Read(f))) ko2->Tag = 1;
		for (f=0; f<tr2->GetCount(); f++) if (ko2 = KerMain->Objs->GetObject(tr2->Read(f))) ko2->Tag++;
		for (f=0; f<tr1->GetCount(); f++) if (ko2 = KerMain->Objs->GetObject(tr1->Read(f))) {
			if (ko2->Tag == 1) {
				if (ko->PlacedInfo->CollCfg&eKCCtriggerBit)
					KerMain->message(CodeLine,obj,KnownNames[eKKNtriggerOff].Name,eKerCTmsg,0,2,eKTname,eKTobject,KnownNames[eKKNobjType].Name,KnownNames[eKKNobject].Name,ko2->Type->Name ,tr1->Read(f));
				else KerMain->message(CodeLine,tr1->Read(f),KnownNames[eKKNtriggerOff].Name,eKerCTmsg,0,2,eKTname,eKTobject,KnownNames[eKKNobjType].Name,KnownNames[eKKNobject].Name,ko->Type->Name  ,obj);
			}
			ko2->Tag = 0;
		}
		for (f=0; f<tr2->GetCount(); f++) if (ko2 = KerMain->Objs->GetObject(tr2->Read(f))) {
			if (ko2->Tag == 1) {
				if (ko->PlacedInfo->CollCfg&eKCCtriggerBit)
					KerMain->message(CodeLine,obj,KnownNames[eKKNtriggerOn].Name,eKerCTmsg,0,2,eKTname,eKTobject,KnownNames[eKKNobjType].Name,KnownNames[eKKNobject].Name,ko2->Type->Name ,tr2->Read(f));
				else KerMain->message(CodeLine,tr2->Read(f),KnownNames[eKKNtriggerOn].Name,eKerCTmsg,0,2,eKTname,eKTobject,KnownNames[eKKNobjType].Name,KnownNames[eKKNobject].Name,ko->Type->Name ,obj);
			}
			ko2->Tag = 0;
		}

	} catch (CKernelPanic) {
		if (tr1 != &tr_1) SAFE_DELETE(tr1);
		if (tr2 != &tr_2) SAFE_DELETE(tr2);
	}

	if (tr1 != &tr_1) SAFE_DELETE(tr1);
	if (tr2 != &tr_2) SAFE_DELETE(tr2);
	REINIT_CONTEXT;
}



///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// umisti trigger do mapy
void CKerMapInfo::PlaceTriggerToMap(int CodeLine, CKerObject *ko, CKerArrInt *CellsBuff) {
	// umisteni:
	int f, pos;
	int count = CellsBuff->GetCount() / 3;
	for (f=0; f<count; f++) {
		pos = CellsBuff->Read(f*3)-FirstCellx + (CellsBuff->Read(f*3+1)-FirstCelly)*NumCellsx + (CellsBuff->Read(f*3+2)-lowerlevel)*NumCellsx*NumCellsy;
		Triggers[pos] = new CKerMapTrigger(ko->thisO,Triggers[pos]);
	}
	ko->elements = new CGEnElement*[1];

	// Triggerovani:
	CKerArrObject Objs2, *Objs;
	Objs = GetObjsFromCellSet(CodeLine,CellsBuff,&Objs2);
	for (f=0; f<Objs->GetCount(); f++) {
		if (TestTrigger(ko,KerMain->Objs->GetObject(Objs->Read(f)))) {
			KerMain->message(CodeLine,ko->thisO,KnownNames[eKKNtriggerOn].Name,eKerCTmsg,0,2,eKTname,eKTobject,KnownNames[eKKNobjType].Name,KnownNames[eKKNobject].Name,KerMain->GetObjType(Objs->Read(f)) ,Objs->Read(f));
		}
	}
	if (Objs!=&Objs2) delete Objs;
		
}


// odebere trigger z mapy
void CKerMapInfo::RemoveTriggerFromMap(CKerObject *ko) {
	CKerArrInt *CellsBuff = CellColisionB(ko->thisO);
	int count = CellsBuff->GetCount() / 3;
	int f, pos;
	CKerMapTrigger **tr1, *tr2;


	for (f=0; f<count; f++) {
		pos = CellsBuff->Read(f*3)-FirstCellx + (CellsBuff->Read(f*3+1)-FirstCelly)*NumCellsx + (CellsBuff->Read(f*3+2)-lowerlevel)*NumCellsx*NumCellsy;
		tr1 = &(Triggers[pos]);
		while (*tr1) {
			if ((**tr1).triger == ko->thisO) {
				tr2 = *tr1;
				*tr1 = tr2->next;
				delete tr2;
				break;
			} 
			tr1 = &((**tr1).next);
		}
	}
	SAFE_DELETE_ARRAY(ko->elements);
	SAFE_DELETE(ko->PlacedInfo);  
}




// umisti cast triggeru do mapy
void CKerMapInfo::PlaceTriggerToMap2(CKerObject *ko, CKerArrInt *CellsBuff) {
	// umisteni:
	int f, pos;
	int count = CellsBuff->GetCount() / 3;
	for (f=0; f<count; f++) {
		pos = CellsBuff->Read(f*3)-FirstCellx + (CellsBuff->Read(f*3+1)-FirstCelly)*NumCellsx + (CellsBuff->Read(f*3+2)-lowerlevel)*NumCellsx*NumCellsy;
		Triggers[pos] = new CKerMapTrigger(ko->thisO,Triggers[pos]);
	}		
}


// odebere cast triggeru z mapy
void CKerMapInfo::RemoveTriggerFromMap2(CKerObject *ko, CKerArrInt *CellsBuff) {
	int count = CellsBuff->GetCount() / 3;
	int f, pos;
	CKerMapTrigger **tr1, *tr2;
	for (f=0; f<count; f++) {
		pos = CellsBuff->Read(f*3)-FirstCellx + (CellsBuff->Read(f*3+1)-FirstCelly)*NumCellsx + (CellsBuff->Read(f*3+2)-lowerlevel)*NumCellsx*NumCellsy;
		tr1 = &(Triggers[pos]);
		while (*tr1) {
			if ((**tr1).triger == ko->thisO) {
				tr2 = *tr1;
				*tr1 = tr2->next;
				delete tr2;
				break;
			} 
			tr1 = &((**tr1).next);
		}
	}
}



// najde vsechny triggery v oblasti. Druhym parametrem se da specifukovat pole, kam funkce bude vracet. Jinak je pole alokovano
CKerArrObject *CKerMapInfo::GetTriggers(CKerArrInt *CellsArray, CKerArrObject *ObjectArray) {
	if (!ObjectArray) ObjectArray = new CKerArrObject;
	ObjectArray->SetCount(0);
	if (!CellsArray) return ObjectArray;
	DeleteOutOfMapCalls(*CellsArray);
	int f,count,pos;
	CKerMapTrigger *p;
	CKerObject *ko;
	
	count = CellsArray->GetCount()/3;
	for (f=0; f<count; f++) {
		pos = CellsArray->Read(f*3)-FirstCellx + (CellsArray->Read(f*3+1)-FirstCelly)*NumCellsx + (CellsArray->Read(f*3+2)-lowerlevel)*NumCellsx*NumCellsy;
		p = Triggers[pos];
		while (p) {
			ko = KerMain->Objs->GetObject(p->triger);
			if (ko && ko->PlacedInfo && ko->Tag==0) {
				ko->Tag=1;
				ObjectArray->Add(p->triger);
			}
			p = p->next;
		}
	}	
	
	count = ObjectArray->GetCount();
	for (f=0; f<count; f++) KerServices.ClearTag(ObjectArray->Read(f));
	
	return ObjectArray;
}



// otestuje kolizi trigeru a objektu
int CKerMapInfo::TestTrigger(CKerObject *kTrigger, CKerObject *ko) {
	if (!kTrigger || !ko) return 0;
	KerMain->GetAccessToObject(kTrigger);
	CKerName *oName;
	CKerName *AddGr, *SubGr, *FceGr;
	CKerCollisionInfo *ObjInfo1 = kTrigger->PlacedInfo;
	CKerCollisionInfo *ObjInfo2 = ko->PlacedInfo;

	// nahraju info o objektu
	ReadColisionGroups(kTrigger,&AddGr,&SubGr,&FceGr);
	oName = ko->Type->Name;
	// mnozinovy test:
	if (KerServices.IsFMemberOfS(oName,AddGr)) {  // je obj v addgroup?
		if (!KerServices.IsFMemberOfS(oName,SubGr)) { // je obj mimo subgroup?
			if (!(ObjInfo1->CollCfg & ObjInfo2->CollCfg & eKCClevelMask)) {
				return 0;// neni kolize
			} else {
				// geometricky test:
				if (ObjInfo1->CollCfg&eKCCcenterColBit) {
					if (((ObjInfo1->CubeX1+ObjInfo1->x <= ObjInfo2->x && ObjInfo1->CubeX2+ObjInfo1->x >= ObjInfo2->x) ) &&
					((ObjInfo1->CubeY1+ObjInfo1->y <= ObjInfo2->y && ObjInfo1->CubeY2+ObjInfo1->y >= ObjInfo2->y) ) &&
					((ObjInfo1->CubeZ1+ObjInfo1->z <= ObjInfo2->z && ObjInfo1->CubeZ2+ObjInfo1->z >= ObjInfo2->z) )) {
						if ( KerServices.IsFMemberOfS(oName,FceGr)) {
							if (KerMain->callint(0,kTrigger->thisO,KnownNames[eKKNtestCollision].Name,eKTint,1, eKTobject,KnownNames[eKKNobject].Name,ko->thisO)) return 1;
						} else return 1;
					}
				} else if (((ObjInfo1->CubeX1+ObjInfo1->x < ObjInfo2->CubeX2+ObjInfo2->x && ObjInfo1->CubeX2+ObjInfo1->x > ObjInfo2->CubeX1+ObjInfo2->x) || ObjInfo1->CubeX1+ObjInfo1->x == ObjInfo2->CubeX1+ObjInfo2->x) &&
					((ObjInfo1->CubeY1+ObjInfo1->y < ObjInfo2->CubeY2+ObjInfo2->y && ObjInfo1->CubeY2+ObjInfo1->y > ObjInfo2->CubeY1+ObjInfo2->y) || ObjInfo1->CubeY1+ObjInfo1->y == ObjInfo2->CubeY1+ObjInfo2->y) &&
					((ObjInfo1->CubeZ1+ObjInfo1->z < ObjInfo2->CubeZ2+ObjInfo2->z && ObjInfo1->CubeZ2+ObjInfo1->z > ObjInfo2->CubeZ1+ObjInfo2->z) || ObjInfo1->CubeZ1+ObjInfo1->z == ObjInfo2->CubeZ1+ObjInfo2->z)) {
						if ( KerServices.IsFMemberOfS(oName,FceGr)) {
							if (KerMain->callint(0,kTrigger->thisO,KnownNames[eKKNtestCollision].Name,eKTint,1, eKTobject,KnownNames[eKKNobject].Name,ko->thisO)) return 1;
						} else return 1;
					}
			}
		}
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////
///
///			I N I T A C E   M A P Y
///
////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////
//  Vola objekt mapa ve svem konstruktoru.
void CKerMapInfo::RegisterMap(int _CellType, int _leftx, int _lefty, int _rightx, int _righty, int _lowerlevel, int _upperlevel, int cellsizeX, int cellsizeY, int cellsizeZ) {
	DeleteTriggers();
	registered = 1;
	CellType = _CellType;
	leftx = _leftx; lefty = _lefty; rightx = _rightx; righty = _righty;
	lowerlevel = _lowerlevel;  upperlevel = _upperlevel; 
	CellSizeX = cellsizeX; CellSizeY = cellsizeY; CellSizeZ = cellsizeZ; 
	GetNumberOfCells2();
	NewTriggers();

	OMapa = KerContext->KCthis;
}




///////////////////////////////////////////////
// nehraje Map Info z ulozeny hry
int CKerMapInfo::GLoadMapInfo(CFSRegKey *k) {
	if (!k) return 0;
	CFSRegister *r = k->GetSubRegister();

	DeleteTriggers();

	if (k=r->FindKey("registered")) registered = k->readi(); else RETERR;
	if (k=r->FindKey("CellType")) CellType = k->readi(); else RETERR;
	if (k=r->FindKey("leftx")) leftx = k->readi(); else RETERR;
	if (k=r->FindKey("lefty")) lefty = k->readi(); else RETERR;
	if (k=r->FindKey("rightx")) rightx = k->readi(); else RETERR;
	if (k=r->FindKey("righty")) righty = k->readi(); else RETERR;
	if (k=r->FindKey("lowerlevel")) lowerlevel = k->readi(); else RETERR;
	if (k=r->FindKey("upperlevel")) upperlevel = k->readi(); else RETERR;
	if (k=r->FindKey("CellSizeX")) CellSizeX = k->readi(); else RETERR;
	if (k=r->FindKey("CellSizeY")) CellSizeY = k->readi(); else RETERR;
	if (k=r->FindKey("CellSizeZ")) CellSizeZ = k->readi(); else RETERR;

	GetNumberOfCells2();

	if (k=r->FindKey("OMapa")) OMapa = k->readi(); else RETERR;
	if (k=r->FindKey("ScrollX")) ScrollX = k->readi(); else RETERR;
	if (k=r->FindKey("ScrollY")) ScrollY = k->readi(); else RETERR;
	if (k=r->FindKey("ScrollWX")) ScrollWX = k->readi(); else RETERR;
	if (k=r->FindKey("ScrollWY")) ScrollWY = k->readi(); else RETERR;

	NewTriggers();
	return 1;
}


// ulozi Map Info
void CKerMapInfo::GSaveMapInfo(CFSRegKey *k) {
	CFSRegister *r = k->GetSubRegister();

	r->AddKey("registered",FSRTint)->writei(registered);
	r->AddKey("CellType",FSRTint)->writei(CellType);
	r->AddKey("leftx",FSRTint)->writei(leftx);
	r->AddKey("lefty",FSRTint)->writei(lefty);
	r->AddKey("rightx",FSRTint)->writei(rightx);
	r->AddKey("righty",FSRTint)->writei(righty);
	r->AddKey("lowerlevel",FSRTint)->writei(lowerlevel);
	r->AddKey("upperlevel",FSRTint)->writei(upperlevel);
	r->AddKey("CellSizeX",FSRTint)->writei(CellSizeX);
	r->AddKey("CellSizeY",FSRTint)->writei(CellSizeY);
	r->AddKey("CellSizeZ",FSRTint)->writei(CellSizeZ);

	r->AddKey("OMapa",FSRTint)->writei(OMapa);
	r->AddKey("ScrollX",FSRTint)->writei(ScrollX);
	r->AddKey("ScrollY",FSRTint)->writei(ScrollY);
	r->AddKey("ScrollWX",FSRTint)->writei(ScrollWX);
	r->AddKey("ScrollWY",FSRTint)->writei(ScrollWY);
}



/////////////////////////
// pomocna fce
void RMPlaceObjAgain2(CKerObject *ko) {
	KerMain->MapInfo->RMPlaceObjAgain(ko);
}

//////////////////////////////////////////////////////////////////
// volat pote, co se zmeni vlastnosti mapy. Dojde k Reregistraci. znovu se umisti objekty (trigery), do mapy. Pokud je Obj mimo, bude zabit
void CKerMapInfo::ResizeMap() {
	KerMain->call(0,OMapa,KnownNames[eKKNresizeMap].Name,0);
	KerMain->Objs->WalkThrowAllObjs(&RMPlaceObjAgain2);
	REINIT_CONTEXT;
}

/////////////////////////////////////////////////////////////////
// znovu umisti objekty (triger), do mapy. Pokud je Obj mimo, bude zabit
void CKerMapInfo::RMPlaceObjAgain(CKerObject *ko) {
	if (ko->elements) {
		CKerArrInt *buff = CellColisionB(ko->thisO); // do jakych bunek se bude umistovat
	
		if (buff->GetCount()) {
			if (ko->PlacedInfo->CollCfg & eKCCtriggerBit) PlaceTriggerToMap2(ko,buff);
			else {
				KerMain->call(0,OMapa,KnownNames[eKKNplaceObjToMap].Name,2, eKTobject,KnownNames[eKKNobject].Name,ko->thisO, eKTarrInt,KnownNames[eKKNcellsArray].Name,buff);
			} 
		} else {
			// odeber a kill it
			KerMain->call(0,ko->thisO,KnownNames[eKKNmapRemoved].Name,0); // jeste zavolam objekt
			KerServices.MDisconnectObj(ko->thisO); // rozpojim se
			if (ko->PlacedInfo->AktivAuto) {
				int f;
				// odeberu objekt z GEnu
				CKerAuto *A = ko->PlacedInfo->AktivAuto->Auto;
				for (f=0;f<A->SonsNum;f++) {
					GEnMain->DeleteElement(ko->elements[f]);
				}
				SAFE_DELETE_ARRAY(ko->elements);
				SAFE_DELETE(ko->PlacedInfo);  // objekt uz neni umisten.
			} else {
				SAFE_DELETE_ARRAY(ko->elements);
				SAFE_DELETE(ko->PlacedInfo);  // objekt uz neni umisten.
			}
			KerMain->DeleteObject(0,ko->thisO);
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
//		KONSTRUKTOR
//		Nastavi se defaultni rozmery levlu
CKerMapInfo::CKerMapInfo() {
	Movers = 0;
	registered = 0;
	CellType = eKCTctverce;
	leftx = 0; lefty = 0; rightx = 100; righty = 100;
	lowerlevel = 0;  upperlevel = 0; 
	CellSizeX = 20;  CellSizeY = 20;  CellSizeZ = 20; 
	OMapa=0;
	ScrollX=0; ScrollY=0;
	ScrollWX=0; ScrollWY=0;
	GetNumberOfCells2();
	NewTriggers();
}



void CKerMapInfo::DeleteTriggers(int Kill) {
	int f;
	if (!Triggers) return;
	int count = NumCellsx*NumCellsy*(upperlevel-lowerlevel+1);
	CKerMapTrigger *tr, *tr2;
	for (f=0;f<count;f++) {
		if (Kill) {
			while (Triggers[f]) KerMain->DeleteObject(0,Triggers[f]->triger);
		} else {
			tr = Triggers[f];
			while (tr) {
				tr2 = tr;
				tr = tr->next;
				delete tr2;
			}
		}
	}
	SAFE_DELETE_ARRAY(Triggers);
}


//////////////////////////////////////
/// Destruktor
CKerMapInfo::~CKerMapInfo() {
	while (Movers) delete Movers;
	DeleteTriggers();
}



///////////////////////////////////////////////////////////////////////////////
//	Spocita kolik a jeke bunky se do vymezeneho herniho okna vejdou
void CKerMapInfo::GetNumberOfCells2() {
	int endx, endy;
	switch (CellType) {
		case eKCTctverce:
			FirstCellx = DownDivide(leftx + CellSizeX/2 , CellSizeX);
			FirstCelly = DownDivide(lefty + CellSizeY/2 , CellSizeY);
			endx = DownDivide(rightx-1 + CellSizeX/2 , CellSizeX);
			endy = DownDivide(righty-1 + CellSizeY/2 , CellSizeY);
			break;
		case eKCTkosoctverce:
		//	FirstCellx = (int)floor(leftx / (CellSizeX/2) + MEPSILON);
		//	FirstCelly = (int)floor(lefty / CellSizeY + MEPSILON);
		//	endx = (int)floor(rightx / (CellSizeX/2) + 1 - MEPSILON);
		//	endy = (int)floor(righty / CellSizeY + 0.5 - MEPSILON);
		//	break;
		//case eKCTplocheHexy:
		//	FirstCellx = (int)floor(leftx / CellSizeX + 1/3.0 +  MEPSILON);
		//	FirstCelly = (int)floor(lefty / CellSizeY + MEPSILON);
		//	endx = (int)floor(rightx / CellSizeX + 1 - 1/3.0 - MEPSILON);
		//	endy = (int)floor(righty / CellSizeY + 0.5 - MEPSILON);
		//	break;
		//case eKCTspicateHexy:
		//	FirstCellx = (int)floor(leftx / CellSizeX + MEPSILON);
		//	FirstCelly = (int)floor(lefty / CellSizeY + 1/3.0 +  MEPSILON);
		//	endx = (int)floor(rightx / CellSizeX + 0.5 - MEPSILON);
		//	endy = (int)floor(righty / CellSizeY + 1 - 1/3.0 - MEPSILON);
			break;
	}
	NumCellsx = endx - FirstCellx +1;
	NumCellsy = endy - FirstCelly +1;
}





///////////////////////////////////////////////////////////////////////
///
///		P R A C E   S   B U N K A M A
///		hledani, prevod souradnic, zjistovani objektu
///
///////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
// fce se zepta mapy, jake objekty jsou na dane bunce. 
// Fce muze ale nemusi(!) vyuzit pole RetObjs pro vraceceni objektu.
// V pripade ze fce pole naalokuje nekde jinde je treba ho dealokovat
CKerArrObject *CKerMapInfo::GetObjsFromCell(int CodeLine, int cx, int cy, int cz, CKerArrObject *RetObjs) { 
	CKerArrObject *objs;
	CKerArrInt buff;
	buff.Add(cx); buff.Add(cy); buff.Add(cz);
	objs = (CKerArrObject*)KerMain->callint(CodeLine,OMapa,KnownNames[eKKNmGetObjects].Name,eKTarrObject,2, eKTarrInt, KnownNames[eKKNcellsArray].Name,&buff, eKTarrObject, KnownNames[eKKNobjectArray].Name,RetObjs);
    if (!objs) KerMain->Errors->LogError(eKRTEmapNotWorking,0,"MGetObjs");
	return objs;
}


////////////////////////////////////////////////////////////////////
// fce se zepta mapy, jake objekty jsou na danych bunkach. 
// Pole ma tvar B1x,B1y,B1z, B2x,B2y,B2z, ... 
// Fce muze ale nemusi(!) vyuzit pole RetObjs pro vraceceni objektu. 
// V pripade ze fce pole naalokuje nekde jinde je treba ho dealokovat
CKerArrObject *CKerMapInfo::GetObjsFromCellSet(int CodeLine, CKerArrInt *CellsArray, CKerArrObject *RetObjs) { 
	CKerArrObject *objs;
	objs = (CKerArrObject*)KerMain->callint(CodeLine,OMapa,KnownNames[eKKNmGetObjects].Name,eKTarrObject,2, eKTarrInt, KnownNames[eKKNcellsArray].Name,CellsArray, eKTarrObject, KnownNames[eKKNobjectArray].Name,RetObjs);
    if (!objs) KerMain->Errors->LogError(eKRTEmapNotWorking,0,"MGetObjs");
	return objs;
}




///////////////////////////////////////////////////////////////////////////
// zjisti do ktere bunky bod x,y,z patri. Vrati 0, jestlize jsi mimo mapu, jinak 1
int CKerMapInfo::FindCell(int x, int y, int z, int &cx, int &cy, int &cz) {
	switch (CellType) {
		case eKCTctverce:
			cx = DownDivide(x*2 + CellSizeX , CellSizeX*2);
			cy = DownDivide(y*2 + CellSizeY , CellSizeY*2);
			cz = DownDivide(z*2 + CellSizeZ , CellSizeZ*2);
			break;
	}
	return IsCellInMap(cx,cy,cz);
}




///////////////////////////////////////////////////////////////
// ze souradnic zjisti primarni bunku objektu. Napred je treba zavolat GetAccessToObj
int CKerMapInfo::FindObjCell(CKerObject *kObj, int &cx, int &cy, int &cz) {
	if (!kObj->PlacedInfo) {
		ReadObjCoords(kObj,cx,cy,cz);
		return FindCell(cx,cy,cz,cx,cy,cz);
	} else {
		return FindCell(kObj->PlacedInfo->x,kObj->PlacedInfo->y,kObj->PlacedInfo->z,cx,cy,cz);
	}
}




///////////////////////////////////////////////////////////////////////////
// Najde centralni pozici bunky
void CKerMapInfo::FindCellPosition(int cx, int cy, int cz, int &x, int &y, int &z) {
	switch (CellType) {
		case eKCTctverce:
				x = cx * CellSizeX;
				y = cy * CellSizeY;
				z = cz * CellSizeZ;
				break;
	}
}








//////////////////////////////////////////////////////////////////////////////
///
///		D E T E K C E   K O L I Z I
///
///////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
// vrati pole trojic souradnic x y z bunek do kterych objekt zasahuje. Funkce bud vyuzije OutPutBuff, nebo sveje vzdy stejne pole v tompripade nedealokovat
CKerArrInt * CKerMapInfo::CellColisionB(OPointer Obj, CKerArrInt *OutPutBuff, int dx, int dy, int dz ) {
	int a,b,c;
	int a1,b1,c1;
	int x,y,z;
	int f,g,h;
	int ff;
	CKerArrInt *CollBuff;
	CKerCollisionInfo CollisionInfo;
	CKerCollisionInfo *CollInfo = &CollisionInfo;
	
	CKerObject *ko = KerMain->Objs->GetObject(Obj);
	if (OutPutBuff) CollBuff = OutPutBuff;		// nastavim si, kam pujde vystup
	else CollBuff = &CollBuffB;			
	(*CollBuff).SetCount(0);
	if (ko) {
		// nactu data z objektu:
		if (ko->PlacedInfo) CollInfo = ko->PlacedInfo;   
		else CollInfo->LoadObjectInfo(Obj);
		x = CollInfo->x+dx;
		y = CollInfo->y+dy;
		z = CollInfo->z+dz;
		switch (CellType) {
			case eKCTctverce:
				switch(CollInfo->CollCfg&eKCCareaMask) {
					case eKCCpoint:
						(*CollBuff)[0] = DownDivide(x*2 + CellSizeX , CellSizeX*2);
						(*CollBuff)[1] = DownDivide(y*2 + CellSizeY , CellSizeY*2);
						(*CollBuff)[2] = DownDivide(z*2 + CellSizeZ , CellSizeZ*2);
						break;
					case eKCConeCell:
						a1 = DownDivide(x, CellSizeX);
						b1 = DownDivide(y, CellSizeY);
						c1 = DownDivide(z, CellSizeZ);
						a = DownDivide(x+CellSizeX-1 , CellSizeX) - a1;
						b = DownDivide(y+CellSizeY-1 , CellSizeY) - b1;
						c = DownDivide(z+CellSizeZ-1 , CellSizeZ) - c1;
						ff = 0; 
						for (f=0;f<=a;f++) for (g=0;g<=b;g++) for (h=0;h<=c;h++) {
							(*CollBuff)[ff*3] = a1+f;
							(*CollBuff)[ff*3+1] = b1+g;
							(*CollBuff)[ff*3+2] = c1+h;
							ff++;
						}
						break;
					case eKCCrect1:
						if (!CollInfo->CellX) {
							a1 = DownDivide(x + CellSizeX/2, CellSizeX);
							a = 0;
						} else {
							a1 = DownDivide(x, CellSizeX);
							a = DownDivide(x+CellSizeX*CollInfo->CellX-1 , CellSizeX) - a1;
						}
						if (!CollInfo->CellY) {
							b1 = DownDivide(y + CellSizeY/2, CellSizeY);
							b = 0;
						} else {
							b1 = DownDivide(y, CellSizeY);
							b = DownDivide(y+CellSizeY*CollInfo->CellY-1 , CellSizeY) - b1;
						}
						if (!CollInfo->CellZ) {
							c1 = DownDivide(z + CellSizeZ/2, CellSizeZ);
							c = 0;
						} else {
							c1 = DownDivide(z, CellSizeZ);
							c = DownDivide(z+CellSizeZ*CollInfo->CellZ-1 , CellSizeZ) - c1;
						}
						ff = 0; 
						for (f=0;f<=a;f++) for (g=0;g<=b;g++) for (h=0;h<=c;h++) {
							(*CollBuff)[ff*3] = a1+f;
							(*CollBuff)[ff*3+1] = b1+g;
							(*CollBuff)[ff*3+2] = c1+h;
							ff++;
						}
						break;
					case eKCCcolCube:
						a1 = DownDivide(x+CollInfo->CubeX1+CellSizeX/2, CellSizeX);
						b1 = DownDivide(y+CollInfo->CubeY1+CellSizeY/2, CellSizeY);
						c1 = DownDivide(z+CollInfo->CubeZ1+CellSizeZ/2, CellSizeZ);
						a = DownDivide(x+CollInfo->CubeX2+CellSizeX/2, CellSizeX) - a1;
						b = DownDivide(y+CollInfo->CubeY2+CellSizeY/2, CellSizeY) - b1;
						c = DownDivide(z+CollInfo->CubeZ2+CellSizeZ/2, CellSizeZ) - c1;
						ff = 0; 
						for (f=0;f<=a;f++) for (g=0;g<=b;g++) for (h=0;h<=c;h++) {
							(*CollBuff)[ff*3] = a1+f;
							(*CollBuff)[ff*3+1] = b1+g;
							(*CollBuff)[ff*3+2] = c1+h;
							ff++;
						}
						break;
				}
				break;
		}
	}
	DeleteOutOfMapCalls((*CollBuff));
	REINIT_CONTEXT;
	return CollBuff;
}



////////////////////////////////////////////////////////
// z pole odstrani bunky, ktere jsou mimo mapu
void CKerMapInfo::DeleteOutOfMapCalls(CKerArrInt &Buff) {
	int f=0, sub=0;
	int arraysize = Buff.GetCount()/3;
	for(f=0;f<arraysize;f++) {
		if (Buff[f*3]<FirstCellx || Buff[f*3]-FirstCellx>=NumCellsx || Buff[f*3+1]<FirstCelly || Buff[f*3+1]-FirstCelly>=NumCellsy || Buff[f*3+2]<lowerlevel || Buff[f*3+2]>upperlevel) sub++;
		else { Buff[(f-sub)*3] = Buff[f*3]; Buff[(f-sub)*3+1] = Buff[f*3+1]; Buff[(f-sub)*3+2] = Buff[f*3+2]; }
	}
	arraysize -= sub;
	Buff.SetCount(arraysize*3);
}


/// Overi, zda je bunka v mape
int CKerMapInfo::IsCellInMap(int cx, int cy, int cz) {
	if (cx<FirstCellx || cx-FirstCellx>=NumCellsx || cy<FirstCelly || cy-FirstCelly>=NumCellsy || cz<lowerlevel || cz>upperlevel) return 0;
	else return 1;
}





//////////////////////////////////////////////////////////
// vrati 1 pokud objekt koliduje, jinak 0, vrati 1 take pokud je obj mimo mapu
int CKerMapInfo::IsObjInCollision(int CodeLine, OPointer obj, int dx, int dy, int dz) {
	CKerObject *ko = KerMain->GetAccessToObject(obj);
	if (!ko) {
		REINIT_CONTEXT;
		return 0;
	}
	SET_CODE_LINE(CodeLine);
	CKerCollisionInfo CollInfo;
	CKerCollisionInfo *ObjInfo1 = &CollInfo, *ObjInfo2;
	int ret = 0;
	int f;
	CKerArrInt *buff;
	CKerArrObject *objs = 0;
	CKerArrObject objs2;
	CKerName *oName;
	CKerName *AddGr, *SubGr, *FceGr;

	// nahraju info o objektu
	if (ko->PlacedInfo) ObjInfo1 = ko->PlacedInfo;
	else {
		ObjInfo1->LoadObjectInfo(obj);
		ObjInfo1->CalculateCube();
	}
	ReadColisionGroups(ko,&AddGr,&SubGr,&FceGr);
	buff = CellColisionB(obj,0,dx,dy,dz); // zjistim kolizi s bunkama
	if (buff->GetCount()==0) {
		// objekt je mimo mapu
		REINIT_CONTEXT;
		return 1;
	}
	// nactu objekty
	objs = (CKerArrObject*)KerMain->callint(CodeLine,OMapa,KnownNames[eKKNmGetObjects].Name,eKTarrObject,2, eKTarrInt, KnownNames[eKKNcellsArray].Name,buff, eKTarrObject, KnownNames[eKKNobjectArray].Name,&objs2);
	if (!objs) KerMain->Errors->LogError(eKRTEmapNotWorking,0,"MGetObjects");
	for (f=0;f<objs->GetCount();f++) if (objs->Read(f) != obj) {
		oName = KerMain->GetObjType(objs->Read(f));
		// mnozinovy test:
		if (KerServices.IsFMemberOfS(oName,AddGr)) {  // je obj v addgroup?
			if (!KerServices.IsFMemberOfS(oName,SubGr)) { // je obj mimo subgroup?
				ObjInfo2 = KerMain->Objs->GetObject(objs->Read(f))->PlacedInfo;
				if (((ObjInfo1->CollCfg&eKCCareaMask) == eKCCpoint && (ObjInfo2->CollCfg&eKCCareaMask) == eKCCpoint) || !(ObjInfo1->CollCfg & ObjInfo2->CollCfg & eKCClevelMask) ) {
					// neni kolize
				} else {
					// geometricky test:
					bool floor = (ObjInfo1->CubeZ1+ObjInfo1->z+dz == ObjInfo2->CubeZ1+ObjInfo2->z) && (ObjInfo1->CollCfg & ObjInfo2->CollCfg & eKCCfloor);
					if ((ObjInfo1->CubeX1+ObjInfo1->x+dx < ObjInfo2->CubeX2+ObjInfo2->x && ObjInfo1->CubeX2+ObjInfo1->x+dx > ObjInfo2->CubeX1+ObjInfo2->x) &&
						(ObjInfo1->CubeY1+ObjInfo1->y+dy < ObjInfo2->CubeY2+ObjInfo2->y && ObjInfo1->CubeY2+ObjInfo1->y+dy > ObjInfo2->CubeY1+ObjInfo2->y) &&
						((ObjInfo1->CubeZ1+ObjInfo1->z+dz < ObjInfo2->CubeZ2+ObjInfo2->z && ObjInfo1->CubeZ2+ObjInfo1->z+dz > ObjInfo2->CubeZ1+ObjInfo2->z) || floor)) {
							if ( KerServices.IsFMemberOfS(oName,FceGr)) {
								if (KerMain->callint(CodeLine,obj,KnownNames[eKKNtestCollision].Name,eKTint,1, eKTobject,KnownNames[eKKNobject].Name,objs->Read(f))) {
									ret = 1;
									break;
								}
							} else {
								ret = 1;
								break;
							}
						}
				}
			}
		}
	}

	REINIT_CONTEXT;
	if (objs!=&objs2) SAFE_DELETE(objs);
	return ret;
}



//////////////////////////////////////////////////////////
// Vrati seznam objektu se kterymi objekt koliduje
CKerArrObject *CKerMapInfo::FindCollidingObjs(int CodeLine, OPointer obj, CKerArrObject *ret, int dx, int dy, int dz) {
	SET_CODE_LINE(CodeLine);
	if (!ret) ret = new CKerArrObject();
	else ret->SetCount(0);
	CKerObject *ko = KerMain->GetAccessToObject(obj);
	if (!ko) {
		REINIT_CONTEXT;
		return ret;
	}
	CKerCollisionInfo CollInfo;
	CKerCollisionInfo *ObjInfo1 = &CollInfo, *ObjInfo2;
	int f;
	CKerArrInt *buff;
	CKerArrObject *objs = 0;
	CKerArrObject objs2;
	CKerName *oName;
	CKerName *AddGr, *SubGr, *FceGr;

	// nahraju info o objektu
	if (ko->PlacedInfo) ObjInfo1 = ko->PlacedInfo;
	else {
		ObjInfo1->LoadObjectInfo(obj);
		ObjInfo1->CalculateCube();
	}
	ReadColisionGroups(ko,&AddGr,&SubGr,&FceGr);
	buff = CellColisionB(obj,0,dx,dy,dz); // zjistim kolizi s bunkama
	if (buff->GetCount()==0) {
		// objekt je mimo mapu
		REINIT_CONTEXT;
		return ret;
	}
	// nactu objekty
	objs = (CKerArrObject*)KerMain->callint(CodeLine,OMapa,KnownNames[eKKNmGetObjects].Name,eKTarrObject,2, eKTarrInt, KnownNames[eKKNcellsArray].Name,buff, eKTarrObject, KnownNames[eKKNobjectArray].Name,&objs2);
	if (!objs) KerMain->Errors->LogError(eKRTEmapNotWorking,0,"MGetObjects");
	for (f=0;f<objs->GetCount();f++) if (objs->Read(f) != obj) {
		oName = KerMain->GetObjType(objs->Read(f));
		// mnozinovy test:
		if (KerServices.IsFMemberOfS(oName,AddGr)) {  // je obj v addgroup?
			if (!KerServices.IsFMemberOfS(oName,SubGr)) { // je obj mimo subgroup?
				ObjInfo2 = KerMain->Objs->GetObject(objs->Read(f))->PlacedInfo;
				if (((ObjInfo1->CollCfg&eKCCareaMask) == eKCCpoint && (ObjInfo2->CollCfg&eKCCareaMask) == eKCCpoint) || !(ObjInfo1->CollCfg & ObjInfo2->CollCfg & eKCClevelMask) ) {
					// neni kolize
				} else {
					// geometricky test:
					bool floor = (ObjInfo1->CubeZ1+ObjInfo1->z+dz == ObjInfo2->CubeZ1+ObjInfo2->z) && (ObjInfo1->CollCfg & ObjInfo2->CollCfg & eKCCfloor);
					if ((ObjInfo1->CubeX1+ObjInfo1->x+dx < ObjInfo2->CubeX2+ObjInfo2->x && ObjInfo1->CubeX2+ObjInfo1->x+dx > ObjInfo2->CubeX1+ObjInfo2->x) &&
						(ObjInfo1->CubeY1+ObjInfo1->y+dy < ObjInfo2->CubeY2+ObjInfo2->y && ObjInfo1->CubeY2+ObjInfo1->y+dy > ObjInfo2->CubeY1+ObjInfo2->y) &&
						((ObjInfo1->CubeZ1+ObjInfo1->z+dz < ObjInfo2->CubeZ2+ObjInfo2->z && ObjInfo1->CubeZ2+ObjInfo1->z+dz > ObjInfo2->CubeZ1+ObjInfo2->z) || floor)) {
							if ( KerServices.IsFMemberOfS(oName,FceGr)) {
								if (KerMain->callint(CodeLine,obj,KnownNames[eKKNtestCollision].Name,eKTint,1, eKTobject,KnownNames[eKKNobject].Name,objs->Read(f))) {
									ret->Add(objs->Read(f));
								}
							} else {
								ret->Add(objs->Read(f));
							}
						}
				}
			}
		}
	}

	REINIT_CONTEXT;
	if (objs!=&objs2) SAFE_DELETE(objs);
	return ret;
}







////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//		PRISTUP K OBJEKTOVYM PROMENNYM

////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// precte z objektu souradnice ma-li je (jinak vraci nuly). Napred je treba zavolat GetAccessToObj
void CKerMapInfo::ReadObjCoords(CKerObject *kObj, int &x, int &y, int &z) {
	x=0; y=0; z=0;
	if (!kObj || !kObj->Type->SpecificKnownVars) return;
	CKerOVar **Var = kObj->Type->SpecificKnownVars;
	if (Var[eKVUx]) {
		if (!Var[eKVUx]->KSVar) {	assert(kObj->ISData); x = *(int*)(kObj->ISData + Var[eKVUx]->Offset); } 
		else  x = **(int**)(Var[eKVUx]->KSVar);
	}
	if (Var[eKVUy]) {
		if (!Var[eKVUy]->KSVar) {	assert(kObj->ISData); y = *(int*)(kObj->ISData + Var[eKVUy]->Offset); } 
		else  y = **(int**)(Var[eKVUy]->KSVar);
	}
	if (Var[eKVUz]) {
		if (!Var[eKVUz]->KSVar) {	assert(kObj->ISData); z = *(int*)(kObj->ISData + Var[eKVUz]->Offset); } 
		else  z = **(int**)(Var[eKVUz]->KSVar);
	}
}


//////////////////////////////////////////////////////////////
// zapise do objektu souradnice ma-li je.  Napred je treba zavolat GetAccessToObj
void CKerMapInfo::WriteObjCoords(CKerObject *kObj, int x, int y, int z) {
	if (!kObj || !kObj->Type->SpecificKnownVars) return;
	CKerOVar **Var = kObj->Type->SpecificKnownVars;
	if (Var[eKVUx]) {
		if (!Var[eKVUx]->KSVar) {	assert(kObj->ISData); *(int*)(kObj->ISData + Var[eKVUx]->Offset) = x; } 
		else  **(int**)(Var[eKVUx]->KSVar) = x;
	}
	if (Var[eKVUy]) {
		if (!Var[eKVUy]->KSVar) {	assert(kObj->ISData); *(int*)(kObj->ISData + Var[eKVUy]->Offset) = y; } 
		else  **(int**)(Var[eKVUy]->KSVar) = y;
	}
	if (Var[eKVUz]) {
		if (!Var[eKVUz]->KSVar) {	assert(kObj->ISData); *(int*)(kObj->ISData + Var[eKVUz]->Offset) = z; } 
		else  **(int**)(Var[eKVUz]->KSVar) = z; 
	}
}


///////////////////////////////////////////////////////////////////
// zmeni souradnice objektu, objekt bude pripadne premisten.	
void CKerMapInfo::ChangeObjCoords(int CodeLine, OPointer obj, int x, int y, int z) {
	if(IsObjInMap(obj)) {
		MoveObjTo(CodeLine,obj,x,y,z);
	} else {
		CKerObject *ko = KerMain->GetAccessToObject(obj);
		WriteObjCoords(ko,x,y,z);
		REINIT_CONTEXT;
	}
}





////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

// vrati mnoziny pro vypocet kolizi
void CKerMapInfo::ReadColisionGroups(CKerObject *ko, CKerName **AddGr, CKerName **SubGr, CKerName **FceGr) {	
	void *var;
	*AddGr = KnownNames[eKKNeverything].Name;
	*SubGr = 0; *FceGr=0;
	if (var = KerMain->GetAccessToVar3(ko,eKVUclzAddGr)) *AddGr = *(CKerName**)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUclzSubGr)) *SubGr = *(CKerName**)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUclzFceGr)) *FceGr = *(CKerName**)var;
}

///////////////////////////////////////////////////////////////////



/// nastavi defaultni hodnoty
CKerCollisionInfo::CKerCollisionInfo() {
	CollCfg = eKCCdefault;
	x=0; y=0; z=0;
	CellX=1, CellY=1; CellZ=1; CellR=1;
	CubeX1=-10; CubeY1=-10; CubeZ1=0;
	CubeX2=10; CubeY2=10; CubeZ2=20;
	APicture = 0;
}

// nahraje hodnoty z objektu
void CKerCollisionInfo::LoadObjectInfo(OPointer obj) {
	CKerObject *ko = KerMain->GetAccessToObject(obj);
	void *var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcollizionCfg)) CollCfg = *(UC*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUx)) x = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUy)) y = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUz)) z = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcellx)) CellX = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcelly)) CellY = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcellz)) CellZ = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcellr)) CellR = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcubeX1)) CubeX1 = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcubeY1)) CubeY1 = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcubeZ1)) CubeZ1 = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcubeX2)) CubeX2 = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcubeY2)) CubeY2 = *(int*)var;
	if (var = KerMain->GetAccessToVar3(ko,eKVUcubeZ2)) CubeZ2 = *(int*)var;
	APicture = ko->Type->DefAuto;
	if (var = KerMain->GetAccessToVar3(ko,eKVUaPicture)) APicture = *(CKerName**)var;
	if (APicture==KnownNames[eKKNdefaultAuto].Name) APicture = ko->Type->DefAuto;

}


// vypocita z hodnot kolizni krychli
void CKerCollisionInfo::CalculateCube() {
	switch (KerMain->MapInfo->CellType) {
		case eKCTctverce:
			switch (CollCfg&eKCCareaMask) {
				case eKCConeCell:
					CubeX1 =  - KerMain->MapInfo->CellSizeX/2;
					CubeY1 =  - KerMain->MapInfo->CellSizeY/2;
					CubeZ1 = 0;
					CubeX2 =  + KerMain->MapInfo->CellSizeX/2;
					CubeY2 =  + KerMain->MapInfo->CellSizeY/2;
					CubeZ2 =  + KerMain->MapInfo->CellSizeZ;
					break;
				case eKCCrect1:
				case eKCCrect2:
					if (CellX) {
						CubeX1 =  - KerMain->MapInfo->CellSizeX/2;
						CubeX2 =  + CellX * KerMain->MapInfo->CellSizeX - KerMain->MapInfo->CellSizeX/2;
					} else CubeX1 = CubeX2 = 0;
					if (CellY) {
						CubeY1 =  - KerMain->MapInfo->CellSizeY/2;
						CubeY2 =  + CellY * KerMain->MapInfo->CellSizeY - KerMain->MapInfo->CellSizeY/2;
					} else CubeY1 = CubeY2 = 0;
					CubeZ1 = 0;
					CubeZ2 =  + CellZ * KerMain->MapInfo->CellSizeZ;
					break;
				case eKCCpoint:
					CubeX1 = CubeX2 = 0;
					CubeY1 = CubeY2 = 0;
					CubeZ1 = CubeZ2 = 0;
			}
			break;
	}
	if ((CollCfg&eKCClevelMask)==eKCCfloor) CubeZ2 = CubeZ1;
}




//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// ulozi Placed Info do registru
void CKerPlacedInfo::GSave(CFSRegKey *k) {
	CFSRegister *r;
	r = k->GetSubRegister();
	int f;

	r->AddKey("CollCfg",FSRTchar)->writec(CollCfg);
	k=r->AddKey("Coords",FSRTint);
	k->writei(x); k->writei(y); k->writei(z);
	k=r->AddKey("Cell",FSRTint);
	k->writei(CellX); k->writei(CellY); k->writei(CellZ); k->writei(CellR); 
	k=r->AddKey("Cube",FSRTint);
	k->writei(CubeX1); k->writei(CubeY1); k->writei(CubeZ1); 
	k->writei(CubeX2); k->writei(CubeY2); k->writei(CubeZ2); 
	KerSaveLoad.SaveName2(r->AddKey("APicture",FSRTstring),APicture);

	k = r->AddKey("ConnectedObjs",FSRTint);
	for (f=0; f<ConnectedObjs.GetCount(); f++) k->writei(ConnectedObjs.Read(f));

	KerSaveLoad.SaveName2(r->AddKey("AktivAuto",FSRTstring),AktivAuto);
	r->AddKey("Tag",FSRTint)->writei(Tag);

	k = r->AddKey("MConnectedObjs",FSRTint);
	for (f=0; f<MConnectedObjs.GetCount(); f++) k->writei(MConnectedObjs.Read(f));
}


// nahraje Placed Info z registru
int CKerPlacedInfo::GLoad(CFSRegKey *k) {
	CFSRegister *r;
	if (!k) return 0;
	r = k->GetSubRegister();

	if (k=r->FindKey("CollCfg")) CollCfg = k->readc(); else RETERR;
	if (k=r->FindKey("Coords")) {
		x = k->readi(); y = k->readi(); z = k->readi();
	} else RETERR;
	if (k=r->FindKey("Cell")) {
		CellX = k->readi(); CellY = k->readi(); CellZ = k->readi(); CellR = k->readi();
	} else RETERR;
	if (k=r->FindKey("Cube")) {
		CubeX1 = k->readi(); CubeY1 = k->readi(); CubeZ1 = k->readi();
		CubeX2 = k->readi(); CubeY2 = k->readi(); CubeZ2 = k->readi();
	} else RETERR;
	if (k=r->FindKey("APicture")) APicture = KerSaveLoad.LoadName2(k); else RETERR;

	if(!(k=r->FindKey("ConnectedObjs"))) RETERR;
	while (!k->eof()) ConnectedObjs.Add(k->readi());

	if (k=r->FindKey("AktivAuto")) AktivAuto = KerSaveLoad.LoadName2(k); else RETERR;
	if (k=r->FindKey("Tag")) Tag = k->readi(); else RETERR;

	if(!(k=r->FindKey("MConnectedObjs"))) RETERR;
	while (!k->eof()) MConnectedObjs.Add(k->readi());

	return 1;
}



////////////////////////////////////////////////////////////////////////////////
///
///		C K e r M o v e T i m e r
///
////////////////////////////////////////////////////////////////////////////////




void CKerMoveTimer::GSaveMover(CFSRegKey *k) {
	CFSRegister *r = k->GetSubRegister();
	r->AddKey("time",FSRTint)->writei(endTime - KerMain->GetTime());
	k = r->AddKey("dest",FSRTint);
	k->writei(endx); k->writei(endy); k->writei(endz); 
}

///////////////////////////////////////////////
// nahraje informace o plynulem pohybu
int CKerMapInfo::GLoadMover(CFSRegKey *k, CKerObject *ko) {
	if (!k) return 1;
	CFSRegister *r = k->GetSubRegister();
	int time, x,y,z;
	if (k = r->FindKey("time")) time = k->readi(); else RETERR;
	if (!(k = r->FindKey("dest"))) RETERR;
	x = k->readi(); y = k->readi(); z = k->readi(); 
	new CKerMoveTimer(ko,ko->PlacedInfo->x,ko->PlacedInfo->y,ko->PlacedInfo->z, x,y,z, time);
	return 1;
}



//////////////////////////////////////////////////////////
/// Konstruktor
CKerMoveTimer::CKerMoveTimer(CKerObject *ko, int startX, int startY, int startZ, int endX, int endY, int endZ, UI time) {
	int f;
	int count;
	kMovingObj = ko;
	startx = startX; starty = startY; startz = startZ;
	endx = endX; endy = endY; endz = endZ;
	startTime = KerMain->GetTime();
	endTime = startTime + time;
	DeleteMe = 0;

	// pridam se do seznamu:
	SAFE_DELETE(ko->PlacedInfo->Mover);
	ko->PlacedInfo->Mover = this;
	next = KerMain->MapInfo->Movers;
	prev = 0;
	if (KerMain->MapInfo->Movers) KerMain->MapInfo->Movers->prev = this;
	KerMain->MapInfo->Movers = this;

	// nastartuju plynuly pohyb v GEnu:
	if (ko->Type->Name == KnownNames[eKKNscrollObj].Name) KerMain->MapInfo->WindowScroll(endx-startx,endy-starty,time);
	if (ko->PlacedInfo->AktivAuto) {
		count = ko->PlacedInfo->AktivAuto->Auto->SonsNum;
		for (f = 0; f<count; f++) {
			GEnMain->MoveElem(ko->elements[f],(float)startx,(float)starty,(float)startz,(float)endx,(float)endy,(float)endz,time);
		}
	}
}



//////////////////////////////////////////////////////////
/// Konstruktor - okopiruju pohyb z jineho moveru
CKerMoveTimer::CKerMoveTimer(CKerObject *ko, CKerMoveTimer *mover) {
	int f;
	int count;
	kMovingObj = ko;
	int time = mover->endTime-KerMain->GetTime();
	int TotalTime = mover->endTime - mover->startTime;
	startx = ko->PlacedInfo->x; starty = ko->PlacedInfo->y; startz = ko->PlacedInfo->z;
	endx = startx + ((mover->endx - mover->startx)* time) / TotalTime;
	endy = starty + ((mover->endy - mover->starty)*time) / TotalTime;
	endz = startz + ((mover->endz - mover->startz)*time) / TotalTime;
	startTime = KerMain->GetTime();
	endTime = startTime + time;
	DeleteMe = 0;

	// pridam se do seznamu:
	SAFE_DELETE(ko->PlacedInfo->Mover);
	ko->PlacedInfo->Mover = this;
	next = KerMain->MapInfo->Movers;
	prev = 0;
	if (KerMain->MapInfo->Movers) KerMain->MapInfo->Movers->prev = this;
	KerMain->MapInfo->Movers = this;

	// nastartuju plynuly pohyb v GEnu:
	if (ko->Type->Name == KnownNames[eKKNscrollObj].Name) KerMain->MapInfo->WindowScroll(endx-startx,endy-starty,time);
	if (ko->PlacedInfo->AktivAuto) {
		count = ko->PlacedInfo->AktivAuto->Auto->SonsNum;
		for (f = 0; f<count; f++) {
			GEnMain->MoveElem(ko->elements[f],(float)startx,(float)starty,(float)startz,(float)endx,(float)endy,(float)endz,time);
		}
	}
}


////////////////////////////////////////////////////////////
///		Destructor
CKerMoveTimer::~CKerMoveTimer() {
	if (DeleteMe) KerMain->message(0,kMovingObj->thisO,KnownNames[eKKNmoveEnded].Name,eKerCTmsg,0,0);
	kMovingObj->PlacedInfo->Mover = 0;
	if (next) next->prev = prev;
	if (prev) prev->next = next;
	else KerMain->MapInfo->Movers = next;
}



////////////////////////////////////////////////////////////////////////////////
///		Provedu jeden krok pohybu
///		Aktualizuju pozici objektu v mape
void CKerMoveTimer::DoMove() {
	int time = KerMain->GetTime() - startTime;
	int TotalTime = endTime - startTime;
	if (time>=TotalTime) {  // pohyb je ukoncen
		KerMain->MapInfo->MoveObjTo2(0,kMovingObj,kMovingObj->thisO,endx,endy,endz,0);
		DeleteMe = 1;
	} else {
		int x = startx + ((endx-startx)*time) / TotalTime;
		int y = starty + ((endy-starty)*time) / TotalTime;
		int z = startz + ((endz-startz)*time) / TotalTime;
		KerMain->MapInfo->MoveObjTo2(0,kMovingObj,kMovingObj->thisO,x,y,z,0);
	}
	if (DeleteMe) delete this;
}



/////////////////////////////////////////////////////////////////////////
///		Projdu vsechny kontrolery plynulych pohybu a pohnu s objekty
void CKerMapInfo::MoveMovingObjs() {
	CKerMoveTimer *m1, *m2;
	m1 = Movers;
	while (m1) {
		m2 = m1->next;
		m1->DoMove(); // (mover muzer umrit)
		m1 = m2;
	}
}







