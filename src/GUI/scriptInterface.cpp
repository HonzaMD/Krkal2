///////////////////////////////////////////////
//
// scriptInterface.cpp
//
//	implementace iterfacu pro propojeni GUI se skriptama
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "scriptInterface.h"
#include "ObjProperty.h"
#include "gui.h"


#include "kernel.h"

//////////////////////////////////////////////////////////////////////
// CGUIScriptVariableInterface
//////////////////////////////////////////////////////////////////////

CEDObjectProperty* CGUIScriptVariableInterface::op=0;
CGUITreeItem* CGUIScriptVariableInterface::rootTI=0;

CGUITreeItem* CGUIScriptVariableInterface::selectedGroupTI = 0;

CGUITreeItem* CGUIScriptVariableInterface::groupItem = 0;
int CGUIScriptVariableInterface::numGroupItems = 0;
int CGUIScriptVariableInterface::buttonIDCounter = 1;


int CGUIScriptVariableInterface::ScriptedVariableInit(CEDObjectProperty* _op, CKerOVar* OVar, CGUITreeItem* _rootTI)
{	
	op = _op;
	rootTI = _rootTI;

	selectedGroupTI = rootTI;	// na zacatku je vybrana skupina korenova polozka

	if(!op->scriptHandler)
		op->scriptHandler = new CGUIScriptButtonsHandler(op);

	KerMain->call(0,op->obj,OVar->Name,1,eKTint,KnownNames[eKKNitemID].Name,OVar->ItemID);
	CGUIScriptVariableInterface::ScriptedVariableFinish();

	return 0;
}

int CGUIScriptVariableInterface::ScriptInterfaceInit(CEDObjectProperty* _op, CGUITreeItem* _rootTI)
{	
	op = _op;
	rootTI = _rootTI;

	selectedGroupTI = rootTI;	// na zacatku je vybrana skupina korenova polozka

	return 0;
}

int CGUIScriptVariableInterface::ScriptedVariableFinish()
{
	if(!op)
		return 0;

	op->UpdateTree();

	rootTI->SetRollState(1);

	op=0;
	rootTI=0;
	selectedGroupTI=0;
	groupItem=0;
	numGroupItems=0;

	return 1;
}

int CGUIScriptVariableInterface::AddItem(CKerOVar* OVar, int where, int before)
{
	if(!selectedGroupTI)
	{
		if(OVar)
			delete OVar;
		return 0;
	}


	CGUITreeItem* ti=0;
	typeID whereID=0;
	bool first = before ? true : false;

	if(where)
	{
		ti = selectedGroupTI->GetItemWithIndex(where-1);
		if(ti)
			whereID=ti->GetID();
	}

	CKerObject* ko = KerMain->GetAccessToObject(op->obj);
	if(!ko)
		return 0;

	if(groupItem)
		ti = op->AddEditItem(groupItem,OVar,whereID, first,true);
	else
		ti = op->AddEditItem(selectedGroupTI,OVar,whereID, first,true);

	op->AddVarProperty(ko,OVar,ti,groupItem,numGroupItems);

	return ti->GetIndex()+1;
}

int CGUIScriptVariableInterface::AddGap(int where, int before)
{
	if(!selectedGroupTI)
	{
		return 0;
	}

	CGUITreeItem* ti=0;
	typeID whereID=0;
	bool first = before ? true : false;

	if(where)
	{
		ti = selectedGroupTI->GetItemWithIndex(where-1);
		if(ti)
			whereID=ti->GetID();
	}

	ti = selectedGroupTI->AddTreeItem(whereID,0,0,0,false,first);
	ti->itemVisible=false;
	ti->itemMarker=0;
	
	CGUILine* line = new CGUILine(5,5,220,5,0xFF000000);
	line->SetPos(0,0);
	line->SetSize(220,10);
	ti->SetItemElement(line,false);

	return ti->GetIndex()+1;
}

int CGUIScriptVariableInterface::AddText(char* text, char* help, int where, int before)
{
	if(!selectedGroupTI)
	{
		return 0;
	}


	CGUITreeItem* ti=0;
	typeID whereID=0;
	bool first = before ? true : false;

	if(where)
	{
		ti = selectedGroupTI->GetItemWithIndex(where-1);
		if(ti)
			whereID=ti->GetID();
	}

	//ti = selectedGroupTI->AddTreeItem(whereID,text,0,help,false,first);
	ti = selectedGroupTI->AddTreeItem(whereID,0,0,help,false,first);
	ti->itemVisible=false;
	ti->itemMarker=0;

	CGUIStaticText* st = new CGUIStaticText(text,(CGUIFont*)RefMgr->Find("GUI.F.Courier.12.B"),0,0,0xFF000000);
	float stsx,stsy;
	st->GetSize(stsx,stsy);
	st->SetPos(-floorf((220-stsx)/2),0);
	ti->SetItemElement(st,false);


	return ti->GetIndex()+1;
}

