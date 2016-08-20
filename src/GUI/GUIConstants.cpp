///////////////////////////////////////////////
//
// GUIConstants.h
// hlavni obecne konstanty a typy GUI
//	urcuji defaultni chovani a vzhled prvku GUI, Editoru
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"
#include "GUIConstants.h"

int		STD_FONT_SIZE				= 16;
float	STD_TITLE_HEIGHT			= 20;
float	STD_PANEL_HEIGHT			= 25;
float	STD_WINICON_HEIGHT			= 14;
float	STD_SCROLLBAR_WIDTH			= 14;
bool	STD_SCROLLBAR_AUTOHIDE		= true;
bool	STD_SCROLLBAR_SLIDER_RESIZE	= true;
float	STD_SCROLLBAR_SHIFT			= 10;
int		STD_SCROLLBAR_MWHEEL_COEF	= 3;	// kolirat se shiftne pri otocenim koleckem
float	STD_SCROLLBAR_TIMER1		= 0.3f;	// casova prodleva mezi prvnim posunem
float	STD_SCROLLBAR_TIMER2		= 0.05f;// casova prodleva mezi ostatnimi posuny

int		STD_TITLE_ACTIVE_COLOR		= 0xFFFFFFFF;	// bila
int		STD_TITLE_DEACTIVE_COLOR	= 0xFFBBBBBB;	// seda
int		STD_TITLE_LABEL_COLOR		= 0xFF750000;	// cervena
int		STD_RESIZER_COLOR			= 0xFFE9F874;

float	STD_PANEL_SHIFT			= 15;
float	STD_ICON_SHIFT			= 6;
float	STD_BUTTON_SHIFT		= 3;
float	STD_PANEL_BUTTON_SIZE	= 100;
float	STD_PANEL_BUTTON_HEIGHT	= 20;
float	MIN_PANEL_BUTTON_SIZE	= 30;	//50
int		STD_PANEL_TEXT_SIZE		= 8;

float	STD_PANEL_ICON_SIZE		= 16;
float	MARK_PANEL_ICON_SIZE	= 30;
//#define STD_PANEL_ICON_GAP		(STD_PANEL_ICON_SIZE+(MARK_PANEL_ICON_SIZE-STD_PANEL_ICON_SIZE)/2+1)
float	STD_PANEL_ICON_GAP		= 5;

float	HELP_DELAY		= 1.0f;		// za jak dlouho se objevi kontextova napoveda [s]
float	HELP_TIME		= 15.0f;	// jak dlouho bude zobrazena kontextova napoveda [s]

float	STD_TOOLBAR_TITLE_HEIGHT			= 12;
int		STD_TOOLBAR_TITLE_LABEL_COLOR		= 0xFFEAE330;	// cervena
int		STD_TOOLBAR_TITLE_ACTIVE_COLOR		= 0xFF075FC2;	// bila
int		STD_TOOLBAR_TITLE_DEACTIVE_COLOR	= 0xFF155296;	// seda
int		STD_TOOLBAR_BGCOLOR					= 0xFF1B1B50; //  0xAA5C5995
int		STD_TOOLBAR_BORDER_COLOR			= 0xFF004593;	

int		STD_MENU_TEXT_SIZE				= 10;
float	STD_MENU_PICTURE_STRIP_SIZE		= 18;
int		STD_MENU_PICTURE_STRIP_COLOR	= 0xFFCFA943;
float	STD_MENU_PICTURE_SIZE			= 14;
float	STD_MENU_EXTENSION_SIZE			= 12;
int		STD_MENU_MARK_BORDER_COLOR		= 0xFF8E7533;
int		STD_MENU_MARK_BGCOLOR_COLOR		= 0xAAE0CC8D;
int		STD_MENU_TEXT_COLOR				= 0xFF000000;
int		STD_MENU_TEXT_INACTIVE_COLOR	= 0xFFD3D2E2;
int		STD_MENU_BORDER_COLOR			= 0xFF8E7533;
int		STD_MENU_BGCOLOR_COLOR			= 0xFFE5E0C0;
float	STD_MENU_HEIGHT					= 18;
float	STD_MENU_WIDTH					= 150;
int		STD_MENU_GAP_COLOR				= 0xFF8E7533;

/*
int		STD_MENU_TEXT_SIZE				= 10;
float	STD_MENU_PICTURE_STRIP_SIZE		= 18;
int		STD_MENU_PICTURE_STRIP_COLOR	= 0xFF7DB3DF;
float	STD_MENU_PICTURE_SIZE			= 14;
float	STD_MENU_EXTENSION_SIZE			= 12;
int		STD_MENU_MARK_BORDER_COLOR		= 0xFF37318C;
int		STD_MENU_MARK_BGCOLOR_COLOR		= 0xAA698FEF;
int		STD_MENU_TEXT_COLOR				= 0xFF000000;
int		STD_MENU_TEXT_INACTIVE_COLOR	= 0xFFD3D2E2;
int		STD_MENU_BORDER_COLOR			= 0xFF3D81BB;
int		STD_MENU_BGCOLOR_COLOR			= 0xFFA5C5DF;
float	STD_MENU_HEIGHT					= 18;
float	STD_MENU_WIDTH					= 150;
int		STD_MENU_GAP_COLOR				= 0xFF3D81BB;
*/

