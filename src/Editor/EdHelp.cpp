///////////////////////////////////////////////
//
// Edhelp.h
//
// Editor Help
//
// A: MD
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "EdHelp.h"
#include "gui.h"
#include "window.h"
#include "widgets.h"
#include "refmgr.h"
#include "krkal.h"
#include "primitives.h"


char* edhelptextcs[] = {

	"1 Tutorál k editoru levelù",
	
	"3 Nahrajeme level base",
	"I",
	"* Na hlavní obrazovce hry oznaè 'base'.",
	"* Zmáèkni F2 (èi Edit Level) pro otevøení levelu v editoru.",
	"  Base je vhodný výchozí level, má nìkolik pøednastavených vìcí, které nám ušetøí práci.",
	
	"3 Editor - první seznámení",
	"I",
	"#1Mapa - sem se umísují viditelné objekty",
	"# Seznam objektù",
	"# Vybraný objekt a jeho vlastnosti",
	"# Objekty mimo mapu (neviditelné)",
	"# Režimy editoru",
	"# Menu",
	
	"3 Jdeme na to",
	"* V seznamu objektù (2) najdi Dráèka (v sekci Bestiáø) a oznaè ho.",
	"* Levým tlaèítkem ho umísti do mapy (1).",
	"* V sekci Podlahy oznaè Exit.",
	"* Umísti do mapy.",
	"* Mùžeš vyzkoušet i jiné objekty ...",

	"3 Uložíme jako",
	"  Nyní je potøeba level uložit pod novým jménem (nechceme pøepsat level 'base').",
	"* Zmaèkni Ctrl + Shift + S (nebo v menu Start zvol 'Ulož jako').",
	"* Zadej nové jméno a potvrï tlaèítkem OK.",

	"3 Vyzkoušíme to",
	"* Je nutno mít uloženo, následující operace zapomíná neuložené zmìny.",
	"* Zmáèkni Shift + F2 pro pøepnutí editoru do herního režimu:",
	"*1Mùžeš hrát",
	"*1A editovat zároveò",
	"*1Nejde ukládat",
	"* Zmáèkni F2 pro restart (opìt v herním režimu).",
	"* Zmáèkni Shift + F2 pro restart a návrat do normálního editoru.",

	"2 Ovládání Editoru",

	"3 Select Mod",
	"* Aktivace: stiskem 'S'",
	"* Slouží k výbìru  už umístìných objektù. (Levým tlaèítkem).",
	"* Vlastnosti živých objektù mají zelený titulek. Mùžeš je mìnit a zmìna se projeví na objektu.",
	"* Vybraný objekt mùžeš i smazat klávesou Delete.",
	"* Editor neumí hromadné oznaèování a hromadnou editaci vlastností u více objektù.",

	"3 Insert Mod",
	"* Slouží k umísování kopií prototypu do mapy.",
	"* Aktivace (získání prototypu):",
	"*1Výbìrem objektu ze seznamu objektù.",
	"*1Okopírováním vlastností už existujícího objektu: Oznaè objekt (Select Mod) a zmáèkni 'I'.",
	"* Prototyp je pomocný objekt, který si mùžeme nakonfigurovat podle potøeby. Má žlutý titulek.",
	"* Nové objekty umísujeme levým tlaèítkem myši.",
	"* Objekty zdìdí vlastnosti prototypu. (Vzniknou jako kopie)",

	"3 Scrolling mapy",
	"* Doporuèuju nepoužívat scrollbary.",
	"* Pokud je kurzor nad mapou a držíš mezerník, mùžeš posouvat mapu pohybem myši.",
	"* Mapa se posune, když dojedeš myší k okraji obrazovky.",

	"3 Režimy editoru",
	"  Hlavní akce každého režimu se provádí levým tlaèítkem myši. Pravé tlaèítko maže.",

	"L Select - S",
	"L Select všeho na buòce - B",
	"*1Vhodné, když se objekty pøekrývají a bìžný select mi nestaèí.",
	"*1Po vybrání buòky se zobrazí seznam všech jejích objektù.",
	"*1Esc seznam zavøe.",
	"L Insert - I",
	"*1I - vytvoøí prototyp okopírováním vybraného objektu.",
	"*1F - jako I, ale umísujeme na souøadnice nezarovnané s buòkou.",
	"*1C - jako I, ale umísujeme na souøadnice zarovnané s buòkou.",
	"*1Aktivace také vybráním objektu se seznamu objektù.",
	"*1Shift pøi umísování - umísuje 3x3.",
	"*1Ctrl pøi umísování - smaže kolidující objekty.",
	"L Delete - D",
	"*1Maže objekty",
	"*1Dokud držíš levé tlaèítko, mažou se objekty jen ve stejné hladinì jako byl první smazaný objekt (napø.jen podlahy).",
	"*1Shift - maže 3x3 oblast.",
	"*1Ctrl - maže jen objekty, které jsou stejného typu jako první smazaný.",
	"L NoConnect mod - N",
	"*1Nìkteré objekty pøi umísování samy na sebe graficky navazují. Napø.stìny.",
	"*1Oznaèením objektu v tomto režimu mùžeš navazovaní nastavovat. Vìtšinou chceš zakázat navázání do nìkterých smìrù.",
	"L Move mod - M",
	"*1Slouží k pøesunutí vybraného objektu na jiné místo.",
	"*1Mùžeš pøesunout objekt i mimo mapu èi naopak. (Mimo mapu si objekt zachovává takové souøadnice, jaké mìl v mapì.)",
	"*1Ctrl - smaže kolidující objekty.",
	"L Výbìr pro editovanou promìnnou.",
	"*1Editace nìkterých promìnných u objektu se dìje \"výbìrem\". Mùže jít o výbìr ze seznamu, výbìr typu objektu, výbìr konkrétního živého objektu, výbìr souøadnic na mapì, ...",
	"*1Esc zruší režim výbìru.",

	"2 Editace Vlastností Objektù",
	"I",
	"  Objekt mùže mít celou øadu nastavitelných promìnných, které pak ovlivòují jeho chování ve høe. Vìtšina promìnných má na sobì nápovìdu, která se zobrazí po podržení kurzoru nad promìnnou. Nápovìda popisuje význam promìnné i pravidla pro její editaci.",
	"  Výbìrové promìnné se editují zmáèknutím tlaèítka vpravo od editaèního pole.",

	"2 Dùležitá globální nastavení",
	"3 Velikost mapy",
	"* V seznamu globálních objektù vyber objekt Mapa.",
	"* Nastav souøadnice rohù mapy, jsou v pixelech. (Jedna buòka má 40 x 40 pixelù.)",
	"* Zmáèkni refresh button. Pozor: pokud mapu zmenšuješ, objekty mimo budou smazány.",
	"3 Globální svìtlo",
	"* V seznamu globálních objektù vyber objekt Globální svìtlo.",
	"* Mùžeš zmìnit barvu a intenzitu svìtla.",
	"* Klikni na button \"Zmen Nasviceni\"",
	"3 Hudba",
	"* Otevøi Package browser ikonou na spodní lištì.",
	"* Najdi balíky s hudbou: \"music.pkg\" a \"music2.pkg\".",
	"* Uvnitø vyber skladbu a u ní v kontextovém menu zvol \"nastav jako hudbu k levelu\"",
	"3 Nastavení Autora a vlastností levelu",
	"* V menu start zvol položku \"Vlastnosti levelu\"",
	"3 Editace úvodního textu",
	"* Edituje se externím textovým editorem.",
	"* U levelu je potøeba vytvoøit soubor LevelIntro.xml",
	"* Pøesná pravidla editace jsou nad rámec tohoto textu. Doporuèuju napodobit LevelIntro.xml z jiného levelu.",

	"2 Podrobnìji",
	"  Obsáhlejší informace najdeš v externí dokumentaci, která je pøiložena u hry.",

	"Konec",
};

