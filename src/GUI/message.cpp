///////////////////////////////////////////////
//
// message.cpp
//
// implementation for the CGUIMessage class
//	objekt prenasejici zpravu CGUIMessage
//	fronta zprav CGUIMsgQueue
//	metody CGUIElement pracujici se zpravami: SendMsg, SendIntoMsg, GetMsg, ForwardMsg
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"
#include "GUI.h"
#include <time.h>

#include "editor.h"

#include "dx.h"
#include "kernel.h"
#include "KerErrors.h"

//////////////////////////////////////////////////////////////////////
// CGUIMessage
//////////////////////////////////////////////////////////////////////

typeID CGUIMessage::lastID=0;			// citac ID zprav
typeID CGUIEventProgram::lastID=1;		// citac ID programu rizenych udalostmi
										// 0 - vyhrazeno pro chybu

CGUIMessage::CGUIMessage(enum EMsgs _type, float _x, float _y, typeID _param, UINT _state, CGUIElement *_to, UINT _time)
{
	type = _type;
	x = _x;
	y = _y;
	param = _param;
	state = _state;
	time = _time;
	element = 0;
	to = _to;
	list = 0;
	focus_msg=false;
	mID = lastID++;
}

CGUIMessage::CGUIMessage(enum EMsgs _type, typeID _param, UINT _state, CGUIElement *_to, UINT _time)
{
	type = _type;
	param = _param;
	state = _state;
	time = _time;
	x = y = -1;
	element = 0;
	to = _to;
	list = 0;
	focus_msg=false;
	mID = lastID++;
}
CGUIMessage::CGUIMessage(enum EMsgs _type, typeID _command, CGUIElement *_element, CGUIElement *_to, UINT _time)
{
	type = _type;
	element = _element;
	param = _command;
	time = _time;

	state = 0;
	x = y = -1;
	to = _to;
	list = 0;
	focus_msg=false;
	mID = lastID++;
}

CGUIMessage::~CGUIMessage()
{
	if(list)
	{
		list->Remove(this);	// pokud je zprava jeste zarazena v nejakem seznamu, odstran ji ze seznamu
							// spravne ale mela byt jiz odstranena
		assert(false);		// !@#$ jen pro ladeni seznamu (kontrola spravneho odstranovani ze seznamu)
		//if(!std::uncaught_exception())
		//	throw CExc(eGUI,E_INTERNAL, "CGUIMessage::~CGUIMessage> Incorrect work with list");
	}
}

void CGUIMessage::InnerCoords(float dx, float dy)
{
	x -= dx;
	y -= dy;
}


//////////////////////////////////////////////////////////////////////
// CGUIMsgQueue
//////////////////////////////////////////////////////////////////////

CGUIMessage *CGUIMsgQueue::GetWaitingMsgs()
{
	CGUIMessage *msg;
	if(msg=priority_message_queue.RemoveFromHead())
		return msg;
	else
		return message_queue.RemoveFromHead();
}

void CGUIMsgQueue::AddMsgToQueue(CGUIMessage *msg, CGUIElement *to)
{
	if(msg)
	{
		msg->to=to;
		message_queue.AddToBack(msg);
	}
}

void CGUIMsgQueue::AddPriorityMsgToQueue(CGUIMessage *msg, CGUIElement *to)
{
	if(msg)
	{
		msg->to=to;
		priority_message_queue.AddToBack(msg);
	}
}

CGUIMessage *CGUIMsgQueue::RemoveMsgFromQueue(CGUIElement *to)
{
	CGUIMessage *msg;
	if(msg=Remove<CGUIMessage,CGUIElement*>(&priority_message_queue,to))
		return msg;
	else
		return Remove<CGUIMessage,CGUIElement*>(&message_queue,to);
}

void CGUIMsgQueue::DeleteAllMsgFromQueue(CGUIElement *to)
{
	CGUIMessage *msg;

	CGUIListElement<CGUIMessage>* le = priority_message_queue.GetNextListElement(0);
	while(le)
	{
		if((msg=priority_message_queue.GetObjectFromListElement(le)) && (msg->to==to))
		{
			CGUIListElement<CGUIMessage>* oldle=le;
			le=priority_message_queue.GetNextListElement(le);
			delete priority_message_queue.RemoveListElement(oldle);
		}else
			le=priority_message_queue.GetNextListElement(le);
	}


	le = message_queue.GetNextListElement(0);
	while(le)
	{
		if((msg=message_queue.GetObjectFromListElement(le)) && (msg->to==to))
		{
			CGUIListElement<CGUIMessage>* oldle=le;
			le=message_queue.GetNextListElement(le);
			delete message_queue.RemoveListElement(oldle);
		}else
			le=message_queue.GetNextListElement(le);
	}
}

