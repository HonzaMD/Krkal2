///////////////////////////////////////////////
//
// editorLevelWin.cpp
//
//	implementace pro propojeni GUI s game window enginu
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "genmain.h"

#include "editorLevelWin.h"
#include "ObjProperty.h"
#include "editor.h"
#include "menu.h"
#include "desktop.h"
#include "widgets.h"

#include "dx.h"
#include "krkal.h"

#include "kernel.h"
#include "levelmap.h"
#include "ComConstants.h"


CEDLevelWindow::CEDLevelWindow(float _x, float _y, float _sx, float _sy)
	: CGUIGameWindow(_x,_y,_sx,_sy)
{
	RemoveFromTrash();
	title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("EI_Map")->GetTexture(0)));
	title->SetText("Map");
	if(KerMain && GEnMain)
	{
		cur = GEnMain->CreateCursor(KerMain->MapInfo->CellType,(float)KerMain->MapInfo->CellSizeX,(float)KerMain->MapInfo->CellSizeY,(float)KerMain->MapInfo->CellSizeZ);

		for(int i=0;i<8;i++)
			curs[i]=GEnMain->CreateCursor(KerMain->MapInfo->CellType,(float)KerMain->MapInfo->CellSizeX,(float)KerMain->MapInfo->CellSizeY,(float)KerMain->MapInfo->CellSizeZ);

		cellMarker = GEnMain->CreateCursor(KerMain->MapInfo->CellType,(float)KerMain->MapInfo->CellSizeX,(float)KerMain->MapInfo->CellSizeY,(float)KerMain->MapInfo->CellSizeZ,CURSOR_CELL_MARKER_COLOR,CURSOR_CELL_MARKER_COLOR2);
		//GEnMain->DrawCursor(cur,(float)curX,(float)curY,(float)curZ);
		//GEnMain->SetVisible(cur,0);
	}
	else
	{
		cur=0;
		for(int i=0;i<8;i++)
			curs[i]=0;
		cellMarker=0;
	}

	pointMarker = new CGUIRectHost(0,0,styleSet->Get("PointMarker")->GetTexture(0));
	pointMarker->AddToEngine(markerWin);
	pointMarker->SetVisible(0);

	areaMarkerRect = new CGUIRectHost(0,0,10,10,CURSOR_AREA_MARKER_COLOR2);
	areaMarkerRect->AddToEngine(markerWin);
	areaMarkerRect->SetVisible(0);

	areaMarkerBorder[0] = new CGUILine(0,0,10,0,CURSOR_CELL_MARKER_COLOR);
	areaMarkerBorder[1] = new CGUILine(10,0,10,10,CURSOR_CELL_MARKER_COLOR);
	areaMarkerBorder[2] = new CGUILine(10,10,0,10,CURSOR_CELL_MARKER_COLOR);
	areaMarkerBorder[3] = new CGUILine(0,10,0,0,CURSOR_CELL_MARKER_COLOR);

	for(int i = 0; i<4; i++)
	{
		areaMarkerBorder[i]->AddToEngine(markerWin);
		areaMarkerBorder[i]->SetVisible(0);
	}

	cursorCellSet=false;
	colX=colY=colZ=0;

	objDeleteType=0;
	layerDeleteType=0;
	mouseButtonDown=false;
	mouseClicked=false;
	sdx=sdy=0;

	scrolling=false;
	scroll_x=scroll_y=0;

	lx=ly=lz=0;

	MsgAcceptConsume(MsgMouseL|MsgMouseR|MsgMouseOver|MsgKeyboard|MsgKeyboardUp|MsgTimer);

	keyClose=false;

	AddToTrash();
}


CEDLevelWindow::~CEDLevelWindow()
{
	timerGUI->DeleteAllRequests(this);

	// odstraneni oznacovacu z enginu + smazani
	pointMarker->RemoveFromEngine();
	delete pointMarker;

	areaMarkerRect->RemoveFromEngine();
	delete areaMarkerRect;

	for(int i = 0; i<4; i++)
	{
		areaMarkerBorder[i]->RemoveFromEngine();
		delete areaMarkerBorder[i];
	}

	if(editor)
	{
		CEDCellObjectsList *ol=dynamic_cast<CEDCellObjectsList*>(nameServer->ResolveID(editor->cellObjectsListID));
		if(ol && desktop)
		{
			desktop->DeleteBackElem(ol);
		}
	}

	hold=false;
	ShowStdCursor();

	///*
	if(GEnMain)
	{
		if(cur)
			GEnMain->DeleteCursor(cur);

		for(int i=0;i<8;i++)
			if(curs[i])
				GEnMain->DeleteCursor(curs[i]);

		if(cellMarker)
			GEnMain->DeleteCursor(cellMarker);
	}

	if(Input->IsGameKeysDisabled())
		Input->DisableGameKeys(0);

	//*/
}


int CEDLevelWindow::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	int ret = 1; //CGUIGameWindow::MouseLeft(x,y,mouseState,keyState);

	if(KerMain && ret && editor->GetEditMod()!=eEMnormal && mouseState)
	{
		InnerCoords(x,y);
		if(TransformCoordsFrontToBack(x,y))
		{
			float xx=x, yy=y;
			x-=shiftX;
			y-=shiftY;
			int cx,cy,cz;
			int curX,curY,curZ;
			KerMain->MapInfo->FindCell((int)x,(int)y,0,cx,cy,cz);
			KerMain->MapInfo->FindCellPosition(cx,cy,cz,curX,curY,curZ);

			mouseClicked=true;

			if(STD_GAMEWIN_SCROLLING_INWIN_USE)
			{
				if(!timerGUI->ContainRequest(this))
				{
					float vx,vy,vsx,vsy;
					float scrollInterval = STD_GAMEWIN_SCROLLING_INWIN_LIMIT;
					GetVPSize(vsx,vsy);
					GetVPPos(vx,vy);

					if(xx<vx+scrollInterval)
					{
						mouseButtonDown=true;
						sdx = xx - (vx+scrollInterval);
					}
					else if(xx > vx+vsx-scrollInterval)
					{
						mouseButtonDown=true;
						sdx = xx - (vx+vsx-scrollInterval);
					}else
					{
						sdx = 0;
						mouseButtonDown=false;
					}

					
					if(yy<vy+scrollInterval)
					{
						mouseButtonDown=true;
						sdy = yy - (vy+scrollInterval);
					}
					else if(yy > vy+vsy-scrollInterval)
					{
						mouseButtonDown=true;
						sdy = yy - (vy+vsy-scrollInterval);
					}
					else
						sdy = 0;

					if(mouseButtonDown)
					{
						timerGUI->AddRequest(this,STD_GAMEWIN_SCROLLING_INWIN_TIMER);
					}
				}else
					mouseButtonDown=false;
			}



			int continuousMod;
			if(editor->GetContinuosMod()==1)
			{
				continuousMod=1;
			}else{
				continuousMod=0;
			}

			if(editor->GetEditMod()==eEMinsert)
			{
				CEDObjectProperty* ID(op,editor->objPropertyID);
				if(op && !op->objAlreadyPlaced)
				{
					if(editor->GetFreeMod())
						op->PlaceObjToMap((int)x,(int)y,0,false,false);
					else
					{
						if(editor->GetLargeMod())
						{
							PlaceObjectLarge(op,cx,cy,cz,false);
						}else
							op->PlaceObjToMap(curX,curY,curZ,true,false);
					}
				}
			}
			else if(editor->GetEditMod()==eEMmoveObj)
			{
				CEDObjectProperty* ID(op,editor->objPropertyID);
				if(op && op->objAlreadyPlaced)
				{
					if(editor->GetFreeMod())
						op->PlaceObjToMap((int)x,(int)y,0,false,false);
					else
						op->PlaceObjToMap(curX,curY,curZ,true,false);
				}
			}
			else if(editor->GetEditMod()==eEMdelete)
			{
				DeleteObject(x,y,true);
				if(editor->GetLargeMod())
					DeleteObjectsFrom3x3Cells(cx,cy,cz);
			}
			else if(editor->GetEditMod()==eEMselect)
			{
				SelectObject(x,y);
			}
			else if(editor->GetEditMod()==eEMselectCell)
			{
				CreateCellObjectsList(cx,cy,cz);
			}
			else if(editor->GetEditMod()==eEMselectObj)
			{
				SelectObjectVar(x,y);
			}
			else if(editor->GetEditMod()==eEMnoConnect)
			{
				SelectObject(x,y);
				SetObjNoConnect(x,y);
			}
			else if(editor->GetEditMod()==eEMselectLocation)
			{
				SetPoint(1,(int)x,(int)y,0,cx,cy,cz);
			}

			ShowCursorByEditMod(cx,cy,cz);
		}
	}
	else
		mouseClicked=false;

	return ret;
}

