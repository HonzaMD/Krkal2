///////////////////////////////////////////////////////////////////////
//
// dialogs.h
//
// obsahuje dialogy: 
//		dialog s ok a cancel buttonem
//		dialog s editboxem
//		messagebox
//
// A: Petr Altman
//
///////////////////////////////////////////////////////////////////////

#ifndef DIALOGS_H
#define DIALOGS_H

#include "widget.h"

///////////////////////////////////////////////////////////////////////
// dialog s Ok a Cancel buttonem

class CGUIDlgOkCancel : public CGUIStdWindow {

public:
	CGUIDlgOkCancel(float _x, float _y, float _sx, float _sy, char *title_text, CGUIRectHost *_icon=0, bool resizeable=true, UINT resizer_color=STD_DLG_BORDER_COLOR, int withBackground=1, int bgColor=STD_DLG_BGCOLOR_COLOR);
	virtual ~CGUIDlgOkCancel();

	void InitOkCancel(CGUIButton *ok=NULL, CGUIButton *cancel=NULL);

	void SetOkCancelKey(int enter=0, int cancel=1, int ctrlenter=1);

	static const int numEvents;
	static const int availableEvents[];

	void BadInput(typeID IDbadElem);

	CGUIButton *ok, *cancel;

	int GetCloseType(){return closetype;} //0=unknown, 1=ok, 2=cancel

protected:

	int closetype; //0=unknown, 1=ok, 2=cancel

	virtual int Keyboard(UINT ch, UINT state); //klavesnice
	virtual int Ok(){return 1;}
	virtual int Cancel(){CloseWindow();return 1;}

	virtual void EventHandler(CGUIEvent *event);

	typeID IDbadElem;
	int badelfoc;
	int canceling;

	int key_enter,key_cancel,key_ctrlenter;
};

///////////////////////////////////////////////////////////////////////
//dialog s editboxem

class CGUIDlgEditbox : public CGUIDlgOkCancel {

public:
	CGUIDlgEditbox(float _x, float _y, float _sx, char *title_text, char *label_text , CGUIRectHost *_icon=0, UINT resizer_color=STD_DLG_BORDER_COLOR, int withBackground=1, int bgColor=STD_DLG_BGCOLOR_COLOR);
	~CGUIDlgEditbox();

	void ShowErrorMsg(char *msg=0);

	int BindEditBox(void *buf, int type, int size=1);
	int AcceptEditBoxEvent(typeID id, int event=EOk);
	int SyncEditBox(bool out=true);

	class CGUIEditWindow* GetEditBox(){return editbox;}
    
protected:

	//virtual void EventHandler(CGUIEvent *event);

	CGUIStaticText *errtext;
	class CGUIEditWindow *editbox;
};

///////////////////////////////////////////////////////////////////////
// message box

class CGUIMessageBox : public CGUIDlgOkCancel {

public:
	CGUIMessageBox(float _x, float _y, char *title_text, char *message_text, DWORD message_color=STD_DLG_TEXT_COLOR, CGUIRectHost *_icon=0, UINT resizer_color=STD_DLG_BORDER_COLOR, int withBackground=1, int bgColor=STD_DLG_BGCOLOR_COLOR);

	virtual int Ok(){CloseWindow();return 1;}

protected:

	CGUIStaticText *message;
};

CGUIMessageBox* GUIMessageBox(char *title_text, char *message_text, int modal=0, DWORD message_color=STD_DLG_TEXT_COLOR); //vytvori centrovany msgbox
CGUIMessageBox* GUIMessageBox(float x, float y, char *title_text, char *message_text, int modal=0, DWORD message_color=STD_DLG_TEXT_COLOR); //vytvori msgbox

CGUIMessageBox* GUIMessageBoxOkCancel(char *title_text, char *message_text, int modal=0, DWORD message_color=STD_DLG_TEXT_COLOR); //vytvori centrovany msgbox s cudlitky ok a cancel


class CGUIMessageBoxYesNoCancel : public CGUIDlgOkCancel 
{

public:
	CGUIMessageBoxYesNoCancel(float _x, float _y, char *title_text, char *message_text, DWORD message_color=STD_DLG_TEXT_COLOR, CGUIRectHost *_icon=0, UINT resizer_color=STD_DLG_BORDER_COLOR, int withBackground=1, int bgColor=STD_DLG_BGCOLOR_COLOR);

	virtual int Ok(){CloseWindow();return 1;}

	CGUIButton *no;

	/*
	int GetCloseType(){return closetype;} //0=unknown, 1=yes, 2=cancel, 3=no
	*/
protected:
	virtual void EventHandler(CGUIEvent *event);
	CGUIStaticText *message;
};
CGUIMessageBoxYesNoCancel* GUIMessageBoxYesNoCancel(char *title_text, char *message_text, int modal=0, DWORD message_color=STD_DLG_TEXT_COLOR); //vytvori centrovany msgbox s cudlitky ano/ne/storno


#endif