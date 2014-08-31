//////////////////////////////////////////////////////////////////////////////
///
///		K r k a l   C o n v e r t or
///
///		konvertor levlu, ze stareho Krkala do noveho
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "KrkalConvertor.h"
#include "fs.h"
#include "register.h"
#include "KerServices.h"

COldMain OldMain;

COldNewVeci OldNewVeci[MAXOLDNEWVECI] = {
	{"Unknown",1000,1000,eOTunknown},

	{"_KSID_oSliz_0001_FFFF_0001_0001",0,0,eOTplace,0},
	{"_KSID_oVoda_0001_FFFF_0001_0001",1,1,eOTplace,0},
	{"_KSID_oPas_0001_FFFF_0001_0001",2,5,eOTpas,1},
	{"_KSID_oExit_0001_FFFF_0001_0001",6,6,eOTplace,0},
	{"_KSID_oLed_0001_FFFF_0001_0001",7,7,eOTplace,0},
	{"_KSID_floor_0001_FFFF_0001_0001",8,43,eOTplace,0},
	{"_KSID_floor_0001_FFFF_0001_0001",73,73,eOTplace,0}, // save point
	{"_KSID_oHrbolataP_A1CF_6963_8DA6_D822",50,51,eOTplace,0}, // hrbolata podlaha
	{"_KSID_oDira_0001_FFFF_0001_0001",44,45,eOTplace,0},
	{"_KSID_oPropadlo_0001_FFFF_0001_0001",46,47,eOTplace,0},
	{"_KSID_oLava_0001_FFFF_0001_0001",48,49,eOTplace,0},
	{"_KSID_oNaslapnaPodlaha_0001_FFFF_0001_0001",52,55,eOTprepinac,0},

	{"_KSID_oKrabice_0001_FFFF_0001_0001",56,56,eOTplace,0},
	{"_KSID_oKamen_0001_FFFF_0001_0001",57,57,eOTplace,0},
	{"_KSID_oPneumatika_0001_FFFF_0001_0001",58,58,eOTplace,0},
	{"_KSID_oBalonek_0001_FFFF_0001_0001",59,59,eOTplace,0},
	{"_KSID_oSebratelnaBomba_0001_FFFF_0001_0001",65,65,eOTplace,0},
	{"_KSID_oBomba_0001_FFFF_0001_0001",66,66,eOTplace,0},
	{"_KSID_oSebratelnaMina_0001_FFFF_0001_0001",67,67,eOTplace,0},
	{"_KSID_mina_0001_FFFF_0001_0001",68,68,eOTplace,0},
	{"_KSID_oZnSmerova_0001_FFFF_0001_0001",69,72,eOTseSmerem,1},
	{"_KSID_oZnZakazVjezdu_0001_FFFF_0001_0001",77,77,eOTplace,0},
	{"_KSID_oZnSedesat_0001_FFFF_0001_0001",78,78,eOTplace,0},
	{"_KSID_oZnNeSedesat_0001_FFFF_0001_0001",79,79,eOTplace,0},
	{"_KSID_oVoda_0001_FFFF_0001_0001",74,76,eOTplace,0},
	{"_KSID_oklic_0001_FFFF_0001_0001",86,88,eOTsBarvou,1},

	{"_KSID_wall_0001_FFFF_0001_0001",91,106,eOTnoConnect,0},
	{"_KSID_oPruleznaStena_0001_FFFF_0001_0001",62,62,eOTplace,0},
	{"_KSID_oZnicitelnaStena_0001_FFFF_0001_0001",107,113,eOTplace,0},
	{"_KSID_oHlina_0001_FFFF_0001_0001",114,116,eOTplace,0},
	{"_KSID_oPrepinac_0001_FFFF_0001_0001",117,118,eOTprepinac,0},
	{"_KSID_oZamek_0001_FFFF_0001_0001",119,121,eOTsBarvou,1},
	{"_KSID_oFotobunka_0001_FFFF_0001_0001",123,123,eOTprepinac,0},
	{"_KSID_oAntiMagnet_0001_FFFF_0001_0001",124,124,eOTplace,0},
	{"_KSID_oMagnet_0001_FFFF_0001_0001",145,148,eOTseSmerem,1},
	{"_KSID_oTeleport_0001_FFFF_0001_0001",149,149,eOTteleport,0},
	{"_KSID_oHajzl_0001_FFFF_0001_0001",150,152,eOThajzl,1},

	{"_KSID_oPasovec_0001_FFFF_0001_0001",153,156,eOTseSmerem,1},
	{"_KSID_oHemr_0001_FFFF_0001_0001",157,160,eOTseSmerem,1},
	{"_KSID_oDracek_0001_FFFF_0001_0001",161,164,eOTseSmerem,1},
	{"_KSID_oPriseraOt_0001_FFFF_0001_0001",165,168,eOTprisera,1},
	{"_KSID_oPriseraSNavadeci_0001_FFFF_0001_0001",169,172,eOTseSmerem,1},
	{"_KSID_oPriseraDopravni_0001_FFFF_0001_0001",173,176,eOTprisera,1},
	{"_KSID_oPriseraKlaustrofobni_0001_FFFF_0001_0001",177,180,eOTprisera,1},

	{"_KSID_oElektroda_A1CF_6963_8DA6_D822",133,136,eOTelektroda,1},
	{"_KSID_oDvere_A1CF_6963_8DA6_D822",89,90,eOTdvere,1},

};