int CEDLevelWindow::MouseOver(float x, float y, UINT over,UINT state)
{
	int ret = CGUIGameWindow::MouseOver(x,y,over,state);

	float win_x=x, win_y=y;	// poloha kurzoru v souradnicich materskeho okna

	if(KerMain && ret) // && editor->GetEditMod()!=eEMnormal
	{
		InnerCoords(x,y);
		int tr = TransformCoordsFrontToBack(x,y);

		float xx=x, yy=y;
		x-=shiftX;
		y-=shiftY;
		int cx,cy,cz;
		int curX,curY,curZ;
		KerMain->MapInfo->FindCell((int)x,(int)y,0,cx,cy,cz);
		KerMain->MapInfo->FindCellPosition(cx,cy,cz,curX,curY,curZ);


		float scroll_dx=0, scroll_dy=0;	// o kolik se ma okno prescrollovat

		if(mainGUI->input->IsKeyDown(DIK_SPACE))
		{
			if(scrolling)
			{
				scroll_dx = win_x-scroll_x;		// nastavi o kolik se kurzor posunul vuci sve pocatecni pozici
        		scroll_dy = win_y-scroll_y;		// (kurzor se po kazdem prescrollovani vraci na svou puvodni pozici)
			}
			else
			{
				if(!IsFocused())
					FocusMe();
				scrolling=true;		// nastartovani scrollingu
				scroll_x=win_x;		// zapamatovat si pocatecni pozici kurzoru (vuci ni se bude scrollovat)
				scroll_y=win_y;
				//GetVPPos(svp_x,svp_y);	// zapamatovat si pocatecni pozici VP okna (jak je okno nascrollovano)
				//mainGUI->cursor->GetPos(ccx,ccy);	// pocatecni pozice kurzoru v souradnicich aplikace (desktopu)

				// princip prace:
				// zjistim aktualni zmenu pozice kurzoru (scroll_dx,scroll_dy)
				// o stejnou cast posunu vnitrkem okna (scrolling VP)
			}

			if(scroll_dx || scroll_dy)
			{
				float vx,vy,nx,ny;
				float curx, cury;
				float newCurX, newCurY;

				GetVPPos(vx,vy);	// aktualni pozice VP (stav nascrollovani okna)

				SetWindowPositionVP(scroll_dx,scroll_dy);	// posunu okno o velikost zmeny pozice kurzoru
								// okno nemusi prescrollovat o celou cast (zarazi se na krajich)

				GetVPPos(nx,ny);	// zjistim novou pozici VP (po prescrollovani)

				mainGUI->cursor->GetPos(curx,cury);	// aktualni pozice kurzoru (curx,cury)

				// vypocet nove pozice kurzoru:
				// snazim se vratit na puvodni misto - pokud uspelo prescrollovani na novou pozici 
				// pokud prescrollovat neslo, necham kurzor posunuty 
				// (kvuli moznosti najet do rohu okna, kdyz uz nelze dale scrollovat)
				// (pocita i s castecnym prescrollovanim)

				// musim si uvedomit, ze nyni je kurzor posunut. Pokud ho chci drzet na miste, musim ho
				// vratit do puvodni polohy, pokud s nim chci hybat, musim ho nechat tam, jak je ted
				newCurX = curx - (nx-vx);		// (nx-vx) je skutecna zmena, o kterou se okno prescrollovalo
				newCurY = cury - (ny-vy);		// cur je aktualni pozice kurzoru, tedy posunuta vuci pocatecni
												// newCur je nova pozice kurzoru, kam ho chci presunout

				//KRKAL->SetCursorPos((int)ccx,(int)ccy);
				if(curx!=newCurX || cury!=newCurY)	// pokud chci kurzor presnout, udelej to:
					KRKAL->SetCursorPos((int)newCurX,(int)newCurY);	// presune kurzor v aplikaci + posle novou zpravu o pozici kurzoru
			}
		}
		else
		{
			scrolling=false;
		}


		if(tr)
		{

			if(cursorCellSet)
			{
				if(curX==colX && curY==colY && curZ==colZ)
				{
					if((editor->GetLargeMod() && (editor->GetEditMod()==eEMinsert || editor->GetEditMod()==eEMdelete)) || !editor->GetFreeMod())
					{
						return ret;
					}
					else
						cursorCellSet=false;

				}
				else
				{
					colX = curX; colY = curY; colZ = curZ;
					if(!(editor->GetLargeMod() && (editor->GetEditMod()==eEMinsert || editor->GetEditMod()==eEMdelete)) && editor->GetFreeMod())
					{
						cursorCellSet=false;
					}
				}
			}
			else
			{
				colX = curX; colY = curY; colZ = curZ;
				if((!editor->GetFreeMod() && editor->GetEditMod()!=eEMselectLocation) || (editor->GetLargeMod() && (editor->GetEditMod()==eEMinsert || editor->GetEditMod()==eEMdelete)))
					cursorCellSet=true;
			}

			bool cursorShown=false;
				
			if(editor->GetContinuosMod() && IsFocused())
			{
				if((state & MK_LBUTTON) && mouseClicked)
				{
					if(editor->GetEditMod()==eEMinsert)
					{
						CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
						if(op && !op->objAlreadyPlaced)
						{
							if(editor->GetFreeMod())
								op->PlaceObjToMap((int)x,(int)y,0,false,true);
							else
							{
								if(editor->GetLargeMod())
								{
									PlaceObjectLarge(op,cx,cy,cz,true);
								}else
									op->PlaceObjToMap(curX,curY,curZ,true,true);
							}
						}
					}
					else if(editor->GetEditMod()==eEMdelete)
					{
						DeleteObject(x,y,false);
						if(editor->GetLargeMod())
							DeleteObjectsFrom3x3Cells(cx,cy,cz);
					}
					else if(editor->GetEditMod()==eEMselect)
					{
						SelectObject(x,y);
					}
					else if(editor->GetEditMod()==eEMselectCell)
					{
						CreateCellObjectsList(cx,cy,cz);
					}
					else if(editor->GetEditMod()==eEMselectLocation)
					{
						SetPoint(2,(int)x,(int)y,0,cx,cy,cz);
					}
				}
				else if((state & MK_RBUTTON))
				{
					if(editor->GetEditMod()==eEMselectLocation)
					{
						SetPoint(2,(int)x,(int)y,0,cx,cy,cz);
					}
					else
					{
						/*
						if(editor->GetEditMod()==eEMinsert || editor->GetEditMod()==eEMmoveObj || editor->GetEditMod()==eEMselectCell)
							DeleteObjectLikeSelected(cx,cy,cz,false);
						else
							DeleteObject(x,y,false);
							*/
						if(editor->GetFreeMod())
							DeleteObjectLikeSelectedFree(x,y,false);
						else
							DeleteObjectLikeSelected(cx,cy,cz,false);
						
						if(editor->GetLargeMod())
							DeleteObjectsFrom3x3Cells(cx,cy,cz);

						HideAnyCursor();

						ShowCursorDelete(true,cx,cy,cz);
						cursorShown=true;
					}
				}
			}

			if(!cursorShown)
			{
				ShowCursorByEditMod(cx,cy,cz);
			}

		}else{
			if(cursorChanged)
				ShowStdCursor();
		}
	}
	else if(cursorChanged)
		ShowStdCursor();

	return ret;
}