void CGUIMsgQueue::DeleteMsgQueue()
{
	CGUIMessage *msg;
	while(msg=priority_message_queue.RemoveFromHead())
		delete msg;
	while(msg=message_queue.RemoveFromHead())
		delete msg;
}

CGUIMessage *CGUIMsgQueue::GetMsg(typeID mID)
{
	CGUIMessage *msg;
	if(msg=Get<CGUIMessage,typeID>(&priority_message_queue,mID))
		return msg;
	else
		return Get<CGUIMessage,typeID>(&message_queue,mID);
}

CGUIMessage *CGUIMsgQueue::RemoveMsg(typeID mID)
{
	CGUIMessage *msg;
	if(msg=Remove<CGUIMessage,typeID>(&priority_message_queue,mID))
		return msg;
	else
		return Remove<CGUIMessage,typeID>(&message_queue,mID);
}

void CGUIMsgQueue::DeleteMsg(typeID mID)
{
	Delete<CGUIMessage,typeID>(&priority_message_queue,mID);
	Delete<CGUIMessage,typeID>(&message_queue,mID);
}


//////////////////////////////////////////////////////////////////////
// CGUIElement (jen metody pro zpracovani zprav)
//////////////////////////////////////////////////////////////////////

void CGUIElement::SendIntoMsg(CGUIMessage *msg, CGUIElement *to)
{
	if(to)
	{
		float ex,ey;		
		GetPos(ex,ey);
		msg->InnerCoords(ex,ey);	// prevod souradnic zprav ze s.s. predka do s.s. adresata
									// prijemce (adresat) dostane zpravu v s.s. sveho nadrazeneho objektu (odesilatele)
									// !@#$ musi to byt vzdy opravdu s.s. nadrazeneho objektu (co kdyz odesilatel != nadrazeny objekt)
		if(to->back_element && type >= PTWindow) 
		{	// prevod souradnic mezi s.s. okna (front) a s.s. back wnd.
			// uplatnuje se jen pokud se jedna o okno a jeho podelement je vnitrni (tedy v back wnd.)
			((CGUIWindow*)this)->TransformCoordsFrontToBack(msg->x,msg->y);
		}
	}
	SendMsg(msg,to);
}

void CGUIElement::SendIntoPriorityMsg(CGUIMessage *msg, CGUIElement *to)
{
	if(to)
	{
		float ex,ey;
		GetPos(ex,ey);
		msg->InnerCoords(ex,ey);	// prevod souradnic zprav ze s.s. predka do s.s. odesilatele
									// prijemce (adresat) dostane zpravu v s.s. sveho nadrazeneho objektu (odesilatele)
									// !@#$ musi to byt vzdy opravdu s.s. nadrazeneho objektu (co kdyz odesilatel != nadrazeny objekt)
		if(to->back_element && type >= PTWindow) 
		{	// prevod souradnic mezi s.s. okna (front) a s.s. back wnd.
			// uplatnuje se jen pokud se jedna o okno a jeho podelement je vnitrni (tedy v back wnd.)
			((CGUIWindow*)this)->TransformCoordsFrontToBack(msg->x,msg->y);
		}
	}

	SendPriorityMsg(msg,to);
}


void CGUIElement::SendMsg(CGUIMessage *msg,  CGUIElement *to)
{
	msgQueue->AddMsgToQueue(msg,to);
}

void CGUIElement::SendPriorityMsg(CGUIMessage *msg,  CGUIElement *to)
{
	msgQueue->AddPriorityMsgToQueue(msg,to);
}

