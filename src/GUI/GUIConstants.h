///////////////////////////////////////////////
//
// GUIConstants.h
//	hlavni obecne konstanty a typy GUI
//	urcuji defaultni chovani a vzhled prvku GUI, Editoru
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef GUIConstants_H
#define GUIConstants_H


#define MAX_CURSORS 15		//	max. pocet kurzoru ve spravci kurzoru



// std. typy fontu:
#define TIMES "Times New Roman"
#define ARIAL "Arial"
#define COURIER "Courier New"



extern int		STD_FONT_SIZE;
extern float	STD_TITLE_HEIGHT;			// vyska titulku okna
extern float	STD_PANEL_HEIGHT;			// vyska panelu ("TaskBar")
extern float	STD_WINICON_HEIGHT;			// velikost ikony v titulku okna

// ScrollBar
extern float	STD_SCROLLBAR_WIDTH;		// sirka scrollbaru
extern bool		STD_SCROLLBAR_AUTOHIDE;		// zda se scrollbar skryva, pokud neni potreba / je videt stale
extern bool		STD_SCROLLBAR_SLIDER_RESIZE;// zda se stredovy posuvnik na scrollbaru zvetsuje dle pomeru velikosti okna a jeho viditelne casti, nebo ne
extern float	STD_SCROLLBAR_SHIFT;		// o kolik bodu se posune vnitrek okna pri scrollovani (1x stisk tlacitka)
extern int		STD_SCROLLBAR_MWHEEL_COEF;	// kolirat se shiftne pri otocenim koleckem
extern float	STD_SCROLLBAR_TIMER1;		// casova prodleva mezi prvnim posunem
extern float	STD_SCROLLBAR_TIMER2;		// casova prodleva mezi ostatnimi posuny

// Titulek okna
extern int		STD_TITLE_ACTIVE_COLOR;		// jakou barvou se prebarvuje titulek, kdyz je okno aktivni (focusovane)
extern int		STD_TITLE_DEACTIVE_COLOR;	// jakou barvou se prebarvuje titulek, kdyz je okno neaktivni (defocusovane)
extern int		STD_TITLE_LABEL_COLOR;		// barva popisku (textu) na titulku
extern int		STD_RESIZER_COLOR;			// barva okraju okna (okraje umoznuji zmenu velikosti okna)

// WindowPanel (TaskBar):
extern float	STD_PANEL_SHIFT;			// vyska presahu panelu (urcuje, jak hodne mohou ikony atp. na panelu presahovat panelovou listu smerem nahoru)
extern float	STD_ICON_SHIFT;				// vyska posunu ikon na panelu od jeho horniho okraje smerem dolu
extern float	STD_BUTTON_SHIFT;			// vyska posunu tlacitek na panelu od jeho horniho okraje smerem dolu
extern float	STD_PANEL_BUTTON_SIZE;		// max. velikost (sirka) tlacitka na panelu 
extern float	STD_PANEL_BUTTON_HEIGHT;
extern float	MIN_PANEL_BUTTON_SIZE;		// min. velikost (sirka) tlacitka na panelu, dale se jiz nezmensuje a nektere tlacitka uz nejsou videt 
extern int		STD_PANEL_TEXT_SIZE;		// velikost fontu, kterym se pise do tlacitek na panelu
extern float	STD_PANEL_ICON_SIZE;		// velikost ikony na panelu v normalnim stavu
extern float	MARK_PANEL_ICON_SIZE;		// velikost ikony na panelu v oznacenem stavu (najetim mysi)
extern float	STD_PANEL_ICON_GAP;			// mezera mezi ikonama

extern float	HELP_DELAY;					// za jak dlouho se objevi kontextova napoveda
extern float	HELP_TIME;					// na jak dlouho se objevi kontextova napoveda

// ToolBar:
extern float	STD_TOOLBAR_TITLE_HEIGHT;
extern int		STD_TOOLBAR_TITLE_LABEL_COLOR;
extern int		STD_TOOLBAR_TITLE_ACTIVE_COLOR;
extern int		STD_TOOLBAR_TITLE_DEACTIVE_COLOR;
extern int		STD_TOOLBAR_BGCOLOR;
extern int		STD_TOOLBAR_BORDER_COLOR;

