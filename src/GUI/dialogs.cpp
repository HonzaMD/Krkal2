///////////////////////////////////////////////////////////////////////
//
// dialogs.cpp
//
// obsahuje dialogy: 
//		dialog s ok a cancel buttonem
//		dialog s editboxem
//		messagebox
//
// A: Petr Altman
//
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "dialogs.h"

#include "textedit.h"


const int CGUIDlgOkCancel::numEvents = 22 + 3;
const int CGUIDlgOkCancel::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,EVisibilityChanged,
													EMouseOver,EMouseButton,EAddElement,EDeleteAllElements,ESetAllElementsVisible,
													EUpdateScrollbars,EFocusChanged, EMouseWheel,
													EMinimize,EMaximize,ENormalize,EClose,EDeleted,
													EKeyboard,EKeyboardChar,EKeyboardUp,EMessage,
													EChangeTABElement,
													//nove pridany udalosti
													EOk,ECancel,EBadInput};

CGUIDlgOkCancel::CGUIDlgOkCancel(float _x, float _y, float _sx, float _sy, char *title_text, CGUIRectHost *_icon, bool resizeable, UINT resizer_color, int withBackground, int bgColor)
:CGUIStdWindow(_x,_y,_sx,_sy,title_text,_icon,resizeable,resizer_color,withBackground,bgColor)
{
	ok=NULL;
	cancel=NULL;

	IDbadElem=0;

	SetAvailableEvents(availableEvents,numEvents); //nastavi eventy
	//AcceptEvent(GetID(),EBadInput);
	AcceptEvent(GetID(),EBadInput);

	canceling = 0;

	SetOkCancelKey();
}

void CGUIDlgOkCancel::InitOkCancel(CGUIButton *_ok, CGUIButton *_cancel)
{

	if(_ok) ok=_ok;
	if(_cancel) cancel=_cancel;

	if(ok){
		ok->AcceptEvent(GetID(),EClicked);
	}

	if(cancel){
		cancel->AcceptEvent(GetID(),EClicked);
		cancel->AcceptEvent(GetID(),EStateChanged);
	}
}

CGUIDlgOkCancel::~CGUIDlgOkCancel()
{
}

void CGUIDlgOkCancel::BadInput(typeID id)
{
	if(!IDbadElem || id==IDbadElem)
	{
		badelfoc=0;
		IDbadElem = id;
		EventArise(EBadInput,0,id);
	}
}

void CGUIDlgOkCancel::EventHandler(CGUIEvent *event)
{

	if(!event) return;

	if(event->sender == IDbadElem && badelfoc){
		IDbadElem=0;
	}else
	if(event->sender == ok->GetID()) //ok button
	{
		if(event->eventID == EClicked  && !IDbadElem) 
		{
			if(Ok()) {closetype = 1; EventArise(EOk);}
		}
	}else
	if(event->sender == cancel->GetID()) //cancel button
	{
		if(event->eventID == EClicked) 
		{
			if(Cancel()) {closetype = 2; EventArise(ECancel);}
		}
		else
		if(event->eventID == EStateChanged)
		{
			int cs = cancel->GetState();
			
			if(cs == 2) 
				canceling = 1;//cancel je zamacklej
			else
			{
				if(IDbadElem){
					CGUIElement *el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(IDbadElem));
					if(el) SetFocusEl(el,0);
					IDbadElem = 0;
				}
				canceling = 0;
			}
		}
	}
	else
	if(event->eventID == EBadInput && !canceling)
	{
		CGUIElement *el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(event->pID));
		if(!front_focus && IsFocused() )
			if(el) SetFocusEl(el,0);
		badelfoc=1;
		//IDbadElem = 0;
	}

	CGUIStdWindow::EventHandler(event); //zavolam eventhander predka
}

int CGUIDlgOkCancel::Keyboard(UINT ch, UINT state) //klavesnice
{
	int st = CGUIStdWindow::Keyboard(ch,state);
	
	if(ch==13)
	{
		if(mainGUI->key_ctrl)
		{
			if(key_ctrlenter)
			{
				CGUIEditWindow *wnd = dynamic_cast<CGUIEditWindow*>(GetFocusEl());
				if(wnd) wnd->SendEOk();
				ok->Click();
				st=0;
			}
		}else{
			if(key_enter)
			{
				CGUIEditWindow *wnd = dynamic_cast<CGUIEditWindow*>(GetFocusEl());
				if(wnd) wnd->SendEOk();
				ok->Click();
				st=0;
			}
		}
	}else
	if(ch==27)
	{
		if(key_cancel)
		{
			if(Cancel()) {closetype = 2; EventArise(ECancel);}
			st=0;
		}
	}
	
	return st;
}

