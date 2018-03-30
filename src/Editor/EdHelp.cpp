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

	"1 Tutorial for levels editor",

	"3 We load level base",
	"I",
	"* On the main game screen highlight 'base'.",
	"* Press F2 (or Edit Level) and this will open the level in editor.",
	"  Base is a suitable starting level, has several predefined items, which will save us work.",

	"3 Editor - first introduction",
	"I",
	"#1Map - here is where you place visible objects",
	"# List of objects",
	"# Selected object and its properties",
	"# Objects outside of map (invisible)",
	"# Editor Mode",
	"# Menu",

	"3 Let's get on with it",
	"* On the list of objects (2) find Dragon (in the Creatures section) and highlight him.",
	"* Place it on the map using the left mouse button (1).",
	"* In the sub section Floors highlight Exit.",
	"* Place it on the map.",
	"* You can try other objects as well ...",

	"3 Save as",
	"  Now it is necessary to save the level under new name (we don't want to overwrite the level 'base').",
	"* Press Ctrl+Shift+S (or on the Start menu select 'Save as').",
	"* Enter a new name and confirm it by pressing OK.",

	"3 Let's try it",
	"* It is necessary to have it saved; the following operation forgets unsaved changes.",
	"* Press Shift+F2 and that will switch the editor into game mode:",
	"*1You can play",
	"*1And edit at the same time",
	"*1Cannot be saved",
	"* Press F2 to restart (again in game mode).",
	"* Press Shift+F2 to restart and return to the standard editor.",

	"2 Editor controls",

	"3 Select Mode",
	"* Activation: by pressing 'S'",
	"* Used to select already placed objects (Left button).",
	"* Live objects properties have a green title. You can change these and the change will appear on the object.",
	"* You can also easily delete the selected object by using the Delete key.",
	"* Editor is unable to perform bulk highlighting or bulk property editing of multiple objects.",

	"3 Insert Mode",
	"* It is used to copy prototype into the map.",
	"* Activation (obtaining a prototype):",
	"*1By selection from the list of objects.",
	"*1Copying properties of an already existing object: Highlight an object (Select Mode) and press 'I'.",
	"* Prototype is an auxiliary object, which we can configure as needed. It has a yellow title.",
	"* We place new objects using left mouse button.",
	"* Objects will inherit prototype properties. (By placing we create prototype’s copies.)",

	"3 Scrolling the map",
	"* I recommend not using scrollbars.",
	"* If the cursor is over the map and you are pressing the space bar, you can move the map by mouse movements.",
	"* Map will move, when your mouse pointer reaches the screen's edge.",

	"3 Editor Modes",
	"  Main action of each mode is performed using the left mouse. Right button deletes.",

	"L Select - S",
	"L Select all on a cell - B",
	"*1Suitable when objects overlap and common select is not enough.",
	"*1After selecting a cell the list of all of its objects will appear.",
	"*1Esc closes the list.",
	"L Insert - I",
	"*1I - generates a prototype by copying the selected object.",
	"*1F - same as I, but we place on coordinates not aligned with cell.",
	"*1C - same as I, but we place on coordinates aligned with cell.",
	"*1Activation also by selecting an object from the list of objects.",
	"*1Shift while placing - places 3x3.",
	"*1Ctrl while placing - deletes colliding objects.",
	"L Delete - D",
	"*1Deletes objects",
	"*1While holding the left button, objects will be deleted only at the same level as the first deleted object (e.g. only floors).",
	"*1Shift - deletes 3x3 area.",
	"*1Ctrl - deletes only objects, which are of the same type as the first deleted.",
	"L NoConnect mode - N",
	"*1Some objects have graphics which interconnect with themselves. E.g. walls.",
	"*1You can control graphics connections by highlighting object in this mode. Mostly you want to restrict the connections in certain directions.",
	"L Move mode - M",
	"*1Serves primarily to move the selected object to another location.",
	"*1You may also move the object outside of the map or vice versa. (Outside of the map the object keeps the same coordinates, which it had on the map.)",
	"*1Ctrl - deletes colliding objects.",
	"L Selection for an editable variable.",
	"*1Editing some object variables occurs through \"selection\". This can be selection from list, selection of object type, selection of specific live object, selection of map coordinates ...",
	"*1Esc cancels selection mode.",

	"2 Editing Object Properties",
	"I",
	"  Object can have a number of editable variables, which then impact its behavior in the game. Most of the variables include help, which will display after keeping the cursor over the variable. Help describes significance of the variable and rules for its editing.",
	"  Selection variables are edited by clicking the button on the right of the editing field.",

	"2 Important global settings",
	"3 Map size",
	"* On the list of global objects select the Map object.",
	"* Set map corner coordinates, these are in pixels. (One cell has 40 x 40 pixels.)",
	"* Click the refresh button. Attention: if you are downsizing the map, then objects outside will be deleted.",
	"3 Global light",
	"* Select the Global light object on the list of global objects.",
	"* You can change the light's color and intensity.",
	"* Click on the \"Changes lighting\" button.",
	"3 Music",
	"* Open the Package browser using the icon at the bottom bar.",
	"* Find the music packs: \"music.pkg\" and \"music2.pkg\".",
	"* Inside select a song, open its context menu and select \"Set as Music to the Level\".",
	"3 Setting level author and properties",
	"* On the start menu select the item \"Level Properties\".",
	"3 Intro text editing",
	"* Is edited by an external text editor.",
	"* For a level it is necessary to create a LevelIntro.xml file.",
	"* Specific editing rules are above the scope of this text. I recommend copying LevelIntro.xml from another level.",

	"2 Learn more",
	"  More detailed information can be found in external documentation, which is part of the game.",

	"Konec",
};


char* pictureNames[] = {

	"$TEX$\\editor\\BaseSelectS.jpg",
	"$TEX$\\editor\\EditorOknaS2.png",
	"$TEX$\\editor\\EditorMods\\select_free.png",
	"$TEX$\\editor\\EditorMods\\select_cell.png",
	"$TEX$\\editor\\EditorMods\\insertMod.png",
	"$TEX$\\cursors\\editor\\delete.png",
	"$TEX$\\cursors\\editor\\noConnect.png",
	"$TEX$\\editor\\EditorMods\\moveObj.png",
	"$TEX$\\editor\\ChooseCursors.png",
	"$TEX$\\editor\\ObjPropertiesS.png",
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