// Menu:
extern float	STD_MENU_PICTURE_STRIP_SIZE;		// sirka prouzku u kontextoveho menu (prouzek je podkladem pro obrazky u polozek menu)
extern int		STD_MENU_PICTURE_STRIP_COLOR;		// barva prouzku (podrobnosti viz. vyse)
extern float	STD_MENU_PICTURE_SIZE;				// velikost obrazku u polozek menu
extern float	STD_MENU_EXTENSION_SIZE;			// velikost pro obrazek oznacujici polozku, ktera vytvari podmenu (submenu)
extern int		STD_MENU_MARK_BORDER_COLOR;			// barva okraju u oznacovace polozek menu
extern int		STD_MENU_MARK_BGCOLOR_COLOR;		// barva pozadi u oznacovace polozek menu
extern int		STD_MENU_TEXT_SIZE;					
extern int		STD_MENU_TEXT_COLOR;
extern int		STD_MENU_TEXT_INACTIVE_COLOR;
extern int		STD_MENU_BORDER_COLOR;
extern int		STD_MENU_BGCOLOR_COLOR;
extern float	STD_MENU_HEIGHT;					// velikost menu (vyska), pokud se neurci jinak
extern float	STD_MENU_WIDTH;						// velikost menu (sirka), pokud se neurci jinak
extern int		STD_MENU_GAP_COLOR;					// barva oddelujici cary mezi polozkami menu

// Tree:
extern float	STD_TREE_HEIGHT;					// std. vyska polozky stromu
extern int		STD_TREE_TEXT_COLOR;
extern int		STD_TREE_TEXT_INACTIVE_COLOR;
extern int		STD_TREE_TEXT_SIZE;
extern float	STD_TREE_PICTURE_SIZE;				// velikost obrazku v polozke stromu
extern float	STD_TREE_ROLLER_SIZE;				// velikost obrazku symbolizuji typ polozky (list, rozbaleny/zabaleny podstrom)
extern float	STD_TREE_GAP;						// mezera mezi obrazkem a rollerem a textem polozky
extern int		STD_TREE_MARK_BORDER_COLOR;			// barva okraju u oznacovace polozek stromu
extern int		STD_TREE_MARK_BGCOLOR_COLOR;		// barva pozadi u oznacovace polozek stromu
extern int		STD_TREE_MARK_TEXT_COLOR;			// barva textu u oznacene polozky
extern float	STD_TREE_SHIFT;						// x-ovy posun polozky podstromu vuci korenove polozce (zanoreni stromu)
extern int		STD_TREE_BORDER_COLOR;
extern int		STD_TREE_BGCOLOR_COLOR;
extern float	STD_TREE_ELEMENT_SHIFT;				// y-ovy posun elementu polozky od sve polozky (v pripade neradkoveho elementu)
extern float	STD_TREE_ITEM_START_X;				// kde se zacnou vytvaret polozky v okne
extern float	STD_TREE_ITEM_START_Y;				
extern float	STD_TREE_END_GAP_X;					// velikost mezery za posledni polozkou k okraji okna
extern float	STD_TREE_END_GAP_Y;

extern float	STD_TREE_BEFORE_SLIDER_GAP;			// mezera mezi polozkami a posuvnikem (posuvnik rozdeluje okno stromu na dve casti - levou a pravou. V leve jsou polozky a v prave jejich radkove elementy)
extern float	STD_TREE_AFTER_SLIDER_GAP;			// mezera mezi posuvnikem a elementem polozky
extern int		STD_TREE_SLIDER_COLOR;				// barva posuvniku
extern int		STD_TREE_SLIDER_MOVECOLOR;			// barva posuvniku pri jeho posuvu
extern int		STD_TREE_SLIDER_BACKGROUND_COLOR;	// barva pozadi prave casti okna stromu (pod radkovymi elementy polozek)
extern int		STD_TREE_ROOTITEM_BACKGROUND_COLOR;	// barva pozadi pod korenovymi polozkami stromu 
extern int		STD_TREE_ROOTITEM_LINES_COLOR;		// barva ohranicujicich car kolem korenovych polozek
extern int		STD_TREE_ROOTITEM_BACKGROUND_MARK_COLOR;	// barva pozadi pod oznacenou korenovou polozkou

// Dialog:
extern int		STD_DLG_TEXT_COLOR;		
extern int		STD_DLG_TEXT_ERR_COLOR;
extern int		STD_DLG_BORDER_COLOR;
extern int		STD_DLG_BGCOLOR_COLOR;

// Scrollovani oknem chycenim mysi a tahem (CTRL + leve tlacitko)
extern int		STD_WINDOW_HOLDER_SCROLLING;	// smer scrollovani okna pri scrollovani chycenim za vnitrek (0 - normal, 1 - inverse)
extern float	STD_WINDOW_SCROLLING_BOOSTER;	// zrychlovac scrollingu (1 = normal speed)