int CGUIElement::GetMsg(CGUIMessage *msg)
// standardni zpracovani prichozi zpravy elementem
// lze pretizit a definovat si vlastni zpusob prijmu
// vezmu prichozi zpravu, porovnam jeji typ s maskou, ktera urcuje jake typy zprav prijimam
// pokud to klapne zavolam obsluznou fci pro konkretni typ zpravy s vyplnenymi argumenty podle prichozi zpravy
// obsluzne fce si implementuje kazdy element sam pomoci dedicnosti a pretizeni
// obsluzne fce vraci 0/1 (1 v pripade, ze se zprava ma dorucovat dale vnitrnim elementum okna nebo multiwidgetu atp., 0 - dale se nedorucuje)
// v pripade, ze element je schopen tento typ zpravy dale posilat (produkovat) - nastavuje se maskou,
// a obsluzna fce vratila 1, vola se fce ForwardMsg, ktera ma za ukol zpravu predat vnitrnim elementum podle dorucovaciho scenare (vetsinou dle polohy zpravy a elementu)
{
	int produceIt = 0;

	if(msg->type & msgConsume)		// I can process this message
	{
		if(msg->focus_msg)
		{
			switch(msg->type)
			{
			case MsgMouseL:				// mouse left
				if(msg->param)
					produceIt = MouseDownFocus(msg->x, msg->y, msg->state,msg->mID);
				else
					produceIt = 1;
				produceIt &= MouseLeftFocus(msg->x, msg->y, (UINT)msg->param, msg->state,msg->mID);
				break;
			case MsgMouseR:   			// mouse right
				if(msg->param)
					produceIt = MouseDownFocus(msg->x, msg->y, msg->state,msg->mID);
				else
					produceIt = 1;
				produceIt &= MouseRightFocus(msg->x, msg->y, (UINT)msg->param, msg->state,msg->mID);
				break;
			case MsgMouseM:  			// mouse middle
				if(msg->param)
					produceIt = MouseDownFocus(msg->x, msg->y, msg->state,msg->mID);
				else
					produceIt = 1;
				produceIt &= MouseMiddleFocus(msg->x, msg->y, (UINT)msg->param, msg->state,msg->mID);
				break;
			case MsgMouseDL: 			// mouse left double click
				produceIt = DblClickLeftFocus(msg->x, msg->y, msg->state,msg->mID);
				break;		
			case MsgMouseDR: 			// mouse right double click
				produceIt = DblClickRightFocus(msg->x, msg->y, msg->state,msg->mID);
				break;
			case MsgMouseWU:   			// mouse wheel up
			case MsgMouseWD:  			// mouse wheel down
				produceIt = WheelFocus(msg->x, msg->y, (msg->type == MsgMouseWU) ? 1 : -1, (int)msg->param / 120, msg->state,msg->mID);
				break;
			case MsgMouseOver:			// mouse over
				produceIt = MouseOverFocus(msg->x, msg->y, (UINT)msg->param, msg->state,msg->mID);
				break;
			case MsgKeyboardChar:		// keyboardChar
				produceIt = KeyboardCharFocus((UINT)msg->param, msg->state,msg->mID);
				break;
			case MsgKeyboard:			// keyboard
				produceIt = KeyboardFocus((UINT)msg->param, msg->state,msg->mID);
				break;
			case MsgKeyboardUp:			// keyboardUp
				produceIt = KeyboardUpFocus((UINT)msg->param, msg->state,msg->mID);
				break;
			case MsgTimer:				// timer impulse
				produceIt = TimerImpulseFocus(msg->param, msg->x, msg->mID);
				break;
			case MsgFocusChange:		// focus changed
				produceIt = FocusChangedFocus(msg->state, msg->param, msg->mID);
				break;
			case MsgCommand:			// command
				produceIt = CommandFocus(msg->param, msg,msg->mID);
				break;

			default:
				throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIElement::GetMsg> Unknow Message #%d (%d) [%d, %d] focus (ID=%d)",
					msg->type, msg->param, msg->x, msg->y,msg->mID);
			}
		}else{
			switch(msg->type)
			{
			case MsgMouseL:				// mouse left
				if(msg->param)
					produceIt = MouseDown(msg->x, msg->y, msg->state);
				else
					produceIt = 1;
				produceIt &= MouseLeft(msg->x, msg->y, (UINT)msg->param, msg->state);
				break;
			case MsgMouseR:   			// mouse right
				if(msg->param)
					produceIt = MouseDown(msg->x, msg->y, msg->state);
				else
					produceIt = 1;
				produceIt &= MouseRight(msg->x, msg->y,(UINT) msg->param, msg->state);
				break;
			case MsgMouseM:  			// mouse middle
				if(msg->param)
					produceIt = MouseDown(msg->x, msg->y, msg->state);
				else
					produceIt = 1;
				produceIt &= MouseMiddle(msg->x, msg->y, (UINT)msg->param, msg->state);
				break;
			case MsgMouseDL: 			// mouse left double click
				produceIt = DblClickLeft(msg->x, msg->y, msg->state);
				break;		
			case MsgMouseDR: 			// mouse right double click
				produceIt = DblClickRight(msg->x, msg->y, msg->state);
				break;
			case MsgMouseWU:   			// mouse wheel up
			case MsgMouseWD:  			// mouse wheel down
				produceIt = Wheel(msg->x, msg->y, (msg->type == MsgMouseWU) ? 1 : -1, (int)msg->param / 120, msg->state);
				break;
			case MsgMouseOver:			// mouse over
				produceIt = MouseOver(msg->x, msg->y, (UINT)msg->param, msg->state);
				break;
			case MsgMouseRelMove:		// mouse relative move
				produceIt = MouseRelativeMove(msg->x, msg->y);
				break;
			case MsgKeyboardChar:		// keyboardChar
				produceIt = KeyboardChar((UINT)msg->param, msg->state);
				break;
			case MsgKeyboard:			// keyboard
				produceIt = Keyboard((UINT)msg->param, msg->state);
				break;
			case MsgKeyboardUp:			// keyboardUp
				produceIt = KeyboardUp((UINT)msg->param, msg->state);
				break;
			case MsgTimer:				// timer impulse
				produceIt = TimerImpulse(msg->param, msg->x);
				break;
			case MsgFocusChange:		// focus changed
				produceIt = FocusChanged(msg->state, msg->param);
				break;
			case MsgCommand:			// command
				produceIt = Command(msg->param, msg);
				break;

			default:
				throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIElement::GetMsg> Unknow Message #%d (%d) [%d, %d]",
					msg->type, (UINT)msg->param, msg->x, msg->y);
			}
		}

		if(!produceIt)
		{
			delete msg;
			msg = 0;
			return DELIVER_ACCEPTED;
		}
	}
	else				// I'll either produce this msg, or throw it out
		produceIt = 1;

	if(produceIt && (msg->type & msgProduce))		// I'll send this message to element 
	{											// under me
		return ForwardMsg(msg);
	}
	else
	{
		delete msg;
		msg = 0;
		return DELIVER_NOT_ACCEPTED;
	}

	return DELIVER_ACCEPTED;
}