char* edhelptexten[] = {

	"1 Sorry. Not yet translated.",
	"1 Tutorál k editoru levelù",

	"3 Nahrajeme level base",
	"I",
	"* Na hlavní obrazovce hry oznaè 'base'.",
	"* Zmáèkni F2 (èi Edit Level) pro otevøení levelu v editoru.",
	"  Base je vhodný výchozí level, má nìkolik pøednastavených vìcí, které nám ušetøí práci.",

	"3 Editor - první seznámení",
	"I",
	"#1Mapa - sem se umísují viditelné objekty",
	"# Seznam objektù",
	"# Vybraný objekt a jeho vlastnosti",
	"# Objekty mimo mapu (neviditelné)",
	"# Režimy editoru",
	"# Menu",

	"3 Jdeme na to",
	"* V seznamu objektù (2) najdi Dráèka (v sekci Bestiáø) a oznaè ho.",
	"* Levým tlaèítkem ho umísti do mapy (1).",
	"* V sekci Podlahy oznaè Exit.",
	"* Umísti do mapy.",
	"* Mùžeš vyzkoušet i jiné objekty ...",

	"3 Uložíme jako",
	"  Nyní je potøeba level uložit pod novým jménem (nechceme pøepsat level 'base').",
	"* Zmaèkni Ctrl + Shift + S (nebo v menu Start zvol 'Ulož jako').",
	"* Zadej nové jméno a potvrï tlaèítkem OK.",

	"3 Vyzkoušíme to",
	"* Je nutno mít uloženo, následující operace zapomíná neuložené zmìny.",
	"* Zmáèkni Shift + F2 pro pøepnutí editoru do herního režimu:",
	"*1Mùžeš hrát",
	"*1A editovat zároveò",
	"*1Nejde ukládat",
	"* Zmáèkni F2 pro restart (opìt v herním režimu).",
	"* Zmáèkni Shift + F2 pro restart a návrat do normálního editoru.",

	"2 Ovládání Editoru",

	"3 Select Mod",
	"* Aktivace: stiskem 'S'",
	"* Slouží k výbìru  už umístìných objektù. (Levým tlaèítkem).",
	"* Vlastnosti živých objektù mají zelený titulek. Mùžeš je mìnit a zmìna se projeví na objektu.",
	"* Vybraný objekt mùžeš i smazat klávesou Delete.",
	"* Editor neumí hromadné oznaèování a hromadnou editaci vlastností u více objektù.",

	"3 Insert Mod",
	"* Slouží k umísování kopií prototypu do mapy.",
	"* Aktivace (získání prototypu):",
	"*1Výbìrem objektu ze seznamu objektù.",
	"*1Okopírováním vlastností už existujícího objektu: Oznaè objekt (Select Mod) a zmáèkni 'I'.",
	"* Prototyp je pomocný objekt, který si mùžeme nakonfigurovat podle potøeby. Má žlutý titulek.",
	"* Nové objekty umísujeme levým tlaèítkem myši.",
	"* Objekty zdìdí vlastnosti prototypu. (Vzniknou jako kopie)",

	"3 Scrolling mapy",
	"* Doporuèuju nepoužívat scrollbary.",
	"* Pokud je kurzor nad mapou a držíš mezerník, mùžeš posouvat mapu pohybem myši.",
	"* Mapa se posune, když dojedeš myší k okraji obrazovky.",

	"3 Režimy editoru",
	"  Hlavní akce každého režimu se provádí levým tlaèítkem myši. Pravé tlaèítko maže.",

	"L Select - S",
	"L Select všeho na buòce - B",
	"*1Vhodné, když se objekty pøekrývají a bìžný select mi nestaèí.",
	"*1Po vybrání buòky se zobrazí seznam všech jejích objektù.",
	"*1Esc seznam zavøe.",
	"L Insert - I",
	"*1I - vytvoøí prototyp okopírováním vybraného objektu.",
	"*1F - jako I, ale umísujeme na souøadnice nezarovnané s buòkou.",
	"*1C - jako I, ale umísujeme na souøadnice zarovnané s buòkou.",
	"*1Aktivace také vybráním objektu se seznamu objektù.",
	"*1Shift pøi umísování - umísuje 3x3.",
	"*1Ctrl pøi umísování - smaže kolidující objekty.",
	"L Delete - D",
	"*1Maže objekty",
	"*1Dokud držíš levé tlaèítko, mažou se objekty jen ve stejné hladinì jako byl první smazaný objekt (napø.jen podlahy).",
	"*1Shift - maže 3x3 oblast.",
	"*1Ctrl - maže jen objekty, které jsou stejného typu jako první smazaný.",
	"L NoConnect mod - N",
	"*1Nìkteré objekty pøi umísování samy na sebe graficky navazují. Napø.stìny.",
	"*1Oznaèením objektu v tomto režimu mùžeš navazovaní nastavovat. Vìtšinou chceš zakázat navázání do nìkterých smìrù.",
	"L Move mod - M",
	"*1Slouží k pøesunutí vybraného objektu na jiné místo.",
	"*1Mùžeš pøesunout objekt i mimo mapu èi naopak. (Mimo mapu si objekt zachovává takové souøadnice, jaké mìl v mapì.)",
	"*1Ctrl - smaže kolidující objekty.",
	"L Výbìr pro editovanou promìnnou.",
	"*1Editace nìkterých promìnných u objektu se dìje \"výbìrem\". Mùže jít o výbìr ze seznamu, výbìr typu objektu, výbìr konkrétního živého objektu, výbìr souøadnic na mapì, ...",
	"*1Esc zruší režim výbìru.",

	"2 Editace Vlastností Objektù",
	"I",
	"  Objekt mùže mít celou øadu nastavitelných promìnných, které pak ovlivòují jeho chování ve høe. Vìtšina promìnných má na sobì nápovìdu, která se zobrazí po podržení kurzoru nad promìnnou. Nápovìda popisuje význam promìnné i pravidla pro její editaci.",
	"  Výbìrové promìnné se editují zmáèknutím tlaèítka vpravo od editaèního pole.",

	"2 Dùležitá globální nastavení",
	"3 Velikost mapy",
	"* V seznamu globálních objektù vyber objekt Mapa.",
	"* Nastav souøadnice rohù mapy, jsou v pixelech. (Jedna buòka má 40 x 40 pixelù.)",
	"* Zmáèkni refresh button. Pozor: pokud mapu zmenšuješ, objekty mimo budou smazány.",
	"3 Globální svìtlo",
	"* V seznamu globálních objektù vyber objekt Globální svìtlo.",
	"* Mùžeš zmìnit barvu a intenzitu svìtla.",
	"* Klikni na button \"Zmen Nasviceni\"",
	"3 Hudba",
	"* Otevøi Package browser ikonou na spodní lištì.",
	"* Najdi balíky s hudbou: \"music.pkg\" a \"music2.pkg\".",
	"* Uvnitø vyber skladbu a u ní v kontextovém menu zvol \"nastav jako hudbu k levelu\"",
	"3 Nastavení Autora a vlastností levelu",
	"* V menu start zvol položku \"Vlastnosti levelu\"",
	"3 Editace úvodního textu",
	"* Edituje se externím textovým editorem.",
	"* U levelu je potøeba vytvoøit soubor LevelIntro.xml",
	"* Pøesná pravidla editace jsou nad rámec tohoto textu. Doporuèuju napodobit LevelIntro.xml z jiného levelu.",

	"2 Podrobnìji",
	"  Obsáhlejší informace najdeš v externí dokumentaci, která je pøiložena u hry.",

	"Konec",
};