// CheckBox
extern int		STD_CHECKBOX_COLOR;
extern int		STD_CHECKBOX_INACTIVE_COLOR;

// Register Viewer
extern int		STD_REGEDIT_MAX_KEYVALUES;	// pocet hodnot polozek zobrazenych najednou pri prohlizeni registru
extern int		STD_REGEDIT_VALUE_COLOR;

// Button
extern int		STD_BUTTON_LABEL_COLOR;
extern int		STD_BUTTON_LABEL_DISABLED_COLOR;
extern char		STD_BUTTON_FONT[];

extern int		STD_GAMEWINDOW_BACKGROUND_COLOR;	// barva pozadi pod hernim oknem v editoru

// barvy bunkovych kurzoru v editoru
// podle editacniho modu
extern int		CURSOR_CELL_INSERT_NORMAL_COLOR;
extern int		CURSOR_CELL_INSERT_NORMAL_COLOR2;
extern int		CURSOR_CELL_INSERT_REPLACE_COLOR;
extern int		CURSOR_CELL_INSERT_REPLACE_COLOR2;
extern int		CURSOR_CELL_INSERT_ERROR_COLOR;
extern int		CURSOR_CELL_INSERT_ERROR_COLOR2;
extern int		CURSOR_CELL_SELECTLOCATION_COLOR;
extern int		CURSOR_CELL_SELECTLOCATION_COLOR2;
extern int		CURSOR_CELL_MARKER_COLOR;
extern int		CURSOR_CELL_MARKER_COLOR2;
extern int		CURSOR_AREA_MARKER_COLOR2;
extern int		CURSOR_SELECTCELL_COLOR;
extern int		CURSOR_SELECTCELL_COLOR2;


// scrolling herniho okna pomoci najeti mysi na okraj okna aplikace
extern float	STD_GAMEWIN_SCROLLING_TIMER;	// casova prodleva mezi posuny
extern float	STD_GAMEWIN_SCROLLING_SHIFT;	// o kolik pixelu se okno posune behem jednoho cyklu scrollovani
extern float	STD_GAMEWIN_SCROLLING_LIMIT;

// scrolling herniho okna pomoci umistovani objektu ke kraji herniho okna
extern bool		STD_GAMEWIN_SCROLLING_INWIN_USE;	// zda se scrolling pouzije nebo bude vypnuty
extern float	STD_GAMEWIN_SCROLLING_INWIN_TIMER;	// casova prodleva mezi posuny
extern float	STD_GAMEWIN_SCROLLING_INWIN_LIMIT;	// jak siroka je hranice na kraji okna, ktera vyvolava scrollovani


// GAME Menu
extern float	STD_GAMEMENU_HEIGHT;	// vyska herniho menu
extern int		STD_GAMEMENU_BGCOLOR;	
extern int		STD_GAMEMENU_COUNTCOLOR;
extern int		STD_GAMEMENU_PROGRESSBAR1_COLOR;
extern int		STD_GAMEMENU_PROGRESSBAR2_COLOR;

extern int		STD_ANNOUNCER_COLOR;


//	typy vyjimek
#define E_INTERNAL			0
#define E_FILE_NOT_FOUND    2
#define E_BAD_ARGUMENTS     3

//	typy ukotveni
#define WIDGET_FREE 0
#define WIDGET_FIX  1


// vycet typu objektu (zakladni typy)
enum EGUIPrimitivType	
{
	PTTrash = 0,
	PTPrimitive,
	PTRectHost,
	PTElement,
	PTWidget,
	PTMultiWidget,
	PTWindow,
	PTStdWindow
};

// zarovnani textu do bunek u StaticTextu
enum EGUIAlignment
{
	aLeft,
	aRight,
	aCenter
};

// datove typy pro predavani dat mezi elementy GUI a uzivatelskym programem
enum EGUIDataTypes
{
	dtInt,
	dtChar,
	dtFloat,
	dtDouble,
	dtString
	// pole se zada vyberem jednoducheho typu + nastavenim velikosti > 1 (data_size), velikost = pocet polozek pole
};

//	typy ScrollBaru
enum EScrollBarTypes {
	Vertical,
	Horizontal
};

//	ciselny typ pro cislovani zprav, Observru, atp. -  (ID)
typedef unsigned _int64 typeID;
//typedef unsigned int typeID;

// velikost hasovacich tabulek pro NameServer
#define NAME_SERVER_HT_SIZE	10000

#endif 