int CEDLevelWindow::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	int ret = 1; //CGUIGameWindow::MouseLeftFocus(x,y,mouseState,keyState,ID);

	mouseClicked=false;

	return ret;
}

int CEDLevelWindow::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	int ret = CGUIGameWindow::MouseOverFocus(x,y,over,state,ID);

	if(cursorChanged)
		ShowStdCursor();

	if(mainGUI->input->IsKeyDown(DIK_SPACE) && scrolling)
	{
		float scroll_dx=0, scroll_dy=0;

		scroll_dx = x-scroll_x;
        scroll_dy = y-scroll_y;

		if(scroll_dx || scroll_dy)
		{
			float vx,vy,nx,ny;
			float curx, cury;
			float newCurX, newCurY;

			GetVPPos(vx,vy);	// aktualni pozice VP (stav nascrollovani okna)

			SetWindowPositionVP(scroll_dx,scroll_dy);	// posunu okno o velikost zmeny pozice kurzoru
							// okno nemusi prescrollovat o celou cast (zarazi se na krajich)

			GetVPPos(nx,ny);	// zjistim novou pozici VP (po prescrollovani)

			mainGUI->cursor->GetPos(curx,cury);	// aktualni pozice kurzoru (curx,cury)

			// vypocet nove pozice kurzoru:
			// snazim se vratit na puvodni misto - pokud uspelo prescrollovani na novou pozici 
			// pokud prescrollovat neslo, necham kurzor posunuty 
			// (kvuli moznosti najet do rohu okna, kdyz uz nelze dale scrollovat)
			// (pocita i s castecnym prescrollovanim)

			// musim si uvedomit, ze nyni je kurzor posunut. Pokud ho chci drzet na miste, musim ho
			// vratit do puvodni polohy, pokud s nim chci hybat, musim ho nechat tam, jak je ted
			newCurX = curx - (nx-vx);		// (nx-vx) je skutecna zmena, o kterou se okno prescrollovalo
			newCurY = cury - (ny-vy);		// cur je aktualni pozice kurzoru, tedy posunuta vuci pocatecni
											// newCur je nova pozice kurzoru, kam ho chci presunout

			//KRKAL->SetCursorPos((int)ccx,(int)ccy);
			if(curx!=newCurX || cury!=newCurY)	// pokud chci kurzor presnout, udelej to:
				KRKAL->SetCursorPos((int)newCurX,(int)newCurY);	// presune kurzor v aplikaci + posle novou zpravu o pozici kurzoru
		}
	}

	return ret;
}

int CEDLevelWindow::FocusChanged(UINT focus, typeID dualID)
{
	int ret = CGUIGameWindow::FocusChanged(focus,dualID);

	if(focus)
	{
		if(Input->IsGameKeysDisabled())
			Input->DisableGameKeys(0);
	}
	else
	{
		if(!Input->IsGameKeysDisabled())
			Input->DisableGameKeys(1);
	}

	if(!focus ) // && cursorChanged
		ShowStdCursor();

	return ret;
}

/*
int CEDLevelWindow::Keyboard(UINT character, UINT state)
{
	int ret = CGUIGameWindow::Keyboard(character,state);

	if(ret && (character == VK_CONTROL || character == VK_SHIFT))
		mainGUI->SendCursorPos();

	return ret;
}

int CEDLevelWindow::KeyboardUp(UINT character, UINT state)
{
	int ret = CGUIGameWindow::KeyboardUp(character,state);

	if(ret && (character == VK_CONTROL || character == VK_SHIFT))
		mainGUI->SendCursorPos();

	return ret;
}
*/