char * OldMusic[MAXOLDMUSIC] = {
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/EUROPE.mm",
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/SUNSET2.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/OCEANIC.mm",//OCEANIC
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/NIGHT.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/AMERICA.mm",
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/WARS2.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/STRAW.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/RHITM.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/ETHERAL.mm",
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/SUNSET.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/MEGAHIT.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/OOF.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/AFRIKA.mm", //"FTRIAL",
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/EXP.mm",//
"$GAMES$/Krkal_4F88_78B7_A01C_48AB\\music_65B6_399E_4613_0839.pkg/NO-TRIX.mm"//
};

int COldVeci::NajdiNVec(int vec) {
	int f;
	for (f=1; f<MAXOLDNEWVECI; f++) if (vec >= OldNewVeci[f].startpos && vec <= OldNewVeci[f].endpos) return f;
	return 0;
}

COldVeci::COldVeci(UC _x,UC _y,UC _vec, COldVeci *_next) {
	x = _x; y = _y; vec = _vec;
	SaveIndex=0;
	next = _next;
	int f;
	for (f=0; f<6; f++) a[f].vec2=0;

	InMap=1;
	vecIndex = 0;
	nvec = NajdiNVec(vec); 
	if (nvec) vecIndex = vec-OldNewVeci[nvec].startpos;
}


int COldMain::LoadLevel(char *file) {
	// nacteni souboru
	int fsize = FS->GetFileSize(file);
	if (!fsize) return 0;
	UC *buff = new UC[fsize];
	if (!FS->ReadFile(file,(char*)buff,fsize)) {
		delete[] buff;
		return 0;
	}

	COldVeci* pole[2][32][22];
	int f,g,i;
	for (f=0; f<32; f++) for (g=0; g<22; g++) pole[0][f][g] = 0;
	for (f=0; f<32; f++) for (g=0; g<22; g++) pole[1][f][g] = 0;
	int p;

	usek = buff[0];	hudba = buff[1];
	int topv = buff[2];
	int topp = buff[3];
	if (fsize<11 + 2*32*22 + 6*topv + 22*topp) {
		delete[] buff;
		return 0;
	}

	// nactu patra
	for (i=0;i<2;i++)
		for (g=0;g<22;g++)
			for (f=0;f<32;f++) {
				if (i==0 || buff[i*704+g*32+f+11]>0) pole[i][f][g] = Veci = new COldVeci(f,g,buff[i*704+g*32+f+11],Veci);
			}
	p = 11 + 2 * 22 * 32;

	int x,y;
	COldVeci *vec;
	COldVeci **Veci2 = new COldVeci*[topv];
	// nactu veci
	for (f=0;f<topv;f++) {
		x = buff[p]; y = buff[p+1];
		vec = 0;
		if (pole[0][x][y] && pole[0][x][y]->vec == buff[p+2]) vec = pole[0][x][y];
		else if (pole[1][x][y] && pole[1][x][y]->vec == buff[p+2]) vec = pole[1][x][y];
		if (vec) {
			vec->a1 = buff[p+3];
			vec->a2 = buff[p+4];
			vec->a3 = buff[p+5];
		}
		Veci2[f] = vec;
		p+=6;
	}

	// nactu prepinace
	for (f=0;f<topp;f++) {
		x = buff[p]; y = buff[p+1];
		vec = 0;
		if (pole[0][x][y] && pole[0][x][y]->vec == buff[p+2]) vec = pole[0][x][y];
		else if (pole[1][x][y] && pole[1][x][y]->vec == buff[p+2]) vec = pole[1][x][y];
		if (vec) {
			vec->zvp = buff[p+3]-1;
			for (g=0;g<6;g++)
			{
				vec->a[g].x = buff[p+4+g*3];
				vec->a[g].y = buff[p+5+g*3];
				vec->a[g].vec = buff[p+6+g*3];
				if (vec->a[g].x==128 && vec->a[g].vec<topv) vec->a[g].vec2 = Veci2[vec->a[g].vec];
				vec->a[g].nvec = 0;
				if (vec->a[g].x<32 && vec->a[g].nvec < 200) {
					vec->a[g].nvec = vec->NajdiNVec(vec->a[g].vec);
					if (OldNewVeci[vec->a[g].nvec].PrepCreate) {
						vec->a[g].vec2 = Veci = new COldVeci(vec->a[g].x,vec->a[g].y,vec->a[g].vec,Veci);
						Veci->InMap = 0;
					}
				}
			}
		}
		p+=22;
	}


	delete[] buff;
	delete[] Veci2;
	return 1;
}