int CGUIElement::ForwardMsg(CGUIMessage *msg)
{
	// element sam neni schopen zpravy predavat svym podelementum, nebot zadne nema
	// tato fce musi byt pretizena u elementu schopnych predavat zpravy dale (window, multiwidget)
	// pokud element neni schopen zpravy dale predavat, mel by mit nastaveno MsgSetProduce(MsgNone)
	throw CExc(eGUI, E_INTERNAL, "CGUIElement::ForwardMsg> Element is not able to ForwardMsgs #%d (%d) [%d, %d]",
		msg->type, msg->param, msg->x, msg->y);
}


//////////////////////////////////////////////////////////////////////
// CGUIEventProgram 
//////////////////////////////////////////////////////////////////////

CGUIEventProgram::CGUIEventProgram()
{
	objID = lastID++;
	objName = 0;
	
	list = 0;
	nameHT = 0;
	idHT = 0;

	nameServer->Register(this);
}

CGUIEventProgram::~CGUIEventProgram()
{
	nameServer->UnRegister(this);
	if(objName)
		delete[] objName;
}

void CGUIEventProgram::SetName(char *name)
{
	if(objName)
	{
		nameServer->UnRegisterName(objName);
		delete[] objName;
	}

	objName = new char[strlen(name)+1];
	strcpy(objName,name);
	nameServer->RegisterName(this);
}


//////////////////////////////////////////////////////////////////////
// CGUINameServer 
//////////////////////////////////////////////////////////////////////

CGUINameServer::CGUINameServer()
{
	int i;
	nameHT = new CGUIList<CGUIEventProgram>*[NAME_SERVER_HT_SIZE];
	for(i=0;i<NAME_SERVER_HT_SIZE;i++)
		nameHT[i] = 0;//new CGUIList<CGUIEventProgram>(false);

	idHT = new CGUIList<CGUIEventProgram>*[NAME_SERVER_HT_SIZE];
	for(i=0;i<NAME_SERVER_HT_SIZE;i++)
		idHT[i] = 0;//new CGUIList<CGUIEventProgram>(false);
}

CGUINameServer::~CGUINameServer()
{
	int i;

	if(nameHT)
		for(i=0;i<NAME_SERVER_HT_SIZE;i++)
			SAFE_DELETE(nameHT[i]);
	SAFE_DELETE_ARRAY(nameHT);
	if(idHT)
		for(i=0;i<NAME_SERVER_HT_SIZE;i++)
			SAFE_DELETE(idHT[i]);
	SAFE_DELETE_ARRAY(idHT);
}