int CGUIScriptVariableInterface::DeleteItem(int index)
{
	if(!selectedGroupTI)
	{
		return 0;
	}


	CGUITreeItem* ti = selectedGroupTI->GetItemWithIndex(index-1);
	if(ti)
	{
		if(ti==op->editedGroupItem)
			op->SelectLocationEditModByOVar(ti,0,false);
		selectedGroupTI->DeleteTreeItem(ti);
		return 1;
	}

	return 0;
}

int CGUIScriptVariableInterface::AddButton(int where, int shift, char* label, char* help, int userID, CKerName* scriptFunction)
{
	if(!selectedGroupTI)
	{
		return 0;
	}


	CGUITreeItem* ti=0;
	typeID whereID=0;

	int buttonID=0;

	if(where)
	{
		ti = selectedGroupTI->GetItemWithIndex(where-1);
		if(ti)
			whereID=ti->GetID();
	}

	if(shift==0)
	{
		if(!ti || ti->userTagType != eUTdataClass)
			return 0;
		CGUIScriptButtonsItemData* sbd = dynamic_cast<CGUIScriptButtonsItemData*>(ti->ut.userDataClass);
		if(!sbd)
			return 0;

		CGUIWindow* win = (CGUIWindow*)ti->elem;
		if(!win)
			return 0;

		float bx=0;
		CGUIButton* b = new CGUIButton(bx,2,100,22,"GUI.But.Std",label,0,help);
		b->AcceptEvent(op->scriptHandler->GetID(),EClicked);
		b->SetMark(true);
		b->SetTabOrder((float)sbd->GetNumButtons() + 1);
		win->AddBackElem(b);
		typeID buttonGUI_ID = b->GetID();

		buttonID = buttonIDCounter++;
		sbd->AddButton(new CScriptButtonData(buttonGUI_ID,buttonID,userID,scriptFunction));

		sbd->RefreshButtonsPositions(230,100);
	}
	else
	{
		if(shift==-1)
			ti = selectedGroupTI->AddTreeItem(whereID,0,0,0,false,true);
		else if(shift==1)
			ti = selectedGroupTI->AddTreeItem(whereID,0,0,0,false,false);

		// k nove polozce pridat okno na buttony, zneviditelenit label
		// pridat novy button

		CGUIWindow* win = new CGUIWindow(0,0,230,26,0);
		win->onWindowPanel=false;

		float bx=floorf((230-100)/2);
		CGUIButton* b = new CGUIButton(bx,2,100,22,"GUI.But.Std",label,0,help);
		b->AcceptEvent(op->scriptHandler->GetID(),EClicked);
		b->SetMark(true);
		b->SetTabOrder(1);
		win->AddBackElem(b);
		typeID buttonGUI_ID = b->GetID();

		win->SetFocusEl(0,0);
		ti->SetItemElement(win,false);
		ti->itemVisible=false;
		ti->itemMarker=0;

		CGUIScriptButtonsItemData* sbd = new CGUIScriptButtonsItemData();
		buttonID = buttonIDCounter++;
		sbd->AddButton(new CScriptButtonData(buttonGUI_ID,buttonID,userID,scriptFunction));

		// nastavit data u polozky 
		ti->userTagType = eUTdataClass;
		ti->ut.userDataClass = sbd;
	}

	return buttonID;
}

int CGUIScriptVariableInterface::DeleteButton(int buttonID)
{
	if(!selectedGroupTI)
	{
		return 0;
	}

	if(!rootTI || !rootTI->items || rootTI->state==0)
		return 0;

	CGUIListElement<CGUITreeItem>* le = rootTI->items->GetNextListElement(0);
	CGUITreeItem *ti=0;
	CGUIScriptButtonsItemData* sbd=0;

	while(le)
	{
		if((ti=rootTI->items->GetObjectFromListElement(le)))
		{
			if(ti->userTagType == eUTdataClass && (sbd=dynamic_cast<CGUIScriptButtonsItemData*>(ti->ut.userDataClass)))
			{
				if(DeleteButtonElem(ti,buttonID))
					return 1;
			}
			if(ti->state)
			{
				if(DeleteButtonRecursiv(ti,buttonID))
					return 1;
			}
		}
		le=rootTI->items->GetNextListElement(le);
	}

	return 0;
}