void COldMain::DeleteVeci() {
	COldVeci *v1=Veci, *v2;
	while (v1) {
		v2=v1;
		v1=v1->next; 
		delete v2;
	}
	Veci = 0;
}


void COldMain::AddSmer(CFSRegister *r2,COldVeci *v) {
	CFSRegister *r3;
	r3 = r2->AddKey("_KSOV_oSeSmerem_0001_FFFF_0001_0001__M_smer_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKTname);
	if (v->vecIndex==0) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Vychod_0001_FFFF_0001_0001");
	else if (v->vecIndex==1) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Sever_0001_FFFF_0001_0001");
	else if (v->vecIndex==2) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Zapad_0001_FFFF_0001_0001");
	else if (v->vecIndex==3) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Jih_0001_FFFF_0001_0001");
}

void COldMain::AddOnOff(CFSRegister *r2,int OnOff) {
	CFSRegister *r3;
	r3 = r2->AddKey("_KSOV_oOnOff_0001_FFFF_0001_0001__M_OnOff_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKTname);
	if (OnOff) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_On_0001_FFFF_0001_0001");
	else r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Off_0001_FFFF_0001_0001");
}

void COldMain::SaveLevel(CFSRegister *reg) {
	int order = reg->GetNumberOfKeys()+1;
	CFSRegister *r1, *r2, *r3;
	CFSRegKey *k;
	COldVeci *v = Veci;
	// ulozim vse krome prepinacu
	while (v) {
		if (v->nvec && OldNewVeci[v->nvec].typ != eOTprepinac) {
			// zaklad
			r1 = reg->AddKey(OldNewVeci[v->nvec].object,FSRTregister)->GetSubRegister();
			r1->AddKey("Edited",FSRTchar)->writec(1);
			r1->AddKey("Is In Map",FSRTchar)->writec(v->InMap);
			r1->AddKey("Order",FSRTint)->writei(order);
			v->SaveIndex = order;
			r2 = r1->AddKey("Basic Data",FSRTregister)->GetSubRegister();
			k = r1->AddKey("Data Stream",FSRTchar);
			order++;
			
			// z,y,APicture
			r3 = r2->AddKey("_KSOV_placeable_0001_FFFF_0001_0001__M__KN_ObjPosX_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTint);
			r3->AddKey("Value",FSRTint)->writei(v->x*40);
			r3 = r2->AddKey("_KSOV_placeable_0001_FFFF_0001_0001__M__KN_ObjPosY_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTint);
			r3->AddKey("Value",FSRTint)->writei(v->y*40);
			r3 = r2->AddKey("_KSOV_placeable_0001_FFFF_0001_0001__M__KN_APicture_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTname);
			r3->AddKey("Value",FSRTstring)->stringwrite("_KSID__KN_DefaultAuto");

			switch (OldNewVeci[v->nvec].typ) {
				case eOTsBarvou:
					if (v->vec>100) r3 = r2->AddKey("_KSOV_oZamek_0001_FFFF_0001_0001__M_barva_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					else r3 = r2->AddKey("_KSOV_oklic_0001_FFFF_0001_0001__M_barva_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTname);
					if (v->vecIndex==0) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Cervena_0001_FFFF_0001_0001");
					else if (v->vecIndex==1) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Modra_0001_FFFF_0001_0001");
					else if (v->vecIndex==2) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Zluta_0001_FFFF_0001_0001");
					break;
				case eOTseSmerem:
					AddSmer(r2,v);
					break;
				case eOTpas:
					AddSmer(r2,v);
					r3 = r2->AddKey("_KSOV_oPas_0001_FFFF_0001_0001__M_OnOff_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTname);
					if (v->a3) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_On_0001_FFFF_0001_0001");
					else r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Off_0001_FFFF_0001_0001");
					break;
				case eOTelektroda:
					AddSmer(r2,v);
					AddOnOff(r2,1);
					r3 = r2->AddKey("_KSOV_oElektroda_A1CF_6963_8DA6_D822__M_StartDelay_A1CF_6963_8DA6_D822",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTint);
					r3->AddKey("Value",FSRTint)->writei(v->a3 * 16);
					break;
				case eOTdvere:
					AddOnOff(r2,1);
					r3 = r2->AddKey("_KSOV_oSeSmerem3_A1CF_6963_8DA6_D822__M_smer_A1CF_6963_8DA6_D822",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTname);
					if (v->vecIndex==0) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Vychod_0001_FFFF_0001_0001");
					else if (v->vecIndex==1) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Sever_0001_FFFF_0001_0001");
					r3 = r2->AddKey("_KSOV_oDvere_A1CF_6963_8DA6_D822__M_ReagujeNa_A1CF_6963_8DA6_D822",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTname);
					r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_oManik_0001_FFFF_0001_0001");
					break;
				case eOTprisera:
					AddSmer(r2,v);
					if (v->vec<=168) { // Otaciva
						r3 = r2->AddKey("_KSOV_oPriseraOt_0001_FFFF_0001_0001__M_Tocivost_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
						r3->AddKey("Type",FSRTint)->writei(eKTname);
						if (v->a3) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_LevoTociva_0001_FFFF_0001_0001");
						else r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_PravoTociva_0001_FFFF_0001_0001");
					} else { // Dopravni
						r3 = r2->AddKey("_KSOV_oPriseraZnackova_0001_FFFF_0001_0001__M_Rychlost_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
						r3->AddKey("Type",FSRTint)->writei(eKTint);
						if (v->a3) r3->AddKey("Value",FSRTint)->writei(17);
						else r3->AddKey("Value",FSRTint)->writei(33);
					}
					break;
				case eOThajzl:
					r3 = r2->AddKey("_KSOV_oHajzl_0001_FFFF_0001_0001__M_MeniNa_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTname);
					if (v->vecIndex==0) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_oPasovec_0001_FFFF_0001_0001");
					else if (v->vecIndex==1) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_oHemr_0001_FFFF_0001_0001");
					else r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_oDracek_0001_FFFF_0001_0001");
					r3 = r2->AddKey("_KSOV_oTeleport_0001_FFFF_0001_0001__M_DestX_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTint);
					r3->AddKey("Value",FSRTint)->writei(v->x);
					r3 = r2->AddKey("_KSOV_oTeleport_0001_FFFF_0001_0001__M_DestY_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTint);
					r3->AddKey("Value",FSRTint)->writei(v->y);
					break;
				case eOTteleport:
					r3 = r2->AddKey("_KSOV_oTeleport_0001_FFFF_0001_0001__M_DestX_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTint);
					r3->AddKey("Value",FSRTint)->writei(v->a1);
					r3 = r2->AddKey("_KSOV_oTeleport_0001_FFFF_0001_0001__M_DestY_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTint);
					r3->AddKey("Value",FSRTint)->writei(v->a2);
					break;
				case eOTnoConnect:
					r3 = r2->AddKey("_KSOV_wall_0001_FFFF_0001_0001__M__KN_ANoConnect_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
					r3->AddKey("Type",FSRTint)->writei(eKTint);
					r3->AddKey("Value",FSRTint)->writei(0);
			}
		}
		v = v->next;
	}

	// ulozim prepinace
	v = Veci;
	while (v) {
		if (v->nvec && OldNewVeci[v->nvec].typ == eOTprepinac) {
			// zaklad
			r1 = reg->AddKey(OldNewVeci[v->nvec].object,FSRTregister)->GetSubRegister();
			r1->AddKey("Edited",FSRTchar)->writec(1);
			r1->AddKey("Is In Map",FSRTchar)->writec(1);
			r1->AddKey("Order",FSRTint)->writei(order);
			r2 = r1->AddKey("Basic Data",FSRTregister)->GetSubRegister();
			k = r1->AddKey("Data Stream",FSRTchar);
			order++;
			
			// z,y,APicture
			r3 = r2->AddKey("_KSOV_placeable_0001_FFFF_0001_0001__M__KN_ObjPosX_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTint);
			r3->AddKey("Value",FSRTint)->writei(v->x*40);
			r3 = r2->AddKey("_KSOV_placeable_0001_FFFF_0001_0001__M__KN_ObjPosY_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTint);
			r3->AddKey("Value",FSRTint)->writei(v->y*40);
			r3 = r2->AddKey("_KSOV_placeable_0001_FFFF_0001_0001__M__KN_APicture_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTname);
			r3->AddKey("Value",FSRTstring)->stringwrite("_KSID__KN_DefaultAuto");

			r3 = r2->AddKey("_KSOV_oPrepinace_0001_FFFF_0001_0001__M_Prepina_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTint);
			if (v->zvp==2) r3->AddKey("Value",FSRTint)->writei(1);
			else r3->AddKey("Value",FSRTint)->writei(0);
			r3 = r2->AddKey("_KSOV_oPrepinace_0001_FFFF_0001_0001__M_Nekoliduj_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTint);
			r3->AddKey("Value",FSRTint)->writei(0);
			int PocetAkci = 0;

			if (v->vec==117) {
				r3 = r2->AddKey("_KSOV_oPrepinac_0001_FFFF_0001_0001__M_OnOff_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
				r3->AddKey("Type",FSRTint)->writei(eKTname);
				r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_On_0001_FFFF_0001_0001");
			} else if (v->vec==118) {
				r3 = r2->AddKey("_KSOV_oPrepinac_0001_FFFF_0001_0001__M_OnOff_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
				r3->AddKey("Type",FSRTint)->writei(eKTname);
				r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_Off_0001_FFFF_0001_0001");
			} else {
				r3 = r2->AddKey("_KSOV_oTriggerPrepinac_0001_FFFF_0001_0001__M_Reaguje_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
				r3->AddKey("Type",FSRTint)->writei(eKTname);
				if (v->zvp==2) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_ReagujeVzdy_0001_FFFF_0001_0001");
				else if (v->zvp==0) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_PriZapnuti_0001_FFFF_0001_0001");
				else if (v->zvp==1) r3->AddKey("Value",FSRTstring)->stringwrite("_KSID_PriVypnuti_0001_FFFF_0001_0001");
			}

			PocetAkci = SaveAkce(k,v);
			r3 = r2->AddKey("_KSOV_oPrepinace_0001_FFFF_0001_0001__M_PocetAkci_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
			r3->AddKey("Type",FSRTint)->writei(eKTint);
			r3->AddKey("Value",FSRTint)->writei(PocetAkci);
		}
		v = v->next;
	}
}



int COldMain::SaveAkce(CFSRegKey *k,COldVeci *v) {
	int PocetAkci=0;
	int f;
	KerSaveLoad.Open(k,eSLSsaveLevel);

	for (f=0; f<6; f++) {
		if (v->a[f].vec2) {
			PocetAkci++;
			if (v->a[f].x==128) { // on / off
				KerSaveLoad.SaveName("_KSID_PrepMessage_0001_FFFF_0001_0001");
				KerSaveLoad.SaveName("_KSID_NaObjekty_0001_FFFF_0001_0001");
				assert(v->a[f].vec2->SaveIndex);
				KerSaveLoad.SaveObjPtrA(v->a[f].vec2->SaveIndex);
				KerSaveLoad.SaveObjPtrA(0);
				if (v->zvp==2) {
					KerSaveLoad.SaveName("_KSID_ChangeOnOff_0001_FFFF_0001_0001");
					KerSaveLoad.SaveName("_KSID_ChangeOnOff_0001_FFFF_0001_0001");
				} else if (v->vec==117 || v->zvp==1) {
					KerSaveLoad.SaveName("_KSID_TurnOff_0001_FFFF_0001_0001");
					KerSaveLoad.SaveName("_KSID_TurnOn_0001_FFFF_0001_0001");
				} else {
					KerSaveLoad.SaveName("_KSID_TurnOn_0001_FFFF_0001_0001");
					KerSaveLoad.SaveName("_KSID_TurnOff_0001_FFFF_0001_0001");
				}
				KerSaveLoad.SaveName("");
			} else { // umisti vytvoreny objekt
				KerSaveLoad.SaveName("_KSID_PrepUmisti_0001_FFFF_0001_0001");
				KerSaveLoad.SaveName("_KSID_NaObjekty_0001_FFFF_0001_0001");
				assert(v->a[f].vec2->SaveIndex);
				KerSaveLoad.SaveObjPtrA(v->a[f].vec2->SaveIndex);
				KerSaveLoad.SaveObjPtrA(0);
				KerSaveLoad.SaveName("");
				KerSaveLoad.SaveName("");
				KerSaveLoad.SaveName("");
			}
		} else if (v->a[f].x < 32) {
			if (v->a[f].vec == 251 || v->a[f].vec == 252) { // Odeber
				PocetAkci++;
				KerSaveLoad.SaveName("_KSID_PrepOdeber_0001_FFFF_0001_0001");
				KerSaveLoad.SaveName("_KSID_NaOblast_0001_FFFF_0001_0001");
				KerSaveLoad.SaveInt(v->a[f].x); KerSaveLoad.SaveInt(v->a[f].y);
				KerSaveLoad.SaveInt(v->a[f].x); KerSaveLoad.SaveInt(v->a[f].y);
				KerSaveLoad.SaveName("_KSID_Patro_0001_FFFF_0001_0001");
				KerSaveLoad.SaveName("");
				KerSaveLoad.SaveName("");
			} else if (v->a[f].nvec) { // umisti
				PocetAkci++;
				KerSaveLoad.SaveName("_KSID_PrepUmisti_0001_FFFF_0001_0001");
				KerSaveLoad.SaveName("_KSID_NaOblast_0001_FFFF_0001_0001");
				KerSaveLoad.SaveInt(v->a[f].x); KerSaveLoad.SaveInt(v->a[f].y);
				KerSaveLoad.SaveInt(v->a[f].x); KerSaveLoad.SaveInt(v->a[f].y);
				KerSaveLoad.SaveName(OldNewVeci[v->a[f].nvec].object);
				KerSaveLoad.SaveName("");
				KerSaveLoad.SaveName("");
			}
		}
	}

	KerSaveLoad.Close();
	return PocetAkci;
}


// Prevede level ze staryho krkala do novyho. 0 chyba, 1 OK
int COldMain::ConvertLevel(int oldlev, char *newlev_name, char *newlev_dir) {
	char OldLev[10];
	char NewLev[100];
	char Version[30];
	if (newlev_name && strlen(newlev_name)>50) return 0;
	FS->ChangeDir("$DATA$");
	CFSRegister *reg = new CFSRegister("OldKrkalBase.lev","KRKAL LEVEL");
	if (reg->GetOpenError() != FSREGOK) {
		delete reg;
		return 0;
	}
	KerServices.GenerateVersionNumber(Version);
	if (!FS->ChangeDir(newlev_dir)) {
		delete reg; return 0;
	}
	sprintf(OldLev,"%i.lv",oldlev);
	sprintf(NewLev,"(%02i)%s_%s.lv",oldlev,newlev_name,Version);
	FS->CreateDir(NewLev);
	FS->ChangeDir(NewLev);
	reg->ChangePath("!level");
	FS->ChangeDir("$CONVERTOR_INPUT$");
	if (!LoadLevel(OldLev)) {
		delete reg; return 0;
		DeleteVeci();
	}
	FS->ChangeDir("$KRKAL$");
	SaveLevel(reg->FindKey("Objects")->GetSubRegister());
	if (hudba>=0 && hudba < MAXOLDMUSIC) reg->AddKey("Music",FSRTstring)->stringwrite(OldMusic[hudba]);
	int ret = reg->WriteFile();
	delete reg;
	DeleteVeci();
	return ret;
}