float	STD_TREE_HEIGHT					= 18;
int		STD_TREE_TEXT_COLOR				= 0xFF000000;
int		STD_TREE_TEXT_INACTIVE_COLOR	= 0xFFD3D2E2;
int		STD_TREE_TEXT_SIZE				= 10;
float	STD_TREE_PICTURE_SIZE			= 14;
float	STD_TREE_ROLLER_SIZE			= 14;
float	STD_TREE_GAP					= 4;
int		STD_TREE_MARK_BORDER_COLOR		= 0xFF37318C;
int		STD_TREE_MARK_BGCOLOR_COLOR		= 0xAA698FEF;
int		STD_TREE_MARK_TEXT_COLOR		= 0xFFFFFFFF;
float	STD_TREE_SHIFT					= 18;
int		STD_TREE_BORDER_COLOR			= 0xFF004593;
int		STD_TREE_BGCOLOR_COLOR			= 0xFFA5C5DF;
float	STD_TREE_ELEMENT_SHIFT			= 2;
float	STD_TREE_ITEM_START_X			= 10;
float	STD_TREE_ITEM_START_Y			= 10;
float	STD_TREE_END_GAP_X				= 10;	// okraj na konci
float	STD_TREE_END_GAP_Y				= 10;	// okraj na konci

float	STD_TREE_BEFORE_SLIDER_GAP		= 10;
float	STD_TREE_AFTER_SLIDER_GAP		= 5;
int		STD_TREE_SLIDER_COLOR			= 0xFF004593;
int		STD_TREE_SLIDER_MOVECOLOR		= 0xFF698FEF;
int		STD_TREE_SLIDER_BACKGROUND_COLOR = 0xFF7CABD1;
int		STD_TREE_ROOTITEM_BACKGROUND_COLOR = 0xFF95B5DF;
int		STD_TREE_ROOTITEM_LINES_COLOR	= 0xFFFFFFFF;
int		STD_TREE_ROOTITEM_BACKGROUND_MARK_COLOR = 0xFF000000;

int		STD_DLG_TEXT_COLOR				= 0xFF000000;
int		STD_DLG_TEXT_ERR_COLOR			= 0xFFFF0000;
int		STD_DLG_BORDER_COLOR			= 0xFF004593;
int		STD_DLG_BGCOLOR_COLOR			= 0xFFA5C5DF;

int		STD_WINDOW_HOLDER_SCROLLING		= 0;	// 0 - normal, 1 - inverse
float	STD_WINDOW_SCROLLING_BOOSTER	= 4.0f;	// zrychlovac scrollingu (1 = normal speed)


int		STD_CHECKBOX_COLOR				= 0xFFFFFFFF;
int		STD_CHECKBOX_INACTIVE_COLOR		= 0xFFA4A3A3;

int		STD_REGEDIT_MAX_KEYVALUES		= 10;
int		STD_REGEDIT_VALUE_COLOR			= 0xFF004593;


int		STD_BUTTON_LABEL_COLOR = 0xFF004593;
int		STD_BUTTON_LABEL_DISABLED_COLOR = 0xFF969696;
char	STD_BUTTON_FONT[] = "GUI.F.Arial.10";

int		STD_GAMEWINDOW_BACKGROUND_COLOR	= 0xFF1B1B50;

int		CURSOR_CELL_INSERT_NORMAL_COLOR		= 0xFFFFFFFF;
int		CURSOR_CELL_INSERT_NORMAL_COLOR2	= 0x40FFFFFF;
int		CURSOR_CELL_INSERT_REPLACE_COLOR	= 0xFF930023;
int		CURSOR_CELL_INSERT_REPLACE_COLOR2	= 0x40930023;
int		CURSOR_CELL_INSERT_ERROR_COLOR		= 0xFFFDFF4D;
int		CURSOR_CELL_INSERT_ERROR_COLOR2		= 0x40FDFF4D;
int		CURSOR_CELL_SELECTLOCATION_COLOR	= 0xFF569FFA;
int		CURSOR_CELL_SELECTLOCATION_COLOR2	= 0x40569FFA;
int		CURSOR_CELL_MARKER_COLOR			= 0xFFECB626;
int		CURSOR_CELL_MARKER_COLOR2			= 0x40ECB626;
int		CURSOR_AREA_MARKER_COLOR2			= 0x20ECB626;
int		CURSOR_SELECTCELL_COLOR				= 0xFF057A13;
int		CURSOR_SELECTCELL_COLOR2			= 0x40057A13;



// scrolling herniho okna pomoci najeti mysi na okraj okna aplikace
float	STD_GAMEWIN_SCROLLING_TIMER		= 0.01f;	// casova prodleva mezi posuny
float	STD_GAMEWIN_SCROLLING_SHIFT		= 25;//15	// o kolik pixelu se okno posune behem jednoho cyklu scrollovani
float	STD_GAMEWIN_SCROLLING_LIMIT		= 2;		// jak siroka je hranice na kraji okna, ktera vyvolava scrollovani

// scrolling herniho okna pomoci umistovani objektu ke kraji herniho okna
bool	STD_GAMEWIN_SCROLLING_INWIN_USE			= false;	// zda se scrolling pouzije nebo bude vypnuty
float	STD_GAMEWIN_SCROLLING_INWIN_TIMER		= 0.10f;	// casova prodleva mezi posuny
float	STD_GAMEWIN_SCROLLING_INWIN_LIMIT		= 50;		// jak siroka je hranice na kraji okna, ktera vyvolava scrollovani

// GAME Menu
float	STD_GAMEMENU_HEIGHT		= 50;	// vyska herniho menu
int	STD_GAMEMENU_BGCOLOR	= 0xFF000000;	
int	STD_GAMEMENU_COUNTCOLOR	= 0xFF606060;
int	STD_GAMEMENU_PROGRESSBAR1_COLOR	= 0xFF8BBFFF;
int	STD_GAMEMENU_PROGRESSBAR2_COLOR	= 0xFFC00F62;


int		STD_ANNOUNCER_COLOR		= 0xFFFDFA24;
