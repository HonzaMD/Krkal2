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

char* credittextcs[]={

	"AUTOØI",

	"Jan Krèek (MD) md@krkal.org",
	"verze 2.5 a 3.0, kernel, skriptování, vlastní hra KRKAL, levely,... ",
	"Petr Altman (pal) pal@krkal.org",
	"grafický engine, editor textu, editor skriptù, grafika pro hru, www,...",
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
	"Je možné program používat, modifikovat v nìm vytváøené hry, tvoøit nové levly.",
	"Program používáte na vaše vlastní nebezpeèí. Nepøebíráme zodpovìdnost za jakékoli škody vzniklé v souvislosti s používáním programu.",
	"Bez našeho svolení není možné použít Systém Krkal jako souèást nìjakého komerènì prodávaného produktu.",
	"Veškerá další autorská práva vyhrazena (c) 2017",

};

char* credittexten[]={

	"AUTHORS",

	"Jan Krcek (MD) md@krkal.org",
	"version 2.5 and 3.0, kernel, scripting, game KRKAL, levels,...",

	"Petr Altman (pal)",
	"graphics engine, text editor, script editor, graphics, www,...",

	"Jan Poduska (Morgue)",
	"GUI, level editor",

	"Jiri Margaritov",
	"compiler, interpret",

	"Jara Altmann",
	"levels, ideas, suggestions, comments",

	"Jan Frolik",
	"music - what's left to say - EX - TRA - OR - DI - NA - RY",

    "Tomy, Dvoracek, Pavel P.",
	"levels",

	"KRKAL HOMEPAGE",

	"www.krkal.org",
	"https://github.com/HonzaMD/Krkal2",

	"LICENSE",

	"Program Krkal 2.5 can be free downloaded and you are allowed to distribute the program further, but only for free.",
	"You may use the program, modify the games created within the system, create new levels.",
	"Use this program at your own risk. We cannot be held responsible for any loss or damage this program might have caused.",
	"You are NOT allowed to distribute KRKAL for money or even use it as a part of any commercial product without our explicit permission.",
	"All rights reserved (c) 2017 P. Altman, J. Krcek, J. Margaritov, J. Poduska, J.Altmann, J. Frolik"

};

char* helptextcs[]={

	"HRA KRKAL: OVLÁDÁNÍ",

	"pohyb postavièky: kurzorové klávesy",
	"Postavièka se vždy pohybuje pouze horizontálním nebo vertikálním smìrem - nikdy ne šikmo. Lze držet i více smìrových kláves najednou, poté se bude postavièka pohybovat jedním ze smìrù, kam mùže jít, pøièemž se bude	preferovat naposledy zmáèknutý smìr. To je neocenitelná vlastnost v rùzných bludištích.",
    "pøepínání mezi postavièkami: TAB",
	"v nìkterých levelech je více postavièek, nìkdy je možné mezi postavièkami pøepínat, jindy se všechny ovládají najednou (když pak napø. stiskne šipku vpravo - všechny postavièky pùjdou doprava)",
	"použití bomby: Z",
	"použití miny: X",
	"sebrání znaèky: A",
	"položení (otoèení) smìrové znaèky: 1+klávesa smìru",
	"položení zákazu vjezdu: 2",
	"položení šedesátky: 3",
	"položení konce šedesátky: 4",
	"zpìt do úvodního menu: ESC",

   "Hráè mùže sbírat nìkteré pøedmìty. Od každého pøedmìtu mùže mít maximálnì 12 kusù. Vìtšina pøedmìtù se sebere sama, jakmile hráè vstoupí na políèko s pøedmìtem. U tìchto pøedmìtù není možné rozhodnout se, jestli je chci nebo ne. Výjimkou jsou znaèky, ty se sbírají speciální klávesou 'A'.",
   "Sebrané pøedmìty nelze jen tak zahodit. Nìkteré pøedmìty je možné použít speciální klávesou (viz výše). Jiné pøedmìty se použijí samy - napø. klíè se použije vždy, když hráè chce vstoupit na políèko, kde je zámek. Tady bych chtìl upozornit na jednu záludnost - pokud hráè má klíè a nìjaká síla ho pøinutí vjet (napø. klouže se po ledu) na políèko se zámkem - tak se zámek vždy odemkne, a hráè chce nebo ne.",

   "KONFIGURACE",

   "Konfigurace se provádí ruèní editací souboru \"krkal.cfg\", je možné napøíklad mìnit rozlišení ve høe, nastavovat hlasitost,..",

   "TECHNICKÁ PODPORA, INFORMACE, NÁPOVÌDA",

   "Velké množství informací najdete pøímo v dokumentaci, která je souèástí Krkala (v adresáøi Dokumentace). V dokumentaci najdete popis objektù ve høe Krkal a jak ovládat editor. Další informace naleznete na našich webových stránkách:",
   "      www.krkal.org",
   "Na náš email nám mùžete posílat námìty, levely, žádosti o rady, pøipomínky, hlášení chyb, ...:",
   "      support@krkal.org",

};

