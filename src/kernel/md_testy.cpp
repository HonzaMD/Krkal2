/////////////////////////////////////////////////////////////////////////////////
///
///		M D   T E S T Y 
///
///		Pokusné volání, rùzných vìcí zaúèelem ladìní a testování
///		A: M.D.
///
/////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "md_testy.h"
#include "register.h"
#include "kernel.h"
#include "LevelMap.h"
#include "ComCompiler.h"
#include "autos.h"
#include "KrkalConvertor.h"

//#define MDTCREATE_FILES


void Titles() {
	FILE *f1,*f2;
//	f1 = fopen("d:\\md1_cz.sub","rb");
	f2 = fopen("d:\\md2.sub","wb");
	int a,c,aa=0;
/*	c = fgetc(f1);
	while (!feof(f1)) {
		fputc(c,f2);
		c = fgetc(f1);
	}
	fclose(f1);*/
	f1 = fopen("d:\\md.sub","rb");
	c = fgetc(f1);
	while (!feof(f1)) {
		if (!aa) {
			fputc(c,f2);
			if (c=='{') {aa=1; a=0;}
		} else {
			if (c=='}') {
				aa=0;
				a -= 580;
				fprintf(f2,"%i}",a);
			} else {
				a*=10;
				a+= (c-'0');
			}
		}
		c = fgetc(f1);
	}
	fclose(f1);
	fclose(f2);
}