char* pictureNames[] = {

	"$DATA$\\BaseSelectS.jpg",
	"$DATA$\\EditorOknaS2.png",
	"$TEX$\\editor\\EditorMods\\select_free.png",
	"$TEX$\\editor\\EditorMods\\select_cell.png",
	"$TEX$\\editor\\EditorMods\\insertMod.png",
	"$TEX$\\cursors\\editor\\delete.png",
	"$TEX$\\cursors\\editor\\noConnect.png",
	"$TEX$\\editor\\EditorMods\\moveObj.png",
	"$DATA$\\ChooseCursors.png",
	"$DATA$\\ObjPropertiesS.png",
};


typeID CEditorHelp::ShowHelp()
{
	char** helptext;
	char *text;
	if (KRKAL->cfg.lang == langEN) helptext = edhelptexten;
	else helptext = edhelptextcs;

	float screenx, screeny;
	desktop->GetVPSize(screenx, screeny);
	float xx = 0;
	if (screenx > 600) {
		xx = floor((screenx - 600) / 2);
		screenx = 600;
	}
	CGUIStdWindow *cw = new CGUIStdWindow(xx, 0, screenx, screeny, "en{{Editor Help}}cs{{Nápovìda}}", new CGUIRectHost(0, 0, styleSet->Get("LM_HelpIcon")->GetTexture(0)), true, STD_RESIZER_COLOR, 1, 0xDD202070);
	cw->title->SetButtons(true, true, true);
	desktop->AddBackElem(cw);
	cw->FocusMe();
	cw->excapeMinimize = true;

	CGUIStaticText *st;
	CGUIStaticPicture* picture;

	float mx = (float)screenx - 30;
	xx = 10;
	float yy = 0;
	float sxx, syy, syy2;
	int ti = 0;
	int pi = 0;
	UI tc = 0xFFFFFFFF;
	UI nc = 0xFFAFFF6D;
	UI lc = 0xFF8E7533;
	int listCounter = 0;
	int offset;
	char buffer[255];


	CGUIFont *f = (CGUIFont*)RefMgr->Find("GUI.F.Verdana.10");
	CGUIFont *fb = (CGUIFont*)RefMgr->Find("GUI.F.Verdana.10.B");
	CGUIFont *fn3 = (CGUIFont*)RefMgr->Find("GUI.F.Courier.12.B");
	CGUIFont *fn2 = (CGUIFont*)RefMgr->Find("GUI.F.Courier.14.B");
	CGUIFont *fn1 = (CGUIFont*)RefMgr->Find("GUI.F.Courier.18.B");

	bool doWork = true;
	while (doWork) {
		text = helptext[ti++];
		switch (text[0]) {
			case 'K':
				doWork = false;
				break;
			case '1':
				yy += 25;
				st = new CGUIStaticText(text + 2, fn1, xx, yy, nc, mx);
				st->GetSize(sxx, syy); yy += syy + 5; cw->AddBackElem(st);
				break;
			case '2':
				yy += 25;
				st = new CGUIStaticText(text + 2, fn2, xx, yy, nc, mx);
				st->GetSize(sxx, syy); yy += syy + 5; cw->AddBackElem(st);
				break;
			case '3':
				yy += 15;
				st = new CGUIStaticText(text + 2, fn3, xx, yy, nc, mx);
				st->GetSize(sxx, syy); yy += syy + 3; cw->AddBackElem(st);
				break;
			case 'I':
				yy += 5;
				picture = new CGUIStaticPicture(xx + 20, yy, pictureNames[pi++]);
				picture->GetSize(sxx, syy); yy += syy + 5; cw->AddBackElem(picture);
				break;
			case ' ':
				yy += 5;
				st = new CGUIStaticText(text + 2, f, xx, yy, tc, mx);
				st->GetSize(sxx, syy); yy += syy; cw->AddBackElem(st);
				break;
			case '*':
				offset = 0;
				if (text[1] >= '0' && text[1] <= '9')
					offset = text[1] - '0';
				picture = new CGUIStaticPicture(xx + 20 * offset, yy + 5, styleSet->Get("MenuExtender")->GetTexture(0));
				cw->AddBackElem(picture);
				st = new CGUIStaticText(text + 2, f, xx + 20 + 20 * offset, yy, tc, mx - 20 - 20 * offset);
				st->GetSize(sxx, syy); yy += syy; cw->AddBackElem(st);
				break;
			case '#':
				if (text[1] == '1')
					listCounter = 1;
				sprintf(buffer, "%i.", listCounter);
				listCounter++;
				st = new CGUIStaticText(buffer, fb, xx, yy, lc);
				cw->AddBackElem(st);
				st = new CGUIStaticText(text + 2, f, xx + 20, yy, tc, mx - 20);
				st->GetSize(sxx, syy); yy += syy; cw->AddBackElem(st);
				break;
			case 'L':
				picture = new CGUIStaticPicture(xx, yy, pictureNames[pi++]);
				picture->GetSize(sxx, syy); cw->AddBackElem(picture);
				st = new CGUIStaticText(text + 2, f, xx + sxx + 8, yy + 5, tc, mx - sxx - 8);
				st->GetSize(sxx, syy2); cw->AddBackElem(st);
				syy2 += 5;
				if (syy > syy2)
					yy += syy;
				else
					yy += syy2;
				break;
		}

	}


	yy += 10;
	cw->SetBackWindowNormalSize(mx + 10, yy);

	return cw->GetID();
}