int CEDLevelWindow::MouseRight(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	int ret = CGUIGameWindow::MouseRight(x,y,mouseState,keyState);

	if(KerMain && ret && editor->GetEditMod()!=eEMnormal && mouseState)
	{
		InnerCoords(x,y);
		if(TransformCoordsFrontToBack(x,y))
		{
			x-=shiftX;
			y-=shiftY;
			int cx,cy,cz;
			KerMain->MapInfo->FindCell((int)x,(int)y,0,cx,cy,cz);

			if(editor->GetEditMod()==eEMselectLocation)
			{
				SetPoint(2,(int)x,(int)y,0,cx,cy,cz);
			}
			else
			{
				/*
				if(editor->GetEditMod()==eEMinsert || editor->GetEditMod()==eEMmoveObj || editor->GetEditMod()==eEMselectCell)
					DeleteObjectLikeSelected(cx,cy,cz,true);
				else
					DeleteObject(x,y,true);
					*/
				if(editor->GetFreeMod())
					DeleteObjectLikeSelectedFree(x,y,true);
				else
					DeleteObjectLikeSelected(cx,cy,cz,true);

				if(editor->GetLargeMod())
					DeleteObjectsFrom3x3Cells(cx,cy,cz);

				HideAnyCursor();
				//ShowCursorInsert(false,cx,cy,cz);
				//ShowCursorDelete(false,cx,cy,cz);

				ShowCursorDelete(true,cx,cy,cz);
			}
		}
	}else if(!mouseState)
	{
		InnerCoords(x,y);
		if(TransformCoordsFrontToBack(x,y))
		{
			x-=shiftX;
			y-=shiftY;
			int cx,cy,cz;
			KerMain->MapInfo->FindCell((int)x,(int)y,0,cx,cy,cz);

			objDeleteType=0;
			layerDeleteType=0;

			ShowCursorByEditMod(cx,cy,cz);
		}
	}

	return ret;
}

int CEDLevelWindow::MouseRightFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	int ret = CGUIGameWindow::MouseRightFocus(x,y,mouseState,keyState,ID);

	return ret;
}

int CEDLevelWindow::TimerImpulse(typeID timerID, float time)
{
	if(!STD_GAMEWIN_SCROLLING_INWIN_USE || scrolling)
		return 0;	// vyrazeni scrollingu kolem vnitrnich okraju okna z provozu

	float vx,vy,nx,ny;
	GetVPPos(vx,vy);
	SetWindowPositionVP(sdx,sdy);
	GetVPPos(nx,ny);

	if(nx!=vx || ny!=vy)
	{
		cursorCellSet=false;
		mainGUI->SendCursorPos();
	}

	return 0;
}

int CEDLevelWindow::KeyboardUp(UINT character, UINT state)
{
	int ret = CGUIWindow::KeyboardUp(character,state);

	if(!mainGUI->input->IsKeyDown(DIK_SPACE))
		scrolling=false;	// jen vypnuti scrollingu po vymacknuti SPACE (zapina se u mouseOver)

	return ret;
}

/*
void CEDLevelWindow::UnMaximize()
{
	if(editor)
	{
		editor->RefreshMapPosition();
		windowState=WS_Standard;
		EventArise(EMaximize,0);
	}
	else
		CGUIWindow::UnMaximize();
}
*/

void CEDLevelWindow::Normalize()
{
	if(editor)
	{
		editor->RefreshMapPosition();
		windowState=WS_Normalized;
		EventArise(ENormalize);
	}
	else
		CGUIWindow::Normalize();
}


void CEDLevelWindow::ShowCursorByEditMod(int cx, int cy, int cz)
{
	HideAnyCursor();

	/*
	if(editor->GetEditMod()!=eEMinsert && editor->GetEditMod()!=eEMmoveObj)
		ShowCursorInsert(false,cx,cy,cz);
	if(editor->GetEditMod()!=eEMdelete)
		ShowCursorDelete(false,cx,cy,cz);
*/


	if(editor->GetEditMod()==eEMnormal)
	{
		mainGUI->cursor->Set(0);
		if(!hold)
		{
			cursorChanged=false;
			mainGUI->cursor->SetVisible(1);
		}
	}
	else if(editor->GetEditMod()==eEMinsert || editor->GetEditMod()==eEMmoveObj)
		ShowCursorInsert(true,cx,cy,cz);
	else if(editor->GetEditMod()==eEMdelete)
		ShowCursorDelete(true,cx,cy,cz);
	else if(editor->GetEditMod()==eEMselect)
	{
		mainGUI->cursor->Set(5);
		mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro vyber objektu (free)
		cursorChanged=true;
	}
	else if(editor->GetEditMod()==eEMselectCell)
	{
		ShowCursorSelectCell(cx,cy,cz);
		/*
		mainGUI->cursor->Set(6);
		mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro vyber objektu (cell)
		cursorChanged=true;
		*/
	}
	else if(editor->GetEditMod()==eEMnoConnect)
	{
		mainGUI->cursor->Set(9);
		mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro noConnect (free)
		cursorChanged=true;
	}
	else if(editor->GetEditMod()==eEMselectObj)
	{
		mainGUI->cursor->Set(7);
		mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro selectObj (free)
		cursorChanged=true;
	}
	else if(editor->GetEditMod()==eEMselectLocation)
	{
		if(editor->GetSelectLocationMod() == 0 || editor->GetSelectLocationMod() == 2)
		{
			mainGUI->cursor->Set(11);
			mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro selectLocation (free)
			cursorChanged=true;
		}
		else
		{
			ShowCursorSelectCellLocation(cx,cy,cz);
		}
	}

}

void CEDLevelWindow::ShowStdCursor()
{
	cursorCellSet=false;	// krychlovy kurzor neni nastaven

	// skryji krychlove kurzory
	if(cur)
		cur->SetVisible(0);

	for(int i=0;i<8;i++)
		if(curs[i])
			curs[i]->SetVisible(0);
	if(mainGUI->cursor->GetCursorIndex()>4)
	{
		// zobrazim std. kurzor
		if(editor && editor->GetEditMod()==eEMselectObj)
			mainGUI->cursor->Set(7);
		else
			mainGUI->cursor->Set(0);

	}

	if(!hold)
	{
		if(editor && editor->GetEditMod()!=eEMselectObj)
			cursorChanged=false;
		mainGUI->cursor->SetVisible(1);
	}
}

void CEDLevelWindow::HideAnyCursor()
{
	// skryji krychlove kurzory
	if(cur)
		cur->SetVisible(0);

	for(int i=0;i<8;i++)
		if(curs[i])
			curs[i]->SetVisible(0);

	// skryj std. kurzor
	mainGUI->cursor->SetVisible(0);
	cursorChanged=true;
}