void MDTcreate_files() {
//	Titles();
	
	#ifdef MDTCREATE_FILES
	
	char str[255];
//	CFSRegister *pr, *kpr;

//	OldMain.ConvertLevel(1,"Zeme Krtku");
////	OldMain.ConvertLevel(25,"Zakryte Diry");
//	//OldMain.ConvertLevel(2,"Utikej!");
// OldMain.ConvertLevel(6,"3 Vodni Plochy");
//	OldMain.ConvertLevel(4,"Pozor Diry");
//	OldMain.ConvertLevel(9,"Cesta na Pasech");
//	OldMain.ConvertLevel(10,"Padajici Kameny");
//	OldMain.ConvertLevel(40,"Kam se to svali");
//	OldMain.ConvertLevel(56,"Ledove Mesto");
//	OldMain.ConvertLevel(36,"Zabij Je!");
//	OldMain.ConvertLevel(13,"Znic Steny");
//	OldMain.ConvertLevel(31,"Ledova Pevnost");
//
//	OldMain.ConvertLevel(3,"Kachni Jezero");
//	OldMain.ConvertLevel(12,"So Einfach!");
//	OldMain.ConvertLevel(14,"Kde Je NAHORE");
//	OldMain.ConvertLevel(18,"Sama Bomba");
//	OldMain.ConvertLevel(20,"Fotobunka a Led");
//	OldMain.ConvertLevel(23,"8x Modry Klic");
//	OldMain.ConvertLevel(24,"Probij Se");
//	OldMain.ConvertLevel(27,"Samoobsluha");

//	OldMain.ConvertLevel(61,"Ramova Pole");

	FS->ChangeDir("$GAME$");
	CFSRegister *r;
// SEM komentar start
	/*
	r = new CFSRegister("(09)Cesta na Pasech_DB03_8940_38AE_720B.lv\\!level","KRKAL LEVEL");

	CFSRegKey *fk;
	fk = r->FindKey("Scripts File");
	fk->top = 0;
	fk->stringwrite("$GAME$\\krkal2_A1CF_6963_8DA6_D822.kc");
	fk = r->FindKey("Scripts Version");
	fk->top = 0;
	fk->stringwrite("A1CF_6963_8DA6_D822");
	fk = r->FindKey("Scripts Version 64");
	fk->top = 0;
	fk->write64(0xA1CF69638DA6D822);

	r->WriteFile();
	delete r;
*/
// SEM komentar end

	//r = new CFSRegister("jedna_44A0_721C_001C_BF01.lv\\!level","KRKAL LEVEL",FSREGCLEARIT);
	//r->SetRegisterToBeUnCompressed();
	//r->AddKey("Scripts File",FSRTstring)->stringwrite("$GAME$\\test_0001_FFFF_0001_0001.kc");
	//r->AddKey("Scripts Version",FSRTstring)->stringwrite("0001_FFFF_0001_0001");
	//r->AddKey("Scripts Version 64",FSRTint64)->write64(0x0001ffff00010001);
	//r->AddKey("Level Name",FSRTstring)->stringwrite("Testovaci Level");
	//r->AddKey("Author",FSRTstring)->stringwrite("Honza M.D. Krcek");
	//r->AddKey("Game",FSRTstring)->stringwrite("Unknown");

////	r->AddKey("Globals",FSRTregister);
//
//	CFSRegister *objs = new CFSRegister();
//	CFSRegister *obj = objs->AddKey("_KSG_Map_0001_000F_0001_1001",FSRTregister)->GetSubRegister();
//	obj->AddKey("Edited",FSRTchar)->writec(1);
//	obj->AddKey("Order",FSRTint)->writei(1);
//	obj->AddKey("Data Stream",FSRTchar);
//	CFSRegister *bd; 
//	CFSRegister *bdv;
//	bd = obj->AddKey("Basic Data",FSRTregister)->GetSubRegister();
//	bdv = bd->AddKey("_KSOV_OMap_0001_000F_0001_1001__M_leftx_0001_000F_0001_1001",FSRTregister)->GetSubRegister();
//	bdv->AddKey("Type",FSRTint)->writei(eKTint);
//	bdv->AddKey("Value",FSRTint)->writei(-20);
//	bdv = bd->AddKey("_KSOV_OMap_0001_000F_0001_1001__M_lefty_0001_000F_0001_1001",FSRTregister)->GetSubRegister();
//	bdv->AddKey("Type",FSRTint)->writei(eKTint);
//	bdv->AddKey("Value",FSRTint)->writei(-20);
//	bdv = bd->AddKey("_KSOV_OMap_0001_000F_0001_1001__M_rightx_0001_000F_0001_1001",FSRTregister)->GetSubRegister();
//	bdv->AddKey("Type",FSRTint)->writei(eKTint);
//	bdv->AddKey("Value",FSRTint)->writei(460);
//	bdv = bd->AddKey("_KSOV_OMap_0001_000F_0001_1001__M_righty_0001_000F_0001_1001",FSRTregister)->GetSubRegister();
//	bdv->AddKey("Type",FSRTint)->writei(eKTint);
//	bdv->AddKey("Value",FSRTint)->writei(300);
//	
//
//	//obj = objs->AddKey("_KSID_bomba_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	//obj->AddKey("Edited",FSRTchar)->writec(1);
//	//obj->AddKey("Order",FSRTint)->writei(2);
//	//obj->AddKey("Is In Map",FSRTchar)->writec(1);
//	//CFSRegister *bd = obj->AddKey("Basic Data",FSRTregister)->GetSubRegister();
//	//CFSRegister *bdv = bd->AddKey("_KSOV_bomba_0001_FFFF_0001_0001__M__KN_ObjPosX_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	//bdv->AddKey("Type",FSRTint)->writei(eKTint);
//	//bdv->AddKey("Value",FSRTint)->writei(40);
//	//bdv = bd->AddKey("_KSOV_bomba_0001_FFFF_0001_0001__M__KN_ObjPosY_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	//bdv->AddKey("Type",FSRTint)->writei(eKTint);
//	//bdv->AddKey("Value",FSRTint)->writei(80);
//	//bdv = bd->AddKey("_KSOV_bomba_0001_FFFF_0001_0001__M_timer_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	//bdv->AddKey("Type",FSRTint)->writei(eKTint);
//	//bdv->AddKey("Value",FSRTint)->writei(777);
//	r->AddRegisterToKey("Objects",objs);
//
//	FS->ChangeDir("$KRKAL$");
//	r->WriteFile();
//	delete r;
//

////////////////////////////////////////////////////////////////////////////////

	FS->ChangeDir("$GAME$");

	r = new CFSRegister("BasicsTests_0002_4521_FFFF_0001.a","KRKAL AUTOMATICS",FSREGCLEARIT);
	r->SetRegisterToBeUnCompressed();
	CFSRegister *r3, *r4;
	CFSRegKey *k;
	r->AddKey("Version",FSRTstring)->stringwrite("0002_4521_FFFF_0001");
	r->AddKey("Closed File",FSRTint)->writei(1);
	CFSRegister *r2 = r->AddKey("Names",FSRTregister)->GetSubRegister();
	
	// 0
	r3 = r2->AddKey("_KSAD_EditTest_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 1
	r3 = r2->AddKey("_KSAD_bomba_0001_FFFF_0001_0001__M_3random_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATrandom);
	r3->AddKey("Sons Num",FSRTint)->writei(3);
	k = r3->AddKey("Psti",FSRTdouble);
		k->writed(0.4);
		k->writed(0.65);
		k->writed(1);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(2);
		k->writei(3);
		k->writei(4);

	// 2
	r3 = r2->AddKey("_KSAD_bomba_0001_FFFF_0001_0001__M_3random_0002_4521_FFFF_0001__M_var1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0000.tex");

	// 3
	r3 = r2->AddKey("_KSAD_bomba_0001_FFFF_0001_0001__M_3random_0002_4521_FFFF_0001__M_var2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("kamen_1234_4BB1_7894_A0E3.pkg\\kamen1.ani");

	// 4
	r3 = r2->AddKey("_KSAD_bomba_0001_FFFF_0001_0001__M_3random_0002_4521_FFFF_0001__M_var3_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package1_1234_4BB1_7894_A0E3.pkg\\koule.tex");

	// 5
	r3 = r2->AddKey("_KSAD_bomba_0001_FFFF_0001_0001__M_pattern_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpattern);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	k = r3->AddKey("Pattern Consts",FSRTint);
		k->writei(1);
		k->writei(2);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(2);
		k->writei(3);
		k->writei(4);
		k->writei(6);

	// 6
	r3 = r2->AddKey("_KSAD_bomba_0001_FFFF_0001_0001__M_pattern_0002_4521_FFFF_0001__M_var4_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package1_1234_4BB1_7894_A0E3.pkg\\manik1.tex");

	// 7
	r3 = r2->AddKey("_KSAD_klic_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 8
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_small_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATconnect);
	r3->AddKey("Aggressivity",FSRTchar)->writec(0);
	r3->AddKey("Sons Num",FSRTint)->writei(16);
	r3->AddKey("Grid Size",FSRTint)->writei(4);
	r3->AddKey("Mask",FSRTint)->writei(0xF);
	r4 = r3->AddKey("GFNames",FSRTregister)->GetSubRegister();
		r4->AddKey("Names",FSRTint)->writei(9);
		r4->AddKey("Names",FSRTint)->writei(10);
		r4->AddKey("Names",FSRTint)->writei(11);
		r4->AddKey("Names",FSRTint)->writei(12);
	r4 = r3->AddKey("MaskLook",FSRTregister)->GetSubRegister();
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(1);

		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(1); k->writei(1);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(13);
		k->writei(14);
		k->writei(15);
		k->writei(2);
		k->writei(16);
		k->writei(17);
		k->writei(18);
		k->writei(19);
		k->writei(184);
		k->writei(185);
		k->writei(186);
		k->writei(187);
		k->writei(188);
		k->writei(189);
		k->writei(190);
		k->writei(191);

	// 9
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_Nzleva_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 10
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_Nzdola_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 11
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_Nzprava_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 12
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_Nshora_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);

	// 13
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_8000_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena1000.tex");

	// 14
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_0020_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0010.tex");

	// 15
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_8020_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena1010.tex");

	// 16
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_0604_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0101.tex");

	// 17
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_0004_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0001.tex");

	// 18
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_0600_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0100.tex");

	// 19
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_1111_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena1111.tex");

	// 20
	r3 = r2->AddKey("_KSAD_wall_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 21
	r3 = r2->AddKey("_KSAD_floor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 22
	r3 = r2->AddKey("StylPodlahy1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package1_1234_4BB1_7894_A0E3.pkg\\podlaha.tex");

	// 23
	r3 = r2->AddKey("_KSAD_smejdic_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 24
	r3 = r2->AddKey("_KSAD_KamennaPodlaha_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATrandom);
	r3->AddKey("Sons Num",FSRTint)->writei(5);
	k = r3->AddKey("Psti",FSRTdouble);
		k->writed(0.2);
		k->writed(0.4);
		k->writed(0.6);
		k->writed(0.8);
		k->writed(1);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(25);
		k->writei(26);
		k->writei(27);
		k->writei(28);
		k->writei(29);

	// 25
	r3 = r2->AddKey("_KSAD_KamennaPodlaha_0002_4521_FFFF_0001__M_var1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("podlahy_CACE_24F4_AE9E_B529.pkg\\kamenyA.tex");

	// 26
	r3 = r2->AddKey("_KSAD_KamennaPodlaha_0002_4521_FFFF_0001__M_var2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("podlahy_CACE_24F4_AE9E_B529.pkg\\kamenyB.tex");
	
	// 27
	r3 = r2->AddKey("_KSAD_KamennaPodlaha_0002_4521_FFFF_0001__M_var3_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("podlahy_CACE_24F4_AE9E_B529.pkg\\kamenyC.tex");

	// 28
	r3 = r2->AddKey("_KSAD_KamennaPodlaha_0002_4521_FFFF_0001__M_var4_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("podlahy_CACE_24F4_AE9E_B529.pkg\\kamenyD.tex");

	// 29
	r3 = r2->AddKey("_KSAD_KamennaPodlaha_0002_4521_FFFF_0001__M_var5_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("podlahy_CACE_24F4_AE9E_B529.pkg\\kamenyE.tex");

	// 30
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATconnect);
	r3->AddKey("Aggressivity",FSRTchar)->writec(1);
	r3->AddKey("Sons Num",FSRTint)->writei(10);
	r3->AddKey("Grid Size",FSRTint)->writei(8);
	r3->AddKey("Mask",FSRTint)->writei(0xFF);
	r4 = r3->AddKey("GFNames",FSRTregister)->GetSubRegister();
		r4->AddKey("Names",FSRTint)->writei(31);
		r4->AddKey("Names",FSRTint)->writei(32);
		r4->AddKey("Names",FSRTint)->writei(33);
		r4->AddKey("Names",FSRTint)->writei(34);
		r4->AddKey("Names",FSRTint)->writei(35);
		r4->AddKey("Names",FSRTint)->writei(36);
		r4->AddKey("Names",FSRTint)->writei(37);
		r4->AddKey("Names",FSRTint)->writei(38);
	r4 = r3->AddKey("MaskLook",FSRTregister)->GetSubRegister();
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(0); k->writei(0); k->writei(0); k->writei(0); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(1); k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(0); k->writei(0); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(0); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(0); k->writei(0);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(39);
		k->writei(24);

		k->writei(41);
		k->writei(42);
		k->writei(43);
		k->writei(44);
		k->writei(45);
		k->writei(46);
		k->writei(47);
		k->writei(48);

	// 31
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_Nzleva_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 32
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_Nzdola_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 33
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_Nzprava_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 34
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_Nzhora_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 35
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_NzJZ_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 36
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_NzJV_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 37
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_NzSV_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);
	// 38
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_NzSZ_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATname);

	// 39
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_obr0000_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny5.tex");

	// 40
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_obr1111_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("podlahy_CACE_24F4_AE9E_B529.pkg\\kamenyD.tex");

	// 41
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_hr1000_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny4.tex");

	// 42
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_roh1000_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny1.tex");

	// 43
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_hr0100_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny2.tex");

	// 44
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_roh0100_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny3.tex");

	// 45
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_hr0010_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny6.tex");

	// 46
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_roh0010_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny9.tex");

	// 47
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_hr0001_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny8.tex");

	// 48
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_roh0001_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny7.tex");

	// 49
	r3 = r2->AddKey("_KSAD_PAutoTest_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 50
	r3 = r2->AddKey("_KSAD_PodlNavaznost2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATconnect);
	r3->AddKey("Aggressivity",FSRTchar)->writec(1);
	r3->AddKey("Sons Num",FSRTint)->writei(14);
	r3->AddKey("Grid Size",FSRTint)->writei(8);
	r3->AddKey("Mask",FSRTint)->writei(0xFF);
	r4 = r3->AddKey("GFNames",FSRTregister)->GetSubRegister();
		r4->AddKey("Names",FSRTint)->writei(31);
		r4->AddKey("Names",FSRTint)->writei(32);
		r4->AddKey("Names",FSRTint)->writei(33);
		r4->AddKey("Names",FSRTint)->writei(34);
		r4->AddKey("Names",FSRTint)->writei(35);
		r4->AddKey("Names",FSRTint)->writei(36);
		r4->AddKey("Names",FSRTint)->writei(37);
		r4->AddKey("Names",FSRTint)->writei(38);
	r4 = r3->AddKey("MaskLook",FSRTregister)->GetSubRegister();
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(0); k->writei(0); k->writei(0); k->writei(0); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(1); k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(0); k->writei(0); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(0); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(1); k->writei(0); k->writei(0); k->writei(1); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(0); k->writei(1); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(0); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(1); k->writei(0);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(39);
		k->writei(24);

		k->writei(41);
		k->writei(42);
		k->writei(43);
		k->writei(44);
		k->writei(45);
		k->writei(46);
		k->writei(47);
		k->writei(48);

		k->writei(51);
		k->writei(52);
		k->writei(53);
		k->writei(54);

	// 51
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_VnRoh1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny1x.tex");
	// 52
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_VnRoh2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny3x.tex");
	// 53
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_VnRoh3_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny9x.tex");
	// 54
	r3 = r2->AddKey("_KSAD_PodlahovaNavaznost_0002_4521_FFFF_0001__M_VnRoh4_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("PodTest_D06E_4585_7222_FEDE.pkg\\kameny7x.tex");

	// 55
	r3 = r2->AddKey("_KSAD_Mina_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("mina_E9E9_DC80_A193_E322.pkg\\aktivnimina.tex");

	// 56
	r3 = r2->AddKey("_KSAD_mina_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 57
	r3 = r2->AddKey("_KSAD_Vybuch_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("bomba_742A_8645_FCA0_1DE4.pkg\\bomba6.tex");

	// 58
	r3 = r2->AddKey("_KSAD_ovybuch_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 59
	r3 = r2->AddKey("_KSAD_oPropadlo_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 60
	r3 = r2->AddKey("_KSAD_oLaser_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 61
	r3 = r2->AddKey("_KSID_AutoSmer_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 62
	r3 = r2->AddKey("_KSID_Zapad_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 63
	r3 = r2->AddKey("_KSID_Jih_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 64
	r3 = r2->AddKey("_KSID_Vychod_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 65
	r3 = r2->AddKey("_KSID_Sever_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 66
	r3 = r2->AddKey("_KSAD_laser_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("lasery_754C_28ED_5F99_CACA.pkg\\laser4.ani");
	// 67
	r3 = r2->AddKey("_KSAD_laser_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("lasery_754C_28ED_5F99_CACA.pkg\\laser2.ani");
	// 68
	r3 = r2->AddKey("_KSAD_laser_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("lasery_754C_28ED_5F99_CACA.pkg\\laser6.ani");
	// 69
	r3 = r2->AddKey("_KSAD_laser_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("lasery_754C_28ED_5F99_CACA.pkg\\laser8.ani");

	// 70
	r3 = r2->AddKey("_KSAD_laser_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(66);
		k->writei(67);
		k->writei(68);
		k->writei(69);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 71
	r3 = r2->AddKey("_KSAD_oMagnet_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 72
	r3 = r2->AddKey("_KSAD_magnet1_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package3_5E07_939A_E2C8_A91D.pkg\\magnet4.tex");
	// 73
	r3 = r2->AddKey("_KSAD_magnet1_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package3_5E07_939A_E2C8_A91D.pkg\\magnet2.tex");
	// 74
	r3 = r2->AddKey("_KSAD_magnet1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package3_5E07_939A_E2C8_A91D.pkg\\magnet6.tex");
	// 75
	r3 = r2->AddKey("_KSAD_magnet1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package3_5E07_939A_E2C8_A91D.pkg\\magnet8.tex");

	// 76
	r3 = r2->AddKey("_KSAD_magnet1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(72);
		k->writei(73);
		k->writei(74);
		k->writei(75);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 77
	r3 = r2->AddKey("_KSAD_oSmejdivaDira_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 78
	r3 = r2->AddKey("_KSAD_Dira1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("SpecPodlahy_BBB9_E3CF_90F6_F5D3.pkg\\bezpod-dira.tex");

	// 79
	r3 = r2->AddKey("_KSAD_oDira_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 80
	r3 = r2->AddKey("_KSAD_oLava_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 81
	r3 = r2->AddKey("_KSAD_Dira2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("SpecPodlahy_BBB9_E3CF_90F6_F5D3.pkg\\dira.tex");

	// 82
	r3 = r2->AddKey("_KSAD_propadlo1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("SpecPodlahy_BBB9_E3CF_90F6_F5D3.pkg\\propadlo.tex");

	// 83
	r3 = r2->AddKey("_KSAD_oklic_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 84
	r3 = r2->AddKey("_KSID_Barva_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 85
	r3 = r2->AddKey("_KSID_Cervena_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 86
	r3 = r2->AddKey("_KSID_Zluta_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 87
	r3 = r2->AddKey("_KSID_Modra_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 88
	r3 = r2->AddKey("_KSAD_c_klic_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package2_1234_4BB1_7894_A0E3.pkg\\klic_cerveny.tex");

	// 89
	r3 = r2->AddKey("_KSAD_zl_klic_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package2_1234_4BB1_7894_A0E3.pkg\\klic_zluty.tex");

	// 90
	r3 = r2->AddKey("_KSAD_m_klic_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package2_1234_4BB1_7894_A0E3.pkg\\klic_modry.tex");

	// 91
	r3 = r2->AddKey("_KSAD_klic1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(3);
	r3->AddKey("Var Name",FSRTint)->writei(84);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(88);
		k->writei(89);
		k->writei(90);
	k = r3->AddKey("Int",FSRTint);
		k->writei(85);
		k->writei(86);
		k->writei(87);

	// 92
	r3 = r2->AddKey("_KSAD_oPas_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 93
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("pasy_72B7_3847_9AA2_4E82.pkg\\pas4.ani");
	// 94
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("pasy_72B7_3847_9AA2_4E82.pkg\\pas2.ani");
	// 95
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("pasy_72B7_3847_9AA2_4E82.pkg\\pas6.ani");
	// 96
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("pasy_72B7_3847_9AA2_4E82.pkg\\pas8.ani");

	// 97
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(93);
		k->writei(94);
		k->writei(95);
		k->writei(96);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 98
	r3 = r2->AddKey("_KSAD_oPriseraOt_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 99
	r3 = r2->AddKey("_KSAD_prisera1_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package1_1234_4BB1_7894_A0E3.pkg\\manik2.tex");
	// 100
	r3 = r2->AddKey("_KSAD_prisera1_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package1_1234_4BB1_7894_A0E3.pkg\\manik4.tex");
	// 101
	r3 = r2->AddKey("_KSAD_prisera1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package1_1234_4BB1_7894_A0E3.pkg\\manik1.tex");
	// 102
	r3 = r2->AddKey("_KSAD_prisera1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package1_1234_4BB1_7894_A0E3.pkg\\manik3.tex");

	// 103
	r3 = r2->AddKey("_KSAD_prisera1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(99);
		k->writei(100);
		k->writei(101);
		k->writei(102);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 104
	r3 = r2->AddKey("_KSAD_oLed_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 105
	r3 = r2->AddKey("_KSAD_led1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package4_18B8_6140_607E_AB52.pkg\\led.tex");

	// 106
	r3 = r2->AddKey("_KSAD_oAntiMagnet_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 107
	r3 = r2->AddKey("_KSAD_oDracek_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 108
	r3 = r2->AddKey("_KSAD_oExit_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 109
	r3 = r2->AddKey("_KSAD_oSebratelnaMina_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 110
	r3 = r2->AddKey("_KSAD_oHlina_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 111
	r3 = r2->AddKey("_KSAD_oZamek_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 112
	r3 = r2->AddKey("_KSAD_Zamek1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package2_1234_4BB1_7894_A0E3.pkg\\zamek.tex");

	// 113
	r3 = r2->AddKey("_KSAD_oSebratelnaBomba_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 114
	r3 = r2->AddKey("_KSAD_oBomba_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 115
	r3 = r2->AddKey("_KSAD_oZnicitelnaStena_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 116
	r3 = r2->AddKey("_KSAD_oKamen_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 117
	r3 = r2->AddKey("_KSAD_kamen1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("kamen_1234_4BB1_7894_A0E3.pkg\\kamen0000.tex");

	// 118
	r3 = r2->AddKey("_KSAD_oHemr_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 119
	r3 = r2->AddKey("_KSAD_oPasovec_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 120
	r3 = r2->AddKey("_KSAD_oKrabice_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 121
	r3 = r2->AddKey("_KSAD_oPneumatika_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 122
	r3 = r2->AddKey("_KSAD_oBalonek_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 123
	r3 = r2->AddKey("_KSAD_bomba1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("bomba_742A_8645_FCA0_1DE4.pkg\\bomba.ani");

	// 124
	r3 = r2->AddKey("_KSAD_mina1seb_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("mina_E9E9_DC80_A193_E322.pkg\\mina.tex");

	// 125
	r3 = r2->AddKey("_KSAD_exit1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package4_18B8_6140_607E_AB52.pkg\\exit.tex");
	// 126
	r3 = r2->AddKey("_KSAD_hlina1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package4_18B8_6140_607E_AB52.pkg\\hlina.tex");
	// 127
	r3 = r2->AddKey("_KSAD_znicitStena1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package4_18B8_6140_607E_AB52.pkg\\rozbitstena.tex");

	// 128
	r3 = r2->AddKey("_KSAD_c_zamek_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("zamky_D1BA_1ED2_DFA6_997E.pkg\\zamekR.tex");

	// 129
	r3 = r2->AddKey("_KSAD_zl_zamek_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("zamky_D1BA_1ED2_DFA6_997E.pkg\\zamekY.tex");

	// 130
	r3 = r2->AddKey("_KSAD_m_zamek_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("zamky_D1BA_1ED2_DFA6_997E.pkg\\zamekB.tex");

	// 131
	r3 = r2->AddKey("_KSAD_Zamek2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(3);
	r3->AddKey("Var Name",FSRTint)->writei(84);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(128);
		k->writei(129);
		k->writei(130);
	k = r3->AddKey("Int",FSRTint);
		k->writei(85);
		k->writei(86);
		k->writei(87);

	// 132
	r3 = r2->AddKey("_KSAD_voda1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("SpecPodlahy_BBB9_E3CF_90F6_F5D3.pkg\\rozpadA.tex");
	// 133
	r3 = r2->AddKey("_KSAD_oVoda_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 134
	r3 = r2->AddKey("_KSAD_teleport1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package2_1234_4BB1_7894_A0E3.pkg\\teleport.ani");
	// 135
	r3 = r2->AddKey("_KSAD_oTeleport_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 136
	r3 = r2->AddKey("_KSAD_bomba1seb_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("bomba_742A_8645_FCA0_1DE4.pkg\\bomba.tex");

	// 137
	r3 = r2->AddKey("_KSAD_oHajzl_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 138
	r3 = r2->AddKey("_KSAD_oSliz_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 139
	r3 = r2->AddKey("_KSAD_oPriseraSNavadeci_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 140
	r3 = r2->AddKey("_KSAD_oPriseraDopravni_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 141
	r3 = r2->AddKey("_KSAD_oPriseraKlaustrofobni_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 142
	r3 = r2->AddKey("_KSAD_ZnSmerova1_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("znacky_7FF0_880F_3F12_C005.pkg\\prikazanysmer4.tex");
	// 143
	r3 = r2->AddKey("_KSAD_ZnSmerova1_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("znacky_7FF0_880F_3F12_C005.pkg\\prikazanysmer2.tex");
	// 144
	r3 = r2->AddKey("_KSAD_ZnSmerova1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("znacky_7FF0_880F_3F12_C005.pkg\\prikazanysmer6.tex");
	// 145
	r3 = r2->AddKey("_KSAD_ZnSmerova1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("znacky_7FF0_880F_3F12_C005.pkg\\prikazanysmer8.tex");

	// 146
	r3 = r2->AddKey("_KSAD_ZnSmerova1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(142);
		k->writei(143);
		k->writei(144);
		k->writei(145);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 147
	r3 = r2->AddKey("_KSAD_oZnSmerova_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 148
	r3 = r2->AddKey("_KSAD_PriseraOt1_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraotaciva4.tex");
	// 149
	r3 = r2->AddKey("_KSAD_PriseraOt1_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraotaciva2.tex");
	// 150
	r3 = r2->AddKey("_KSAD_PriseraOt1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraotaciva6.tex");
	// 151
	r3 = r2->AddKey("_KSAD_PriseraOt1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraotaciva8.tex");

	// 152
	r3 = r2->AddKey("_KSAD_PriseraOt1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(148);
		k->writei(149);
		k->writei(150);
		k->writei(151);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 153
	r3 = r2->AddKey("_KSAD_PriseraSN1_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvorasamonavadeci4.tex");
	// 154
	r3 = r2->AddKey("_KSAD_PriseraSN1_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvorasamonavadeci2.tex");
	// 155
	r3 = r2->AddKey("_KSAD_PriseraSN1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvorasamonavadeci6.tex");
	// 156
	r3 = r2->AddKey("_KSAD_PriseraSN1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvorasamonavadeci8.tex");

	// 157
	r3 = r2->AddKey("_KSAD_PriseraSN1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(153);
		k->writei(154);
		k->writei(155);
		k->writei(156);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 158
	r3 = r2->AddKey("_KSAD_PriseraDopr1_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraznackova4.tex");
	// 159
	r3 = r2->AddKey("_KSAD_PriseraDopr1_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraznackova2.tex");
	// 160
	r3 = r2->AddKey("_KSAD_PriseraDopr1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraznackova6.tex");
	// 161
	r3 = r2->AddKey("_KSAD_PriseraDopr1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraznackova8.tex");

	// 162
	r3 = r2->AddKey("_KSAD_PriseraDopr1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(158);
		k->writei(159);
		k->writei(160);
		k->writei(161);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 163
	r3 = r2->AddKey("_KSAD_PriseraKl1_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraklaustrofobni4.tex");
	// 164
	r3 = r2->AddKey("_KSAD_PriseraKl1_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraklaustrofobni2.tex");
	// 165
	r3 = r2->AddKey("_KSAD_PriseraKl1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraklaustrofobni6.tex");
	// 166
	r3 = r2->AddKey("_KSAD_PriseraKl1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("potvory_3605_CD59_BFDE_7358.pkg\\potvoraklaustrofobni8.tex");

	// 167
	r3 = r2->AddKey("_KSAD_PriseraKl1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(163);
		k->writei(164);
		k->writei(165);
		k->writei(166);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 168
	r3 = r2->AddKey("_KSAD_Pasovec11_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\pasovec4.tex");
	// 169
	r3 = r2->AddKey("_KSAD_Pasovec11_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\pasovec2.tex");
	// 170
	r3 = r2->AddKey("_KSAD_Pasovec11_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\pasovec6.tex");
	// 171
	r3 = r2->AddKey("_KSAD_Pasovec11_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\pasovec8.tex");

	// 172
	r3 = r2->AddKey("_KSAD_Pasovec11_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(168);
		k->writei(169);
		k->writei(170);
		k->writei(171);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);


	// 173
	r3 = r2->AddKey("_KSAD_prepinca1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(2);
	r3->AddKey("Var Name",FSRTint)->writei(174);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(177);
		k->writei(178);
	k = r3->AddKey("Int",FSRTint);
		k->writei(175);
		k->writei(176);

	// 174
	r3 = r2->AddKey("_KSID_nOnOff_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 175
	r3 = r2->AddKey("_KSID_On_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 176
	r3 = r2->AddKey("_KSID_Off_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 177
	r3 = r2->AddKey("_KSAD_prepinac1_0002_4521_FFFF_0001__M_on_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("prepinace_8DD1_5F30_C6DA_EB77.pkg\\prepinac1.tex");
	// 178
	r3 = r2->AddKey("_KSAD_prepinac1_0002_4521_FFFF_0001__M_off_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("prepinace_8DD1_5F30_C6DA_EB77.pkg\\prepinac0.tex");

	// 179
	r3 = r2->AddKey("_KSAD_fotobunka1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(2);
	r3->AddKey("Var Name",FSRTint)->writei(174);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(180);
		k->writei(181);
	k = r3->AddKey("Int",FSRTint);
		k->writei(175);
		k->writei(176);

	// 180
	r3 = r2->AddKey("_KSAD_fotobunka1_0002_4521_FFFF_0001__M_on_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("prepinace_8DD1_5F30_C6DA_EB77.pkg\\fotobunka.ani");
	// 181
	r3 = r2->AddKey("_KSAD_fotobunka1_0002_4521_FFFF_0001__M_off_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("prepinace_8DD1_5F30_C6DA_EB77.pkg\\fotobunka_0.tex");

	// 182
	r3 = r2->AddKey("_KSAD_oPrepinac_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 183
	r3 = r2->AddKey("_KSAD_oFotobunka_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 184
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_00AA_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0011.tex");
	// 185
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_0AA0_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0110.tex");
	// 186
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_AA00_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena1100.tex");
	// 187
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_A00A_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena1001.tex");
	// 188
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_0AAA_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0111.tex");
	// 189
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_A0AA_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena1011.tex");
	// 190
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_AA0A_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena1101.tex");
	// 191
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_AAA0_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena1110.tex");

	// 192
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001__M_Offzapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("pasy_72B7_3847_9AA2_4E82.pkg\\pas4_0.tex");
	// 193
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001__M_Offjih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("pasy_72B7_3847_9AA2_4E82.pkg\\pas2_0.tex");
	// 194
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001__M_Offvychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("pasy_72B7_3847_9AA2_4E82.pkg\\pas6_0.tex");
	// 195
	r3 = r2->AddKey("_KSAD_pas1_0002_4521_FFFF_0001__M_Offsever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("pasy_72B7_3847_9AA2_4E82.pkg\\pas8_0.tex");

	// 196
	r3 = r2->AddKey("_KSAD_pas1Off_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(192);
		k->writei(193);
		k->writei(194);
		k->writei(195);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 197
	r3 = r2->AddKey("_KSAD_pas2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(2);
	r3->AddKey("Var Name",FSRTint)->writei(174);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(97);
		k->writei(196);
	k = r3->AddKey("Int",FSRTint);
		k->writei(175);
		k->writei(176);


	// 198
	r3 = r2->AddKey("_KSAD_naslapnapodlaha1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(2);
	r3->AddKey("Var Name",FSRTint)->writei(174);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(199);
		k->writei(200);
	k = r3->AddKey("Int",FSRTint);
		k->writei(175);
		k->writei(176);

	// 199
	r3 = r2->AddKey("_KSAD_naslapnapodlaha1_0002_4521_FFFF_0001__M_on_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("prepinace_8DD1_5F30_C6DA_EB77.pkg\\naslapnapodlaha1.tex");
	// 200
	r3 = r2->AddKey("_KSAD_naslapnapodlaha1_0002_4521_FFFF_0001__M_off_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("prepinace_8DD1_5F30_C6DA_EB77.pkg\\naslapnapodlaha0.tex");

	// 201
	r3 = r2->AddKey("_KSAD_oNaslapnaPodlaha_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 202
	r3 = r2->AddKey("_KSAD_hemr11_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\hemr4.tex");
	// 203
	r3 = r2->AddKey("_KSAD_hemr11_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\hemr2.tex");
	// 204
	r3 = r2->AddKey("_KSAD_hemr11_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\hemr6.tex");
	// 205
	r3 = r2->AddKey("_KSAD_hemr11_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\hemr8.tex");

	// 206
	r3 = r2->AddKey("_KSAD_hemr11_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(4);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(202);
		k->writei(203);
		k->writei(204);
		k->writei(205);
	k = r3->AddKey("Int",FSRTint);
		k->writei(62);
		k->writei(63);
		k->writei(64);
		k->writei(65);

	// 207
	r3 = r2->AddKey("_KSAD_hajzl1_0002_4521_FFFF_0001__M_pasovec_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("hajzly_9512_09FE_1EFE_F1F1.pkg\\hajzly - pasovec.tex");

	// 208
	r3 = r2->AddKey("_KSAD_hajzl1_0002_4521_FFFF_0001__M_dracek_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("hajzly_9512_09FE_1EFE_F1F1.pkg\\hajzly - dracek.tex");

	// 209
	r3 = r2->AddKey("_KSAD_hajzl1_0002_4521_FFFF_0001__M_hemr_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("hajzly_9512_09FE_1EFE_F1F1.pkg\\hajzly - hemr.tex");

	// 210
	r3 = r2->AddKey("_KSAD_hajzl1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(3);
	r3->AddKey("Var Name",FSRTint)->writei(214);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(207);
		k->writei(208);
		k->writei(209);
	k = r3->AddKey("Int",FSRTint);
		k->writei(211);
		k->writei(212);
		k->writei(213);

	// 211
	r3 = r2->AddKey("_KSID_oPasovec_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 212
	r3 = r2->AddKey("_KSID_oDracek_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 213
	r3 = r2->AddKey("_KSID_oHemr_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 214
	r3 = r2->AddKey("_KSID_oManikE_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 215
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(2);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(216);
		k->writei(217);
	k = r3->AddKey("Int",FSRTint);
		k->writei(65);
		k->writei(64);

	// 216
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(3);
	r3->AddKey("Var Name",FSRTint)->writei(219);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(223);
		k->writei(224);
		k->writei(225);
	k = r3->AddKey("Int",FSRTint);
		k->writei(220);
		k->writei(221);
		k->writei(222);

	// 117
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(3);
	r3->AddKey("Var Name",FSRTint)->writei(219);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(226);
		k->writei(227);
		k->writei(228);
	k = r3->AddKey("Int",FSRTint);
		k->writei(220);
		k->writei(221);
		k->writei(222);

	// 218
	r3 = r2->AddKey("_KSAD_oDvere_A1CF_6963_8DA6_D822",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 219
	r3 = r2->AddKey("_KSID_nStavOtevreni_A1CF_6963_8DA6_D822",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 220
	r3 = r2->AddKey("_KSID_nZavreno_A1CF_6963_8DA6_D822",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 221
	r3 = r2->AddKey("_KSID_nOteviram_A1CF_6963_8DA6_D822",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 222
	r3 = r2->AddKey("_KSID_nZaviram_A1CF_6963_8DA6_D822",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 223
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001__M_severZ_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("dvere_DBEE_EA88_C4C9_D226.pkg\\dvereH_0000.tex");
	// 224
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001__M_severO1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("dvere_DBEE_EA88_C4C9_D226.pkg\\dvereHot.ani");
	// 225
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001__M_severZ1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("dvere_DBEE_EA88_C4C9_D226.pkg\\dvereHzav.ani");

	// 226
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001__M_vychodZ_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("dvere_DBEE_EA88_C4C9_D226.pkg\\dvereV_0000.tex");
	// 227
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001__M_vychotO1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("dvere_DBEE_EA88_C4C9_D226.pkg\\dvereVot.ani");
	// 228
	r3 = r2->AddKey("_KSAD_dvere1_0002_4521_FFFF_0001__M_vychodZ1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("dvere_DBEE_EA88_C4C9_D226.pkg\\dvereVzav.ani");

	// 229
	r3 = r2->AddKey("_KSAD_propadlo1_0002_4521_FFFF_0001__M_pad_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("SpecPodlahy_BBB9_E3CF_90F6_F5D3.pkg\\propadlo.ani");

	// 230
	r3 = r2->AddKey("_KSAD_propadlo2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(2);
	r3->AddKey("Var Name",FSRTint)->writei(231);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(82);
		k->writei(229);
	k = r3->AddKey("Int",FSRTint);
		k->writei(232);
		k->writei(233);

	// 231
	r3 = r2->AddKey("_KSID_nStavPropadu_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 232
	r3 = r2->AddKey("_KSID_nStavPropaduOK_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);
	// 233
	r3 = r2->AddKey("_KSID_nStavPropaduPad_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATexternName);

	// 234
	r3 = r2->AddKey("_KSAD_krabice1_0002_4521_FFFF_0001__M_severZ1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("prulez_4FA7_6B8E_A990_EEE2.pkg\\krabice.ani");

	// 235
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATrandom);
	r3->AddKey("Sons Num",FSRTint)->writei(5);
	k = r3->AddKey("Psti",FSRTdouble);
		k->writed(0.2);
		k->writed(0.4);
		k->writed(0.6);
		k->writed(0.8);
		k->writed(1);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(236);
		k->writei(237);
		k->writei(238);
		k->writei(239);
		k->writei(240);

	// 236
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_var1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\var1.ani");
	// 237
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_var2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\var2.ani");
	// 238
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_var3_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\var3.ani");
	// 239
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_var4_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\var4.ani");
	// 240
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_var5_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\var5.ani");

	// 241
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num",FSRTint)->writei(2);
	r3->AddKey("Var Name",FSRTint)->writei(61);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(235);
		k->writei(242);
	k = r3->AddKey("Int",FSRTint);
		k->writei(65);
		k->writei(64);

	// 242
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATrandom);
	r3->AddKey("Sons Num",FSRTint)->writei(5);
	k = r3->AddKey("Psti",FSRTdouble);
		k->writed(0.2);
		k->writed(0.4);
		k->writed(0.6);
		k->writed(0.8);
		k->writed(1);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(243);
		k->writei(244);
		k->writei(245);
		k->writei(246);
		k->writei(247);

	// 243
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_rv1_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\rv1.ani");
	// 244
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_rv2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\rv2.ani");
	// 245
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_rv3_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\rv3.ani");
	// 246
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_rv4_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\rv4.ani");
	// 247
	r3 = r2->AddKey("_KSAD_proud1_0002_4521_FFFF_0001__M_rv5_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("proudy_80E4_1C72_330B_B7E5.pkg\\rv5.ani");

	// 248
	r3 = r2->AddKey("_KSAD_pneumatika2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package4_18B8_6140_607E_AB52.pkg\\pneumatika.tex");
	// 249
	r3 = r2->AddKey("_KSAD_balonek2_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
	r3->AddKey("Textures",FSRTstring)->stringwrite("package4_18B8_6140_607E_AB52.pkg\\balonek.tex");

	// 250
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_0000sh_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num",FSRTint)->writei(1);
//	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0000sh.tex");
	r3->AddKey("Textures",FSRTstring)->stringwrite("steny_1234_4BB1_7894_A0E3.pkg\\stena0000.tex");

	// 251
	r3 = r2->AddKey("_KSAD_stena_0001_FFFF_0001_0001__M_stin_0002_4521_FFFF_0001",FSRTregister)->GetSubRegister();
	r3->AddKey("Type",FSRTint)->writei(eKATconnect);
	r3->AddKey("Aggressivity",FSRTchar)->writec(0);
	r3->AddKey("Sons Num",FSRTint)->writei(16);
	r3->AddKey("Grid Size",FSRTint)->writei(4);
	r3->AddKey("Mask",FSRTint)->writei(0xF);
	r4 = r3->AddKey("GFNames",FSRTregister)->GetSubRegister();
		r4->AddKey("Names",FSRTint)->writei(9);
		r4->AddKey("Names",FSRTint)->writei(10);
		r4->AddKey("Names",FSRTint)->writei(11);
		r4->AddKey("Names",FSRTint)->writei(12);
	r4 = r3->AddKey("MaskLook",FSRTregister)->GetSubRegister();
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(1);

		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(0); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(0); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(1); k->writei(0);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(1); k->writei(0); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(1); k->writei(0); k->writei(1); k->writei(1);
		k = r4->AddKey("ML",FSRTint);
		k->writei(0); k->writei(1); k->writei(1); k->writei(1);
	k = r3->AddKey("GoDown",FSRTint);
		k->writei(13);
		k->writei(14);
		k->writei(15);
		k->writei(250);
		k->writei(16);
		k->writei(17);
		k->writei(18);
		k->writei(19);
		k->writei(184);
		k->writei(185);
		k->writei(186);
		k->writei(187);
		k->writei(188);
		k->writei(189);
		k->writei(190);
		k->writei(191);

	// 252
	r3 = r2->AddKey("_KSAD_VeVode_0002_4521_FFFF_0001__M_zapad_0002_4521_FFFF_0001", FSRTregister)->GetSubRegister();
	r3->AddKey("Type", FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num", FSRTint)->writei(1);
	r3->AddKey("Textures", FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\dracek4.tex");
	// 253
	r3 = r2->AddKey("_KSAD_VeVode_0002_4521_FFFF_0001__M_jih_0002_4521_FFFF_0001", FSRTregister)->GetSubRegister();
	r3->AddKey("Type", FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num", FSRTint)->writei(1);
	r3->AddKey("Textures", FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\dracek2.tex");
	// 254
	r3 = r2->AddKey("_KSAD_VeVode_0002_4521_FFFF_0001__M_vychod_0002_4521_FFFF_0001", FSRTregister)->GetSubRegister();
	r3->AddKey("Type", FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num", FSRTint)->writei(1);
	r3->AddKey("Textures", FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\dracek6.tex");
	// 255
	r3 = r2->AddKey("_KSAD_VeVode_0002_4521_FFFF_0001__M_sever_0002_4521_FFFF_0001", FSRTregister)->GetSubRegister();
	r3->AddKey("Type", FSRTint)->writei(eKATpicture);
	r3->AddKey("Sons Num", FSRTint)->writei(1);
	r3->AddKey("Textures", FSRTstring)->stringwrite("manici_9F25_D0A5_007B_90B3.pkg\\dracek8.tex");

	// 256
	r3 = r2->AddKey("_KSAD_VeVode_0002_4521_FFFF_0001", FSRTregister)->GetSubRegister();
	r3->AddKey("Type", FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num", FSRTint)->writei(4);
	r3->AddKey("Var Name", FSRTint)->writei(61);
	k = r3->AddKey("GoDown", FSRTint);
	k->writei(252);
	k->writei(253);
	k->writei(254);
	k->writei(255);
	k = r3->AddKey("Int", FSRTint);
	k->writei(62);
	k->writei(63);
	k->writei(64);
	k->writei(65);

	// 257
	r3 = r2->AddKey("_KSAD_dracek1_0002_4521_FFFF_0001", FSRTregister)->GetSubRegister();
	r3->AddKey("Type", FSRTint)->writei(eKATvar);
	r3->AddKey("Sons Num", FSRTint)->writei(2);
	r3->AddKey("Var Name", FSRTint)->writei(174);
	k = r3->AddKey("GoDown", FSRTint);
	k->writei(103);
	k->writei(256);
	k = r3->AddKey("Int", FSRTint);
	k->writei(176);
	k->writei(175);




	k = r->AddKey("Dependencies",FSRTint);
	k->writei(0); k->writei(1);
	k->writei(7); k->writei(5);
	k->writei(20); k->writei(251);
	k->writei(20); k->writei(8);
	k->writei(21); k->writei(22);
	k->writei(21); k->writei(24);
	k->writei(23); k->writei(8);
	k->writei(49); k->writei(30);
	k->writei(49); k->writei(50);
	k->writei(58); k->writei(57);
	k->writei(56); k->writei(55);
	k->writei(59); k->writei(230);
	k->writei(60); k->writei(70);
	k->writei(71); k->writei(76);
	k->writei(77); k->writei(78);
	k->writei(79); k->writei(81);
	k->writei(80); k->writei(81);
	k->writei(83); k->writei(91);
	k->writei(92); k->writei(197);
	k->writei(98); k->writei(152);
	k->writei(104); k->writei(105);
	k->writei(107); k->writei(257);
	k->writei(108); k->writei(125);
	k->writei(109); k->writei(124);
	k->writei(110); k->writei(126);
	k->writei(111); k->writei(131);
	k->writei(113); k->writei(136);
	k->writei(114); k->writei(123);
	k->writei(115); k->writei(127);
	k->writei(116); k->writei(117);
	k->writei(118); k->writei(206);
	k->writei(119); k->writei(172);
	k->writei(120); k->writei(234);
	k->writei(121); k->writei(4);
	k->writei(122); k->writei(4);
//	k->writei(133); k->writei(132);
	k->writei(135); k->writei(134);
	k->writei(137); k->writei(210);
//	k->writei(138); k->writei(132);
	k->writei(139); k->writei(157);
	k->writei(140); k->writei(162);
	k->writei(141); k->writei(167);
	k->writei(147); k->writei(146);
	k->writei(182); k->writei(173);
	k->writei(183); k->writei(179);
	k->writei(201); k->writei(198);
	k->writei(218); k->writei(215);
	k->writei(121); k->writei(248);
	k->writei(122); k->writei(249);

	k->writei(1); k->writei(2);
	k->writei(1); k->writei(3);
	k->writei(1); k->writei(4);

	k->writei(8); k->writei(2);
	k->writei(8); k->writei(13);
	k->writei(8); k->writei(14);
	k->writei(8); k->writei(15);
	k->writei(8); k->writei(16);
	k->writei(8); k->writei(17);
	k->writei(8); k->writei(18);
	k->writei(8); k->writei(19);

	k->writei(5); k->writei(2);
	k->writei(5); k->writei(3);
	k->writei(5); k->writei(4);
	k->writei(5); k->writei(6);

	k->writei(13); k->writei(10);
	k->writei(14); k->writei(12);
	k->writei(15); k->writei(10);
	k->writei(15); k->writei(12);
	k->writei(16); k->writei(9);
	k->writei(16); k->writei(11);
	k->writei(17); k->writei(11);
	k->writei(18); k->writei(9);
	k->writei(19); k->writei(9);
	k->writei(19); k->writei(10);
	k->writei(19); k->writei(11);
	k->writei(19); k->writei(12);

	k->writei(24); k->writei(25);
	k->writei(24); k->writei(26);
	k->writei(24); k->writei(27);
	k->writei(24); k->writei(28);
	k->writei(24); k->writei(29);

	k->writei(40); k->writei(31);
	k->writei(40); k->writei(32);
	k->writei(40); k->writei(33);
	k->writei(40); k->writei(34);
	k->writei(40); k->writei(35);
	k->writei(40); k->writei(36);
	k->writei(40); k->writei(37);
	k->writei(40); k->writei(38);

	k->writei(25); k->writei(31);
	k->writei(25); k->writei(32);
	k->writei(25); k->writei(33);
	k->writei(25); k->writei(34);
	k->writei(25); k->writei(35);
	k->writei(25); k->writei(36);
	k->writei(25); k->writei(37);
	k->writei(25); k->writei(38);

	k->writei(26); k->writei(31);
	k->writei(26); k->writei(32);
	k->writei(26); k->writei(33);
	k->writei(26); k->writei(34);
	k->writei(26); k->writei(35);
	k->writei(26); k->writei(36);
	k->writei(26); k->writei(37);
	k->writei(26); k->writei(38);

	k->writei(27); k->writei(31);
	k->writei(27); k->writei(32);
	k->writei(27); k->writei(33);
	k->writei(27); k->writei(34);
	k->writei(27); k->writei(35);
	k->writei(27); k->writei(36);
	k->writei(27); k->writei(37);
	k->writei(27); k->writei(38);

	k->writei(28); k->writei(31);
	k->writei(28); k->writei(32);
	k->writei(28); k->writei(33);
	k->writei(28); k->writei(34);
	k->writei(28); k->writei(35);
	k->writei(28); k->writei(36);
	k->writei(28); k->writei(37);
	k->writei(28); k->writei(38);

	k->writei(29); k->writei(31);
	k->writei(29); k->writei(32);
	k->writei(29); k->writei(33);
	k->writei(29); k->writei(34);
	k->writei(29); k->writei(35);
	k->writei(29); k->writei(36);
	k->writei(29); k->writei(37);
	k->writei(29); k->writei(38);

	k->writei(41); k->writei(31);
	k->writei(41); k->writei(32);
	k->writei(41); k->writei(34);
	k->writei(41); k->writei(35);
	k->writei(41); k->writei(38);

	k->writei(43); k->writei(31);
	k->writei(43); k->writei(32);
	k->writei(43); k->writei(33);
	k->writei(43); k->writei(35);
	k->writei(43); k->writei(36);

	k->writei(45); k->writei(32);
	k->writei(45); k->writei(33);
	k->writei(45); k->writei(34);
	k->writei(45); k->writei(36);
	k->writei(45); k->writei(37);

	k->writei(47); k->writei(31);
	k->writei(47); k->writei(33);
	k->writei(47); k->writei(34);
	k->writei(47); k->writei(37);
	k->writei(47); k->writei(38);

	k->writei(42); k->writei(31);
	k->writei(42); k->writei(32);
	k->writei(42); k->writei(35);

	k->writei(44); k->writei(32);
	k->writei(44); k->writei(33);
	k->writei(44); k->writei(36);

	k->writei(46); k->writei(33);
	k->writei(46); k->writei(34);
	k->writei(46); k->writei(37);

	k->writei(48); k->writei(34);
	k->writei(48); k->writei(31);
	k->writei(48); k->writei(38);

	k->writei(30); k->writei(39);
	k->writei(30); k->writei(24);
	k->writei(30); k->writei(41);
	k->writei(30); k->writei(42);
	k->writei(30); k->writei(43);
	k->writei(30); k->writei(44);
	k->writei(30); k->writei(45);
	k->writei(30); k->writei(46);
	k->writei(30); k->writei(47);
	k->writei(30); k->writei(48);

	k->writei(50); k->writei(39);
	k->writei(50); k->writei(24);
	k->writei(50); k->writei(41);
	k->writei(50); k->writei(42);
	k->writei(50); k->writei(43);
	k->writei(50); k->writei(44);
	k->writei(50); k->writei(45);
	k->writei(50); k->writei(46);
	k->writei(50); k->writei(47);
	k->writei(50); k->writei(48);
	k->writei(50); k->writei(51);
	k->writei(50); k->writei(52);
	k->writei(50); k->writei(53);
	k->writei(50); k->writei(54);

	k->writei(51); k->writei(31);
	k->writei(51); k->writei(32);
	k->writei(51); k->writei(33);
	k->writei(51); k->writei(34);
	k->writei(51); k->writei(35);
	k->writei(51); k->writei(36);
	k->writei(51); k->writei(38);

	k->writei(52); k->writei(31);
	k->writei(52); k->writei(32);
	k->writei(52); k->writei(33);
	k->writei(52); k->writei(34);
	k->writei(52); k->writei(35);
	k->writei(52); k->writei(36);
	k->writei(52); k->writei(37);

	k->writei(53); k->writei(31);
	k->writei(53); k->writei(32);
	k->writei(53); k->writei(33);
	k->writei(53); k->writei(34);
	k->writei(53); k->writei(36);
	k->writei(53); k->writei(37);
	k->writei(53); k->writei(38);

	k->writei(54); k->writei(31);
	k->writei(54); k->writei(32);
	k->writei(54); k->writei(33);
	k->writei(54); k->writei(34);
	k->writei(54); k->writei(35);
	k->writei(54); k->writei(37);
	k->writei(54); k->writei(38);

	k->writei(70); k->writei(66);
	k->writei(70); k->writei(67);
	k->writei(70); k->writei(68);
	k->writei(70); k->writei(69);

	k->writei(76); k->writei(72);
	k->writei(76); k->writei(73);
	k->writei(76); k->writei(74);
	k->writei(76); k->writei(75);

	k->writei(91); k->writei(88);
	k->writei(91); k->writei(89);
	k->writei(91); k->writei(90);

	k->writei(97); k->writei(93);
	k->writei(97); k->writei(94);
	k->writei(97); k->writei(95);
	k->writei(97); k->writei(96);

	k->writei(103); k->writei(99);
	k->writei(103); k->writei(100);
	k->writei(103); k->writei(101);
	k->writei(103); k->writei(102);

	k->writei(131); k->writei(128);
	k->writei(131); k->writei(129);
	k->writei(131); k->writei(130);

	k->writei(146); k->writei(142);
	k->writei(146); k->writei(143);
	k->writei(146); k->writei(144);
	k->writei(146); k->writei(145);

	k->writei(152); k->writei(148);
	k->writei(152); k->writei(149);
	k->writei(152); k->writei(150);
	k->writei(152); k->writei(151);

	k->writei(157); k->writei(153);
	k->writei(157); k->writei(154);
	k->writei(157); k->writei(155);
	k->writei(157); k->writei(156);

	k->writei(162); k->writei(158);
	k->writei(162); k->writei(159);
	k->writei(162); k->writei(160);
	k->writei(162); k->writei(161);

	k->writei(167); k->writei(163);
	k->writei(167); k->writei(164);
	k->writei(167); k->writei(165);
	k->writei(167); k->writei(166);

	k->writei(172); k->writei(168);
	k->writei(172); k->writei(169);
	k->writei(172); k->writei(170);
	k->writei(172); k->writei(171);

	k->writei(173); k->writei(177);
	k->writei(173); k->writei(178);

	k->writei(179); k->writei(180);
	k->writei(179); k->writei(181);

	k->writei(8); k->writei(184);
	k->writei(8); k->writei(185);
	k->writei(8); k->writei(186);
	k->writei(8); k->writei(187);
	k->writei(8); k->writei(188);
	k->writei(8); k->writei(189);
	k->writei(8); k->writei(190);
	k->writei(8); k->writei(191);

	k->writei(184); k->writei(11);
	k->writei(184); k->writei(12);
	k->writei(185); k->writei(9);
	k->writei(185); k->writei(12);
	k->writei(186); k->writei(10);
	k->writei(186); k->writei(9);
	k->writei(187); k->writei(10);
	k->writei(187); k->writei(11);

	k->writei(188); k->writei(11);
	k->writei(188); k->writei(12);
	k->writei(188); k->writei(9);
	
	k->writei(189); k->writei(11);
	k->writei(189); k->writei(12);
	k->writei(189); k->writei(10);

	k->writei(190); k->writei(11);
	k->writei(190); k->writei(10);
	k->writei(190); k->writei(9);

	k->writei(191); k->writei(10);
	k->writei(191); k->writei(9);
	k->writei(191); k->writei(12);

	k->writei(196); k->writei(192);
	k->writei(196); k->writei(193);
	k->writei(196); k->writei(194);
	k->writei(196); k->writei(195);

	k->writei(197); k->writei(97);
	k->writei(197); k->writei(196);

	k->writei(198); k->writei(199);
	k->writei(198); k->writei(200);

	k->writei(206); k->writei(202);
	k->writei(206); k->writei(203);
	k->writei(206); k->writei(204);
	k->writei(206); k->writei(205);

	k->writei(210); k->writei(207);
	k->writei(210); k->writei(208);
	k->writei(210); k->writei(209);

	k->writei(215); k->writei(216);
	k->writei(215); k->writei(217);

	k->writei(216); k->writei(223);
	k->writei(216); k->writei(224);
	k->writei(216); k->writei(225);

	k->writei(217); k->writei(226);
	k->writei(217); k->writei(227);
	k->writei(217); k->writei(228);

	k->writei(230); k->writei(82);
	k->writei(230); k->writei(229);

	k->writei(235); k->writei(236);
	k->writei(235); k->writei(237);
	k->writei(235); k->writei(238);
	k->writei(235); k->writei(239);
	k->writei(235); k->writei(240);

	k->writei(241); k->writei(235);
	k->writei(241); k->writei(242);

	k->writei(242); k->writei(243);
	k->writei(242); k->writei(244);
	k->writei(242); k->writei(245);
	k->writei(242); k->writei(246);
	k->writei(242); k->writei(247);

	k->writei(251); k->writei(250);
	k->writei(251); k->writei(13);
	k->writei(251); k->writei(14);
	k->writei(251); k->writei(15);
	k->writei(251); k->writei(16);
	k->writei(251); k->writei(17);
	k->writei(251); k->writei(18);
	k->writei(251); k->writei(19);
	k->writei(251); k->writei(184);
	k->writei(251); k->writei(185);
	k->writei(251); k->writei(186);
	k->writei(251); k->writei(187);
	k->writei(251); k->writei(188);
	k->writei(251); k->writei(189);
	k->writei(251); k->writei(190);
	k->writei(251); k->writei(191);

	k->writei(256); k->writei(252);
	k->writei(256); k->writei(253);
	k->writei(256); k->writei(254);
	k->writei(256); k->writei(255);

	k->writei(257); k->writei(256);
	k->writei(257); k->writei(103);


	r->WriteFile();
	delete r;
	FS->ChangeDir("$KRKAL$");



/////////////////////////////////////////////////////////////////////////////////
	r = new CFSRegister("$ERRORS$","ERRORS",FSREGCLEARIT);
	r->SetRegisterToBeUnCompressed();
	// FATALS:
	r2 = r->AddKey("Ker RTE",FSRTregister)->GetSubRegister();
	sprintf(str,"%u",eKRTEDCNoObj);
	r2->AddKey(str,FSRTstring)->stringwrite("Directly Calling Method of Not Existing Object");
	sprintf(str,"%u",eKRTEDCBadObj);
	r2->AddKey(str,FSRTstring)->stringwrite("Direct Call: This Object doesnt heve called method!");
	sprintf(str,"%u",eKRTEobjcount1);
	r2->AddKey(str,FSRTstring)->stringwrite("Too meny objects was created. Object Counter is 0x10000000");
	sprintf(str,"%u",eKRTEobjcount2);
	r2->AddKey(str,FSRTstring)->stringwrite("Too meny objects was created. Object Counter is 0x80000000");
	sprintf(str,"%u",eKRTEobjcount3);
	r2->AddKey(str,FSRTstring)->stringwrite("Too meny objects was created. Object Counter is 0xFFFFFF00");
	sprintf(str,"%u",eKRTEcallstackw);
	r2->AddKey(str,FSRTstring)->stringwrite("Warning: Too Many function Calls");
	sprintf(str,"%u",eKRTEmessagew);
	r2->AddKey(str,FSRTstring)->stringwrite("Warning: There's too many messages");
	sprintf(str,"%u",eKRTEmapNotRegistered);
	r2->AddKey(str,FSRTstring)->stringwrite("Map Not Registered!");
	sprintf(str,"%u",eKRTEautoNotFound);
	r2->AddKey(str,FSRTstring)->stringwrite("No Automatic Graphic Found for Object");
	sprintf(str,"%u",eKRTEuserFatal);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Fatel Error");
	sprintf(str,"%u",eKRTEnotValidName);
	r2->AddKey(str,FSRTstring)->stringwrite("Invalid Name. The variable is probably undefined.");
	sprintf(str,"%u",eKRTEcycleInbjPtrs);
	r2->AddKey(str,FSRTstring)->stringwrite("Saving Levels: There is a cycle in obj Pointers!");

	// INFOS:
	sprintf(str,"%u",eKRTEKernelInit);
	r2->AddKey(str,FSRTstring)->stringwrite("Kernel Starts! Ready to run scripts");
	sprintf(str,"%u",eKRTEPaniCInfo);
	r2->AddKey(str,FSRTstring)->stringwrite("Kernel Panic: Kernel is shutting Down immediately!!");
	sprintf(str,"%u",eKRTELoadingLevel);
	r2->AddKey(str,FSRTstring)->stringwrite("Kernel is loading level:");
	sprintf(str,"%u",eKRTELoadingScript);
	r2->AddKey(str,FSRTstring)->stringwrite("Kernel is loading scripts. Version:");
	sprintf(str,"%u",eKRTELoadComplete);
	r2->AddKey(str,FSRTstring)->stringwrite("...Loading Complete.");
	sprintf(str,"%u",eKRTECompilating);
	r2->AddKey(str,FSRTstring)->stringwrite("Scripts need to be recompiled. Recompiling..");
	sprintf(str,"%u",eKRTEKernelEnds);
	r2->AddKey(str,FSRTstring)->stringwrite("Kernel is shutting down ..");
	sprintf(str,"%u",eKRTEsavingLevel);
	r2->AddKey(str,FSRTstring)->stringwrite("Saving Level ...");
	sprintf(str,"%u",eKRTEsaveLOK);
	r2->AddKey(str,FSRTstring)->stringwrite("Save Successful!");
	sprintf(str,"%u",eKRTEuserInfo);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Info");
	sprintf(str,"%u",eKRTEgameVictory);
	r2->AddKey(str,FSRTstring)->stringwrite("You are Victorious!! Congratulations!! :)");
	sprintf(str,"%u",eKRTEloadingGame);
	r2->AddKey(str,FSRTstring)->stringwrite("Kernel is loading saved game:");
	sprintf(str,"%u",eKRTEsavingGame);
	r2->AddKey(str,FSRTstring)->stringwrite("Saving Game ...");

	// PANICS:
	sprintf(str,"%u",eKRTENoMemory);
	r2->AddKey(str,FSRTstring)->stringwrite("Out Of Script's Stack Memory!!");
	sprintf(str,"%u",eKRTEDelObjInUse);
	r2->AddKey(str,FSRTstring)->stringwrite("Deleting Object In Use!! (Dectructor called in incorrect time)");
	sprintf(str,"%u",eKRTEobjcountOVERFLOW);
	r2->AddKey(str,FSRTstring)->stringwrite("Object Counter is 0xFFFFFFFF. Object counter has overflowed!!!");
	sprintf(str,"%u",eKRTEPELoadingLevel);
	r2->AddKey(str,FSRTstring)->stringwrite("Error while loading Level");
	sprintf(str,"%u",eKRTEPELoadingScripts);
	r2->AddKey(str,FSRTstring)->stringwrite("Error while loading Scripts");
	sprintf(str,"%u",eKRTEOutOfTime);
	r2->AddKey(str,FSRTstring)->stringwrite("Time Counter has Overflowed!");
	sprintf(str,"%u",eKRTEcallstack);
	r2->AddKey(str,FSRTstring)->stringwrite("Too Many function Calls! Stack Full!");
	sprintf(str,"%u",eKRTEmessage);
	r2->AddKey(str,FSRTstring)->stringwrite("There's too many messages! Quees Full!");
	sprintf(str,"%u",eKRTEuserPanic);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Panic Error");
	sprintf(str,"%u",eKRTECompilationFailed);
	r2->AddKey(str,FSRTstring)->stringwrite("Compilatin Failed!!");
	sprintf(str,"%u",eKRTEpictureNotLoaded);
	r2->AddKey(str,FSRTstring)->stringwrite("Failed to load Picture!");
	sprintf(str,"%u",eKRTEcyclusInKSID);
	r2->AddKey(str,FSRTstring)->stringwrite("There is a cycle in KSID names!");
	sprintf(str,"%u",eKRTEerrorInAuto);
	r2->AddKey(str,FSRTstring)->stringwrite("Automatic graphic is corrupted. Check the file.");
	sprintf(str,"%u",eKRTEmapNotWorking);
	r2->AddKey(str,FSRTstring)->stringwrite("Objet Map is not working correctly.");
	sprintf(str,"%u",eKRTEdeletingBadArray);
	r2->AddKey(str,FSRTstring)->stringwrite("deleting Not existing or Invalid Kernel Array!");
	sprintf(str,"%u",eKRTEbadKeyName);
	r2->AddKey(str,FSRTstring)->stringwrite("This Cannot be a name for key Input!");
	sprintf(str,"%u",eKRTEbadSoundName);
	r2->AddKey(str,FSRTstring)->stringwrite("This Cannot be a name for Sound!");
	sprintf(str,"%u",eKRTEobjPtrConflict);
	r2->AddKey(str,FSRTstring)->stringwrite("There are two same, conflicting ObjPtrs!");
	sprintf(str,"%u",eKRTEinvLvlVersion);
	r2->AddKey(str,FSRTstring)->stringwrite("The level version doesn't match! Hack attempt?");

	// CONVERSION:
	sprintf(str,"%u",eKRTEptrtonum);
	r2->AddKey(str,FSRTstring)->stringwrite("Converting pointer-type param to numeric-type param.");
	sprintf(str,"%u",eKRTEnumtoptr);
	r2->AddKey(str,FSRTstring)->stringwrite("Converting numeric-type param to pointer-type param.");
	sprintf(str,"%u",eKRTEptrconv);
	r2->AddKey(str,FSRTstring)->stringwrite("Bad conversion of pointer-type parameter");
	sprintf(str,"%u",eKRTEstrtonum);
	r2->AddKey(str,FSRTstring)->stringwrite("Converting string to numeric or pointer-type param.");
	sprintf(str,"%u",eKRTEnumtostr);
	r2->AddKey(str,FSRTstring)->stringwrite("Converting numeric or pointer-type param to string.");
	sprintf(str,"%u",eKRTEarrayconv);
	r2->AddKey(str,FSRTstring)->stringwrite("Converting Arrays to Different type");
	sprintf(str,"%u",eKRTEstringError);
	r2->AddKey(str,FSRTstring)->stringwrite("Error in string. End Null not found");
	sprintf(str,"%u",eKRTEuserConversion);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Prm Conversion Error");

	// Assign:
	sprintf(str,"%u",eKRTEmenyToOne);
	r2->AddKey(str,FSRTstring)->stringwrite("Passing many arguments to one.");
	sprintf(str,"%u",eKRTEretMenyToOne);
	r2->AddKey(str,FSRTstring)->stringwrite("Returning many arguments to one.");
	sprintf(str,"%u",eKRTEfceNotReturning);
	r2->AddKey(str,FSRTstring)->stringwrite("Called Function Doesn't return a value.");
	sprintf(str,"%u",eKRTEnothingRetInArg);
	r2->AddKey(str,FSRTstring)->stringwrite("Nothing was returned in this Argument.");
	sprintf(str,"%u",eKRTEretTypeChanged);
	r2->AddKey(str,FSRTstring)->stringwrite("Type of return Function was changed.");
	sprintf(str,"%u",eKRTEuserAssignation);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Prm Assignation Error");

	// Call:
	sprintf(str,"%u",eKRTESCnoObj);
	r2->AddKey(str,FSRTstring)->stringwrite("Safe Inmediate Call: Calling object doesnt exist.");
	sprintf(str,"%u",eKRTEBadMethod);
	r2->AddKey(str,FSRTstring)->stringwrite("Calling Bad Method. Name is probably Null");
	sprintf(str,"%u",eKRTEuserCallingE);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Calling Error");

	// Error:
	sprintf(str,"%u",eKRTEInvalidObjType);
	r2->AddKey(str,FSRTstring)->stringwrite("Constructing Object of unknown or invalid Type.");
	sprintf(str,"%u",eKRTEELoadingScripts);
	r2->AddKey(str,FSRTstring)->stringwrite("Error while loading Scripts");
	sprintf(str,"%u",eKRTEIllegalFree);
	r2->AddKey(str,FSRTstring)->stringwrite("Freeing illegal pointer!");
	sprintf(str,"%u",eKRTEVarLoad);
	r2->AddKey(str,FSRTstring)->stringwrite("Unable to Load Variable");
	sprintf(str,"%u",eKRTECOnoObj);
	r2->AddKey(str,FSRTstring)->stringwrite("Copying not Existing Object");
	sprintf(str,"%u",eKRTEarrayAccErr);
	r2->AddKey(str,FSRTstring)->stringwrite("Invalid acces to Array");
	sprintf(str,"%u",eKRTEarrayIsNull);
	r2->AddKey(str,FSRTstring)->stringwrite("Accessing Null (empty) array");
	sprintf(str,"%u",eKRTEaccessingNEarray);
	r2->AddKey(str,FSRTstring)->stringwrite("Accessing Not existing or Invalid Array");
	sprintf(str,"%u",eKRTEsaveLoadNotAllowed);
	r2->AddKey(str,FSRTstring)->stringwrite("SaveLoad Operation is not allowed here.");
	sprintf(str,"%u",eKRTEuserError);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Error");
	sprintf(str,"%u",eKRTESLwriteInMiddle);
	r2->AddKey(str,FSRTstring)->stringwrite("This type has a variable length. You cannot write it in the middle of the stream.");
	sprintf(str,"%u",eKRTESLloadingVar);
	r2->AddKey(str,FSRTstring)->stringwrite("Manual Load of Variable failed. Invalid type or end of source stream.");
	sprintf(str,"%u",eKRTEstaticLoad);
	r2->AddKey(str,FSRTstring)->stringwrite("Unable to Load Static Object. (object not found in level)");
	sprintf(str,"%u",eKRTEobjectLoad);
	r2->AddKey(str,FSRTstring)->stringwrite("Unable to load Object (object not found in scripts)");
	sprintf(str,"%u",eKRTEsavingOptrToNoSavO);
	r2->AddKey(str,FSRTstring)->stringwrite("Saving Pointer to Object, that will not be saved (pointer converted to Null)");
	sprintf(str,"%u",eKRTEsavingGlobalObjPtr);
	r2->AddKey(str,FSRTstring)->stringwrite("You Cannot Save Obj pointer in a Global Variable");
	sprintf(str,"%u",eKRTEsavingLIOErr);
	r2->AddKey(str,FSRTstring)->stringwrite("Error writing Level to Disk (bad path, disk full, access denied ..?)");
	sprintf(str,"%u",eKRTEinvalidEditType);
	r2->AddKey(str,FSRTstring)->stringwrite("This is not valid modifier for editable item");
	sprintf(str,"%u",eKRTEgameNotSaved);
	r2->AddKey(str,FSRTstring)->stringwrite("Error while saving Game. Game NOT saved.");

	// Warning:
	sprintf(str,"%u",eKRTENoError);
	r2->AddKey(str,FSRTstring)->stringwrite("No Error.");
	sprintf(str,"%u",eKRTEFreeToNull);
	r2->AddKey(str,FSRTstring)->stringwrite("Freeing NULL pointer.");
	sprintf(str,"%u",eKRTEarrAddedNDef);
	r2->AddKey(str,FSRTstring)->stringwrite("Array was expanded using Non Defined values. (accesing far above end)");
	sprintf(str,"%u",eKRTEuserWarning);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Warning");
	sprintf(str,"%u",eKRTEKeyNameExpected);
	r2->AddKey(str,FSRTstring)->stringwrite("IsKeyDown: This Isn't a name for Key Input");
	sprintf(str,"%u",eKRTESoundNameExpected);
	r2->AddKey(str,FSRTstring)->stringwrite("PlaySound: This Isn't a name for Sound");

	// Map Errors:
	sprintf(str,"%u",eKRTEuserMapError);
	r2->AddKey(str,FSRTstring)->stringwrite("User's Map Error");
	sprintf(str,"%u",eKRTEplacingOutOfMap);
	r2->AddKey(str,FSRTstring)->stringwrite("Placing Object out of Map (nothing placed)");
	sprintf(str,"%u",eKRTEplacedOhNoGraphic);
	r2->AddKey(str,FSRTstring)->stringwrite("Placing Object with No Graphic");
	sprintf(str,"%u",eKRTEmovingOutOfMap);
	r2->AddKey(str,FSRTstring)->stringwrite("Moving Object out of Map (move canceled)");

	// J.M.: chyby interpretu
	sprintf(str,"%u",eKRTEIllegalAddressRead);
	r2->AddKey(str,FSRTstring)->stringwrite("Cteni mimo pamet interpretu");
	sprintf(str,"%u",eKRTEInstrOutsideCode);
	r2->AddKey(str,FSRTstring)->stringwrite("Pokus o provedeni instrukce mimo oblast kodu");
	sprintf(str,"%u",eKRTEUnknownInstr);
	r2->AddKey(str,FSRTstring)->stringwrite("Pokus o provedeni nezname instrukce ");
	sprintf(str,"%u",eKRTEIllegalAddressWrite);
	r2->AddKey(str,FSRTstring)->stringwrite("Zapis mimo pamet interpretu");
	sprintf(str,"%u",eKRTEAddrNotAlligned);
	r2->AddKey(str,FSRTstring)->stringwrite("Adresa neni zarovnana na 4 B");
	sprintf(str,"%u",eKRTEStackOverflow);
	r2->AddKey(str,FSRTstring)->stringwrite("Preteceni zasobniku");
	sprintf(str,"%u",eKRTEHeapOverflow);
	r2->AddKey(str,FSRTstring)->stringwrite("Preteceni haldy");
	sprintf(str,"%u",eKRTEStackUnderflow);
	r2->AddKey(str,FSRTstring)->stringwrite("Podteceni zasobniku");
	sprintf(str,"%u",eKRTEDivideByZero);
	r2->AddKey(str,FSRTstring)->stringwrite("Deleni nulou");
	sprintf(str,"%u",eKRTEOutsideTmpStack);
	r2->AddKey(str,FSRTstring)->stringwrite("Pristup mimo tmpStack");
	sprintf(str,"%u",eKRTECorruptedInstr);
	r2->AddKey(str,FSRTstring)->stringwrite("eKRTECorruptedInstr");
	sprintf(str,"%u",eKRTETooLongExecution);
	r2->AddKey(str,FSRTstring)->stringwrite("Zacykleni interpretu, resp. timeout");

	r->WriteFile();
	delete r;




//	r = new CFSRegister("$SCRIPTS$\\0001_FFFF_0001_0001.code","KRKAL SCRIPT CODE",FSREGCLEARIT);
//	r->SetRegisterToBeUnCompressed();
//	r->AddKey("Scripts Version",FSRTstring)->stringwrite("0001_FFFF_0001_0001");
//	r->AddKey("Scripts Version 64",FSRTint64)->write64(0x0001ffff00010001);
//	r->AddKey("Scripts Name",FSRTstring)->stringwrite("Prvni Testovaci Skripty");
//	r->AddKey("Comment",FSRTstring)->stringwrite("Bla Bla Bla Bla ...");
//	r->AddKey("Author",FSRTstring)->stringwrite("Honza M.D. Krcek");
//	r->AddKey("Game",FSRTstring)->stringwrite("Unknown");
//	r->AddKey("Compiled Scripts Version",FSRTint)->writei(1);
//	r->AddKey("Number of Ker Names",FSRTint)->writei(23);
//	CFSRegKey *p = r->AddKey("Ker Names",FSRTstring);
//	p->stringwrite("_KSID_stena_0001_FFFF_0001_0001");                  // 0
//	p->stringwrite("_KSID_bomba_0001_FFFF_0001_0001");					// 1
//	p->stringwrite("_KSID_hlina_0001_FFFF_0001_0001");					// 2
//	p->stringwrite("_KSID_klic_0001_FFFF_0001_0001");					// 3
//	p->stringwrite("_KSID_podlaha_0001_FFFF_0001_0001");				// 4
//	p->stringwrite("_KSID_led_0001_FFFF_0001_0001");					// 5	
//	p->stringwrite("_KSID_voda_0001_FFFF_0001_0001");					// 6
//	p->stringwrite("_KSID_znicitelne_ve_vybuchu_0001_FFFF_0001_0001");	// 7
//	p->stringwrite("_KSID_Destructor");									// 8
//	p->stringwrite("_KSID_znic_se_0001_FFFF_0001_0001");				// 9
//	p->stringwrite("_KSID_znic_se_ve_vybuchu_0001_FFFF_0001_0001");		// 10
//	p->stringwrite("_KSID_spadni_do_diry_0001_FFFF_0001_0001");			// 11
//	p->stringwrite("_KSID_spadni_do_vody_0001_FFFF_0001_0001");			// 12
//	p->stringwrite("_KSID_bomba_0001_FFFF_0001_0001__M_vybouchni_0001_FFFF_0001_0001");//13
//	p->stringwrite("_KSID_Constructor");								// 14
//	p->stringwrite("_KSID_CopyConstructor");							// 15
//	p->stringwrite("_KSID_LoadConstructor");							// 16
//	p->stringwrite("_KSID_EConstructor");								// 17
//	p->stringwrite("_KSID_EDestructor");								// 18
//	p->stringwrite("_KSID_ELoadConstructor");							// 19
//	p->stringwrite("_KSID_OMapa_0001_FFFF_0001_0001");					// 20
//	p->stringwrite("_KSID_OMapa_0001_FFFF_0001_0001__M_Init1_0001_FFFF_0001_0001");	// 21
//	p->stringwrite("_KSID_OMapa_0001_FFFF_0001_0001__M_Init2_0001_FFFF_0001_0001");	// 22
//	p->stringwrite("_KSID__KN_MapSetActivCell");						// 23
//	p->stringwrite("_KSID__KN_MapGetNextObject");						// 24
//	p->stringwrite("_KSID__KN_CellX");									// 25
//	p->stringwrite("_KSID__KN_CellY");									// 26
//	p->stringwrite("_KSID__KN_CellZ");									// 27
//	p = r->AddKey("Dependencies",FSRTint);
//	p->writei(7); p->writei(1);
//	p->writei(7); p->writei(2);
//	p->writei(7); p->writei(3);
//	p->writei(7); p->writei(5);
//	p->writei(8); p->writei(9);
//	p->writei(9); p->writei(10);
//	p->writei(9); p->writei(11);
//	p->writei(11); p->writei(12);
//	r->AddKey("Code",FSRTchar)->blockwrite("MZxx00xx00xx00xx00",12);
//
//	CFSRegister *o = r->AddKey("Objects",FSRTregister)->GetSubRegister();
//	CFSRegister *ko, *m, *km;
////	CFSRegister *pr *kpr;
//	ko = o->AddKey("_KSID_stena_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	ko->AddKey("Name",FSRTint)->writei(0);
//	ko->AddKey("Edit Tag",FSRTint)->writei(0);
////	ko->AddKey("KSVG",FSRTstring)->stringwrite("");
//	// Zadat tagy pro editor
////	m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
//
//	ko = o->AddKey("_KSID_bomba_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	ko->AddKey("Name",FSRTint)->writei(1);
//	ko->AddKey("Edit Tag",FSRTint)->writei(0);
//	ko->AddKey("KSVG",FSRTstring)->stringwrite("_KSVG_0_bomba_0001_FFFF_0001_0001");
//	// Data
//	m = ko->AddKey("Data",FSRTregister)->GetSubRegister();
//	km = m->AddKey("_KSOV_bomba_0001_FFFF_0001_0001__M_timer_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Type",FSRTint)->writei(eKTint);
//	km->AddKey("Use",FSRTint)->writei(1);
//	// Methods
//	m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
//	km = m->AddKey("_KSM_3_constructor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(14);
////	km->AddKey("Jump",FSRTint)->writei(0);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
////	pr = km->AddKey("Params",FSRTregister)->GetSubRegister();
//	//kpr = pr->AddKey("1",FSRTregister)->GetSubRegister();
//	//kpr->AddKey("Name",FSRTint)->writei(555);
//	//kpr->AddKey("Type",FSRTint)->writei(2);
//	//kpr->AddKey("Default",FSRTdouble)->writed(1);
//	km = m->AddKey("_KSM_4_editconstructor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(16);
////	km->AddKey("Jump",FSRTint)->writei(0);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
////	pr = km->AddKey("Params",FSRTregister)->GetSubRegister();
//	km = m->AddKey("_KSM_2_vybouchni_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(13);
////	km->AddKey("Jump",FSRTint)->writei(0);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
////	pr = km->AddKey("Params",FSRTregister)->GetSubRegister();
//	km = m->AddKey("_KSM_1_znic_se_ve_vybuchu_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(10);
////	km->AddKey("Jump",FSRTint)->writei(0);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
////	pr = km->AddKey("Params",FSRTregister)->GetSubRegister();
//	km = m->AddKey("_KSM_22_dc_test_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(0); // 0 - Direct, 1 - safe
//	km->AddKey("DirectName",FSRTstring)->stringwrite("_KSDM_bomba_0001_FFFF_0001_0001__M_dc_test_0001_FFFF_0001_0001");
//
//	ko = o->AddKey("_KSID_OMapa_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	ko->AddKey("Name",FSRTint)->writei(20);
//	ko->AddKey("Edit Tag",FSRTint)->writei(0);
//	ko->AddKey("KSVG",FSRTstring)->stringwrite("_KSVG_0_OMapa_0001_FFFF_0001_0001");
//	// Data
//	// Methods
//	m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
//	km = m->AddKey("_KSM_5_Constructor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(14);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km = m->AddKey("_KSM_6_EConstructor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(17);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km = m->AddKey("_KSM_7_LoadConstructor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(16);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km = m->AddKey("_KSM_8_ELoadConstructor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(19);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km = m->AddKey("_KSM_9_Destructor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(8);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km = m->AddKey("_KSM_10_EDestructor_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(18);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km = m->AddKey("_KSM_11_Init1_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(21);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km = m->AddKey("_KSM_12_Init2_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(22);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km = m->AddKey("_KSM_13_MapSetActivCell_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(23);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	pr = km->AddKey("Params",FSRTregister)->GetSubRegister();
//		kpr = pr->AddKey("1",FSRTregister)->GetSubRegister();
//		kpr->AddKey("Name",FSRTint)->writei(25);
//		kpr->AddKey("Type",FSRTint)->writei(eKTint);
//		kpr = pr->AddKey("1",FSRTregister)->GetSubRegister();
//		kpr->AddKey("Name",FSRTint)->writei(26);
//		kpr->AddKey("Type",FSRTint)->writei(eKTint);
//		kpr = pr->AddKey("1",FSRTregister)->GetSubRegister();
//		kpr->AddKey("Name",FSRTint)->writei(27);
//		kpr->AddKey("Type",FSRTint)->writei(eKTint);
//	km = m->AddKey("_KSM_14_MapGetNextObject_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(24);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
//	km->AddKey("ReturnType",FSRTint)->writei(eKTobject);
//
//	ko = o->AddKey("_KSID_hlina_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	ko->AddKey("Name",FSRTint)->writei(2);
//	ko->AddKey("Edit Tag",FSRTint)->writei(0);
////	ko->AddKey("KSVG",FSRTstring)->stringwrite("");
//	// Zadat tagy pro editor
////	m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
//
//	ko = o->AddKey("_KSID_klic_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	ko->AddKey("Name",FSRTint)->writei(3);
//	ko->AddKey("Edit Tag",FSRTint)->writei(0);
////	ko->AddKey("KSVG",FSRTstring)->stringwrite("");
//	// Zadat tagy pro editor
////	m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
//
//	ko = o->AddKey("_KSID_podlaha_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	ko->AddKey("Name",FSRTint)->writei(4);
//	ko->AddKey("Edit Tag",FSRTint)->writei(0);
////	ko->AddKey("KSVG",FSRTstring)->stringwrite("");
//	// Zadat tagy pro editor
////	m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
//
//	ko = o->AddKey("_KSID_led_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	ko->AddKey("Name",FSRTint)->writei(5);
//	ko->AddKey("Edit Tag",FSRTint)->writei(0);
////	ko->AddKey("KSVG",FSRTstring)->stringwrite("");
//	// Zadat tagy pro editor
////	m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
//
//	ko = o->AddKey("_KSID_voda_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	ko->AddKey("Name",FSRTint)->writei(6);
//	ko->AddKey("Edit Tag",FSRTint)->writei(0);
////	ko->AddKey("KSVG",FSRTstring)->stringwrite("");
//	// Zadat tagy pro editor
//	m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
//	km = m->AddKey("_KSM_0_znic_se_ve_vybuchu_0001_FFFF_0001_0001",FSRTregister)->GetSubRegister();
//	km->AddKey("Name",FSRTint)->writei(10);
////	km->AddKey("Jump",FSRTint)->writei(0);
//	km->AddKey("Compiled",FSRTchar)->writec(1); // 0 - IS, 1 - KS
//	km->AddKey("Safe",FSRTchar)->writec(1); // 0 - Direct, 1 - safe
////	pr = km->AddKey("Params",FSRTregister)->GetSubRegister();
//
//	o = r->AddKey("Globals",FSRTregister)->GetSubRegister();
//	o->AddKey("_KSG_TestInt_0001_FFFF_0001_0001",FSRTint)->writei(eKTint); // 0
//	o->AddKey("_KSG_Mapa_0001_FFFF_0001_0001",FSRTint)->writei(eKTobject); // 1
//	p = r->AddKey("Static Objects",FSRTint); // 2 inty na objekt - cislo glob. prom. a typ objektu
//	p->writei(1); p->writei(20);  // objekt mapa
//	
//
//	r->WriteFile();
//	delete r;
//

	#endif
}



void MDTestKernel() {

	KerMain->Errors->ShowRunLogWindow(5,0,335,640,120);
//	KerMain->NewObject(0,KerMain->KerNamesMain->GetNamePointer("_KSID_bomba_0001_FFFF_0001_0001"));

}











