///////////////////////////////////////////////
//
// MMhelp.cpp
//
// help&credits
//
// A: Petr Altman
//
///////////////////////////////////////////////


#include "stdafx.h"

#include "MainMenuDesktop.h"
#include "MMhelp.h"
#include "gui.h"
#include "window.h"
#include "widgets.h"
#include "refmgr.h"
#include "krkal.h"
#include "input.h"

char* credittextcs[]={

	"AUTOØI",

	"Jan Krèek (MD) md@krkal.org",
	"verze 2.5 a 3.0, kernel, skriptování, vlastní hra KRKAL, levely,... ",
	"Petr Altman (pal)",
	"grafický engine, editor textu, editor skriptù, grafika pro hru, levely, www,...",
	"Jan Poduška (Morgue)",
	"GUI, editor levelù", 
	"Jiøí Margaritov",
	"kompilátor, interpret",
	"Jára Altmann",
	"levely, nápady, návrhy, pøipomínky",
	"Jan Frolík",
	"hudba - co dodat - VY - NI - KA - JÍ - CÍ",
	"Tomy, Dvoøáèek, Pavel P.",
	"levely",

	"INTERNETOVÉ STRÁNKY",

	"www.krkal.org",
    "https://github.com/HonzaMD/Krkal2",


	"LICENCE",

	"Program Krkal 2.5 je volnì ke stažení, mùžete program dál bezplatnì šíøit.",
	"Zdarma dostáváte kompletní plnohodnotnou hru. Program neobsahuje reklamu, mikrotransakce ani žádný nechtìný èi škodlivý kód.",
	"Je možné program používat, modifikovat v nìm vytváøené hry, tvoøit nové levly.",
	"Program používáte na vaše vlastní nebezpeèí. Nepøebíráme zodpovìdnost za jakékoli škody vzniklé v souvislosti s používáním programu.",
	"Bez našeho svolení není možné použít program Krkal jako souèást nìjakého komerènì prodávaného produktu.",
	"Veškerá další autorská práva vyhrazena (c) 2018",

};

char* credittexten[]={

	"AUTHORS",

	"Jan Krcek (MD) md@krkal.org",
	"version 2.5 and 3.0, kernel, scripting, game KRKAL itself, levels, ...",

	"Petr Altman (pal)",
	"graphics engine, text editor, script editor, game graphics, levels, www, ...",

	"Jan Poduska (Morgue)",
	"GUI, level editor",

	"Jiri Margaritov",
	"compiler, interpreter",

	"Jara Altmann",
	"levels, ideas, proposal, comments",

	"Jan Frolik",
	"music - not much to add - EX - QUI - SITE",

    "Tomy, Dvoracek, Pavel P.",
	"levels",

	"KRKAL HOMEPAGE",

	"www.krkal.org",
	"https://github.com/HonzaMD/Krkal2",

	"LICENSE",

	"Program Krkal 2.5 is free to download, and it can be further distributed for free.",
	"You get the complete game free. Program does not contain advertisements, micro-transactions or any unwanted or malicious code.",
	"You may use the program, modify the games created within the system, create new levels.",
	"You are using the program at your own risk. We do not assume any responsibility for any damages that arise relative to the use of the program.",
	"It is not permitted to use Krkal as part of a commercially sold product without our consent.",
	"All other copyrights reserved (c) 2018"

};

