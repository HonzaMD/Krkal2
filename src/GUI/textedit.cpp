//////////////////////////////////////////////////////////////////////
//
// TextEdit.cpp
//
// editor textu - propojeni DXtextu s GUI
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "textedit.h"

#include "dx.h"
#include "refmgr.h"
#include "dxhighlight.h"
#include "dxfont.h"

#include <ctype.h>

#define DEFAULT_EDITFONT "DX.F.Times.12" //defaultni font

#define XMARGIN_WIN 4 //x-okraj, kdyz je to okno s rameckem
#define YMARGIN_WIN 0 //y-okraj, kdyz je to okno s rameckem
#define XMARGIN 4 //x-okraj, kdyz je to okno bez ramecku (editbox)
#define YMARGIN 2 //y-okraj, kdyz je to okno bez ramecku (editbox)


//eventy, ktere okno akceptuje
const int CGUIEditWindow::numEvents = 21 + 3;
const int CGUIEditWindow::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,EVisibilityChanged,
													EMouseOver,EMouseButton,EAddElement,EDeleteAllElements,ESetAllElementsVisible,
													EUpdateScrollbars,EFocusChanged, EMouseWheel,
													EMinimize,EMaximize,ENormalize,EClose,EDeleted,
													EKeyboard,EKeyboardChar,EKeyboardUp,EMessage,
													//nove pridany udalosti
													EOk,ECancel,EChangeTABElement
};

//////////////////////////////////////////////////////////////////////////////////////////////////

//konstruktory

//okno s rameckem
CGUIEditWindow::CGUIEditWindow(float _x, float _y, float _sx, float _sy, int wordwrap, int noscrollbars, char *title_text, CGUIRectHost *_icon, UINT resizer_color, int textColor, int bgColor, CDXFont *font)
:CGUIStdWindow(_x,_y,_sx,_sy,bgColor)
{
	if(!font)
	{
		DWORD col;
		DXHighLightFormat->GetDefault(font,col);
		textColor=col;
	}

	InitEditWindow(_x,_y,_sx,_sy,font,textColor,bgColor,wordwrap,noscrollbars,1,XMARGIN_WIN,YMARGIN_WIN,title_text,_icon,resizer_color);
	allowtab = 1;
}

//viceradkovy editbox
CGUIEditWindow::CGUIEditWindow(float _x, float _y, float _sx, float _sy, int wordwrap, int noscrollbars, int textColor, int bgColor, char *fontname)
:CGUIStdWindow(_x,_y,_sx,_sy,bgColor)
{
	onWindowPanel = false;
	topOnFocus = false;

	CDXFont *font = NULL;
	if(fontname) font = dynamic_cast<CDXFont*>(RefMgr->Find(fontname));

	InitEditWindow(_x,_y,_sx,_sy,0,textColor,bgColor,wordwrap,noscrollbars,1,XMARGIN,YMARGIN);
}

//jednoradkovy editbox s autovatickym vypoctem vysky
CGUIEditWindow::CGUIEditWindow(float _x, float _y, float _sx, int textColor, int bgColor, char *fontname)
:CGUIStdWindow(_x,_y,_sx,5,bgColor)
{
	onWindowPanel = false;
	topOnFocus = false;

	CDXFont *font = NULL;
	if(fontname) font = dynamic_cast<CDXFont*>(RefMgr->Find(fontname));

	InitEditWindow(_x,_y,_sx,-1,font,textColor,bgColor,0,1,0,XMARGIN,YMARGIN);
}