void CEDLevelWindow::ShowCursorInsert(bool show, int cx, int cy, int cz)
{
	int curX, curY, curZ;

	assert(KerMain);
	assert(cur);

	KerMain->MapInfo->FindCellPosition(cx,cy,cz,curX,curY,curZ);

	if(editor->GetFreeMod())
	{
		if(show)
		{
			mainGUI->cursor->Set(10);
			mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro vkladani objektu (free)
			cursorChanged=true;
		}
		else
		{
			assert(false); // !@#$
			mainGUI->cursor->SetVisible(1);		// zobraz std. kurzor
			cursorChanged=false;
		}
		/*
		cur->SetVisible(0);
		for(int i=0;i<8;i++)
			if(curs[i])
				curs[i]->SetVisible(0);
				*/
	}
	else
	{
		if(show)
		{
			CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
			if(op)
			{
				int a = op->IsPossiblePlaceObjToMap(curX,curY,curZ);
				if(a==1)
					cur->SetColor(CURSOR_CELL_INSERT_NORMAL_COLOR,CURSOR_CELL_INSERT_NORMAL_COLOR2);
				else if(a==0)
					cur->SetColor(CURSOR_CELL_INSERT_REPLACE_COLOR,CURSOR_CELL_INSERT_REPLACE_COLOR2);
				else if(a==-1)
					cur->SetColor(CURSOR_CELL_INSERT_ERROR_COLOR,CURSOR_CELL_INSERT_ERROR_COLOR2);
			}else
				cur->SetColor(CURSOR_CELL_INSERT_ERROR_COLOR,CURSOR_CELL_INSERT_ERROR_COLOR2);


			cur->SetCursorPos((float)curX,(float)curY,(float)curZ);
			cur->SetVisible(1);
			if(editor->GetLargeMod() && editor->GetEditMod()!=eEMmoveObj)
				ShowLargeCursorsInsert(cx,cy,cz);
			/*
			else
			{
				for(int i=0;i<8;i++)
					if(curs[i])
						curs[i]->SetVisible(0);
			}
			mainGUI->cursor->SetVisible(0);
			*/
			cursorChanged=true;
		}
		else
		{
			assert(false); // !@#$
			mainGUI->cursor->SetVisible(1);
			cursorChanged=false;
			cur->SetVisible(0);
			for(int i=0;i<8;i++)
				if(curs[i])
					curs[i]->SetVisible(0);
		}
	}
}

void CEDLevelWindow::ShowCursorDelete(bool show, int cx, int cy, int cz)
{
	int curX, curY, curZ;

	assert(KerMain);
	assert(cur);

	if(show)
	{
		if(editor->GetLargeMod())
		{
			cur->SetColor(CURSOR_CELL_INSERT_REPLACE_COLOR,CURSOR_CELL_INSERT_REPLACE_COLOR2);

			KerMain->MapInfo->FindCellPosition(cx,cy,cz,curX,curY,curZ);
			cur->SetCursorPos((float)curX,(float)curY,(float)curZ);
			cur->SetVisible(1);
			ShowLargeCursorsDelete(cx,cy,cz);

			//mainGUI->cursor->SetVisible(0);		// zobraz kurzor pro vyber objektu na smazani
			cursorChanged=true;
		}
		else
		{
			mainGUI->cursor->Set(8);
			mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro vyber objektu na smazani
			cursorChanged=true;

			/*
			cur->SetVisible(0);
			for(int i=0;i<8;i++)
				if(curs[i])
					curs[i]->SetVisible(0);
					*/
		}
	}
	else
	{
		assert(false); // !@#$
		mainGUI->cursor->SetVisible(1);		// zobraz std. kurzor
		cursorChanged=false;

		cur->SetVisible(0);
		for(int i=0;i<8;i++)
			if(curs[i])
				curs[i]->SetVisible(0);
	}
}

void CEDLevelWindow::ShowCursorSelectCellLocation(int cx, int cy, int cz)
{
	int curX, curY, curZ;

	assert(KerMain);
	assert(cur);

	KerMain->MapInfo->FindCellPosition(cx,cy,cz,curX,curY,curZ);

	cur->SetColor(CURSOR_CELL_SELECTLOCATION_COLOR,CURSOR_CELL_SELECTLOCATION_COLOR2);
	cur->SetCursorPos((float)curX,(float)curY,(float)curZ);
	cur->SetVisible(1);
	cursorChanged=true;
}

void CEDLevelWindow::ShowCursorSelectCell(int cx, int cy, int cz)
{
	int curX, curY, curZ;

	assert(KerMain);
	assert(cur);

	KerMain->MapInfo->FindCellPosition(cx,cy,cz,curX,curY,curZ);

	cur->SetColor(CURSOR_SELECTCELL_COLOR,CURSOR_SELECTCELL_COLOR2);
	cur->SetCursorPos((float)curX,(float)curY,(float)curZ);
	cur->SetVisible(1);
	cursorChanged=true;
}

int CEDLevelWindow::SomeObjsToDelete(int cx, int cy, int cz)
{
	assert(KerMain);

	CKerArrObject* oa = KerMain->MapInfo->GetObjsFromCell(0,cx,cy,cz);

	int num = oa->GetCount();
	for(int i = 0;i<num;i++)
	{
		OPointer o = oa->Read(i);
		if(DeleteTypeMatch(o))
			return 1;
	}

	return 0;
}

void CEDLevelWindow::PlaceObjectLarge(CEDObjectProperty *op, int cx, int cy, int cz, bool check)
{
	int ccx,ccy,ccz;
	int curX, curY, curZ;
	ccz=cz;

	ccx=cx-1;ccy=cy-1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
	ccx=cx;ccy=cy-1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
	ccx=cx+1;ccy=cy-1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
	ccx=cx-1;ccy=cy;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
	ccx=cx;ccy=cy;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
	ccx=cx+1;ccy=cy;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
	ccx=cx-1;ccy=cy+1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
	ccx=cx;ccy=cy+1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
	ccx=cx+1;ccy=cy+1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
	{
		KerMain->MapInfo->FindCellPosition(ccx,ccy,ccz,curX,curY,curZ);
		op->PlaceObjToMap(curX,curY,curZ,true,check);
	}
}

void CEDLevelWindow::SetInsertCursorColor(int curIndex, CEDObjectProperty *op, int cx, int cy, int cz)
{
	assert(KerMain);
	assert(curs);
	int curX, curY, curZ;

	KerMain->MapInfo->FindCellPosition(cx,cy,cz,curX,curY,curZ);
	curs[curIndex]->SetCursorPos((float)curX,(float)curY,(float)curZ);
	curs[curIndex]->SetVisible(1);

	int placable = op->IsPossiblePlaceObjToMap(curX,curY,curZ);
	if(placable==1)
		curs[curIndex]->SetColor(CURSOR_CELL_INSERT_NORMAL_COLOR,CURSOR_CELL_INSERT_NORMAL_COLOR2);
	else if(placable==0)
		curs[curIndex]->SetColor(CURSOR_CELL_INSERT_REPLACE_COLOR,CURSOR_CELL_INSERT_REPLACE_COLOR2);
	else if(placable==-1)
		curs[curIndex]->SetColor(CURSOR_CELL_INSERT_ERROR_COLOR,CURSOR_CELL_INSERT_ERROR_COLOR2);
}

void CEDLevelWindow::SetDeleteCursorColor(int curIndex, int cx, int cy, int cz)
{
	assert(KerMain);
	assert(curs);
	int curX, curY, curZ;

	KerMain->MapInfo->FindCellPosition(cx,cy,cz,curX,curY,curZ);
	curs[curIndex]->SetCursorPos((float)curX,(float)curY,(float)curZ);
	curs[curIndex]->SetVisible(1);

	curs[curIndex]->SetColor(CURSOR_CELL_INSERT_REPLACE_COLOR,CURSOR_CELL_INSERT_REPLACE_COLOR2);

	/*
	if(SomeObjsToDelete(cx,cy,cz))
		curs[curIndex]->SetColor(CURSOR_CELL_INSERT_NORMAL_COLOR,CURSOR_CELL_INSERT_NORMAL_COLOR2);
	else
		curs[curIndex]->SetColor(CURSOR_CELL_INSERT_REPLACE_COLOR,CURSOR_CELL_INSERT_REPLACE_COLOR2);
		*/
}


