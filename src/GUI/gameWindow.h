///////////////////////////////////////////////
//
// gameWindow.h
//
//	interface pro propojeni GUI s game window enginu (okno ve kterem se zobrazuje hra)
//  - s hernim oknem se musi pracovat trochu odlisne od normalnich oken
//  - odlisnosti preprogramovava prave CGUIGameWindow
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include "window.h"

class CGUIGameWindow : public CGUIStdWindow
{
public:
	CGUIGameWindow(float _x, float _y, float _sx, float _sy);
	virtual ~CGUIGameWindow();

	void SetShift(float shift_x, float shift_y); 
		// nastavi posun okraje hry od kraje okna 
		// (pouzit v pripde, ze hra zacina na zapornych souradnicich)
		// napr. pokud hra zacina na souradnicich -20,-50 volam: SetShift(20,50);

	void GetShift(float &shift_x, float &shift_y); 
		// vrati nastaveny posun pocatku souradnic hry

	// proprogramovane fce CGUIWindow pro pouziti s hernim oknem:
	virtual void ChangeViewPortBWSize(float &dx, float &dy);
	virtual void SetWindowPositionVP(float dx, float dy, bool updateScrollbars = true);

	void GameWindowScroll(float dx, float dy);
		// informuje okno o zmene nascrollovani
		// vola napr. kernel pote co zmenil nascrollovani hry, aby o tom informoval okno
		// okno aktualizuje scrollbary, markerWin; vlastni scrolling uz neprovadi, nebot ten jiz byl vykonan

protected:
	float shiftX, shiftY;	// posun okraje hry od kraje okna (v pripde, ze hra zacina na zapornych souradnicich)
	CBltWindow *markerWin;	// okno pro oznacovani casti mapy (vrstva nad hrou, ale v okne)
		// herni okno ma krome std. dvou vrstev (CBltWindow) back (vnitrni) a front (vnejsi)
		// jeste jednu vrstvu (markerWin), ktera ma shodnou velikost s vrstvou back
		// lezi nad ni a umoznuje zobrazovat grafiku nad hrou (oznaceni mapy, atp.)
};


#endif 