int CGUINameServer::Register(CGUIEventProgram* program)
{
	if(!program)
		return 1;

	if(program->idHT || program->nameHT)
		return 1;	// program uz je zarazen v HT (chyba, program muze byt registrovan jen jednou)


	int keyHT;

	// vypocet hasovacich pozic z klice
	if(program->objName)
	{
		keyHT = hashName(program->objName);
		if(::IsMember<CGUIEventProgram,char*>(nameHT[keyHT], program->objName))
			return 1;	// // program uz je zarazen v HT (chyba, program muze byt registrovan jen jednou)
	}

	keyHT = hashID(program->objID);
	if(::IsMember<CGUIEventProgram,typeID>(idHT[keyHT], program->objID))
		return 1;	// // program uz je zarazen v HT (chyba, program muze byt registrovan jen jednou)

	// zarazeni programu do hasovacich tabulek:
	if(program->objName)
	{	// pokud program nema jmeno, neregistruje se v HT podle jmena
		keyHT = hashName(program->objName);
		if(!nameHT[keyHT])
			nameHT[keyHT] = new CGUIList<CGUIEventProgram>(false);
		nameHT[keyHT]->AddToBack(program);
		program->nameHT=nameHT[keyHT];
	}

	keyHT = hashID(program->objID);
	if(!idHT[keyHT])
		idHT[keyHT] = new CGUIList<CGUIEventProgram>(false);
	idHT[keyHT]->AddToBack(program);	// pridam program do hasovaci tabulky (so seznamu na vypocitane pozici)
	program->idHT=idHT[keyHT];			// nastavim programu odkaz do hasovaci tabulky (seznamu), kde je ulozen

	return 0;
}

int CGUINameServer::UnRegister(CGUIEventProgram* program)
{
	int keyHT;
	int ret=0;

	if(!program)
		return 1;

	if(program->objName)
	{
		keyHT = hashName(program->objName);
		if(program->nameHT && nameHT[keyHT] != program->nameHT)
		{
			ret=1;	// chyba, program tvrdi, ze je v jinem seznamu HT, nez podle hash. fce ma byt
			assert(false);	// !@#$ prozatimni brutalni kontrola
		}
		if(!nameHT[keyHT] || !nameHT[keyHT]->Remove(program))
			ret=1;	// program nebyl v HT => chyba
		program->nameHT=0;
	}

	keyHT = hashID(program->objID);
	if(program->idHT && idHT[keyHT] != program->idHT)
	{
		ret=1;	// chyba, program tvrdi, ze je v jinem seznamu HT, nez podle hash. fce ma byt
		assert(false);	// !@#$ prozatimni brutalni kontrola
	}
	if(!idHT[keyHT] || !idHT[keyHT]->Remove(program))
		ret=1;	// program nebyl v HT => chyba
	program->idHT=0;

	return ret;
}

int CGUINameServer::UnRegister(typeID objID)
{
	return UnRegister(ResolveID(objID));
}

int CGUINameServer::UnRegister(char *objName)
{
	return UnRegister(ResolveName(objName));
}

int CGUINameServer::UnRegisterName(char *objName)
{
	int ret=0;
	if(!objName)
		return 1;

	int keyHT = hashName(objName);
	CGUIEventProgram* program = ::Get<CGUIEventProgram,char*>(nameHT[keyHT], objName);
	if(!program)
		return 1;

	if(nameHT[keyHT] != program->nameHT)
	{
		ret=1;	// chyba, program tvrdi, ze je v jinem seznamu HT, nez podle hash. fce ma byt
		assert(false);	// !@#$ prozatimni brutalni kontrola
	}
	if(!nameHT[keyHT]->Remove(program))
		ret=1;	// program nebyl v HT => chyba
	program->nameHT=0;
	return ret;
}

int	CGUINameServer::RegisterName(CGUIEventProgram* program)
{
	if(!program || program->nameHT || !program->objName) 
		return 1;	// program nezadan, nebo program uz je zarazen v HT (chyba, program muze byt registrovan jen jednou), nebo program nema jmeno => nelze zaregistrovat

	
	int keyHT = hashName(program->objName);
	if(::IsMember<CGUIEventProgram,char*>(nameHT[keyHT], program->objName))
		return 1;	// // program uz je zarazen v HT (chyba, program muze byt registrovan jen jednou)

	if(!nameHT[keyHT])
		nameHT[keyHT] = new CGUIList<CGUIEventProgram>(false);
	nameHT[keyHT]->AddToBack(program);
	program->nameHT=nameHT[keyHT];

	return 0;
}

CGUIEventProgram* CGUINameServer::ResolveID(typeID objID)
{
	return ::Get<CGUIEventProgram,typeID>(idHT[hashID(objID)], objID);
}

CGUIEventProgram* CGUINameServer::ResolveName(char *objName)
{
	if(!objName)
		return 0;
	return ::Get<CGUIEventProgram,char*>(nameHT[hashName(objName)], objName);
}

CGUIEventProgram* CGUINameServer::operator[](typeID objID)
{
	return ::Get<CGUIEventProgram,typeID>(idHT[hashID(objID)], objID);
}