char* helptextcs[]={

	"HRA KRKAL: OVLÁDÁNÍ",

	"pohyb postavièky: kurzorové klávesy",
	"Postavièka se vždy pohybuje pouze horizontálním nebo vertikálním smìrem - nikdy ne šikmo. Lze držet i více smìrových kláves najednou, poté se bude postavièka pohybovat jedním ze smìrù, kam mùže jít, pøièemž se bude	preferovat naposledy zmáèknutý smìr. To je neocenitelná vlastnost v rùzných bludištích.",
    "pøepínání mezi postavièkami: $$_KSID_keyTab_0001_FFFF_0001_0001$$",
	"v nìkterých levelech je více postavièek, nìkdy je možné mezi postavièkami pøepínat, jindy se všechny ovládají najednou (když pak napø. stiskne šipku vpravo - všechny postavièky pùjdou doprava)",
	"použití bomby: $$_KSID_keyZ_0001_FFFF_0001_0001$$",
	"použití miny: $$_KSID_keyX_0001_FFFF_0001_0001$$",
	"sebrání znaèky: $$_KSID_keyA_0001_FFFF_0001_0001$$",
	"položení (otoèení) smìrové znaèky: $$_KSID_key1_0001_FFFF_0001_0001$$+klávesa smìru",
	"položení zákazu vjezdu: $$_KSID_key2_0001_FFFF_0001_0001$$",
	"položení šedesátky: $$_KSID_key3_0001_FFFF_0001_0001$$",
	"položení konce šedesátky: $$_KSID_key4_0001_FFFF_0001_0001$$",
	"zpìt do úvodního menu: ESC",

   "Hráè mùže sbírat nìkteré pøedmìty. Od každého pøedmìtu mùže mít maximálnì 12 kusù. Vìtšina pøedmìtù se sebere sama, jakmile hráè vstoupí na políèko s pøedmìtem. U tìchto pøedmìtù není možné rozhodnout se, jestli je chci nebo ne. Výjimkou jsou znaèky, ty se sbírají speciální klávesou '$$_KSID_keyA_0001_FFFF_0001_0001$$'.",
   "Sebrané pøedmìty nelze jen tak zahodit. Nìkteré pøedmìty je možné použít speciální klávesou (viz výše). Jiné pøedmìty se použijí samy - napø. klíè se použije vždy, když hráè chce vstoupit na políèko, kde je zámek. Tady bych chtìl upozornit na jednu záludnost - pokud hráè má klíè a nìjaká síla ho pøinutí vjet (napø. klouže se po ledu) na políèko se zámkem - tak se zámek vždy odemkne, a hráè chce nebo ne.",

   "pøepni do editoru skriptù: F1",
   "pøepni do editoru levelù: F2",
   "pøepni do hlavního herního menu: F3",
   "ukonèi Krkala: F4",
   "pøepínání mezi celoobrazovkovým a okenním režimem: Alt+Enter",

   "KONFIGURACE",

   "Konfigurace se provádí ruèní editací souboru \"krkal.cfg\", je možné napøíklad mìnit rozlišení ve høe, nastavovat hlasitost...",

   "TECHNICKÁ PODPORA, INFORMACE, NÁPOVÌDA",

   "Velké množství informací najdete pøímo v dokumentaci, která je souèástí Krkala (v adresáøi Dokumentace.CS). V dokumentaci najdete popis objektù ve høe Krkal a jak ovládat editor. Další informace naleznete na našich webových stránkách:",
   "      www.krkal.org",
   "Na náš email nám mùžete posílat námìty, levely, žádosti o rady, pøipomínky, hlášení chyb, ...:",
   "      support@krkal.org",

};

char* helptexten[]={

	"GAME KRKAL: CONTROLS",

	"character movement: arrow keys",
	"You can move in four basic directions only – you cannot move diagonally. You may hold more arrow keys at once, in which case you will move in one of the directions available at the moment, the last pressed key is preferred. This is a priceless quality in various mazes.",
	"switching between characters: $$_KSID_keyTab_0001_FFFF_0001_0001$$",
	"in some levels you have multiple characters, sometimes it is possible to switch between the characters, other times they are all controlled at once (when then for example you press right arrow - all characters will move right)",
	"using a bomb: $$_KSID_keyZ_0001_FFFF_0001_0001$$",
	"using a mine: $$_KSID_keyX_0001_FFFF_0001_0001$$",
	"pick up a traffic sign: $$_KSID_keyA_0001_FFFF_0001_0001$$",
	"placing (and/or turning) a directional sign: $$_KSID_key1_0001_FFFF_0001_0001$$+direction key",
	"placing do not enter: $$_KSID_key2_0001_FFFF_0001_0001$$",
	"placing sixty: $$_KSID_key3_0001_FFFF_0001_0001$$",
	"placing end of sixty: $$_KSID_key4_0001_FFFF_0001_0001$$",
	"back to the main menu: Esc",

	"A player character can collect certain items. A maximum of 12 pieces of each item is allowed. Most items get picked up automatically, as soon as the character enters the cell. With these items it is not possible to decide whether you want them or not. An exception are signs, they are collected using a special key '$$_KSID_keyA_0001_FFFF_0001_0001$$'.",
	"The collected items cannot be disposed of without a reason. Some items can be used with a special key (see above). Other items are used automatically - e.g. key is used always, when a player wants to enter a cell which a lock. Here I want to point out one sneaky thing - if a player has a key and some force makes him enter (e.g. sliding on ice) a cell with the lock - then the lock will always unlock, whether the player wishes it or not.",

	"switch to the script editor: F1",
	"switch to the level editor: F2",
	"switch to the game main menu: F3",
	"quit Krkal: F4",
	"switching between full screen and windowed mode: Alt+Enter",

	"CONFIGURATION",

	"Configuration is performed by manually editing the \"krkal.cfg\" file. It is e.g. possible to change the game's resolution, set volume...",

	"TECHNICAL SUPPORT, INFORMATION, HELP",

	"Extensive volume of information is directly in the documentation, which is part of Krkal (in the directory Documentation.EN). The documentation includes description of objects in Krkal and how to control the editor. More documentation is on our website:",
	"      www.krkal.org",
	"You can email us your ideas, levels, inquiries for advice, comments, bug reports, ...:",
	"      support@krkal.org",

};