//intiace - vola se ze vsech konstruktoru
void CGUIEditWindow::InitEditWindow(float _x, float _y, float _sx, float _sy, CDXFont *dxfont, int textColor, int bgColor, int wordwrap,int noscrollbars, int _multiline, int xmargin, int ymargin, char *title_text, CGUIRectHost *_icon, UINT resizer_color)
{
	RemoveFromTrash(); //kvuli vyjimkam - viz. GUI

	highlight = 0; //bez highlitovani
	highlighter = 0;

	selectonfocus = 0;
	changed = 0;

	mousesel=0; //mysi se ted nic neoznacuje

	SetAvailableEvents(availableEvents,numEvents); //nastavi eventy

	multiline = _multiline; //viceradkovy editor?

	defaultfont=0; //defaultni font

	allowtab = 0; //zakazu psat tab

	readonly = 0; //neni readonly

	if(title_text)
	{	//pridam titulek, ikonu, resizery a scrolbary

		AddResizers(resizer_color,styleSet->Get("Resizers")); //pridam resizery
		AddTitle(title_text,_icon,STD_TITLE_HEIGHT); //pridam titulek

		//nastavi velikost viewportu
		CGUIStyle &style = *(styleSet->Get("Resizers"));
		SetViewPortBackWnd(style[1], style[0]+STD_TITLE_HEIGHT, _sx - 2*style[1], _sy - 2*style[0]-STD_TITLE_HEIGHT);

		//nastavi limity velikosti
		SetLimitSizes(4*STD_TITLE_HEIGHT,3*STD_SCROLLBAR_WIDTH+STD_TITLE_HEIGHT+2*style[0]);

		if(!noscrollbars)
		{
			//pridam vertikalni scrolbar
			CGUIScrollBar* dual = AddSimpleScrollbar(Vertical, sx-style[1]-STD_SCROLLBAR_WIDTH, style[0]+STD_TITLE_HEIGHT, STD_SCROLLBAR_WIDTH, sy - 2*style[0]-STD_TITLE_HEIGHT, STD_SCROLLBAR_SHIFT, 0, STD_SCROLLBAR_SLIDER_RESIZE,0,0,0);

			if(!wordwrap) //budu zalamovat radky?
			{
				//nebudu - pridam horizontalni scrolbar
				AddSimpleScrollbar(Horizontal, style[1], sy-style[0]-STD_SCROLLBAR_WIDTH, STD_SCROLLBAR_WIDTH, sx - 2*style[1], STD_SCROLLBAR_SHIFT, 0, STD_SCROLLBAR_SLIDER_RESIZE,dual,0,0);
			}
		}

	}else{
		//bez titulku, ramecku
		if(!noscrollbars)
		{
			//pridam vertikalni scrolbar
			CGUIScrollBar* dual = AddSimpleScrollbar(Vertical, sx-STD_SCROLLBAR_WIDTH, 0, STD_SCROLLBAR_WIDTH, sy, STD_SCROLLBAR_SHIFT, 0, STD_SCROLLBAR_SLIDER_RESIZE,0,0,0);

			if(!wordwrap) //budu zalamovat radky?
			{
				//nebudu - pridam horizontalni scrolbar
				AddSimpleScrollbar(Horizontal, 0, sy-STD_SCROLLBAR_WIDTH, STD_SCROLLBAR_WIDTH, sx, STD_SCROLLBAR_SHIFT, 0, STD_SCROLLBAR_SLIDER_RESIZE,dual,0,0);
			}
		}
	}

	//chci dostaveat zpravy od mysi a timeru
	MsgAcceptConsume(MsgMouseL|MsgMouseOver);
	MsgAcceptConsume(MsgTimer);

	text = NULL;
	blttext = NULL;

	focused=0; //nejsem focusovany

	if(!dxfont)
	{   //neni zadam defaultni font
		dxfont = (CDXFont*) RefMgr->Find(DEFAULT_EDITFONT); //najdu defaultfont
		if(!dxfont)
		{
			//chyba - font neexistuje
			throw CExc(eGUI,E_BAD_ARGUMENTS,"Font not exist!");
		}
	}

	dxfont->AddRef(); //zvysim pocet referenci
	defaultfont = dxfont;

	text = new CFormatedText(dxfont,textColor,wordwrap); //vytvori formatovany text

	CTextBlock *b;
	b = new CTextBlock("");
	text->AddTextBlockToEnd(b); //pridam jeden prazdny blok
	
	blttext = new CBltFormatedText(text,xmargin,ymargin); //vytvori zobrazovac formatovaneho textu

	back->AddChild(blttext); //pridam zobrazovac dovnitr okna

	text->CreateCursor(); //vytvori textovy kurzor
	blttext->SetCursor(1);

	if(_sy<0) //pokud _sy<0 -> spocitam vysku editboxu
	{
		_sy = (float)blttext->CmpEditBoxHeight();
		Resize(_sx,_sy);
	}

	float xx,yy;
	GetVPSize(xx,yy); //zjisti velikost viewportu
	blttext->SetWidth((int)xx); //nastavi sirku textu - pro zalamovani
	SetTextSize(); //opravi scrollbary

	ScrollToCursor();

	blttext->BlinkCursor(0); //zmizi kurzor

	cursorblinktimerID=0;
	mousescrolltimerID=0;
	mousescroll=0;

	AddToTrash(); //kvuli vyjimkam - viz. GUI

}