int CGUINameServer::hashID(typeID ID)
{
	int h=0;
	h = (int) (ID % NAME_SERVER_HT_SIZE);
	return h;
}

int CGUINameServer::hashName(char* name)
{
	if(!name)
	{
		assert(false);	// !@#$ brutalni kontrola, vyhodit
		return 0;
	}
	int h=0;
	int len = strlen(name);
	for(int i=0;i<len;i++)
		h += name[i];
	h = h % NAME_SERVER_HT_SIZE;
	return h;
}

//////////////////////////////////////////////////////////////////////
// CGUIEventSupport 
//////////////////////////////////////////////////////////////////////

int CGUIEventSupport::SetMask(typeID eventProgramID, int mask[], int num)
{
	if(!eventProgramID || !mask || !num)
		return 1;

	RejectAllEvents(eventProgramID);
	int ret=0;
	for(int i = 0; i<num; i++)
	{
		if(AcceptEvent(eventProgramID,mask[i]))
			ret=1;
	}
	return ret;
}

int CGUIEventSupport::SetMaskName(typeID eventProgramID, int mask[], int nameMask[], int num)
{
	if(!eventProgramID || !mask || !num)
		return 1;

	RejectAllEvents(eventProgramID);
	int ret=0;
	for(int i = 0; i<num; i++)
	{
		if(RenameEvent(eventProgramID,mask[i],nameMask[i]))
			ret=1;
	}
	return ret;
}

int CGUIEventSupport::RenameEvent(typeID eventProgramID, int stdEventID, int newEventID)
{
	if(!eventProgramID || stdEventID == ENoEvent)
		return 1;

	CGUIEventList* el = ::Get<CGUIEventList,int>(&eventList, stdEventID);
	if(el)
	{	// tato udalost je jiz v seznamu objednanych udalosti
		CGUIEventProgramList* pl = ::Get<CGUIEventProgramList,typeID>(&(el->eventProgs), eventProgramID);
		if(pl)
		{	// program uz v seznamu udalosti je
			pl->eventID = newEventID;
		}else{
			// program v seznamu udalosti neni => pridam ho tam
			pl = new CGUIEventProgramList(eventProgramID,newEventID);
			el->eventProgs.AddToBack(pl);
		}
		return 0;
	}else{
		// tato udalost jeste neni v seznamu objednanych udalosti
		// pokud je to udalost, kterou muze element generovat, pridam ji do seznamu 
		if(IsPossibleEvent(stdEventID))
		{
			el = new CGUIEventList(stdEventID);
			CGUIEventProgramList* pl = new CGUIEventProgramList(eventProgramID,newEventID);
			el->eventProgs.AddToBack(pl);
			eventList.AddToBack(el);
			return 0;
		}else
			return 1;
	}

	return 0;
}

int CGUIEventSupport::AcceptEvent(typeID eventProgramID, int event)
{
	if(!eventProgramID || event == ENoEvent)
		return 1;

	CGUIEventList* el = ::Get<CGUIEventList,int>(&eventList, event);
	if(el)
	{	// tato udalost je jiz v seznamu objednanych udalosti
		if(::Get<CGUIEventProgramList,typeID>(&(el->eventProgs), eventProgramID))
			return 0;	// program uz v seznamu udalosti je
		else{
			// program v seznamu udalosti neni => pridam ho tam
			CGUIEventProgramList* pl = new CGUIEventProgramList(eventProgramID,event);
			el->eventProgs.AddToBack(pl);
			return 0;
		}
	}else{
		// tato udalost jeste neni v seznamu objednanych udalosti
		// pokud je to udalost, kterou muze element generovat, pridam ji do seznamu 
		if(IsPossibleEvent(event))
		{
			el = new CGUIEventList(event);
			CGUIEventProgramList* pl = new CGUIEventProgramList(eventProgramID,event);
			el->eventProgs.AddToBack(pl);
			eventList.AddToBack(el);
			return 0;
		}else
			return 1;
	}

	return 0;
}

int CGUIEventSupport::RejectEvent(typeID eventProgramID, int event)
{
	if(!eventProgramID || event == ENoEvent)
		return 1;

	CGUIEventList* el = ::Get<CGUIEventList,int>(&eventList, event);
	if(el)
	{	
		::Delete<CGUIEventProgramList,typeID>(&(el->eventProgs), eventProgramID);
	}
	return 0;
}

int CGUIEventSupport::AcceptAllEvents(typeID eventProgramID)
{
	if(!eventProgramID)
		return 1;

	int ret=0;

	if(!availableEvents || !numEvents)	// zadne udalosti se nemohou generovat
		return 0;

	for(int i = 0; i<numEvents; i++)
		if(AcceptEvent(eventProgramID, availableEvents[i]))
			ret=1;

	return ret;
}