void CMainMenuHelp::ShowCredits()
{
	char** credittext;
	if(KRKAL->cfg.lang==langEN) credittext=credittexten;
	else credittext=credittextcs;

	int screenx,screeny;
	KRKAL->GetScreenRes(screenx,screeny);
	int winposX = screenx > 1000 ? (screenx - 1000) / 2 : 0;
	if (screenx > 1000)
		screenx = 1000;

	CGUIStdWindow *cw=new CGUIStdWindow((float)winposX,0,(float)screenx,(float)screeny,"ABOUT",0,false,STD_RESIZER_COLOR,1,0xDD202070);
	cw->escapeClose=true;
	cw->title->SetButtons(true,false,false);
	desktop->AddBackElem(cw);
	cw->FocusMe();

	CGUIStaticText *st;

	float mx=(float)screenx-30;
	float xx=10;
	float yy=10;
	float sxx,syy;
	int ti=0;
	UI tc=0xFFFFFFFF;
	UI nc=0xFFAFFF6D;

	CGUIStaticPicture* krkal = new CGUIStaticPicture(0,0,"$TEX$/MainMenu/krkal.png");
	krkal->GetSize(sxx,syy);
	krkal->Move((screenx-sxx-10)/2,0);
	yy+=syy+10;
	cw->AddBackElem(krkal);


	CGUIFont *f=(CGUIFont*) RefMgr->Find("GUI.F.Verdana.10");
	CGUIFont *fb=(CGUIFont*) RefMgr->Find("GUI.F.Verdana.10.B");
	CGUIFont *fn=(CGUIFont*) RefMgr->Find("GUI.F.Courier.14.B");

	//autori
	yy+=10;
	st=new CGUIStaticText(credittext[ti++],fn,xx,yy,nc,mx,0,aCenter);
	st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	for(int i=0;i<7;i++)
	{
		st=new CGUIStaticText(credittext[ti],fb,xx,yy,tc,mx,0,aCenter);
		st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);
		ti++;
		st=new CGUIStaticText(credittext[ti],f,xx,yy,tc,mx,0,aCenter);
		st->GetSize(sxx,syy);yy+=syy+15;cw->AddBackElem(st);
		ti++;
	}

	//web
	yy+=30;
	st=new CGUIStaticText(credittext[ti++],fn,xx,yy,nc,mx,0,aCenter);
	st->GetSize(sxx,syy); yy+=syy+10; cw->AddBackElem(st);
	for(int i=0;i<1;i++)
	{
		st=new CGUIStaticText(credittext[ti],f,xx,yy,tc,mx,0,aCenter);
		st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);
		ti++;
		st = new CGUIStaticText(credittext[ti], f, xx, yy, tc, mx, 0, aCenter);
		st->GetSize(sxx, syy); yy += syy + 15; cw->AddBackElem(st);
		ti++;
	}

	//licence
	yy+=30;
	st=new CGUIStaticText(credittext[ti++],fn,xx,yy,nc,mx,0,aCenter);
	st->GetSize(sxx,syy); yy+=syy+10; cw->AddBackElem(st);
	for(int i=0;i<6;i++)
	{
		st=new CGUIStaticText(credittext[ti],f,xx,yy,tc,mx,0,aCenter);
		st->GetSize(sxx,syy);yy+=syy+10;cw->AddBackElem(st);
		ti++;
	}

	yy+=10;
	cw->SetBackWindowNormalSize(mx+10,yy);

}