void CEDLevelWindow::ShowLargeCursorsInsert(int cx, int cy, int cz)
{
	int ccx,ccy,ccz;
	ccz=cz;

	CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
	if(op)
	{
		ccx=cx-1;ccy=cy-1;
		if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[0])
		{
			SetInsertCursorColor(0,op,ccx,ccy,ccz);
		}
		else if(curs[0])
			curs[0]->SetVisible(0);

		ccx=cx;ccy=cy-1;
		if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[1])
		{
			SetInsertCursorColor(1,op,ccx,ccy,ccz);
		}
		else if(curs[1])
			curs[1]->SetVisible(0);

		ccx=cx+1;ccy=cy-1;
		if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[2])
		{
			SetInsertCursorColor(2,op,ccx,ccy,ccz);
		}
		else if(curs[2])
			curs[2]->SetVisible(0);

		ccx=cx-1;ccy=cy;
		if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[3])
		{
			SetInsertCursorColor(3,op,ccx,ccy,ccz);
		}
		else if(curs[3])
			curs[3]->SetVisible(0);

		ccx=cx+1;ccy=cy;
		if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[4])
		{
			SetInsertCursorColor(4,op,ccx,ccy,ccz);
		}
		else if(curs[4])
			curs[4]->SetVisible(0);

		ccx=cx-1;ccy=cy+1;
		if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[5])
		{
			SetInsertCursorColor(5,op,ccx,ccy,ccz);
		}
		else if(curs[5])
			curs[5]->SetVisible(0);

		ccx=cx;ccy=cy+1;
		if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[6])
		{
			SetInsertCursorColor(6,op,ccx,ccy,ccz);
		}
		else if(curs[6])
			curs[6]->SetVisible(0);

		ccx=cx+1;ccy=cy+1;
		if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[7])
		{
			SetInsertCursorColor(7,op,ccx,ccy,ccz);
		}
		else if(curs[7])
			curs[7]->SetVisible(0);

	}
}


void CEDLevelWindow::ShowLargeCursorsDelete(int cx, int cy, int cz)
{
	int ccx,ccy,ccz;
	ccz=cz;

	ccx=cx-1;ccy=cy-1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[0])
	{
		SetDeleteCursorColor(0,ccx,ccy,ccz);
	}
	else if(curs[0])
		curs[0]->SetVisible(0);

	ccx=cx;ccy=cy-1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[1])
	{
		SetDeleteCursorColor(1,ccx,ccy,ccz);
	}
	else if(curs[1])
		curs[1]->SetVisible(0);

	ccx=cx+1;ccy=cy-1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[2])
	{
		SetDeleteCursorColor(2,ccx,ccy,ccz);
	}
	else if(curs[2])
		curs[2]->SetVisible(0);

	ccx=cx-1;ccy=cy;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[3])
	{
		SetDeleteCursorColor(3,ccx,ccy,ccz);
	}
	else if(curs[3])
		curs[3]->SetVisible(0);

	ccx=cx+1;ccy=cy;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[4])
	{
		SetDeleteCursorColor(4,ccx,ccy,ccz);
	}
	else if(curs[4])
		curs[4]->SetVisible(0);

	ccx=cx-1;ccy=cy+1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[5])
	{
		SetDeleteCursorColor(5,ccx,ccy,ccz);
	}
	else if(curs[5])
		curs[5]->SetVisible(0);

	ccx=cx;ccy=cy+1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[6])
	{
		SetDeleteCursorColor(6,ccx,ccy,ccz);
	}
	else if(curs[6])
		curs[6]->SetVisible(0);

	ccx=cx+1;ccy=cy+1;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz) && curs[7])
	{
		SetDeleteCursorColor(7,ccx,ccy,ccz);
	}
	else if(curs[7])
		curs[7]->SetVisible(0);
}


void CEDLevelWindow::DeleteObjectsFromCell(int cx, int cy, int cz)
{
	if(!KerMain)
		return;

	CKerArrObject* oa = KerMain->MapInfo->GetObjsFromCell(0,cx,cy,cz);

	int num = oa->GetCount();
	for(int i = 0;i<num;i++)
	{
		OPointer o = oa->Read(i);
		DeleteObjectByType(o);
	}

	delete oa;
}

void CEDLevelWindow::CreateCellObjectsList(int cx, int cy, int cz)
{
	if(!KerMain)
		return;

	CEDCellObjectsList *ol=dynamic_cast<CEDCellObjectsList*>(nameServer->ResolveID(editor->cellObjectsListID));
	if(ol)
	{
		desktop->DeleteBackElem(ol);
	}

	CKerArrObject* oa = KerMain->MapInfo->GetObjsFromCell(0,cx,cy,cz);

	ol = new CEDCellObjectsList(editor->listPosition.x,editor->listPosition.y,editor->listPosition.sx,editor->listPosition.sy,oa);
	editor->cellObjectsListID = ol->GetID();
	desktop->AddBackElem(ol);
}

void CEDLevelWindow::SelectObject(float ox, float oy)
{
	if(!KerMain || !GEnMain)
		return;

	CGEnElement* gel = GEnMain->FindTopElement(ox,oy);
	OPointer obj;
	if(gel)
		obj = gel->ObjPtr;
	else
		obj=0;

	CEDObjectProperty* op=0;
	if(op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID)))
	{
		if(op->obj != obj)
		{
			desktop->DeleteBackElem(op);
		}else
		{
			desktop->BringElementToTop(op);		// z hlediska poradi klikatelnosti
			op->BringToTop();					// z hlediska viditelnosti
			return;
		}
	}

	/*	// pokud lze zarucit, ze neni vice nez jeden CEDObjectProperty v editoru, nemusi se hledat touto fci - je pomala
	if(op = ((CEDEditorDesktop*)desktop)->FindObjPropertyWin(obj))
	{
		desktop->BringElementToTop(op);		// z hlediska poradi klikatelnosti
		op->BringToTop();					// z hlediska viditelnosti
		return;	// property okno pro objekt "obj" jiz v desktopu je, nove nevytvaret
	}
	*/

	if(obj && KerMain->Objs->GetObject(obj))
	{
		float ox=0, oy=0, osx=0, osy=0, dsx,dsy;
		desktop->GetVPSize(dsx,dsy);
		if(objectList)
		{
			objectList->GetPos(ox,oy);
			objectList->GetSize(osx,osy);
		}

		op = new CEDObjectProperty(ox,oy+osy,osx,dsy-osy,obj,0,true);
		desktop->AddBackElem(op);
		editor->objPropertyID = op->GetID();
		
		if(objectList)
		{
			op->AcceptEvent(objectList->objHandler->GetID(),EClose);
			op->AcceptEvent(objectList->objHandler->GetID(),EFocusChanged);
		}
	}
}