int CGUIEventSupport::RejectAllEvents(typeID eventProgramID)
{
	if(!eventProgramID)
		return 1;

	// -- prochazeni seznamu pres vsechny objednane udalosti
	CGUIEventList* el;
	el = eventList.GetNext(0);
	while(el)
	{
		::Delete<CGUIEventProgramList,typeID>(&(el->eventProgs), eventProgramID);
		el = eventList.GetNext(el);
	}

	return 0;
}

int CGUIEventSupport::RejectNameEvent(typeID eventProgramID, int event)
{
	if(!eventProgramID || event == ENoEvent)
		return 1;

	// projdu seznam vsech objednanych udalosti a z kazdeho podseznamu smazu prejmenovanou udalost event (jen pokud je od eventProgramID)
	CGUIEventList* el;
	el = eventList.GetNext(0);
	while(el)
	{
		CGUIEventProgramList *pl = ::Get<CGUIEventProgramList,typeID>(&(el->eventProgs), eventProgramID);
		if(pl->eventID == event)
			el->eventProgs.Delete(pl);
		el = eventList.GetNext(el);
	}

	return 0;
}

bool CGUIEventSupport::IsEventOrder(typeID eventProgramID, int event)
{
	if(!eventProgramID || event == ENoEvent)
		return false;

	CGUIEventList* el = ::Get<CGUIEventList,int>(&eventList, event);
	if(el)
	{	
		return ::IsMember<CGUIEventProgramList,typeID>(&(el->eventProgs), eventProgramID);
	}else
		return false;
}

bool CGUIEventSupport::IsPossibleEvent(int eventID)
{
	if(eventID == ENoEvent || !availableEvents || !numEvents)
		return false;
	for(int i = 0; i<numEvents; i++)
		if(availableEvents[i] == eventID)
			return true;

	return false;
}

void CGUIEventSupport::EventArise(int eventID, int pInt, typeID pID, void* pAddr, float pFloat, int pInt2, typeID pID2, float pFloat2)
{
	CGUIEvent* event;

	if(eventID == ENoEvent)
		return;

	CGUIEventList* el = ::Get<CGUIEventList,int>(&eventList, eventID);

	if(!el)
		return;	// zadne obsluzne programy pro tuto udalost nejsou objednany

	CGUIEventProgramList *p;
	CGUIListElement<CGUIEventProgramList>* le = el->eventProgs.GetNextListElement(0);
	while(le)
	{	// pro kazdy program, ktery si objednal udalost vytvorim novou udalost, nastavim jeji parametry a zaradim do fronty
		if((p=el->eventProgs.GetObjectFromListElement(le)))
		{
			event = new CGUIEvent();
			event->eventID = p->eventID;
			event->sender = GetID();
			event->recipient = p->objID;
			event->pInt = pInt;
			event->pID = pID;
			event->pAddr = pAddr;
			event->pFloat = pFloat;
			event->pInt2 = pInt2;
			event->pID2 = pID2;
			event->pFloat2 = pFloat2;
			eventServer->AddEventToQueue(event);
		}
		le=el->eventProgs.GetNextListElement(le);
	}
}

void CGUIEventSupport::EventArise(CGUIEvent* event)
{
	if(!event)
		return;
	
	CGUIEventList* el = ::Get<CGUIEventList,int>(&eventList, event->eventID);

	if(!el)
		return;	// zadne obsluzne programy pro tuto udalost nejsou objednany

	CGUIEventProgramList *p;
	CGUIListElement<CGUIEventProgramList>* le = el->eventProgs.GetNextListElement(0);
	while(le)
	{	// pro kazdy program, ktery si objednal udalost vytvorim novou udalost, nastavim jeji parametry a zaradim do fronty
		if((p=el->eventProgs.GetObjectFromListElement(le)))
		{
			CGUIEvent *evt = new CGUIEvent(*event);
			evt->eventID = p->eventID;
			evt->sender = GetID();
			evt->recipient = p->objID;
			eventServer->AddEventToQueue(evt);
		}
		le=el->eventProgs.GetNextListElement(le);
	}

	delete event;
}

//////////////////////////////////////////////////////////////////////
// CGUIEventServer
//////////////////////////////////////////////////////////////////////