void CMainMenuHelp::ShowHelp()
{
	char buffer[1024];
	char** helptext;
	if(KRKAL->cfg.lang==langEN) helptext=helptexten;
	else helptext=helptextcs;

	int screenx,screeny;
	KRKAL->GetScreenRes(screenx,screeny);
	int winposX = screenx > 1000 ? (screenx - 1000) / 2 : 0;
	if (screenx > 1000)
		screenx = 1000;

	CGUIStdWindow *cw=new CGUIStdWindow((float)winposX,0,(float)screenx,(float)screeny,"HELP",0,false,STD_RESIZER_COLOR,1,0xDD202070);
	cw->escapeClose=true;
	cw->title->SetButtons(true,false,false);
	desktop->AddBackElem(cw);
	cw->FocusMe();

	CGUIStaticText *st;

	float mx=(float)screenx-30;
	float xx=10;
	float yy=10;
	float sxx,syy;
	int ti=0;
	UI tc=0xFFFFFFFF;
	UI nc=0xFFAFFF6D;

	CGUIStaticPicture* krkal = new CGUIStaticPicture(0,0,"$TEX$/MainMenu/krkal.png");
	krkal->GetSize(sxx,syy);
	krkal->Move((screenx-sxx-10)/2,0);
	yy+=syy+10;
	cw->AddBackElem(krkal);


	CGUIFont *f=(CGUIFont*) RefMgr->Find("GUI.F.Verdana.10");
	CGUIFont *fb=(CGUIFont*) RefMgr->Find("GUI.F.Verdana.10.B");
	CGUIFont *fn=(CGUIFont*) RefMgr->Find("GUI.F.Courier.14.B");

	//hra krkal
	yy+=10;
	st=new CGUIStaticText(helptext[ti++],fn,xx,yy,nc,mx);
	st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	
	st = new CGUIStaticText(ConvertKeys(helptext[ti++], buffer), fb, xx, yy, tc, mx);
	st->GetSize(sxx,syy);yy+=syy;cw->AddBackElem(st);
	st = new CGUIStaticText(ConvertKeys(helptext[ti++], buffer), f, xx + 20, yy, tc, mx - 20);
	st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);
	st = new CGUIStaticText(ConvertKeys(helptext[ti++], buffer), fb, xx, yy, tc, mx);
	st->GetSize(sxx,syy);yy+=syy;cw->AddBackElem(st);
	st = new CGUIStaticText(ConvertKeys(helptext[ti++], buffer), f, xx + 20, yy, tc, mx - 20);
	st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);

	for(int i=0;i<8;i++)
	{
		st = new CGUIStaticText(ConvertKeys(helptext[ti], buffer), fb, xx, yy, tc, mx);
		st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);
		ti++;
	}

	yy+=5;
	for(int i=0;i<2;i++)
	{
		st = new CGUIStaticText(ConvertKeys(helptext[ti], buffer), f, xx, yy, tc, mx);
		st->GetSize(sxx,syy);yy+=syy+10;cw->AddBackElem(st);
		ti++;
	}

	for (int i = 0; i<5; i++)
	{
		st = new CGUIStaticText(ConvertKeys(helptext[ti], buffer), fb, xx, yy, tc, mx);
		st->GetSize(sxx, syy); yy += syy + 5; cw->AddBackElem(st);
		ti++;
	}

    //konfigurace
	yy+=25;
	st=new CGUIStaticText(helptext[ti++],fn,xx,yy,nc,mx);
	st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	st=new CGUIStaticText(helptext[ti++],f,xx,yy,tc,mx);
	st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);

	//technicka podpora
	yy+=15;
	st=new CGUIStaticText(helptext[ti++],fn,xx,yy,nc,mx);
	st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	for(int i=0;i<4;i++)
	{
		st=new CGUIStaticText(helptext[ti],f,xx,yy,tc,mx);
		st->GetSize(sxx,syy);yy+=syy+10;cw->AddBackElem(st);
		ti++;
	}

	yy+=10;
	cw->SetBackWindowNormalSize(mx+10,yy);

}



const char *CMainMenuHelp::ConvertKeys(const char *text, char buffer[1024]) {
	const char *ptr = text;
	int dptr = 0;
	const char *key = 0;
	const char *key2;

	for (; *ptr; ptr++) {
		if (ptr[0] == '$' && ptr[1] == '$') {
			ptr++;
			if (!key) {
				key = buffer + dptr;
			} else {
				buffer[dptr] = 0;
				key2 = Input->FindKeyDisplayName(key);
				dptr = key - buffer;
				key = 0;

				while (key2 && *key2 && dptr + 1 < 1024)
				{
					buffer[dptr++] = *key2++;
				}
			}
		}
		else {
			if (dptr + 1 >= 1024)
				break;
			buffer[dptr] = *ptr;
			dptr++;
		}
	}

	buffer[dptr] = 0;
	return buffer;
}