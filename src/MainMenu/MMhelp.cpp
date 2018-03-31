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

	"AUTO�I",

	"Jan Kr�ek (MD) md@krkal.org",
	"verze 2.5 a 3.0, kernel, skriptov�n�, vlastn� hra KRKAL, levely,... ",
	"Petr Altman (pal)",
	"grafick� engine, editor textu, editor skript�, grafika pro hru, levely, www,...",
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
	"Zdarma dost�v�te kompletn� plnohodnotnou hru. Program neobsahuje reklamu, mikrotransakce ani ��dn� necht�n� �i �kodliv� k�d.",
	"Je mo�n� program pou��vat, modifikovat v n�m vytv��en� hry, tvo�it nov� levly.",
	"Program pou��v�te na va�e vlastn� nebezpe��. Nep�eb�r�me zodpov�dnost za jak�koli �kody vznikl� v souvislosti s pou��v�n�m programu.",
	"Bez na�eho svolen� nen� mo�n� pou��t program Krkal jako sou��st n�jak�ho komer�n� prod�van�ho produktu.",
	"Ve�ker� dal�� autorsk� pr�va vyhrazena (c) 2018",

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

	"HRA KRKAL: OVL�D�N�",

	"pohyb postavi�ky: kurzorov� kl�vesy",
	"Postavi�ka se v�dy pohybuje pouze horizont�ln�m nebo vertik�ln�m sm�rem - nikdy ne �ikmo. Lze dr�et i v�ce sm�rov�ch kl�ves najednou, pot� se bude postavi�ka pohybovat jedn�m ze sm�r�, kam m��e j�t, p�i�em� se bude	preferovat naposledy zm��knut� sm�r. To je neoceniteln� vlastnost v r�zn�ch bludi�t�ch.",
    "p�ep�n�n� mezi postavi�kami: $$_KSID_keyTab_0001_FFFF_0001_0001$$",
	"v n�kter�ch levelech je v�ce postavi�ek, n�kdy je mo�n� mezi postavi�kami p�ep�nat, jindy se v�echny ovl�daj� najednou (kdy� pak nap�. stiskne �ipku vpravo - v�echny postavi�ky p�jdou doprava)",
	"pou�it� bomby: $$_KSID_keyZ_0001_FFFF_0001_0001$$",
	"pou�it� miny: $$_KSID_keyX_0001_FFFF_0001_0001$$",
	"sebr�n� zna�ky: $$_KSID_keyA_0001_FFFF_0001_0001$$",
	"polo�en� (oto�en�) sm�rov� zna�ky: $$_KSID_key1_0001_FFFF_0001_0001$$+kl�vesa sm�ru",
	"polo�en� z�kazu vjezdu: $$_KSID_key2_0001_FFFF_0001_0001$$",
	"polo�en� �edes�tky: $$_KSID_key3_0001_FFFF_0001_0001$$",
	"polo�en� konce �edes�tky: $$_KSID_key4_0001_FFFF_0001_0001$$",
	"zp�t do �vodn�ho menu: ESC",

   "Hr�� m��e sb�rat n�kter� p�edm�ty. Od ka�d�ho p�edm�tu m��e m�t maxim�ln� 12 kus�. V�t�ina p�edm�t� se sebere sama, jakmile hr�� vstoup� na pol��ko s p�edm�tem. U t�chto p�edm�t� nen� mo�n� rozhodnout se, jestli je chci nebo ne. V�jimkou jsou zna�ky, ty se sb�raj� speci�ln� kl�vesou '$$_KSID_keyA_0001_FFFF_0001_0001$$'.",
   "Sebran� p�edm�ty nelze jen tak zahodit. N�kter� p�edm�ty je mo�n� pou��t speci�ln� kl�vesou (viz v��e). Jin� p�edm�ty se pou�ij� samy - nap�. kl�� se pou�ije v�dy, kdy� hr�� chce vstoupit na pol��ko, kde je z�mek. Tady bych cht�l upozornit na jednu z�ludnost - pokud hr�� m� kl�� a n�jak� s�la ho p�inut� vjet (nap�. klou�e se po ledu) na pol��ko se z�mkem - tak se z�mek v�dy odemkne, a� hr�� chce nebo ne.",

   "p�epni do editoru skript�: F1",
   "p�epni do editoru level�: F2",
   "p�epni do hlavn�ho hern�ho menu: F3",
   "ukon�i Krkala: F4",
   "p�ep�n�n� mezi celoobrazovkov�m a okenn�m re�imem: Alt+Enter",

   "KONFIGURACE",

   "Konfigurace se prov�d� ru�n� editac� souboru \"krkal.cfg\", je mo�n� nap��klad m�nit rozli�en� ve h�e, nastavovat hlasitost...",

   "TECHNICK� PODPORA, INFORMACE, N�POV�DA",

   "Velk� mno�stv� informac� najdete p��mo v dokumentaci, kter� je sou��st� Krkala (v adres��i Dokumentace.CS). V dokumentaci najdete popis objekt� ve h�e Krkal a jak ovl�dat editor. Dal�� informace naleznete na na�ich webov�ch str�nk�ch:",
   "      www.krkal.org",
   "Na n� email n�m m��ete pos�lat n�m�ty, levely, ��dosti o rady, p�ipom�nky, hl�en� chyb, ...:",
   "      support@krkal.org",

};

char* helptexten[]={

	"GAME KRKAL: CONTROLS",

	"character movement: arrow keys",
	"You can move in four basic directions only � you cannot move diagonally. You may hold more arrow keys at once, in which case you will move in one of the directions available at the moment, the last pressed key is preferred. This is a priceless quality in various mazes.",
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