///////////////////////////////////////////////////////////////////////////////////////////////
//destruktor

CGUIEditWindow::~CGUIEditWindow()
{

	SAFE_DELETE(highlighter);
	SAFE_RELEASE(defaultfont);

	if(blttext)
	{
		back->DeleteChild(blttext);
		blttext = NULL;
	}
	SAFE_DELETE(text);

	timerGUI->DeleteAllRequests(this);

}

///////////////////////////////////////////////////////////////////////////////////////////////

int CGUIEditWindow::SetHighlight(int _highlight)
{
	if(highlight!=_highlight)
	{
		if(_highlight)
		{
			highlighter = new CDXHighlighter(text);
		}
		highlight=_highlight;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void CGUIEditWindow::Resize(float _sx, float _sy)
{
	float xx,yy;

	CGUIStdWindow::Resize(_sx,_sy); //resizne okno
	GetVPSize(xx,yy);

	blttext->SetWidth((int)xx); //nastavi sirku textu

	int xs,ys;

	blttext->CmpTextSize(xs,ys); //spocita delku textu

	SetBackWindowNormalSize((float)xs,(float)ys);
	
}

///////////////////////////////////////////////////////////////////////////////////////////////

//reakce na klavesnici
int CGUIEditWindow::Keyboard(UINT ch, UINT state)
{
	int dy;

	switch(ch)
	{
	case VK_ESCAPE:
		EventArise(ECancel);
		break;
	case VK_RIGHT: //sipka doprava
		text->DoneUndo(); //uzavru posledni krok undo
		if(mainGUI->key_shift){ //se siftem oznacuju
			if(!text->GetMarkStart()) text->SetMarkStart();		
		} else text->UnMark();
		
		if(mainGUI->key_ctrl) //s controlem lezu po slovech
			text->GetCursor()->MoveWordRight();
		else
			text->GetCursor()->MoveRight();

		if(mainGUI->key_shift)
			text->SetMarkEnd();

		blttext->BlinkCursor(1); //zobrazi kurzor (blika)
		ScrollToCursor(); // nascroluje, aby byl kurzor videt
		blttext->SetNeedRecalc();
		CursorMoved();
		break;
	case VK_LEFT: //sipka doleve
		text->DoneUndo(); //uzavru posledni krok undo
		if(mainGUI->key_shift){ //se shiftem oznacuju
			if(!text->GetMarkStart()) text->SetMarkStart();
		}else text->UnMark();

		if(mainGUI->key_ctrl) //s crtl lezu po slovech
			text->GetCursor()->MoveWordLeft();
		else
			text->GetCursor()->MoveLeft();

		if(mainGUI->key_shift)
			text->SetMarkEnd();

		blttext->BlinkCursor(1);
		ScrollToCursor(); //nascroluje, aby byl kurzor videt
		blttext->SetNeedRecalc();
		CursorMoved();
		break;
	case VK_UP: //sipka nahoru
		text->DoneUndo(); //uzavru posledni krok undo
		if(multiline) //viceradkovy editor?
		{
			if(mainGUI->key_shift){ //se shiftem oznacuju
				if(!text->GetMarkStart()) text->SetMarkStart();		
			} else text->UnMark();
			text->GetCursor()->MoveUp(); //lezu nahoru
			if(mainGUI->key_shift)
				text->SetMarkEnd();
			blttext->BlinkCursor(1);
			ScrollToCursor(); //nascroluje, aby byl kurzor videt
			blttext->SetNeedRecalc();
		}else{
			if(changed) {EventArise(EOk); changed=0;}
			EventArise(EChangeTABElement,-1); //presune focus na predchozi element
		}
		CursorMoved();
		break;
	case VK_DOWN: //sipka dolu
		text->DoneUndo(); //uzavru posledni krok undo
		if(multiline)
		{
			if(mainGUI->key_shift){
				if(!text->GetMarkStart()) text->SetMarkStart();		
			} else text->UnMark();
			text->GetCursor()->MoveDown(); //lezu dolu
			if(mainGUI->key_shift) text->SetMarkEnd();
			blttext->BlinkCursor(1);
			ScrollToCursor(); //nascroluje, aby byl kurzor videt
			blttext->SetNeedRecalc();
		}else{
			if(changed) {EventArise(EOk); changed=0;}
			EventArise(EChangeTABElement,1); //presune focus na dalsi element
		}
		CursorMoved();
		break;
	case VK_PRIOR:  //PageUp
		text->DoneUndo();
		if(mainGUI->key_shift){
			if(!text->GetMarkStart()) text->SetMarkStart();		
		} else text->UnMark();
		dy = text->GetCursor()->MovePageUp((int)vp_sy);
		if(mainGUI->key_shift) text->SetMarkEnd();
		blttext->BlinkCursor(1);
		SetWindowPositionVP(0,(float)-dy);
		ScrollToCursor();
		blttext->SetNeedRecalc();
		CursorMoved();
		break;
	case VK_NEXT: //PageDown
		text->DoneUndo();
		if(mainGUI->key_shift){
			if(!text->GetMarkStart()) text->SetMarkStart();		
		} else text->UnMark();
		dy = text->GetCursor()->MovePageDown((int)vp_sy);
		if(mainGUI->key_shift) text->SetMarkEnd();
		blttext->BlinkCursor(1);
		SetWindowPositionVP(0,(float)dy);
		ScrollToCursor();
		blttext->SetNeedRecalc();
		CursorMoved();
		break;
	case VK_HOME: //home
		text->DoneUndo();
		if(mainGUI->key_shift){
			if(!text->GetMarkStart()) text->SetMarkStart();		
		} else text->UnMark();

		if(mainGUI->key_ctrl)
			text->GetCursor()->MoveTextHome(); //ctrl+home -> presune cursor na zacatek textu
		else
			text->GetCursor()->MoveHome();
		if(mainGUI->key_shift) text->SetMarkEnd();
		blttext->BlinkCursor(1);
		ScrollToCursor();
		blttext->SetNeedRecalc();
		CursorMoved();
		break;
	case VK_END: //end
		text->DoneUndo();
		if(mainGUI->key_shift){
			if(!text->GetMarkStart()) text->SetMarkStart();		
		} else text->UnMark();

		if(mainGUI->key_ctrl) //ctrl+end -> presuna cursor na konec textu
			text->GetCursor()->MoveTextEnd();
		else
		text->GetCursor()->MoveEnd();
		if(mainGUI->key_shift) text->SetMarkEnd();
		blttext->BlinkCursor(1);
		ScrollToCursor();
		blttext->SetNeedRecalc();
		CursorMoved();
		break;
	case VK_DELETE:
		if(!readonly){
			if(mainGUI->key_shift) Cut();
			else
			{
				//delete
				if(text->IsMarked())
					text->DeleteSelectedText(); //smazu oznaceny text
				else
					text->GetCursor()->Delete(); //mazu znak
				if(highlight)
				{//highlituju
					highlighter->HighlightFromBlock(text->GetCursor()->GetBlock());
					text->GetCursor()->CalcFormat();		
				}
				blttext->BlinkCursor(1);
				SetTextSize();
				ScrollToCursor();
				blttext->SetNeedRecalc();

				changed = 1; ChangedEvent();		
				CursorMoved();
			}
		}
		break;
	case VK_BACK:
		if(!readonly){	//backspace
			if(text->IsMarked())
				text->DeleteSelectedText(); //smazu oznaceny text
			else
				text->GetCursor()->Backspace(); //smazu znak
			if(highlight)
			{//highlightuju
				highlighter->HighlightFromBlock(text->GetCursor()->GetBlock());
				text->GetCursor()->CalcFormat();		
			}
			blttext->BlinkCursor(1);
			SetTextSize();
			ScrollToCursor();
			blttext->SetNeedRecalc();

			changed = 1; ChangedEvent();
			CursorMoved();
		}
		break;
	case VK_RETURN:
		if(multiline&&!readonly){ //return
			CTextBlock *b=text->GetCursor()->GetBlock();
			if(text->IsMarked())
				text->DeleteSelectedText(1,1); //smazu oznaceny text

			text->GetCursor()->Return(); //vlozim enter

			text->DoIdent(); //vlozim taby na zacatek radky

			if(highlight) 
			{//highlightuju
				highlighter->HighlightFromBlock(b);
				text->GetCursor()->CalcFormat();		
			}

			blttext->BlinkCursor(1);
			SetTextSize();
			ScrollToCursor();
			blttext->SetNeedRecalc();

			changed = 1; ChangedEvent();
			CursorMoved();
		}else
		{ //u jednoradkovyho boxu - presune focus na dalsi element
			text->DoneUndo();
			changed = 0;
			EventArise(EOk);
			EventArise(EChangeTABElement,1); //posunu focus na dalsi element
		}
		break;
	case VK_INSERT:
		if(mainGUI->key_shift)//paste
		{
			Paste();
		}else
		if(mainGUI->key_ctrl)//copy
		{
			Copy();
		}else{//change insert mode	
			text->GetCursor()->ChangeInsertMode();
			blttext->BlinkCursor(1);
			blttext->SetNeedRecalc();
			CursorMoved();
		}
		break;
	case 'A':
		if(mainGUI->key_ctrl) //select all
		{
			text->UnMark();
			text->GetCursor()->MoveTextHome();
			text->SetMarkStart();
			text->GetCursor()->MoveTextEnd();
			text->SetMarkEnd();
			blttext->BlinkCursor(1);
			ScrollToCursor();
			blttext->SetNeedRecalc();
			CursorMoved();
		}
		break;		
	case 'C': //ctrl+C -> copy to clipboard
		if(mainGUI->key_ctrl)
		{
			Copy();
		}
		break;
	case 'V': //ctrl+V ->paste from clipboard
		if(mainGUI->key_ctrl)
		{
			Paste();
		}
		break;
	case 'X': //ctrl+X -> cut
		if(mainGUI->key_ctrl){
			Cut();
		}
		break;
	case 'Z':
		if(mainGUI->key_ctrl) //undo
		{
			Undo();
		}
		break;
	case 'Y': 
		if(mainGUI->key_ctrl) //redo
		{
			Redo();
		}
		break;
	case VK_TAB:
		if(allowtab && !mainGUI->key_ctrl && !readonly){
			CTextCursor *cur=text->GetCursor();
			if(text->IsMarked())
			{
				int insm=-1;

				insm = cur->GetInsertMode();
				cur->SetInsertMode(0);

				CTextBlock *b=text->FindFirstMark()->GetBlock();

				text->IdentBlock(mainGUI->key_shift);

				if(highlight)
				{//highligtuju
					highlighter->HighlightFromBlock(b);
					cur->CalcFormat();		
				}
				cur->SetInsertMode(insm);

				blttext->BlinkCursor(1);
				SetTextSize();
				ScrollToCursor();
				blttext->SetNeedRecalc();

				changed = 1; ChangedEvent();
			}else
			if(!mainGUI->key_shift){
				cur->InsertChar('\t'); //vlozi tab
				if(highlight)
				{//highligtuju
					highlighter->HighlightFromBlock(cur->GetBlock());
					cur->CalcFormat();		
				}
				blttext->BlinkCursor(1);
				SetTextSize();
				ScrollToCursor();
				blttext->SetNeedRecalc();

				changed = 1; ChangedEvent();
			}
			CursorMoved();
		}
		break;

/*	case 'H':
		if(mainGUI->key_ctrl) //vyhighligtuje cely text - pro testovaci ucely
		{
			if(highlight&&highlighter)
			{
				highlighter->Highlight();
				text->GetCursor()->CalcFormat();
				blttext->BlinkCursor(1);
				SetTextSize();
				blttext->SetNeedRecalc();
			}
		}*/
	}

	return 0;
}

//prelozene znaky - psani
int CGUIEditWindow::KeyboardChar(UINT ch, UINT state)
{
	if(readonly) return 0;

	if(mainGUI->key_ctrl)
		return 0;

	if(ch==VK_TAB) return 0;
	
	if(isprint(ch)/*||ch==VK_TAB*/)
	{
		CTextCursor *cur=text->GetCursor();
		int insm=-1;
		if(text->IsMarked())
		{
			text->DeleteSelectedText(1,1); //smazu oznaceny text
			insm = cur->GetInsertMode();
			cur->SetInsertMode(0);
		}
		cur->InsertChar(ch); //vlozi znak

		if(insm!=-1)
			cur->SetInsertMode(insm);
		if(highlight)
		{//highligtuju
			highlighter->HighlightFromBlock(cur->GetBlock());
			cur->CalcFormat();		
		}
		blttext->BlinkCursor(1);
		SetTextSize();
		ScrollToCursor();
		blttext->SetNeedRecalc();

		changed = 1; ChangedEvent();
		CursorMoved();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////

//timer - blikani kurzoru & scrolovani mysi
int CGUIEditWindow::TimerImpulse(typeID timerID, float time)
{

	if(timerID==cursorblinktimerID)
	{
		if(focused) //blikani kurzoru
		{
			blttext->BlinkCursor();
			cursorblinktimerID = timerGUI->AddRequest(this,.5);
		}
	}
	else
	if(timerID==mousescrolltimerID)
	{
		mousescrolltimerID=0; //scrolovani mysi
		if(mousescroll)
		{
				text->DoneUndo();
				float x=(float)mousescx,y=(float)mousescy;
				InnerCoords(x,y);
				TransformCoordsFrontToBack(x,y);
				int xx=(int)x,yy=(int)y;
				blttext->TransformCoords(xx,yy);
				text->GetCursor()->MoveXY(xx,yy);
				text->SetMarkEnd();
				ScrollToCursor();
				blttext->SetNeedRecalc();

				SetMouseScrollTimer();
				CursorMoved();
		}
	}
	return 0;
}

void CGUIEditWindow::SetMouseScrollTimer()
{
	if(!mousescrolltimerID)
	{
		mousescrolltimerID=timerGUI->AddRequest(this,.05f);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////

//zmenil se focus
int CGUIEditWindow::FocusChanged(UINT focus, typeID dualID)
{
	if(focused!=focus) 
	{
		text->DoneUndo();

		focused = focus;

		if(selectonfocus && focused)
		{
			text->UnMark();
			text->GetCursor()->MoveTextEnd();
			text->SetMarkStart();
			text->GetCursor()->MoveTextHome();
			text->SetMarkEnd();
			ScrollToCursor();
		}
		
		blttext->BlinkCursor(focused);
		blttext->SetShowMark(focused);

		if(focused)
		{
			if(!cursorblinktimerID )
				cursorblinktimerID = timerGUI->AddRequest(this,.5);
		}else{
			timerGUI->DeleteRequest(cursorblinktimerID);cursorblinktimerID=0;
			timerGUI->DeleteRequest(mousescrolltimerID);mousescrolltimerID=0;
		}

		if(!focus && changed )
		{
			EventArise(EOk);
			changed = 0;
		}
	}

	return CGUIStdWindow::FocusChanged(focus,dualID);
}

///////////////////////////////////////////////////////////////////////////////////////////////

//nascroluje, aby byl kurzor videt
void CGUIEditWindow::ScrollToCursor(int center)
{
	int cx,cy,lnheight;
	float sx,sy;
	blttext->CmpCursorPos(cx,cy,lnheight);

	GetVPSize(sx,sy);

	cy-=(int)vp_y;
	cx-=(int)vp_x;

	int sc=0;

	if(center){
		sc=1;
		sx = (float)floor(cx-sx/2+.5f);
		sy = (float)floor(cy-sy/2+.5f);
	}else{

	if(cy<0){
		sc = 1;
		sy = (float)cy;
	}
	else
	if(cy+lnheight>sy)
	{
		sc = 1; 
		sy = (float)cy+lnheight-sy;
		
	}
	else sy=0;

	if(cx<20){
		sc = 1;
		sx = (float)cx-20;
	}
	else
	if(cx>sx-20)
	{
		sc = 1; 
		sx = (float)cx-(sx-20);
		
	}else
		sx=0;

	}
	if(sc) 
		SetWindowPositionVP(sx,sy);
}

//upravi scrolbary podle delky textu
void CGUIEditWindow::SetTextSize()
{
	float xx,yy;
	int xs,ys;

	GetVPSize(xx,yy);
	blttext->CmpTextSize(xs,ys);

	SetBackWindowNormalSize((float)xs,(float)ys);
	
}

///////////////////////////////////////////////////////////////////////////////////////////////

//levy tlacitko mysi
int CGUIEditWindow::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{
	mousescroll=0;
	if(CGUIStdWindow::MouseLeft(x,y,mouseState,keyState))
	{
		if(!focused) return 1;

		if(!mouseState && !mousesel) return 1;

		InnerCoords(x,y);
		if(TransformCoordsFrontToBack(x,y))
		{
			text->DoneUndo();

			int xx,yy;
			xx=(int)x;yy=(int)y;
			blttext->TransformCoords(xx,yy);

			if(mouseState && mainGUI->key_shift && !text->GetMarkStart() )
			{
				text->UnMark();
				text->SetMarkStart();
			}

			text->GetCursor()->MoveXY(xx,yy); //nastavi kurzor na pozici mysi

			if(mouseState)
			{
				if(!mainGUI->key_shift)
				{
					text->UnMark();
					text->SetMarkStart();
				}else
					text->SetMarkEnd();
				mousesel=1;
			}else
			{
				text->SetMarkEnd();
				mousesel=0;
			}

			blttext->BlinkCursor(1);
			if(!mouseState) ScrollToCursor();
			blttext->SetNeedRecalc();
			CursorMoved();
		}else
			mousesel=0;
		return 1;
	}
	return 0;
}

//levy tlacitko mysi mimo okno - zrusim oznacovani
int CGUIEditWindow::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	mousescroll=0;
	if(CGUIStdWindow::MouseLeftFocus(x,y,mouseState,keyState,ID))
	{
		mousesel=0;
		return 1;
	}
	return 0;
}

//pohyb mysi nad oknem
int CGUIEditWindow::MouseOver(float x, float y, UINT over,UINT state)
{
	mousescroll=0;
	if(CGUIStdWindow::MouseOver(x,y,over,state))
	{
		if(!focused) return 1;
		if(mousesel&&text->GetMarkStart())
		{
			mousescx=(int)x;mousescy=(int)y;
			InnerCoords(x,y);
			if(TransformCoordsFrontToBack(x,y))
			{
				text->DoneUndo();
				int xx,yy;
				xx=(int)x;yy=(int)y;
				blttext->TransformCoords(xx,yy);
				text->GetCursor()->MoveXY(xx,yy);
				text->SetMarkEnd();
				ScrollToCursor();
				blttext->SetNeedRecalc();		
				CursorMoved();
			}else
			{				
				mousescroll=1;
				SetMouseScrollTimer();
			}
		}
		return 1;
	}
	return 0;
}

//pohym mysi mimo okno
int CGUIEditWindow::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	mousescroll=0;
	if(CGUIStdWindow::MouseOverFocus(x,y,over,state,ID))
	{
		if(mousesel&&text->GetMarkStart())
		{
			//nastavim scrolovani
			mousescx=(int)x;mousescy=(int)y;
			mousescroll=1; 
			SetMouseScrollTimer();
		}
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//prenos dat

//prenese text z editboxu
int CGUIEditWindow::GetParametr(void *value, int type, int size, int which)
{
	switch(which)
	{
	case 0:
		switch(type)
		{
		case dtString: //chci string
			return text->GetText((char*)value,size); //vracim 0, kdyz ok, jinak delku textu (vcetne 0)
		case dtChar: //chci pole charu
			return text->GetText((char*)value,size,0,0); //vracim 0, kdyz ok, jinak delku textu
		case dtInt: //chci int
			return text->GetTextInt(*(int*)value);
			break;
		case dtDouble: //chci double
			return text->GetTextDouble(*(double*)value);
			break;
		}
		break;
	}
	return 1;
}

//prenos dat do editboxu
int CGUIEditWindow::SetParametr(void *value, int type, int size, int which)
{
	char *s;int dels=0;
	int sz=-1;
	switch(which)
	{
	case 0:
		switch(type)
		{
		case dtString:
		case dtChar: //dostal jsem string
			s=(char*)value;
			sz=size;
			break;
		case dtInt: //dostal jsem int
			s  = new char[100]; dels=1;
			_snprintf(s,99,"%i",*(int*)value);
			break;
		case dtDouble: //dostal jsem double
			s  = new char[100]; dels=1;
			_snprintf(s,99,"%.10g",*(double*)value);
			break;
		default:
			return 1;
		}

		text->UnMark();
		text->SetText(s,sz); 
		if(dels) delete[] s;
		text->GetCursor()->MoveTextHome();
		
		if(highlight&&highlighter)
		{
			highlighter->Highlight();
			text->GetCursor()->CalcFormat();
		}

		SetTextSize();
		ScrollToCursor();
		blttext->SetNeedRecalc();

		changed = 1; ChangedEvent();
		CursorMoved();
		break;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void CGUIEditWindow::BadInput()
{
	text->UnMark();
	text->GetCursor()->MoveTextEnd();
	text->SetMarkStart();
	text->GetCursor()->MoveTextHome();
	text->SetMarkEnd();
	ScrollToCursor();

	changed=1; ChangedEvent();
}

void CGUIEditWindow::SendEOk()
{
	EventArise(EOk);
}

///////////////////////////////////////////////////////////////////////////////////////////////

//zmenil se rodic - nekdo me dal do okna
void CGUIEditWindow::ParentNotification()
{
	if(parent)
	{
		CGUIEventSupport *es = dynamic_cast<CGUIEventSupport*>(parent);
		if(es&&!allowtab) AcceptEvent(es->GetID(),EChangeTABElement);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////


int CGUIEditWindow::Undo()
{
	if(readonly) return 0;
	
	text->Undo(highlighter);
	blttext->BlinkCursor(1);
	SetTextSize();
	ScrollToCursor();
	blttext->SetNeedRecalc();

	changed = 1; ChangedEvent();
	CursorMoved();
	
	return 1;
}

int CGUIEditWindow::Redo()
{
	if(readonly) return 0;
		
	text->Redo(highlighter);
	blttext->BlinkCursor(1);
	SetTextSize();
	ScrollToCursor();
	blttext->SetNeedRecalc();

	changed = 1; ChangedEvent();
	CursorMoved();

	return 1;
}

int CGUIEditWindow::Copy()
{
	text->CopyToClipboard(); //copy
	ClipboardEvent();
	return 1;
}

int CGUIEditWindow::Paste()
{
	if(readonly) return 0;

	if(text->IsMarked())
		text->DeleteSelectedText(1,1); //smazu oznaceny text	
	text->PasteFromClipboard(multiline,highlighter); //paste&highlight
	blttext->BlinkCursor(1);
	SetTextSize();
	ScrollToCursor();
	blttext->SetNeedRecalc();

	changed = 1; ChangedEvent();
	CursorMoved();

	return 1;
}

int CGUIEditWindow::Cut()
{
	if(readonly) return 0;
	if(!text->IsMarked()){
		text->GetCursor()->MoveHome();
		text->SetMarkStart();
		text->GetCursor()->MoveEnd();
		text->GetCursor()->MoveRight();
		text->SetMarkEnd();
	}
	
	Copy();
	text->DeleteSelectedText(); //smazu oznaceny text
	
	if(highlight)
	{//highlightuju
		highlighter->HighlightFromBlock(text->GetCursor()->GetBlock());
		text->GetCursor()->CalcFormat();		
	}
	blttext->BlinkCursor(1);
	SetTextSize();
	ScrollToCursor();
	blttext->SetNeedRecalc();

	changed = 1; ChangedEvent();
	CursorMoved();

	return 1;
}

int CGUIEditWindow::MoveCursorToLine(int line, int column)
{
	text->UnMark();
	if(!text->GetCursor()->SetCursorPos(line,column))
	{
		text->GetCursor()->MoveTextEnd();
	}
	blttext->BlinkCursor(1); //zobrazi kurzor (blika)
	ScrollToCursor(1); // nascroluje, aby byl kurzor videt
	blttext->SetNeedRecalc();
	CursorMoved();
	return 1;
}

int CGUIEditWindow::SelectWordRight()
{

	CTextCursor *cur;

	text->UnMark();
	text->SetMarkStart();

	cur = text->GetCursor();

	if(cur->GetChar()=='@')
		cur->MoveRight();
	
	UC c=cur->GetChar();
	if(c)
	{
		if(! ( isspace(c) || ispunct(c) ) )
		{
			//najsem na mezere - prejedu cely slovo
			do{
				cur->MoveRight();
				c=cur->GetChar();
				if(c==0) {
					cur->MoveEnd();
					return 1;
				}
			}while(! ( isspace(c) || ispunct(c) ) );
		}else{
			if(!isspace(c))
			{
				cur->MoveRight();
				c=cur->GetChar();
			}
		}
	}

	text->SetMarkEnd();

	blttext->BlinkCursor(1); //zobrazi kurzor (blika)
	ScrollToCursor(); // nascroluje, aby byl kurzor videt
	blttext->SetNeedRecalc();
	CursorMoved();
	return 1;
}