int CGUIScriptVariableInterface::DeleteButtonRecursiv(CGUITreeItem* gi, int buttonID)
{
	if(!gi || !gi->items || gi->state==0)
		return 0;

	CGUIListElement<CGUITreeItem>* le = gi->items->GetNextListElement(0);
	CGUITreeItem *ti=0;
	CGUIScriptButtonsItemData* sbd=0;

	while(le)
	{
		if((ti=gi->items->GetObjectFromListElement(le)))
		{
			if(ti->userTagType == eUTdataClass && (sbd=dynamic_cast<CGUIScriptButtonsItemData*>(ti->ut.userDataClass)))
			{
				if(DeleteButtonElem(ti,buttonID))
					return 1;
			}
			if(ti->state)
			{
				if(DeleteButtonRecursiv(ti,buttonID))
					return 1;
			}
		}
		le=gi->items->GetNextListElement(le);
	}

	return 0;
}

int CGUIScriptVariableInterface::DeleteButtonElem(CGUITreeItem* ti, int buttonID)
{
	assert(ti->userTagType == eUTdataClass);

	CGUIScriptButtonsItemData* sbd = dynamic_cast<CGUIScriptButtonsItemData*>(ti->ut.userDataClass);
	if(!sbd)
		return 0;

	typeID bID;
	if(bID = sbd->RemoveButton(buttonID))	// pokud v seznamu neni vraci 0
	{	// button se odtranil ze seznamu
		CGUIButton* ID(b,bID);
		if(b && ti->elem)
			((CGUIWindow*)ti->elem)->DeleteBackElem(b);

		if(sbd->GetNumButtons())
			sbd->RefreshButtonsPositions(230,100);	// nejake buttony jeste v polozce zbyly, prepocitej jejich pozici
		else
		{
			// zadne buttony nezbyly, smaz celou polozku
			if(ti==selectedGroupTI)
				selectedGroupTI=rootTI;

			CGUITreeItem* pi = ti->parentItem;
			assert(pi);
			pi->DeleteTreeItem(ti);
		}

		return 1;
	}

	return 0;
}

int CGUIScriptVariableInterface::AddGroupItem(char* label, char* help, int where, int before)
{
	if(!selectedGroupTI)
	{
		return 0;
	}

	CGUITreeItem* ti=0;
	typeID whereID=0;
	bool first = before ? true : false;

	if(where)
	{
		ti = rootTI->GetItemWithIndex(where-1);
		if(ti)
			whereID=ti->GetID();
	}

	ti = rootTI->AddTreeItem(whereID,label,0,help,false,first);

	return ti->GetIndex()+1;
}

int CGUIScriptVariableInterface::DeleteGroupItem(int groupIndex)
{
	if(!selectedGroupTI)
	{
		return 0;
	}

	if(!groupIndex)
		return 0;

	CGUITreeItem* ti = rootTI->GetItemWithIndex(groupIndex-1);
	if(ti)
	{
		if(ti==op->editedGroupItem)
			op->SelectLocationEditModByOVar(ti,0,false);

		rootTI->DeleteTreeItem(ti);
		return 1;
	}

	return 0;
}

int CGUIScriptVariableInterface::SelectGroupItem(int groupIndex)
{
	if(!selectedGroupTI)
	{
		return 0;
	}

	if(groupIndex)
	{
		selectedGroupTI = rootTI->GetItemWithIndex(groupIndex-1);
		if(!selectedGroupTI)
		{
			selectedGroupTI = rootTI;
			return 1;
		}
	}
	else
		selectedGroupTI=rootTI;

	return 0;
}

int CGUIScriptVariableInterface::DeleteAllGroupItems(int groupIndex, int buttonsLet)
{
	if(!selectedGroupTI)
	{
		return 0;
	}

	if(SelectGroupItem(groupIndex))
		return 1;

	while(CGUITreeItem* tti = selectedGroupTI->items->GetFromHead())
	{
		if(!buttonsLet || tti->userTagType != eUTdataClass)
		{
			if(tti==op->editedGroupItem)
				op->SelectLocationEditModByOVar(tti,0,false);
			selectedGroupTI->DeleteTreeItem(tti);
		}
	}

	return 0;
}



//////////////////////////////////////////////////////////////////////
// CScriptButtonData
//////////////////////////////////////////////////////////////////////

CScriptButtonData::CScriptButtonData(typeID _buttonGUI_ID, int _buttonID, int _userID, CKerName* _scriptFunction)
{
	buttonGUI_ID = _buttonGUI_ID;
	buttonID = _buttonID;
	userID = _userID;
	scriptFunction = _scriptFunction;
}