void CEDLevelWindow::SetObjNoConnect(float ox, float oy)
{
	if(!KerMain || !GEnMain)
		return;

	CGEnElement* gel = GEnMain->FindTopElement(ox,oy);
	OPointer obj;
	if(gel)
		obj = gel->ObjPtr;
	else
		obj=0;

	if(obj)
	{
		new CEDNoConnectDialog(obj);
		mainGUI->SendCursorPos();
	}
	else
	{
		CGUIElement* el = GetObjFromID<CGUIToolBar>(editor->noConnectDialogID);
		if(el)
			desktop->DeleteBackElem(el);
	}
}

void CEDLevelWindow::SelectObjectVar(float ox, float oy)
{
	if(!KerMain || !GEnMain)
		return;

	CGEnElement* gel = GEnMain->FindTopElement(ox,oy);
	OPointer obj;
	if(gel)
		obj = gel->ObjPtr;
	else
		obj=0;

	CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(editor->objVarSelectionID);
	if(opl)
		opl->SetObject(obj);
}


void CEDLevelWindow::DeleteObject(float ox, float oy, bool first)
{
	if(!KerMain || !GEnMain)
		return;

	CGEnElement* gel = GEnMain->FindTopElement(ox,oy);
	if(gel)
	{
		if(first)
		{
			objDeleteType = KerMain->GetObjType(gel->ObjPtr);
			CKerObject* ko = KerMain->Objs->GetObject(gel->ObjPtr);
			if(ko)
				layerDeleteType = ko->PlacedInfo->CollCfg & eKCClevelMask;
			
			CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(editor->objPropertyID);
			if(op && op->obj == gel->ObjPtr)
			{
				op->CloseWindow();
				CEDCellObjectsList* ID(col,editor->cellObjectsListID);
				if(col)
					col->RemoveObjectFromList(gel->ObjPtr);
			}
			KerMain->DeleteObject(0,gel->ObjPtr);
		}
		else
		{
			DeleteObjectByType(gel->ObjPtr);
		}
	}
	else if(first)
	{
		objDeleteType=0;
		layerDeleteType = 0;
	}
}

void CEDLevelWindow::DeleteObjectLikeSelected(int cx, int cy, int cz, bool first)
{
	if(!KerMain)
		return;

	if(first)
	{
		CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(editor->objPropertyID);
		if(op)
		{
			objDeleteType = KerMain->GetObjType(op->obj);
			CKerObject* ko = KerMain->GetAccessToObject(op->obj);
			if(ko)
			{
				void* var = KerMain->GetAccessToVar3(ko,eKVUcollizionCfg);
				if(var)
					layerDeleteType = (*(UC*) var) & eKCClevelMask;
				else
					layerDeleteType = eKCCdefault & eKCClevelMask;
			}
		}
		else
		{
			objDeleteType=0;
			layerDeleteType = 0;
		}
	}

	CKerArrObject* oa = KerMain->MapInfo->GetObjsFromCell(0,cx,cy,cz);

	int num = oa->GetCount();
	for(int i = 0;i<num;i++)
	{
		OPointer o = oa->Read(i);
		DeleteObjectByType(o);
	}

	delete oa;
}

void CEDLevelWindow::DeleteObjectLikeSelectedFree(float ox, float oy, bool first)
{
	if(!KerMain || !GEnMain)
		return;

	if(first)
	{
		CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(editor->objPropertyID);
		if(op)
		{
			objDeleteType = KerMain->GetObjType(op->obj);
			CKerObject* ko = KerMain->GetAccessToObject(op->obj);
			if(ko)
			{
				void* var = KerMain->GetAccessToVar3(ko,eKVUcollizionCfg);
				if(var)
					layerDeleteType = (*(UC*) var) & eKCClevelMask;
				else
					layerDeleteType = eKCCdefault & eKCClevelMask;
			}
		}
		else
		{
			objDeleteType=0;
			layerDeleteType = 0;
		}
	}

	CGEnElement* gel = GEnMain->FindTopElement(ox,oy);
	if(gel)
	{
		DeleteObjectByType(gel->ObjPtr);
	}
}

void CEDLevelWindow::DeleteObjectsFrom3x3Cells(int cx, int cy, int cz)
{
	if(!KerMain)
		return;

	int ccx,ccy,ccz;

	ccx=cx-1;ccy=cy-1,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);

	ccx=cx;ccy=cy-1,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);

	ccx=cx+1;ccy=cy-1,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);

	ccx=cx-1;ccy=cy,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);

	ccx=cx;ccy=cy,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);

	ccx=cx+1;ccy=cy,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);

	ccx=cx-1;ccy=cy+1,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);

	ccx=cx;ccy=cy+1,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);

	ccx=cx+1;ccy=cy+1,ccz=cz;
	if(KerMain->MapInfo->IsCellInMap(ccx,ccy,ccz))
		DeleteObjectsFromCell(ccx,ccy,ccz);
}

void CEDLevelWindow::DeleteObjectByType(OPointer obj)
{
	assert(KerMain);
	if(editor->GetSameTypeMod())
	{
		if(KerMain->GetObjType(obj) == objDeleteType)
		{
			CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(editor->objPropertyID);
			if(op && op->obj == obj)
			{
				op->CloseWindow();
				CEDCellObjectsList* ID(col,editor->cellObjectsListID);
				if(col)
					col->RemoveObjectFromList(obj);
			}
			KerMain->DeleteObject(0,obj);
		}
	}
	else
	{
		CKerObject* ko = KerMain->Objs->GetObject(obj);
		if(ko)
		{
			int oCC = ko->PlacedInfo->CollCfg & eKCClevelMask;
			if (oCC == layerDeleteType)
			{
				CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(editor->objPropertyID);
				if(op && op->obj == obj)
				{
					op->CloseWindow();
					CEDCellObjectsList* ID(col,editor->cellObjectsListID);
					if(col)
						col->RemoveObjectFromList(obj);
				}
				KerMain->DeleteObject(0,obj);
			}
		}
	}
}

int CEDLevelWindow::DeleteTypeMatch(OPointer obj)
{
	assert(KerMain);
	if(editor->GetSameTypeMod())
	{
		if(KerMain->GetObjType(obj) == objDeleteType)
		{
			return 1;
		}
	}
	else
	{
		CKerObject* ko = KerMain->Objs->GetObject(obj);
		if(ko)
		{
			int oCC = ko->PlacedInfo->CollCfg & eKCClevelMask;
			if (oCC == layerDeleteType)
//			if((layerDeleteType & eKCCfloorBit) == (oCC & eKCCfloorBit) &&
//				(layerDeleteType & eKCCwall) == (oCC & eKCCwall))
			{
				return 1;
			}
		}
	}
	return 0;
}

