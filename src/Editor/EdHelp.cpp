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

	"1 Tutor�l k editoru level�",
	
	"3 Nahrajeme level base",
	"I",
	"* Na hlavn� obrazovce hry ozna� 'base'.",
	"* Zm��kni F2 (�i Edit Level) pro otev�en� levelu v editoru.",
	"  Base je vhodn� v�choz� level, m� n�kolik p�ednastaven�ch v�c�, kter� n�m u�et�� pr�ci.",
	
	"3 Editor - prvn� sezn�men�",
	"I",
	"#1Mapa - sem se um�s�uj� viditeln� objekty",
	"# Seznam objekt�",
	"# Vybran� objekt a jeho vlastnosti",
	"# Objekty mimo mapu (neviditeln�)",
	"# Re�imy editoru",
	"# Menu",
	
	"3 Jdeme na to",
	"* V seznamu objekt� (2) najdi Dr��ka (v sekci Besti��) a ozna� ho.",
	"* Lev�m tla��tkem ho um�sti do mapy (1).",
	"* V sekci Podlahy ozna� Exit.",
	"* Um�sti do mapy.",
	"* M��e� vyzkou�et i jin� objekty ...",

	"3 Ulo��me jako",
	"  Nyn� je pot�eba level ulo�it pod nov�m jm�nem (nechceme p�epsat level 'base').",
	"* Zma�kni Ctrl + Shift + S (nebo v menu Start zvol 'Ulo� jako').",
	"* Zadej nov� jm�no a potvr� tla��tkem OK.",

	"3 Vyzkou��me to",
	"* Je nutno m�t ulo�eno, n�sleduj�c� operace zapom�n� neulo�en� zm�ny.",
	"* Zm��kni Shift + F2 pro p�epnut� editoru do hern�ho re�imu:",
	"*1M��e� hr�t",
	"*1A editovat z�rove�",
	"*1Nejde ukl�dat",
	"* Zm��kni F2 pro restart (op�t v hern�m re�imu).",
	"* Zm��kni Shift + F2 pro restart a n�vrat do norm�ln�ho editoru.",

	"2 Ovl�d�n� Editoru",

	"3 Select Mod",
	"* Aktivace: stiskem 'S'",
	"* Slou�� k v�b�ru  u� um�st�n�ch objekt�. (Lev�m tla��tkem).",
	"* Vlastnosti �iv�ch objekt� maj� zelen� titulek. M��e� je m�nit a zm�na se projev� na objektu.",
	"* Vybran� objekt m��e� i smazat kl�vesou Delete.",
	"* Editor neum� hromadn� ozna�ov�n� a hromadnou editaci vlastnost� u v�ce objekt�.",

	"3 Insert Mod",
	"* Slou�� k um�s�ov�n� kopi� prototypu do mapy.",
	"* Aktivace (z�sk�n� prototypu):",
	"*1V�b�rem objektu ze seznamu objekt�.",
	"*1Okop�rov�n�m vlastnost� u� existuj�c�ho objektu: Ozna� objekt (Select Mod) a zm��kni 'I'.",
	"* Prototyp je pomocn� objekt, kter� si m��eme nakonfigurovat podle pot�eby. M� �lut� titulek.",
	"* Nov� objekty um�s�ujeme lev�m tla��tkem my�i.",
	"* Objekty zd�d� vlastnosti prototypu. (Vzniknou jako kopie)",

	"3 Scrolling mapy",
	"* Doporu�uju nepou��vat scrollbary.",
	"* Pokud je kurzor nad mapou a dr�� mezern�k, m��e� posouvat mapu pohybem my�i.",
	"* Mapa se posune, kdy� dojede� my�� k okraji obrazovky.",

	"3 Re�imy editoru",
	"  Hlavn� akce ka�d�ho re�imu se prov�d� lev�m tla��tkem my�i. Prav� tla��tko ma�e.",

	"L Select - S",
	"L Select v�eho na bu�ce - B",
	"*1Vhodn�, kdy� se objekty p�ekr�vaj� a b�n� select mi nesta��.",
	"*1Po vybr�n� bu�ky se zobraz� seznam v�ech jej�ch objekt�.",
	"*1Esc seznam zav�e.",
	"L Insert - I",
	"*1I - vytvo�� prototyp okop�rov�n�m vybran�ho objektu.",
	"*1F - jako I, ale um�s�ujeme na sou�adnice nezarovnan� s bu�kou.",
	"*1C - jako I, ale um�s�ujeme na sou�adnice zarovnan� s bu�kou.",
	"*1Aktivace tak� vybr�n�m objektu se seznamu objekt�.",
	"*1Shift p�i um�s�ov�n� - um�s�uje 3x3.",
	"*1Ctrl p�i um�s�ov�n� - sma�e koliduj�c� objekty.",
	"L Delete - D",
	"*1Ma�e objekty",
	"*1Dokud dr�� lev� tla��tko, ma�ou se objekty jen ve stejn� hladin� jako byl prvn� smazan� objekt (nap�.jen podlahy).",
	"*1Shift - ma�e 3x3 oblast.",
	"*1Ctrl - ma�e jen objekty, kter� jsou stejn�ho typu jako prvn� smazan�.",
	"L NoConnect mod - N",
	"*1N�kter� objekty p�i um�s�ov�n� samy na sebe graficky navazuj�. Nap�.st�ny.",
	"*1Ozna�en�m objektu v tomto re�imu m��e� navazovan� nastavovat. V�t�inou chce� zak�zat nav�z�n� do n�kter�ch sm�r�.",
	"L Move mod - M",
	"*1Slou�� k p�esunut� vybran�ho objektu na jin� m�sto.",
	"*1M��e� p�esunout objekt i mimo mapu �i naopak. (Mimo mapu si objekt zachov�v� takov� sou�adnice, jak� m�l v map�.)",
	"*1Ctrl - sma�e koliduj�c� objekty.",
	"L V�b�r pro editovanou prom�nnou.",
	"*1Editace n�kter�ch prom�nn�ch u objektu se d�je \"v�b�rem\". M��e j�t o v�b�r ze seznamu, v�b�r typu objektu, v�b�r konkr�tn�ho �iv�ho objektu, v�b�r sou�adnic na map�, ...",
	"*1Esc zru�� re�im v�b�ru.",

	"2 Editace Vlastnost� Objekt�",
	"I",
	"  Objekt m��e m�t celou �adu nastaviteln�ch prom�nn�ch, kter� pak ovliv�uj� jeho chov�n� ve h�e. V�t�ina prom�nn�ch m� na sob� n�pov�du, kter� se zobraz� po podr�en� kurzoru nad prom�nnou. N�pov�da popisuje v�znam prom�nn� i pravidla pro jej� editaci.",
	"  V�b�rov� prom�nn� se edituj� zm��knut�m tla��tka vpravo od edita�n�ho pole.",

	"2 D�le�it� glob�ln� nastaven�",
	"3 Velikost mapy",
	"* V seznamu glob�ln�ch objekt� vyber objekt Mapa.",
	"* Nastav sou�adnice roh� mapy, jsou v pixelech. (Jedna bu�ka m� 40 x 40 pixel�.)",
	"* Zm��kni refresh button. Pozor: pokud mapu zmen�uje�, objekty mimo budou smaz�ny.",
	"3 Glob�ln� sv�tlo",
	"* V seznamu glob�ln�ch objekt� vyber objekt Glob�ln� sv�tlo.",
	"* M��e� zm�nit barvu a intenzitu sv�tla.",
	"* Klikni na button \"Zmen Nasviceni\"",
	"3 Hudba",
	"* Otev�i Package browser ikonou na spodn� li�t�.",
	"* Najdi bal�ky s hudbou: \"music.pkg\" a \"music2.pkg\".",
	"* Uvnit� vyber skladbu a u n� v kontextov�m menu zvol \"nastav jako hudbu k levelu\"",
	"3 Nastaven� Autora a vlastnost� levelu",
	"* V menu start zvol polo�ku \"Vlastnosti levelu\"",
	"3 Editace �vodn�ho textu",
	"* Edituje se extern�m textov�m editorem.",
	"* U levelu je pot�eba vytvo�it soubor LevelIntro.xml",
	"* P�esn� pravidla editace jsou nad r�mec tohoto textu. Doporu�uju napodobit LevelIntro.xml z jin�ho levelu.",

	"2 Podrobn�ji",
	"  Obs�hlej�� informace najde� v extern� dokumentaci, kter� je p�ilo�ena u hry.",

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
	"* Objects will inherit prototype properties. (By placing we create prototype�s copies.)",

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
	CGUIStdWindow *cw = new CGUIStdWindow(xx, 0, screenx, screeny, "en{{Editor Help}}cs{{N�pov�da}}", new CGUIRectHost(0, 0, styleSet->Get("LM_HelpIcon")->GetTexture(0)), true, STD_RESIZER_COLOR, 1, 0xDD202070);
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
