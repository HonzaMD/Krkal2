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

	"AUTO�I",

	"Jan Kr�ek (MD) md@krkal.org",
	"verze 2.5 a 3.0, kernel, skriptov�n�, vlastn� hra KRKAL, levely,... ",
	"Petr Altman (pal) pal@krkal.org",
	"grafick� engine, editor textu, editor skript�, grafika pro hru, www,...",
	"Jan Podu�ka (Morgue)",
	"GUI, editor level�", 
	"Ji�� Margaritov",
	"kompil�tor, interpret",
	"J�ra Altmann",
	"levely, n�pady, n�vrhy, p�ipom�nky",
	"Jan Frol�k",
	"hudba - co dodat - VY - NI - KA - J� - C�",
	"Tomy, Dvo���ek, Pavel P.",
	"levely",

	"INTERNETOV� STR�NKY",

	"www.krkal.org",
    "https://github.com/HonzaMD/Krkal2",


	"LICENCE",

	"Program Krkal 2.5 je voln� ke sta�en�, m��ete program d�l bezplatn� ���it.",
	"Je mo�n� program pou��vat, modifikovat v n�m vytv��en� hry, tvo�it nov� levly.",
	"Program pou��v�te na va�e vlastn� nebezpe��. Nep�eb�r�me zodpov�dnost za jak�koli �kody vznikl� v souvislosti s pou��v�n�m programu.",
	"Bez na�eho svolen� nen� mo�n� pou��t Syst�m Krkal jako sou��st n�jak�ho komer�n� prod�van�ho produktu.",
	"Ve�ker� dal�� autorsk� pr�va vyhrazena (c) 2017",

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

	"HRA KRKAL: OVL�D�N�",

	"pohyb postavi�ky: kurzorov� kl�vesy",
	"Postavi�ka se v�dy pohybuje pouze horizont�ln�m nebo vertik�ln�m sm�rem - nikdy ne �ikmo. Lze dr�et i v�ce sm�rov�ch kl�ves najednou, pot� se bude postavi�ka pohybovat jedn�m ze sm�r�, kam m��e j�t, p�i�em� se bude	preferovat naposledy zm��knut� sm�r. To je neoceniteln� vlastnost v r�zn�ch bludi�t�ch.",
    "p�ep�n�n� mezi postavi�kami: TAB",
	"v n�kter�ch levelech je v�ce postavi�ek, n�kdy je mo�n� mezi postavi�kami p�ep�nat, jindy se v�echny ovl�daj� najednou (kdy� pak nap�. stiskne �ipku vpravo - v�echny postavi�ky p�jdou doprava)",
	"pou�it� bomby: Z",
	"pou�it� miny: X",
	"sebr�n� zna�ky: A",
	"polo�en� (oto�en�) sm�rov� zna�ky: 1+kl�vesa sm�ru",
	"polo�en� z�kazu vjezdu: 2",
	"polo�en� �edes�tky: 3",
	"polo�en� konce �edes�tky: 4",
	"zp�t do �vodn�ho menu: ESC",

   "Hr�� m��e sb�rat n�kter� p�edm�ty. Od ka�d�ho p�edm�tu m��e m�t maxim�ln� 12 kus�. V�t�ina p�edm�t� se sebere sama, jakmile hr�� vstoup� na pol��ko s p�edm�tem. U t�chto p�edm�t� nen� mo�n� rozhodnout se, jestli je chci nebo ne. V�jimkou jsou zna�ky, ty se sb�raj� speci�ln� kl�vesou 'A'.",
   "Sebran� p�edm�ty nelze jen tak zahodit. N�kter� p�edm�ty je mo�n� pou��t speci�ln� kl�vesou (viz v��e). Jin� p�edm�ty se pou�ij� samy - nap�. kl�� se pou�ije v�dy, kdy� hr�� chce vstoupit na pol��ko, kde je z�mek. Tady bych cht�l upozornit na jednu z�ludnost - pokud hr�� m� kl�� a n�jak� s�la ho p�inut� vjet (nap�. klou�e se po ledu) na pol��ko se z�mkem - tak se z�mek v�dy odemkne, a� hr�� chce nebo ne.",

   "KONFIGURACE",

   "Konfigurace se prov�d� ru�n� editac� souboru \"krkal.cfg\", je mo�n� nap��klad m�nit rozli�en� ve h�e, nastavovat hlasitost,..",

   "TECHNICK� PODPORA, INFORMACE, N�POV�DA",

   "Velk� mno�stv� informac� najdete p��mo v dokumentaci, kter� je sou��st� Krkala (v adres��i Dokumentace). V dokumentaci najdete popis objekt� ve h�e Krkal a jak ovl�dat editor. Dal�� informace naleznete na na�ich webov�ch str�nk�ch:",
   "      www.krkal.org",
   "Na n� email n�m m��ete pos�lat n�m�ty, levely, ��dosti o rady, p�ipom�nky, hl�en� chyb, ...:",
   "      support@krkal.org",

};

char* helptexten[]={

	"GAME KRKAL: CONTROLS",

	"movement: key arrows",
	"You can move in four basic directions only � you cannot move diagonally. You may hold more arrow keys at once, in which case you will move in one of the directions available at the moment, the last pressed key is preferred. This is very useful in the labyrinths.",
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

   "You can pick up some items and collect them in your inventory. You may collect up to 12 items of each type. Most of these items are picked up automatically as you step on the cell they are located. You cannot decide whether to pick up such item or not. The traffic signs are exception, you can choose to pick a sign by pressing �A� key.",
   "Once picked up, the items cannot be discarded, but you can use some of them (as described above). Some items are used automatically � for instance, the key is used after you have tried to step on the cell blocked by the lock. Here we would like to warn of one captious feature: If you have a key and you are forced to move towards the lock (usually on the ice), you will unlock it when either you want or not.",

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