void CGUIDlgOkCancel::SetOkCancelKey(int enter, int cancel, int ctrlenter)
{
	key_enter = enter;
	key_cancel = cancel;
	key_ctrlenter = ctrlenter;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

CGUIDlgEditbox::CGUIDlgEditbox(float _x, float _y, float _sx, char *title_text, char *label_text, CGUIRectHost *_icon, UINT resizer_color, int withBackground, int bgColor)
:CGUIDlgOkCancel(_x,_y,_sx,120,title_text,_icon,false,resizer_color,withBackground,bgColor)
{

	title->SetButtons(1,1,0);

	float bwnsx,bwnsy;
	GetBackWindowNormalSize(bwnsx,bwnsy);

	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

	AddBackElem(new CGUIStaticText(label_text,font,10,10,STD_DLG_TEXT_COLOR));

	editbox = new CGUIEditWindow(10,30,bwnsx-20);
	editbox->SetTabOrder(1);
	editbox->SelectOnFocus();
	editbox->AcceptEvent(GetID(),EOk);
	AddBackElem(editbox);

	errtext=NULL;

	ok = new CGUIButton(floorf(bwnsx/2)-50-10,60,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(2);
	ok->SetMark(1);
	ok->AcceptEvent(GetID(),EClicked);
    AddBackElem(ok);

	cancel = new CGUIButton(floorf(bwnsx/2) + 10,60,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(3);
	cancel->SetMark(1);
	cancel->AcceptEvent(GetID(),EClicked);
    AddBackElem(cancel);

	SetFocusEl(editbox,0);

	InitOkCancel();
	SetOkCancelKey(1,1,1);

}

CGUIDlgEditbox::~CGUIDlgEditbox()
{
}

void CGUIDlgEditbox::ShowErrorMsg(char *msg)
{
	int chngsize=0;

	if(msg)
	{
		if(!errtext) chngsize=1;		
	}else
		if(errtext) chngsize=-1;

	if(errtext) {
		DeleteBackElem(errtext);errtext=NULL;
	}

	if(msg)
	{
		CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");
		errtext = new CGUIStaticText(msg,font,10,55,STD_DLG_TEXT_ERR_COLOR);
		AddBackElem(errtext);

		SetFocusEl(editbox,0);
	}

	if(chngsize)
	{
		float bwnsx,bwnsy;
		GetBackWindowNormalSize(bwnsx,bwnsy);

		float sz=20;

		if(chngsize==1)
		{
			ResizeRel(0,sz);
			SetBackWindowNormalSize(bwnsx,bwnsy+sz);

			ok->MoveRel(0,sz);
			cancel->MoveRel(0,sz);
		}else{
			ok->MoveRel(0,-sz);
			cancel->MoveRel(0,-sz);

			SetBackWindowNormalSize(bwnsx,bwnsy-sz);
			ResizeRel(0,-sz);
		}

	}
}

int CGUIDlgEditbox::BindEditBox(void *buf, int type, int size)
{
	return editbox->BindAttribute(buf,type,size,0);
}
int CGUIDlgEditbox::AcceptEditBoxEvent(typeID id, int event)
{
	return editbox->AcceptEvent(id,event);
}
int CGUIDlgEditbox::SyncEditBox(bool out)
{
	return editbox->Sync(out);
}

///////////////////////////////////////////////////////////////////////////////

CGUIMessageBox::CGUIMessageBox(float _x, float _y, char *title_text, char *message_text, DWORD message_color, CGUIRectHost *_icon, UINT resizer_color, int withBackground, int bgColor)
:CGUIDlgOkCancel(_x,_y,120,80,title_text,_icon,false,resizer_color,withBackground,bgColor)
{

	autogrowing=0;

	title->SetButtons(true,0,0);

	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

	message = new CGUIStaticText(message_text,font,10,10,message_color);
	AddBackElem(message);

	float mx,my;
	message->GetSize(mx,my);

	float xx = mx+20;
	if(xx<140) xx=140;
	SetWindowSizeVP(xx,10+my+10+25+10);
	SetBackWindowNormalSize(xx,10+my+10+25+10);

	ok = new CGUIButton(floorf(xx/2)-25,10+my+10,50,25,"GUI.But.Std","Ok");
	ok->SetTabOrder(1);
	ok->SetMark(1);
    AddBackElem(ok);

	SetFocusEl(ok,0);

	InitOkCancel();
	SetOkCancelKey(1,1,1);
}

CGUIMessageBox* GUIMessageBox(char *title_text, char *message_text, int modal, DWORD message_color)
{
	CGUIMessageBox *msg = GUIMessageBox(0,0,title_text,message_text,modal,message_color);
	msg->Center();
	return msg;
}

CGUIMessageBox* GUIMessageBox(float x, float y, char *title_text, char *message_text, int modal, DWORD message_color)
{
	CGUIMessageBox *msg = new CGUIMessageBox(x,y,title_text,message_text,message_color);
	msg->SetModal(modal);
	desktop->AddBackElem(msg);
	msg->FocusMe();
	return msg;
}

CGUIMessageBox* GUIMessageBoxOkCancel(char *title_text, char *message_text, int modal, DWORD message_color)
{
	CGUIMessageBox *msg = GUIMessageBox(0,0,title_text,message_text,modal,message_color);
	msg->Center();

	float msx,msy;
	float mx,my;
	msg->GetBackWindowNormalSize(msx,msy);

	if(msg->ok)
	{
		msg->ok->GetPos(mx,my);
		msg->ok->Move(floorf(msx/2) - 50 - 10,my);
	}
	else
	{
		mx=0;
		my=10;
	}

	msg->cancel = new CGUIButton(floorf(msx/2) + 10,my,50,25,"GUI.But.Std","Cancel");
	msg->cancel->SetTabOrder(2);
	msg->cancel->SetMark(1);
    msg->AddBackElem(msg->cancel);

	msg->InitOkCancel();
	msg->SetOkCancelKey(0,1,1);

	return msg;
}


CGUIMessageBoxYesNoCancel::CGUIMessageBoxYesNoCancel(float _x, float _y, char *title_text, char *message_text, DWORD message_color, CGUIRectHost *_icon, UINT resizer_color, int withBackground, int bgColor)
:CGUIDlgOkCancel(_x,_y,120,80,title_text,_icon,false,resizer_color,withBackground,bgColor)
{

	autogrowing=0;

	title->SetButtons(true,0,0);

	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

	message = new CGUIStaticText(message_text,font,10,10,message_color);
	AddBackElem(message);

	float mx,my;
	message->GetSize(mx,my);

	float xx = mx+20;
	if(xx<140) xx=140;
	SetWindowSizeVP(xx,10+my+10+25+10);
	SetBackWindowNormalSize(xx,10+my+10+25+10);

	ok = new CGUIButton(floorf(xx/2)-25-10-50,10+my+10,50,25,"GUI.But.Std","Ano");
	ok->SetTabOrder(1);
	ok->SetMark(1);
    AddBackElem(ok);


	no = new CGUIButton(floorf(xx/2)-25,10+my+10,50,25,"GUI.But.Std","Ne");
	no->SetTabOrder(2);
	no->SetMark(1);
	no->AcceptEvent(GetID(),EClicked);
    AddBackElem(no);

	cancel = new CGUIButton(floorf(xx/2)+25+10,10+my+10,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(3);
	cancel->SetMark(1);
    AddBackElem(cancel);

	SetFocusEl(ok,0);

	InitOkCancel();
	SetOkCancelKey(1,1,1);
}

void CGUIMessageBoxYesNoCancel::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	if(event->eventID == EClicked)
	{
		if(event->sender == no->GetID())
		{
			closetype = 3; CloseWindow();
		}
	}

	CGUIDlgOkCancel::EventHandler(event);
}

CGUIMessageBoxYesNoCancel* GUIMessageBoxYesNoCancel(char *title_text, char *message_text, int modal, DWORD message_color)
{
	CGUIMessageBoxYesNoCancel *msg = new CGUIMessageBoxYesNoCancel(0,0,title_text,message_text,message_color);
	msg->SetModal(modal);
	desktop->AddBackElem(msg);
	msg->Center();
	msg->FocusMe();
	return msg;
}