char* helptexten[]={

	"GAME KRKAL: CONTROLS",

	"movement: key arrows",
	"You can move in four basic directions only – you cannot move diagonally. You may hold more arrow keys at once, in which case you will move in one of the directions available at the moment, the last pressed key is preferred. This is very useful in the labyrinths.",
    "switching between the characters: TAB",
	"In some levels, you can control more than one character. Sometimes you control only one character at the moment and may switch between them, while sometimes you control them all together. In this case, if you for example press the right arrow key, all the characters will start moving right.",
	"drop a bomb: Z",
	"put down a mine: X",
	"pick up a traffic sign: A",
	"put down a direction sign: 1 + direction arrow (you may also change the direction of a lying sign)",
	"put down a no fare sign: 2",
	"put down a 60 km/h speed limit sign: 3",
	"put down an end of the 60 km/h speed limit sign: 4",
	"go back to the main menu: ESC",

   "You can pick up some items and collect them in your inventory. You may collect up to 12 items of each type. Most of these items are picked up automatically as you step on the cell they are located. You cannot decide whether to pick up such item or not. The traffic signs are exception, you can choose to pick a sign by pressing ‘A’ key.",
   "Once picked up, the items cannot be discarded, but you can use some of them (as described above). Some items are used automatically – for instance, the key is used after you have tried to step on the cell blocked by the lock. Here we would like to warn of one captious feature: If you have a key and you are forced to move towards the lock (usually on the ice), you will unlock it when either you want or not.",

   "CONFIGURATION",

   "You can configure KRKAL by editing the file \"krkal.cfg\", you can change the screen resolution or set the volume for example...",

   "TECHNICAL SUPPORT, MORE INFORMATION, HELP",

   "Much information can be found in the documentation included in the download pack. You can find more information at KRKAL website:",
   "      www.krkal.org",
   "You can also contact us with tips, levels, notices, help requests and bug reports on our email:",
   "      support@krkal.org",

};

void CMainMenuHelp::ShowCredits()
{
	char** credittext;
	if(KRKAL->cfg.lang==langEN) credittext=credittexten;
	else credittext=credittextcs;

	int screenx,screeny;
	KRKAL->GetScreenRes(screenx,screeny);

	CGUIStdWindow *cw=new CGUIStdWindow(0,0,(float)screenx,(float)screeny,"ABOUT",0,false,STD_RESIZER_COLOR,1,0xDD202070);
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
	st=new CGUIStaticText(credittext[ti++],fn,xx,yy,tc,mx,0,aCenter);
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
	st=new CGUIStaticText(credittext[ti++],fn,xx,yy,tc,mx,0,aCenter);
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
	st=new CGUIStaticText(credittext[ti++],fn,xx,yy,tc,mx,0,aCenter);
	st->GetSize(sxx,syy); yy+=syy+10; cw->AddBackElem(st);
	for(int i=0;i<5;i++)
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
	char** helptext;
	if(KRKAL->cfg.lang==langEN) helptext=helptexten;
	else helptext=helptextcs;

	int screenx,screeny;
	KRKAL->GetScreenRes(screenx,screeny);

	CGUIStdWindow *cw=new CGUIStdWindow(0,0,(float)screenx,(float)screeny,"HELP",0,false,STD_RESIZER_COLOR,1,0xDD202070);
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
	st=new CGUIStaticText(helptext[ti++],fn,xx,yy,tc,mx);
	st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	
	st=new CGUIStaticText(helptext[ti++],fb,xx,yy,tc,mx);
	st->GetSize(sxx,syy);yy+=syy;cw->AddBackElem(st);
	st=new CGUIStaticText(helptext[ti++],f,xx+20,yy,tc,mx-20);
	st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);
	st=new CGUIStaticText(helptext[ti++],fb,xx,yy,tc,mx);
	st->GetSize(sxx,syy);yy+=syy;cw->AddBackElem(st);
	st=new CGUIStaticText(helptext[ti++],f,xx+20,yy,tc,mx-20);
	st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);

	for(int i=0;i<8;i++)
	{
		st=new CGUIStaticText(helptext[ti],fb,xx,yy,tc,mx);
		st->GetSize(sxx,syy);yy+=syy+5;cw->AddBackElem(st);
		ti++;
	}

	yy+=5;
	for(int i=0;i<2;i++)
	{
		st=new CGUIStaticText(helptext[ti],f,xx,yy,tc,mx);
		st->GetSize(sxx,syy);yy+=syy+10;cw->AddBackElem(st);
		ti++;
	}

    //konfigurace
	yy+=20;
	st=new CGUIStaticText(helptext[ti++],fn,xx,yy,tc,mx);
	st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);
	st=new CGUIStaticText(helptext[ti++],f,xx,yy,tc,mx);
	st->GetSize(sxx,syy); yy+=syy+15; cw->AddBackElem(st);

	//technicka podpora
	yy+=15;
	st=new CGUIStaticText(helptext[ti++],fn,xx,yy,tc,mx);
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