CScriptButtonData::~CScriptButtonData()
{
}


//////////////////////////////////////////////////////////////////////
// CGUIScriptButtonsItemData
//////////////////////////////////////////////////////////////////////

CGUIScriptButtonsItemData::CGUIScriptButtonsItemData()
{
	numButtons=0;
}


CGUIScriptButtonsItemData::~CGUIScriptButtonsItemData()
{
	for(int i = 0; i < buttonsArray.GetSize(); i++)
	{
		CScriptButtonData* bd = buttonsArray.Remove(i);
		if(bd)
		{
			delete bd;
		}
	}
}

int CGUIScriptButtonsItemData::AddButton(CScriptButtonData* button)
{
	numButtons++;
	return buttonsArray.Add(button);;
}

typeID CGUIScriptButtonsItemData::RemoveButton(int buttonID)
{
	for(int i = 0; i < buttonsArray.GetSize(); i++)
	{
		CScriptButtonData* bd = buttonsArray.Get(i);
		if(bd && bd->buttonID == buttonID)
		{
			buttonsArray.Remove(i);
			numButtons--;
			typeID bID = bd->buttonGUI_ID;
			delete bd;
			return bID;
		}
	}	
	return 0;
}

CScriptButtonData* CGUIScriptButtonsItemData::GetButtonData(typeID buttonGUI_ID)
{
	for(int i = 0; i < buttonsArray.GetSize(); i++)
	{
		CScriptButtonData* bd = buttonsArray.Get(i);
		if(bd && bd->buttonGUI_ID == buttonGUI_ID)
		{
			return bd;
		}
	}	
	return 0;
}

void CGUIScriptButtonsItemData::RefreshButtonsPositions(float winWidth, float buttonsWidth)
{
	float xx=0;
	float dx = floorf((winWidth - numButtons * buttonsWidth) / numButtons);

	if(dx<2)
		dx=2;

	xx=floorf(dx/2);

	for(int i = 0; i < buttonsArray.GetSize(); i++)
	{
		CScriptButtonData* bd = buttonsArray.Get(i);
		if(bd)
		{
			CGUIButton* ID(b,bd->buttonGUI_ID);
			if(b)
				b->Move(xx,2);
			xx+=dx;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CGUIScriptButtonsHandler	
//////////////////////////////////////////////////////////////////////////////////////////////////

void CGUIScriptButtonsHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;
	
	if(event->eventID == EClicked)
	{
		CGUIButton* b = dynamic_cast<CGUIButton*>(nameServer->ResolveID(event->sender));
		if(b)	// prisla udalost o stisknuti tlacitka
		{									
			CGUIElement* win = b->parent;
			CGUITreeItem* ti = op->FindItemFromElement(win);

			if(ti && ti->userTagType == eUTdataClass)	// mam polozku obsahujici stisknuty button
			{
				CGUIScriptButtonsItemData* sbd = dynamic_cast<CGUIScriptButtonsItemData*>(ti->ut.userDataClass);
				if(sbd)
				{
					CScriptButtonData* bd = sbd->GetButtonData(b->GetID());
					if(bd)
					{
						CGUITreeItem* ri=0;
						int groupIndex=0;

						if(ri = ti->parentItem)
						{
							if(ri->userTagType == eUTOVar || ri->userTagType == eUTOVarDEL)
							{
								CKerOVar* OVar = ri->ut.OVar;
								if(OVar)
								{
									if(!(OVar->Type == eKTvoid && OVar->EditType == eKETscripted))
										ri=0;
								}
								else
									ri=0;
							}
							else if(ri->userTagType == eUTnone)
							{
								groupIndex=ri->GetIndex()+1;
								ri = ri->parentItem;
							}
						}
						
						if(ri && (ri->userTagType == eUTOVar || ri->userTagType == eUTOVarDEL) && ri->ut.OVar && 
							(ri->ut.OVar->Type == eKTvoid && ri->ut.OVar->EditType == eKETscripted))
						{
							CGUIScriptVariableInterface::ScriptInterfaceInit(op,ri);
							KerMain->call(0,op->obj,bd->scriptFunction,3,eKTint,KnownNames[eKKNgroupID].Name,groupIndex,eKTint,KnownNames[eKKNbuttonID].Name,bd->buttonID,eKTint,KnownNames[eKKNbuttonUserID].Name,bd->userID);
							CGUIScriptVariableInterface::ScriptedVariableFinish();
						}
					}
				}
			}
		}
	}

	delete event;
}