int CGUIEventServer::DeliverEvent(bool allEvents)
{
	CGUIEvent *event;
	CGUIEventProgram *ep;

	try
	{
		if(!desktop)
			throw CExc(eGUI, E_INTERNAL,"CGUIEventServer::DeliverEvent> Fatal error - DESKTOP not exists");

		if(allEvents)
		{
			while(event=GetWaitingEvent())
			{
				ep = nameServer->ResolveID(event->recipient);
				if(ep)	// nasel jsem adresata, volam jeho obsluznou fci
					ep->EventHandler(event);
				else	// adresat nenalezen, udalost se rusi
					delete event;
			}
			return 1;
		}else{
			if(event=GetWaitingEvent())
			{
				ep = nameServer->ResolveID(event->recipient);
				if(ep)	// nasel jsem adresata, volam jeho obsluznou fci
					ep->EventHandler(event);
				else	// adresat nenalezen, udalost se rusi
					delete event;
				return 0;
			}else
				return 1;
		}
	}
	catch(CExc c)	// pri zpracovani udalosti muze dojit k vyjimce
	{				// hlavni odchytavac vyjimek z udalosti nejvyssi urovne pro GUI:
		delete event;	// smazu udalost co vyjimku zpusobila

		mainGUI->LogError(c);

		// vypis informace o chybe na obrazovce jako DebugMessage (prozatimni reseni)
		__time64_t ltime;
		_time64( &ltime );
		DebugMessage(0,"%s : %2d:%2d - %s",_ctime64( &ltime ), c.errnum, c.param, c.errstr);

		if(c.errnum != eGUI || (c.errnum == eGUI && c.param == E_INTERNAL))
			throw;	// vyjimky, ktere nejsou od GUI nebo jsou fatalniho charakteru vyhazuji dale do enginu => konec GUI
					// vyjimky typu E_INTERNAL nejsou pro chod GUI fatalne kriticke muze pokracovat dale (nyni se ukoncuje z ladicich duvodu)
	}
	catch (CKernelPanic){
		delete event;	// smazu udalost co vyjimku zpusobila

		if(editor)
			editor->KernelShutDown();
		else
		{
			KerErrorFiles->ShowAllLogs(0,40,640,200);
			SAFE_DELETE(KerMain);
		}
	}

	return 0;
}

CGUIEvent *CGUIEventServer::GetWaitingEvent()
{
	return eventsQueue.RemoveFromHead();
}

void CGUIEventServer::AddEventToQueue(CGUIEvent *event)
{
	if(event)
	{
		eventsQueue.AddToBack(event);
	}
}

void CGUIEventServer::AddPriorityEventToQueue(CGUIEvent *event)
{
	if(event)
	{
		eventsQueue.AddToHead(event);
	}
}

CGUIEvent *CGUIEventServer::RemoveEventFromQueue(typeID ID)
{
	return ::Remove<CGUIEvent,typeID>(&eventsQueue,ID);
}

void CGUIEventServer::DeleteAllEventsFromQueue(typeID ID)
{
	CGUIEvent *event;
	CGUIListElement<CGUIEvent>* le = eventsQueue.GetNextListElement(0);

	while(le)
	{
		if((event=eventsQueue.GetObjectFromListElement(le)) && (event->recipient==ID))
		{
			CGUIListElement<CGUIEvent>* oldle=le;
			le=eventsQueue.GetNextListElement(le);
			delete eventsQueue.RemoveListElement(oldle);
		}else
			le=eventsQueue.GetNextListElement(le);
	}
}

void CGUIEventServer::DeleteEventsQueue()
{
	CGUIEvent *event;
	while(event=eventsQueue.RemoveFromHead())
		delete event;
}

void CGUIEventServer::SendEvent(int eventID, typeID sender, typeID recipient, int pInt, typeID pID, void* pAddr, float pFloat, int pInt2, typeID pID2, float pFloat2)
{
	CGUIEvent* e = new CGUIEvent();
	e->eventID = eventID;
	e->sender = sender;
	e->recipient = recipient;
	e->pInt = pInt;
	e->pID = pID;
	e->pAddr = pAddr;
	e->pFloat = pFloat;
	e->pInt2 = pInt2;
	e->pID2 = pID2;
	e->pFloat2 = pFloat2;

	eventServer->AddEventToQueue(e);
}

void CGUIEventServer::SendPriorityEvent(int eventID, typeID sender, typeID recipient, int pInt, typeID pID, void* pAddr, float pFloat, int pInt2, typeID pID2, float pFloat2)
{
	CGUIEvent* e = new CGUIEvent();
	e->eventID = eventID;
	e->sender = sender;
	e->recipient = recipient;
	e->pInt = pInt;
	e->pID = pID;
	e->pAddr = pAddr;
	e->pFloat = pFloat;
	e->pInt2 = pInt2;
	e->pID2 = pID2;
	e->pFloat2 = pFloat2;

	eventServer->AddPriorityEventToQueue(e);
}