void CEDLevelWindow::DeleteMarker()
{
	cellMarker->SetVisible(0);
	pointMarker->SetVisible(0);

	for(int i=0;i<4;i++)
		areaMarkerBorder[i]->SetVisible(0);
	areaMarkerRect->SetVisible(0);
}

void CEDLevelWindow::ShowPointMarker(int px, int py, int pz)
{
	float nsx,nsy;
	GetBackWindowSize(nsx,nsy);

	if(px<-shiftX || py<-shiftY || px>nsx || py>nsy)
		return;

	pointMarker->Move((float)px-14+shiftX,(float)py-14+shiftY);
	pointMarker->SetVisible(1);
}	

void CEDLevelWindow::ShowCellMarker(int px, int py, int pz)
{
	int curX, curY, curZ;

	assert(KerMain);
	assert(cellMarker);

	if(!KerMain->MapInfo->IsCellInMap(px,py,pz))
		return;

	KerMain->MapInfo->FindCellPosition(px,py,pz,curX,curY,curZ);

	cellMarker->SetCursorPos((float)curX,(float)curY,(float)curZ);
	cellMarker->SetVisible(1);
}

void CEDLevelWindow::ShowAreaMarker(int x1, int y1, int z1, int x2, int y2, int z2)
{
	float nsx,nsy;
	GetBackWindowSize(nsx,nsy);

	if(x1<-shiftX || y1<-shiftY || x1>nsx || y1>nsy)
		return;
	if(x2<-shiftX || y2<-shiftY || x2>nsx || y2>nsy)
		return;

	x1+=(int)shiftX;
	y1+=(int)shiftY;
	x2+=(int)shiftX;
	y2+=(int)shiftY;

	areaMarkerBorder[0]->SetPoints((float)x1,(float)y1,(float)x2,(float)y1);
	areaMarkerBorder[1]->SetPoints((float)x2,(float)y1,(float)x2,(float)y2);
	areaMarkerBorder[2]->SetPoints((float)x2,(float)y2,(float)x1,(float)y2);
	areaMarkerBorder[3]->SetPoints((float)x1,(float)y2,(float)x1,(float)y1);

	for(int i = 0; i<4; i++)
	{
		areaMarkerBorder[i]->SetVisible(1);
	}

	areaMarkerRect->Resize((float)x2-x1,(float)y2-y1);
	areaMarkerRect->Move((float)x1,(float)y1);
	areaMarkerRect->SetVisible(1);
}

void CEDLevelWindow::ShowCellAreaMarker(int cx1, int cy1, int cz1, int cx2, int cy2, int cz2)
{
	int x1,y1,z1,x2,y2,z2;

	if(!KerMain)
		return;

	KerMain->MapInfo->FindCellPosition(cx1,cy1,cz1,x1,y1,z1);
	KerMain->MapInfo->FindCellPosition(cx2,cy2,cz2,x2,y2,z2);

	x1 -= KerMain->MapInfo->CellSizeX/2;
	y1 -= KerMain->MapInfo->CellSizeY/2;
	x2 += KerMain->MapInfo->CellSizeX/2;
	y2 += KerMain->MapInfo->CellSizeY/2;

	float nsx,nsy;
	GetBackWindowSize(nsx,nsy);

	if(x1<-shiftX || y1<-shiftY || x1>nsx || y1>nsy)
		return;
	if(x2<-shiftX || y2<-shiftY || x2>nsx || y2>nsy)
		return;

	x1+=(int)shiftX;
	y1+=(int)shiftY;
	x2+=(int)shiftX;
	y2+=(int)shiftY;

	areaMarkerBorder[0]->SetPoints((float)x1,(float)y1,(float)x2,(float)y1);
	areaMarkerBorder[1]->SetPoints((float)x2,(float)y1,(float)x2,(float)y2);
	areaMarkerBorder[2]->SetPoints((float)x2,(float)y2,(float)x1,(float)y2);
	areaMarkerBorder[3]->SetPoints((float)x1,(float)y2,(float)x1,(float)y1);

	for(int i = 0; i<4; i++)
	{
		areaMarkerBorder[i]->SetVisible(1);
	}

	areaMarkerRect->Resize((float)x2-x1,(float)y2-y1);
	areaMarkerRect->Move((float)x1,(float)y1);
	areaMarkerRect->SetVisible(1);
}

void CEDLevelWindow::SetPoint(int type, int px, int py, int pz, int cx, int cy, int cz)
{
	CEDObjectProperty* ID(op,editor->objPropertyID);
	if(!op)
		return;

	if(editor->GetSelectLocationMod() == 0)
	{
		op->SaveLocationVar(0,px,py,pz);
		ShowPointMarker(px,py,pz);
	}
	else if(editor->GetSelectLocationMod() == 1)
	{
		op->SaveLocationVar(0,cx,cy,cz);
		ShowCellMarker(cx,cy,cz);
	}
	else if(editor->GetSelectLocationMod() == 2 && type)
	{
		if(type==1)
		{
			lx = px; ly = py; lz = pz;
			op->SaveLocationVar(1,px,py,pz);
			op->SaveLocationVar(2,px,py,pz);
			ShowAreaMarker(lx,ly,lz,lx,ly,lz);
		}
		else
		{
			int x1,y1,z1,x2,y2,z2;

			x1 = min(px,lx);
			x2 = max(px,lx);
			y1 = min(py,ly);
			y2 = max(py,ly);
			z1 = min(pz,lz);
			z2 = max(pz,lz);

			op->SaveLocationVar(1,x1,y1,z1);
			op->SaveLocationVar(2,x2,y2,z2);
			//op->GetLocationAreaVar(x1,y1,z1,x2,y2,z2);
			ShowAreaMarker(x1,y1,z1,x2,y2,z2);
		}
	}
	else if(editor->GetSelectLocationMod() == 3 && type)
	{
		if(type==1)
		{
			lx = cx; ly = cy; lz = cz;
			op->SaveLocationVar(1,cx,cy,cz);
			op->SaveLocationVar(2,cx,cy,cz);
			ShowCellAreaMarker(lx,ly,lz,lx,ly,lz);
		}
		else
		{
			int x1,y1,z1,x2,y2,z2;

			x1 = min(cx,lx);
			x2 = max(cx,lx);
			y1 = min(cy,ly);
			y2 = max(cy,ly);
			z1 = min(cz,lz);
			z2 = max(cz,lz);

			op->SaveLocationVar(1,x1,y1,z1);
			op->SaveLocationVar(2,x2,y2,z2);
			//op->GetLocationAreaVar(x1,y1,z1,x2,y2,z2);
			ShowCellAreaMarker(x1,y1,z1,x2,y2,z2);
		}
	}
}
