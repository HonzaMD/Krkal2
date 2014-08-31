///////////////////////////////////////////////
//
// testy.h
//
// Interface pro testovaci objekty GUI
//	jedna se o speceialne upraveny desktop, okno, ...
//	definuji se zde objekty odvozene od zakladnich trid + s definovanou konkretni fci
//	jde tedy o priklady skutecnych pouzitelnych elementu GUI 
//  testuje se funkcnost implementace objektu GUI
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef TESTY_H
#define TESTY_H

#include "gui.h"
#include "element.h"
#include "button.h"

class CGUIMenu;
class CGUITestIconHandler;



class CGUIMyTestProg : public CGUIEventProgram
{
public:
	CGUIMyTestProg() { vint=0; vdouble=0; string[0]='\0'; button=0;};
	virtual void EventHandler(CGUIEvent *event);
	int vint;
	double vdouble;
	char string[20];
	CGUIButton *button;
};

class CGUITestWindow : public CGUIStdWindow
{
public:
	CGUITestWindow(float _x, float _y, float _sx, float _sy, char* title, CGUIRectHost* icon );
	virtual ~CGUITestWindow() { RemoveFromTrash(); SAFE_DELETE(program);};

	int MouseRight(float x, float y, UINT mouseState, UINT keyState);	
	int MouseMiddle(float x, float y, UINT mouseState, UINT keyState);

	CGUIMyTestProg* program;
};



//////////////////////////////////////////////////////////////////////
// CGUITestDesktop
//////////////////////////////////////////////////////////////////////

class CGUITestDesktop : public CGUIDesktop
{
public:
	CGUITestDesktop();
	~CGUITestDesktop();

	void InitDefaultIcons();
	CGUITestIconHandler* iconDefaultHandler;	// zakladni ovladac pro udalosti od icon

	void AddContextMenu();
	void AddStartMenu();

	bool push;

	CGUIMenu* startMenu;

	int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	int MouseRight(float x, float y, UINT mouseState, UINT keyState);
	int MouseMiddle(float x, float y, UINT mouseState, UINT keyState);
};


//////////////////////////////////////////////////////////////////////
// CGUITestIconHandler
//////////////////////////////////////////////////////////////////////

class CGUITestIconHandler : public CGUIEventProgram
{
	virtual void EventHandler(CGUIEvent *event);
public:
	CGUITestIconHandler() { icons = 0; startButtonDown=false;startButtonOff=false;};
	virtual ~CGUITestIconHandler() { SAFE_DELETE_ARRAY(icons); };

	typeID *icons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti
	bool startButtonDown;
	bool startButtonOff;
};

class CGUITestButton : public CGUIButton
{
public:
	CGUITestButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStaticText *_label=0, CGUIRectHost *_picture=0, char* _help=0);
	virtual void OnClick();
	MTRand mtr;
	float to;
};

class CGUITestButton2 : public CGUIButton
{
public:
	CGUITestButton2(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStaticText *_label=0, CGUIRectHost *_picture=0, char* _help=0);
	virtual void OnClick();
};

#endif
