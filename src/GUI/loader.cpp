///////////////////////////////////////////////
//
// loader.cpp
//
// implementace inicializacnich fci GUI a pridruzenych objektu
//
// zajistuje:	loadovani textur do stylu
//				vytvareni std. fontu pro GUI a DX a pridavani do RefMgr
//				loadovani kurzoru
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "GUI.h"
#include "widget.h"

#include "dx.h"
#include "dxbliter.h"
#include "dxfont.h"
#include "refmgr.h"

void CGUI::LoadGUIData()
{
	CGUIStyle* style;
	float *params;

	// ------------------------------------------------------------------------------------------
	// vytvoreni fontu pro DX a ulozeni do seznamu (RefMgr)
	CDXFont *font;
	font=new CDXFont(TIMES);
	RefMgr->Add("DX.F.Times.12",font);
	font->Release();
	font=new CDXFont(COURIER,10);
	RefMgr->Add("DX.F.Courier.10",font);
	font->Release();
	font = new CDXFont(ARIAL,10);
	RefMgr->Add("DX.F.Arial.10",font);
	font->Release();
	font = new CDXFont(ARIAL,8);
	RefMgr->Add("DX.F.Arial.8",font);
	font->Release();
	font = new CDXFont("Tahoma",12);
	RefMgr->Add("DX.F.Tahoma.12",font);
	font->Release();
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// vytvoreni fontu pro GUI a ulozeni do seznamu (RefMgr)
	CGUIFont *guiFont = new CGUIFont();
	guiFont->CreateFont(TIMES,12);
	RefMgr->Add("GUI.F.Times.12",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont(ARIAL,10);
	RefMgr->Add("GUI.F.Arial.10",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont(ARIAL,10,FW_BOLD);
	RefMgr->Add("GUI.F.Arial.10.B",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont(ARIAL,10,0,1);
	RefMgr->Add("GUI.F.Arial.10.I",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont(ARIAL,12);
	RefMgr->Add("GUI.F.Arial.12",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont(COURIER,12);
	RefMgr->Add("GUI.F.Courier.12",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont(COURIER,14,FW_BOLD);
	RefMgr->Add("GUI.F.Courier.14.B",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont(COURIER,18,FW_BOLD);
	RefMgr->Add("GUI.F.Courier.18.B",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont(COURIER,12,FW_BOLD);
	RefMgr->Add("GUI.F.Courier.12.B",guiFont);
	guiFont->Release();

	guiFont = new CGUIFont();
	guiFont->CreateFont("Verdana",10);
	RefMgr->Add("GUI.F.Verdana.10",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont("Verdana",10,FW_BOLD);
	RefMgr->Add("GUI.F.Verdana.10.B",guiFont);
	guiFont->Release();

	guiFont = new CGUIFont();
	guiFont->CreateFont("Tahoma",12);
	RefMgr->Add("GUI.F.Tahoma.12",guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont("Tahoma",12,FW_BOLD);
	RefMgr->Add("GUI.F.Tahoma.12.B",guiFont);
	guiFont->Release();

	guiFont = new CGUIFont();
	guiFont->CreateFontPS("Arial", 14);
	RefMgr->Add("GUI.F.Arial.14PX", guiFont);
	guiFont->Release();

	guiFont = new CGUIFont();
	guiFont->CreateFont("Tahoma", 10);
	RefMgr->Add("GUI.F.Tahoma.10", guiFont);
	guiFont->Release();
	guiFont = new CGUIFont();
	guiFont->CreateFont("Tahoma", 10, FW_BOLD);
	RefMgr->Add("GUI.F.Tahoma.10.B", guiFont);
	guiFont->Release();





	// ------------------------------------------------------------------------------------------

	// promenne pro editor, uvodni spusteni levlu, ...
	/*
#ifdef EDITOR_GAME_MOD
	RUN_EDITOR_SCRIPT = newstrdup("test_0001_FFFF_0001_0001.kc");
	RUN_EDITOR_LEVEL = newstrdup("jedna_44A0_721C_001C_BF01.lv\\!level");
#else
	RUN_EDITOR_LEVEL=0;
	RUN_EDITOR_SCRIPT=0;
#endif
	*/

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro suprbutton (modry vystinovany kulaty)
	style = new CGUIStyle("ButtonUp",9,"$TEX$\\suprbutton\\up");
	params  = new float[4];
	params[0]=31; params[1]=35; params[2]=-3; params[3]=-3;
    style->SetParameters(params,4);
	styleSet->Add(style);

	style = new CGUIStyle("ButtonDown",9,"$TEX$\\suprbutton\\down");
	params  = new float[4];
	params[0]=27; params[1]=28; params[2]=-1; params[3]=-2;
    style->SetParameters(params,4);
	styleSet->Add(style);

	style = new CGUIStyle("ButtonMark",9,"$TEX$\\suprbutton\\sel");
	params  = new float[4];
	params[0]=31; params[1]=35; params[2]=-3; params[3]=-3;
    style->SetParameters(params,4);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro resizery
	style = new CGUIStyle("Resizers",8,"$TEX$\\resizers");
	params  = new float[3];
	params[0]=2; params[1]=2; params[2]=6;
	style->SetParameters(params,3);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro standard button (sedy ctverhrany)
	style = new CGUIStyle("GreyButtonUp",9,"$TEX$\\button\\up");
	params  = new float[4];
	params[0]=27; params[1]=28; params[2]=0; params[3]=0;
    style->SetParameters(params,4);
	styleSet->Add(style);

	style = new CGUIStyle("GreyButtonDown",9,"$TEX$\\button\\down");
	params  = new float[4];
	params[0]=27; params[1]=28; params[2]=0; params[3]=0;
    style->SetParameters(params,4);
	styleSet->Add(style);

	style = new CGUIStyle("TitleBar",1,0,"$TEX$\\Title\\title.png");
	styleSet->Add(style);

	style = new CGUIStyle("Kytka",1,0,"$TEX$\\kytka.jpg");
	styleSet->Add(style);

	style = new CGUIStyle("Ptak",1,0,"$TEX$\\ptak.jpg");
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro scrollbar buttony 
	style = new CGUIStyle("SB_ButtonUp_Up",9,"$TEX$\\scrollbar\\Up");
	params  = new float[4];
	params[0]=0; params[1]=0; params[2]=0; params[3]=0;
    style->SetParameters(params,4);
	styleSet->Add(style);
	style = new CGUIStyle("SB_ButtonUp_Down",9,"$TEX$\\scrollbar\\Up-down");
	params  = new float[4];
	params[0]=0; params[1]=0; params[2]=0; params[3]=0;
    style->SetParameters(params,4);
	styleSet->Add(style);

	style = new CGUIStyle("SB_ButtonPageUp_Up",9,"$TEX$\\scrollbar\\PageUp");
	params  = new float[4];
	params[0]=0; params[1]=0; params[2]=0; params[3]=0;
    style->SetParameters(params,4);
	styleSet->Add(style);
	style = new CGUIStyle("SB_ButtonPageUp_Down",9,"$TEX$\\scrollbar\\PageUp-down");
	params  = new float[4];
	params[0]=0; params[1]=0; params[2]=0; params[3]=0;
    style->SetParameters(params,4);
	styleSet->Add(style);

	style = new CGUIStyle("SB_ButtonSlider_Up",9,"$TEX$\\scrollbar\\Slider");
	params  = new float[4];
	params[0]=0; params[1]=0; params[2]=0; params[3]=0;
    style->SetParameters(params,4);
	styleSet->Add(style);
	style = new CGUIStyle("SB_ButtonSlider_Down",9,"$TEX$\\scrollbar\\Slider-down");
	params  = new float[4];
	params[0]=0; params[1]=0; params[2]=0; params[3]=0;
    style->SetParameters(params,4);
	styleSet->Add(style);

	style = new CGUIStyle("SB_Pictures",7,0,"$TEX$\\scrollbar\\0up.png",
		"$TEX$\\scrollbar\\1slider_v.png", "$TEX$\\scrollbar\\2down.png",
		"$TEX$\\scrollbar\\3left.png", "$TEX$\\scrollbar\\4slider_h.png",
		"$TEX$\\scrollbar\\5right.png", "$TEX$\\scrollbar\\6corner.png");
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro simpleScrollbar buttony 
	style = new CGUIStyle("SSB_ButtonUp_Up",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonUp_Up.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonUp_Down",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonUp_Down.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonPageUp_Up",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonPageUp_Up.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonPageUp_Down",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonPageUp_Down.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonSliderV_Up",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonSliderV_Up.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonSliderV_Down",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonSliderV_Down.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonDown_Up",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonDown_Up.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonDown_Down",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonDown_Down.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);


	style = new CGUIStyle("SSB_ButtonLeft_Up",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonLeft_Up.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonLeft_Down",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonLeft_Down.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonPageLeft_Up",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonPageLeft_Up.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonPageLeft_Down",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonPageLeft_Down.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonSliderH_Up",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonSliderH_Up.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonSliderH_Down",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonSliderH_Down.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonRight_Up",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonRight_Up.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SSB_ButtonRight_Down",1,0,"$TEX$\\scrollbar\\Simple2\\ButtonRight_Down.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro CloseButton
	style = new CGUIStyle("CloseButtonUp",1,0,"$TEX$\\Title\\close_button_up.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("CloseButtonMark",1,0,"$TEX$\\Title\\close_button_mark.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("CloseButtonDown",1,0,"$TEX$\\Title\\close_button_down.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------	

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro MinimizeButton
	style = new CGUIStyle("MinimizeButtonUp",1,0,"$TEX$\\Title\\min_button_up.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("MinimizeButtonMark",1,0,"$TEX$\\Title\\min_button_mark.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("MinimizeButtonDown",1,0,"$TEX$\\Title\\min_button_down.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro MaximizeButton
	style = new CGUIStyle("MaximizeButtonUp",1,0,"$TEX$\\Title\\max_button_up.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("MaximizeButtonMark",1,0,"$TEX$\\Title\\max_button_mark.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("MaximizeButtonDown",1,0,"$TEX$\\Title\\max_button_down.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro DeMaximizeButton
	style = new CGUIStyle("DeMaximizeButtonUp",1,0,"$TEX$\\Title\\demax_button_up.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("DeMaximizeButtonMark",1,0,"$TEX$\\Title\\demax_button_mark.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("DeMaximizeButtonDown",1,0,"$TEX$\\Title\\demax_button_down.png");
	params  = new float[6];
	params[0]=17; params[1]=17; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikonu "Slavia"
	style = new CGUIStyle("Slavia1",1,0,"$TEX$\\icons\\s1.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("Slavia2",1,0,"$TEX$\\icons\\s2.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("Slavia3",1,0,"$TEX$\\icons\\s3.png");
	params  = new float[6];
	params[0]=24; params[1]=24; params[2]=-1; params[3]=-2; params[4]=-5; params[5]=-5;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikonu "SlaviaTB" (TaskBar)
	style = new CGUIStyle("SlaviaTB1",1,0,"$TEX$\\icons\\s1b.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SlaviaTB2",1,0,"$TEX$\\icons\\s2b.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SlaviaTB3",1,0,"$TEX$\\icons\\s3.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("SlaviaTB4",1,0,"$TEX$\\icons\\s4.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikonu "KrkalTB" (TaskBar)
	style = new CGUIStyle("KrkalTB1",1,0,"$TEX$\\icons\\krkal.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("KrkalTB2",1,0,"$TEX$\\icons\\krkal2.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("KrkalTB3",1,0,"$TEX$\\icons\\krkal.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("KrkalTB4",1,0,"$TEX$\\icons\\krkal3.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikonu "EyeTB" (TaskBar)
	style = new CGUIStyle("EyeTB1",1,0,"$TEX$\\icons\\eye1.png");
	params  = new float[6];
	params[0]=20; params[1]=27; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("EyeTB2",1,0,"$TEX$\\icons\\eye1.png");
	params  = new float[6];
	params[0]=20; params[1]=27; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("EyeTB3",1,0,"$TEX$\\icons\\eye2.png");
	params  = new float[6];
	params[0]=34; params[1]=45; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-18;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("EyeTB4",1,0,"$TEX$\\icons\\eye1.png");
	params  = new float[6];
	params[0]=20; params[1]=27; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikonu "LebkyTB" (TaskBar)
	style = new CGUIStyle("LebkyTB1",1,0,"$TEX$\\icons\\lebky1.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("LebkyTB2",1,0,"$TEX$\\icons\\lebky2.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("LebkyTB3",1,0,"$TEX$\\icons\\lebky3.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("LebkyTB4",1,0,"$TEX$\\icons\\lebky4.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro panel (taskbar / windowpanel) - podkladovy pruh
	style = new CGUIStyle("Panel",1,0,"$TEX$\\Panel\\panel.png");
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro menu - tecka u polozky rozbalujici submenu
	style = new CGUIStyle("MenuExtender",1,0,"$TEX$\\menu\\menu_extender.png");
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro panel - styly tlacitek zastupujicich otevrena okna
	style = new CGUIStyle("PanelButton_Up",1,0,"$TEX$\\Panel\\button_up.png");
	params  = new float[6];
	params[0]=100; params[1]=20; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("PanelButton_UpMark",1,0,"$TEX$\\Panel\\button_upMark.png");
	params  = new float[6];
	params[0]=100; params[1]=20; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("PanelButton_Down",1,0,"$TEX$\\Panel\\button_down.png");
	params  = new float[6];
	params[0]=100; params[1]=20; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("PanelButton_DownMark",1,0,"$TEX$\\Panel\\button_downMark.png");
	params  = new float[6];
	params[0]=100; params[1]=20; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ToolBar - styly tlacitek 
	style = new CGUIStyle("ToolBarButton_Up",1,0,"$TEX$\\ToolBar\\up.png");
	params  = new float[6];
	params[0]=43; params[1]=40; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("ToolBarButton_Down",1,0,"$TEX$\\ToolBar\\down.png");
	params  = new float[6];
	params[0]=43; params[1]=40; params[2]=+1; params[3]=+2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("ToolBarButton_MarkUp",1,0,"$TEX$\\ToolBar\\markUp.png");
	params  = new float[6];
	params[0]=43; params[1]=40; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("ToolBarButton_MarkDown",1,0,"$TEX$\\ToolBar\\markDown.png");
	params  = new float[6];
	params[0]=43; params[1]=40; params[2]=+1; params[3]=+2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("ToolBarButton_Inactive",1,0,"$TEX$\\ToolBar\\inactive.png");
	params  = new float[6];
	params[0]=43; params[1]=40; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Tree - obrazky pred polozkou stromu - tecka (list), "+"/"-" zabaleny/rozbaleny strom
	style = new CGUIStyle("Rollers",3,0,"$TEX$\\tree\\roller+.png","$TEX$\\tree\\roller-.png","$TEX$\\tree\\itemator.png");
	styleSet->Add(style);

	/*
	style = new CGUIStyle("Roller-",1,0,"$TEX$\\tree\\roller-.png");
	styleSet->Add(style);

	style = new CGUIStyle("Roller0",1,0,"$TEX$\\tree\\itemator.png");
	styleSet->Add(style);
	*/
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro StartMenu - cudlitko start
	style = new CGUIStyle("Start1",1,0,"$TEX$\\buttons\\start\\start.png");
	params  = new float[6];
	params[0]=50; params[1]=20; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("Start2",1,0,"$TEX$\\buttons\\start\\start2.png");
	params  = new float[6];
	params[0]=50; params[1]=20; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("Start3",1,0,"$TEX$\\buttons\\start\\start3.png");
	params  = new float[6];
	params[0]=50; params[1]=20; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	
	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ObjectBrowser - obrazky u polozek stromu
	style = new CGUIStyle("OB_Object",1,0,"$TEX$\\ObjectBrowser\\object.png");
	styleSet->Add(style);
	style = new CGUIStyle("OB_Method",1,0,"$TEX$\\ObjectBrowser\\method.png");
	styleSet->Add(style);
	style = new CGUIStyle("OB_Attribut",1,0,"$TEX$\\ObjectBrowser\\attribut.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro NameBrowser - zobaky
	style = new CGUIStyle("NB_<<",1,0,"$TEX$\\NameBrowser\\zobakL.png");
	styleSet->Add(style);
	style = new CGUIStyle("NB_>>",1,0,"$TEX$\\NameBrowser\\zobakR.png");
	styleSet->Add(style);


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro CheckBox
	style = new CGUIStyle("CB_Check",1,0,"$TEX$\\checkbox\\checkbox_check.png");
	styleSet->Add(style);
	style = new CGUIStyle("CB_UnCheck",1,0,"$TEX$\\checkbox\\checkbox_uncheck.png");
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro stdbutton
	CGUIStyle *up=0,*down=0,*mark=0,*disabled=0; 
	CGUIButtonStyle *bs=0;

#define CREATE_STYLE_BUT(stylename,dirname,sx,sy,dx,dy,color) \
	style = new CGUIStyle(stylename,9,dirname); \
	params  = new float[4]; \
	params[0]=sx; params[1]=sy; params[2]=dx; params[3]=dy; \
    style->SetParameters(params,4); \
	style->paramUINT = color; \
	styleSet->Add(style);

#define CREATE_STYLE_BUT_SIMPLE(stylename,filename,sx,sy,dx,dy,ptx,pty,color) \
	style = new CGUIStyle(stylename,1,0,filename); \
	params  = new float[6]; \
	params[0]=sx; params[1]=sy; params[2]=dx; params[3]=dy; params[4]=ptx; params[5]=pty;\
    style->SetParameters(params,6); \
	style->paramUINT = color; \
	styleSet->Add(style);


	// ------------------------------------------------------------------------------------------
	//std. button
	up = CREATE_STYLE_BUT("BUT_Std_Up","$TEX$\\buttons\\std\\up",9,9,0,0,STD_BUTTON_LABEL_COLOR);
	down = CREATE_STYLE_BUT("BUT_Std_Down","$TEX$\\buttons\\std\\down",9,9,0,0,STD_BUTTON_LABEL_COLOR);
	mark = CREATE_STYLE_BUT("BUT_Std_Mark","$TEX$\\buttons\\std\\mark",9,9,0,0,STD_BUTTON_LABEL_COLOR);
	disabled = CREATE_STYLE_BUT("BUT_Std_Disabled","$TEX$\\buttons\\std\\disabled",9,9,0,0,STD_BUTTON_LABEL_DISABLED_COLOR);

	CGUIFont* stdbutfont = (CGUIFont*) RefMgr->Find(STD_BUTTON_FONT);

	bs = new CGUIButtonStyle(up,down,mark,disabled,stdbutfont,0);
	RefMgr->Add("GUI.But.Std",bs);
	bs->Release();


	// ------------------------------------------------------------------------------------------
	// BUT_MM2_Left
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MM2_Left_Up","$TEX$\\MainMenu2\\mmLevelZobL.png",13,13,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MM2_Left_Down","$TEX$\\MainMenu2\\mmLevelZobL.png",13,13,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MM2_Left_Mark","$TEX$\\MainMenu2\\mmLevelZobL.png",13,13,0,0,-3,1,0);

	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MM2_Left",bs);
	bs->Release();

	// ------------------------------------------------------------------------------------------
	// BUT_MM2_Right
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MM2_Right_Up","$TEX$\\MainMenu2\\mmLevelZobR.png",13,13,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MM2_Right_Down","$TEX$\\MainMenu2\\mmLevelZobR.png",13,13,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MM2_Right_Mark","$TEX$\\MainMenu2\\mmLevelZobR.png",13,13,0,0,3,1,0);

	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MM2_Right",bs);
	bs->Release();

	// ------------------------------------------------------------------------------------------
	// copy button
	up = CREATE_STYLE_BUT_SIMPLE("BUT_Copy_Up","$TEX$\\buttons\\copy\\copy.png",15,13,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_Copy_Down","$TEX$\\buttons\\copy\\copy_down.png",15,13,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_Copy_Mark","$TEX$\\buttons\\copy\\copy_mark.png",30,28,0,0,-5,-6,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_Copy_Disabled","$TEX$\\buttons\\copy\\copy_disable.png",15,13,0,0,0,0,0);

	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.Copy",bs);
	bs->Release();
	
	// ------------------------------------------------------------------------------------------
	// paste button
	up = CREATE_STYLE_BUT_SIMPLE("BUT_Paste_Up","$TEX$\\buttons\\paste\\paste.png",16,15,0,0,0,-2,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_Paste_Down","$TEX$\\buttons\\paste\\paste_down.png",16,15,0,0,0,-2,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_Paste_Mark","$TEX$\\buttons\\paste\\paste_mark.png",28,28,0,0,-4,-7,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_Paste_Disabled","$TEX$\\buttons\\paste\\paste_disable.png",16,15,0,0,0,-2,0);

	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.Paste",bs);
	bs->Release();

	// ------------------------------------------------------------------------------------------
	// cut button
	up = CREATE_STYLE_BUT_SIMPLE("BUT_Cut_Up","$TEX$\\buttons\\cut\\cut.png",16,16,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_Cut_Down","$TEX$\\buttons\\cut\\cutDown.png",16,16,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_Cut_Mark","$TEX$\\buttons\\cut\\cutMark.png",30,30,0,0,-7,-9,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_Cut_Disabled","$TEX$\\buttons\\cut\\cutDis.png",16,16,0,0,0,0,0);

	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.Cut",bs);
	bs->Release();

	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// undo button
	up = CREATE_STYLE_BUT_SIMPLE("BUT_Undo_Up","$TEX$\\buttons\\undo\\undo.png",16,16,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_Undo_Down","$TEX$\\buttons\\undo\\undoDown.png",16,16,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_Undo_Mark","$TEX$\\buttons\\undo\\undoMark.png",30,30,0,0,-7,-9,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_Undo_Disabled","$TEX$\\buttons\\undo\\undoDis.png",16,16,0,0,0,0,0);
	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.Undo",bs);
	bs->Release();
	// ------------------------------------------------------------------------------------------
	// redo button
	up = CREATE_STYLE_BUT_SIMPLE("BUT_Redo_Up","$TEX$\\buttons\\redo\\redo.png",16,16,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_Redo_Down","$TEX$\\buttons\\redo\\redoDown.png",16,16,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_Redo_Mark","$TEX$\\buttons\\redo\\redoMark.png",30,30,0,0,-7,-9,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_Redo_Disabled","$TEX$\\buttons\\redo\\redoDis.png",16,16,0,0,0,0,0);
	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.Redo",bs);
	bs->Release();
	// ------------------------------------------------------------------------------------------
	// save button
	up = CREATE_STYLE_BUT_SIMPLE("BUT_Save_Up","$TEX$\\buttons\\save\\save.png",16,16,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_Save_Down","$TEX$\\buttons\\save\\saveDown.png",16,16,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_Save_Mark","$TEX$\\buttons\\save\\saveMark.png",30,30,0,0,-7,-9,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_Save_Disabled","$TEX$\\buttons\\save\\saveDis.png",16,16,0,0,0,0,0);
	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.Save",bs);
	bs->Release();
	// ------------------------------------------------------------------------------------------
	// object browser
	up = CREATE_STYLE_BUT_SIMPLE("BUT_ObjectBrowser_Up","$TEX$\\ObjectBrowser\\ObjectBrowser.png",16,16,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_ObjectBrowser_Down","$TEX$\\ObjectBrowser\\ObjectBrowserDown.png",16,16,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_ObjectBrowser_Mark","$TEX$\\ObjectBrowser\\ObjectBrowserMark.png",30,30,0,0,-7,-13,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_ObjectBrowser_Disabled","$TEX$\\ObjectBrowser\\ObjectBrowserDis.png",16,16,0,0,0,0,0);
	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.ObjectBrowser",bs);
	bs->Release();
	// ------------------------------------------------------------------------------------------
	// file browser
	up = CREATE_STYLE_BUT_SIMPLE("FileBrowser","$TEX$\\FileBrowser\\FileBrowser.png",16,16,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("FileBrowser2","$TEX$\\FileBrowser\\FileBrowser2.png",16,16,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("FileBrowser3","$TEX$\\FileBrowser\\FileBrowser3.png",30,30,0,0,-7,-13,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("FileBrowserDis","$TEX$\\FileBrowser\\FileBrowserDis.png",16,16,0,0,0,0,0);
	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.FileBrowser",bs);
	bs->Release();
	// ------------------------------------------------------------------------------------------
	// name browser
	up = CREATE_STYLE_BUT_SIMPLE("BUT_NameBrowser_Up","$TEX$\\NameBrowser\\NameBrowser.png",16,16,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_NameBrowser_Down","$TEX$\\NameBrowser\\NameBrowserDown.png",16,16,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_NameBrowser_Mark","$TEX$\\NameBrowser\\NameBrowserMark.png",30,30,0,0,-7,-13,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_NameBrowser_Disabled","$TEX$\\NameBrowser\\NameBrowserDis.png",16,16,0,0,0,0,0);
	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.NameBrowser",bs);
	bs->Release();

	// ------------------------------------------------------------------------------------------
	// saveall button
	up = CREATE_STYLE_BUT_SIMPLE("BUT_SaveAll_Up","$TEX$\\buttons\\saveall\\saveall.png",16,16,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_SaveAll_Down","$TEX$\\buttons\\saveall\\saveallDown.png",16,16,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_SaveAll_Mark","$TEX$\\buttons\\saveall\\saveallMark.png",30,30,0,0,-7,-9,0);
	disabled = CREATE_STYLE_BUT_SIMPLE("BUT_SaveAll_Disabled","$TEX$\\buttons\\saveall\\saveallDis.png",16,16,0,0,0,0,0);
	bs = new CGUIButtonStyle(up,down,mark,disabled,0,1);
	RefMgr->Add("GUI.But.SaveAll",bs);
	bs->Release();

	// ------------------------------------------------------------------------------------------
	// Main Menu
	
	//play
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MMPlay_Up","$TEX$\\MainMenu\\play.png",149,51,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MMPlay_Down","$TEX$\\MainMenu\\playdown.png",149,51,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MMPlay_Mark","$TEX$\\MainMenu\\playmark.png",177,65,0,0,-14,-7,0);
	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MMPlay",bs);
	bs->Release();
	//exit
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MMExit_Up","$TEX$\\MainMenu\\exit.png",127,48,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MMExit_Down","$TEX$\\MainMenu\\exitdown.png",127,48,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MMExit_Mark","$TEX$\\MainMenu\\exitmark.png",155,62,0,0,-14,-7,0);
	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MMExit",bs);
	bs->Release();
	//edit level
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MMEditLevel_Up","$TEX$\\MainMenu\\editlevel.png",137,78,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MMEditLevel_Down","$TEX$\\MainMenu\\editleveldown.png",137,78,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MMEditLevel_Mark","$TEX$\\MainMenu\\editlevelmark.png",150,87,0,0,-7,-4,0);
	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MMEditLevel",bs);
	bs->Release();
	//edit script
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MMEditScript_Up","$TEX$\\MainMenu\\editscript.png",152,78,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MMEditScript_Down","$TEX$\\MainMenu\\editscriptdown.png",152,78,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MMEditScript_Mark","$TEX$\\MainMenu\\editscriptmark.png",172,89,0,0,-10,-5,0);
	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MMEditScript",bs);
	bs->Release();
	//about
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MMAbout_Up","$TEX$\\MainMenu\\about.png",83,22,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MMAbout_Down","$TEX$\\MainMenu\\aboutdown.png",83,22,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MMAbout_Mark","$TEX$\\MainMenu\\aboutmark.png",120,32,0,0,0,-5,0);
	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MMAbout",bs);
	bs->Release();
	//help
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MMHelp_Up","$TEX$\\MainMenu\\help.png",59,23,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MMHelp_Down","$TEX$\\MainMenu\\helpdown.png",59,23,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MMHelp_Mark","$TEX$\\MainMenu\\helpmark.png",83,32,0,0,0,-5,0);
	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MMHelp",bs);
	bs->Release();
	//restart level
	up = CREATE_STYLE_BUT_SIMPLE("BUT_MMRestartLevel_Up","$TEX$\\MainMenu\\restart.png",120,50,0,0,0,0,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_MMRestartLevel_Down","$TEX$\\MainMenu\\restartdown.png",120,50,0,0,0,0,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_MMRestartLevel_Mark","$TEX$\\MainMenu\\restartmark.png",135,57,0,0,-7,-4,0);
	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.MMRestartLevel",bs);
	bs->Release();

	// end of MainMenu

	// ------------------------------------------------------------------------------------------
	// Package browser:
	
	// package button
	up = CREATE_STYLE_BUT_SIMPLE("BUT_Package_Up","$TEX$\\packagebrowser\\package.png",16,14,0,0,0,2,0);
	down = CREATE_STYLE_BUT_SIMPLE("BUT_Package_Down","$TEX$\\packagebrowser\\packageDown.png",16,14,0,0,0,2,0);
	mark = CREATE_STYLE_BUT_SIMPLE("BUT_Package_Mark","$TEX$\\packagebrowser\\packageMark.png",30,30,0,0,-7,-9,0);
	bs = new CGUIButtonStyle(up,down,mark,0,0,1);
	RefMgr->Add("GUI.But.Package",bs);
	bs->Release();

	// tex
	style = new CGUIStyle("PBtex",1,0,"$TEX$\\PackageBrowser\\tex.png");
	params  = new float[6];
	params[0]=15; params[1]=15; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// anim
	style = new CGUIStyle("PBani",1,0,"$TEX$\\PackageBrowser\\anim.png");
	params  = new float[6];
	params[0]=15; params[1]=15; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// elsource
	style = new CGUIStyle("PBels",1,0,"$TEX$\\PackageBrowser\\elsource.png");
	params  = new float[6];
	params[0]=15; params[1]=15; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	// MusicModule
	style = new CGUIStyle("PBmm",1,0,"$TEX$\\PackageBrowser\\MusicModule.png");
	params  = new float[6];
	params[0]=15; params[1]=15; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// MusicSample
	style = new CGUIStyle("PBms",1,0,"$TEX$\\PackageBrowser\\MusicSample.png");
	params  = new float[6];
	params[0]=15; params[1]=15; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);


	// endof PackageBrowser

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro panel u scripeditu
	style = new CGUIStyle("ScriptPanel",1,0,"$TEX$\\Panel\\scriptpanel.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro filebrowser
/*	style = new CGUIStyle("FileBrowser",1,0,"$TEX$\\FileBrowser\\filebrowser.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("FileBrowser2",1,0,"$TEX$\\FileBrowser\\filebrowser2.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("FileBrowser3",1,0,"$TEX$\\FileBrowser\\filebrowser3.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);
*/
	style = new CGUIStyle("FBdir",1,0,"$TEX$\\FileBrowser\\dir.png");
	params  = new float[6];
	params[0]=15; params[1]=15; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("FBfile",1,0,"$TEX$\\FileBrowser\\file.png");
	params  = new float[6];
	params[0]=15; params[1]=15; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("FBscriptfile",1,0,"$TEX$\\FileBrowser\\scriptfile.png");
	params  = new float[6];
	params[0]=15; params[1]=15; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("FBlevel",1,0,"$TEX$\\FileBrowser\\level.png");
	params  = new float[6];
	params[0]=14; params[1]=14; params[2]=0; params[3]=0; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("ObjPropertyPanel",1,0,"$TEX$\\Panel\\propertyPanel.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro EditorMods:
	style = new CGUIStyle("EM_Insert",1,0,"$TEX$\\editor\\EditorMods\\insertMod.png");
	styleSet->Add(style);
	style = new CGUIStyle("EM_SelectFree",1,0,"$TEX$\\editor\\EditorMods\\select_free.png");
	styleSet->Add(style);
	style = new CGUIStyle("EM_SelectCell",1,0,"$TEX$\\editor\\EditorMods\\select_cell.png");
	styleSet->Add(style);
	style = new CGUIStyle("EM_SelectObj",1,0,"$TEX$\\editor\\EditorMods\\selectObj.png");
	styleSet->Add(style);
	style = new CGUIStyle("EM_Delete",1,0,"$TEX$\\editor\\EditorMods\\delete.png");
	styleSet->Add(style);
	style = new CGUIStyle("EM_MoveObj",1,0,"$TEX$\\editor\\EditorMods\\moveObj.png");
	styleSet->Add(style);
	style = new CGUIStyle("EM_NoConnect",1,0,"$TEX$\\editor\\EditorMods\\noConnect.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro LevelMenu:
	style = new CGUIStyle("LM_NewLevel",1,0,"$TEX$\\editor\\newLevel.png");
	styleSet->Add(style);
	style = new CGUIStyle("LM_LoadLevel",1,0,"$TEX$\\editor\\loadlevel.png");
	styleSet->Add(style);
	style = new CGUIStyle("LM_ClearLevel",1,0,"$TEX$\\editor\\clearLevel.png");
	styleSet->Add(style);
	style = new CGUIStyle("LM_CloseLevel",1,0,"$TEX$\\editor\\closeLevel.png");
	styleSet->Add(style);
	style = new CGUIStyle("LM_HelpIcon", 1, 0, "$DATA$\\HelpIcon.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Editor Icons:
	style = new CGUIStyle("EI_Map",1,0,"$TEX$\\editor\\mapIcon.png");
	styleSet->Add(style);
	style = new CGUIStyle("EI_ObjProperty",1,0,"$TEX$\\editor\\objProperty.png");
	styleSet->Add(style);
	style = new CGUIStyle("EI_VarSelector",1,0,"$TEX$\\editor\\selectObj.png");
	styleSet->Add(style);
	style = new CGUIStyle("EI_CellSelector",1,0,"$TEX$\\editor\\select_cell.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Editor GlobalObjects TB:
	style = new CGUIStyle("GOTB_GlobalVars",1,0,"$TEX$\\editor\\globalVars.png");
	styleSet->Add(style);
	style = new CGUIStyle("GOTB_StdObjectTex",1,0,"$TEX$\\editor\\stdObjectTex.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro tlacitka na panelu pro Editor:

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikonu "LevelMenu" (TaskBar)
	style = new CGUIStyle("LevelMenuTB1",1,0,"$TEX$\\editor\\levelMenu.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("LevelMenuTB2",1,0,"$TEX$\\editor\\levelMenu.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("LevelMenuTB3",1,0,"$TEX$\\editor\\levelMenu.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikonu "EditorMod" (TaskBar)
	style = new CGUIStyle("EditorModTB1",1,0,"$TEX$\\editor\\editorModIcon.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("EditorModTB2",1,0,"$TEX$\\editor\\editorModIcon.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("EditorModTB3",1,0,"$TEX$\\editor\\editorModIcon.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikonu "LevelMenu" (TaskBar)
	style = new CGUIStyle("GlobalObjsTB1",1,0,"$TEX$\\editor\\globalObjsTBIcon.png");
	params  = new float[6];
	params[0]=40; params[1]=13; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("GlobalObjsTB2",1,0,"$TEX$\\editor\\globalObjsTBIcon.png");
	params  = new float[6];
	params[0]=40; params[1]=13; params[2]=-1; params[3]=-2; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("GlobalObjsTB3",1,0,"$TEX$\\editor\\globalObjsTBIcon.png");
	params  = new float[6];
	params[0]=60; params[1]=19; params[2]=-1; params[3]=-2; params[4]=-10; params[5]=-4;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Editor NoConnectDialog:
	style = new CGUIStyle("NCD_Connect",1,0,"$TEX$\\editor\\connect.png");
	styleSet->Add(style);
	style = new CGUIStyle("NCD_NoConnect",1,0,"$TEX$\\editor\\noConnect.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Editor StdAutoTex:
	style = new CGUIStyle("STD_AUTO_TEX",1,0,"$TEX$\\editor\\autoStd.png");
	styleSet->Add(style);

	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Editor PointMarker:
	style = new CGUIStyle("PointMarker",1,0,"$TEX$\\editor\\pointMarker.png");
	styleSet->Add(style);


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Editor - GAME MOD indikator
	style = new CGUIStyle("GM_Indicator",1,0,"$TEX$\\editor\\gameMode.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);



	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro ikony na panel u ObjectProperty - SetDoDefault, MakeShortCut
	style = new CGUIStyle("OP_SetToDefautTB1",1,0,"$TEX$\\editor\\SetToDefaultUp.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("OP_SetToDefautTB3",1,0,"$TEX$\\editor\\SetToDefaultMark.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("OP_SetToDefautTB4",1,0,"$TEX$\\editor\\SetToDefaultDisable.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------
	style = new CGUIStyle("OP_ShortCutTB1",1,0,"$TEX$\\editor\\shortCutUp.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("OP_ShortCutTB3",1,0,"$TEX$\\editor\\shortCutMark.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------
	style = new CGUIStyle("OP_RefreshTB1",1,0,"$TEX$\\editor\\RefreshUp.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("OP_RefreshTB3",1,0,"$TEX$\\editor\\RefreshMark.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("OP_RefreshTB4",1,0,"$TEX$\\editor\\RefreshDisable.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------
	style = new CGUIStyle("AutoDlgTB1",1,0,"$TEX$\\editor\\autoUp.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("AutoDlgTB3",1,0,"$TEX$\\editor\\autoMark.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("AutoDlgTB4",1,0,"$TEX$\\editor\\autoDisable.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------
	style = new CGUIStyle("AutoSaveTB1",1,0,"$TEX$\\editor\\autoSaveUp.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("AutoSaveTB3",1,0,"$TEX$\\editor\\autoSaveMark.png");
	params  = new float[6];
	params[0]=MARK_PANEL_ICON_SIZE; params[1]=MARK_PANEL_ICON_SIZE; params[2]=-1; params[3]=-2; params[4]=-7; params[5]=-13;
    style->SetParameters(params,6);
	styleSet->Add(style);

	style = new CGUIStyle("AutoSaveTB4",1,0,"$TEX$\\editor\\autoSaveDisable.png");
	params  = new float[6];
	params[0]=STD_PANEL_ICON_SIZE; params[1]=STD_PANEL_ICON_SIZE; params[2]=-3; params[3]=-3; params[4]=0; params[5]=0;
    style->SetParameters(params,6);
	styleSet->Add(style);
	// ------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Game Menu
	style = new CGUIStyle("GameMenuItem",1,0,"$TEX$\\MainMenu\\gameMenuItem.png");
	styleSet->Add(style);
	style = new CGUIStyle("GameMenuCounter",1,0,"$TEX$\\MainMenu\\gameMenuCounter.png");
	styleSet->Add(style);
	style = new CGUIStyle("GameMenuProgressBar",1,0,"$DATA$\\gameMenuProgressBar.png");
	styleSet->Add(style);
	style = new CGUIStyle("GameMenuProgressBarCompressed",1,0,"$DATA$\\gameMenuProgressBarCompressed.png");
	styleSet->Add(style);
	style = new CGUIStyle("GameMenuSvetlo", 1, 0, "$DATA$\\MenuSvetlo.png");
	styleSet->Add(style);
	style = new CGUIStyle("GameMenuPozadi", 1, 0, "$DATA$\\MenuPozadi2.png");
	styleSet->Add(style);
	style = new CGUIStyle("GameMenuRamp", 1, 0, "$DATA$\\MenuRamp.png");
	styleSet->Add(style);


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro Start Menu
	style = new CGUIStyle("StartMenu_Game",1,0,"$TEX$\\StartMenu\\game40.png");
	styleSet->Add(style);
	style = new CGUIStyle("StartMenu_Script",1,0,"$TEX$\\StartMenu\\skript40.png");
	styleSet->Add(style);
	style = new CGUIStyle("StartMenu_Level",1,0,"$TEX$\\StartMenu\\level40.png");
	styleSet->Add(style);


	// ------------------------------------------------------------------------------------------
	// nahrani stylu pro novy MainMenu
	style = new CGUIStyle("MainMenu_Back",1,0,"$TEX$\\MainMenu2\\mmBack.jpg");
	styleSet->Add(style);
	style = new CGUIStyle("MainMenu_Top",1,0,"$TEX$\\MainMenu2\\mmTopLogo.png");
	styleSet->Add(style);
	style = new CGUIStyle("MainMenu_LevelUp",1,0,"$TEX$\\MainMenu2\\mmLevelUp.png");
	styleSet->Add(style);
	style = new CGUIStyle("MainMenu_Level_Zal1",1,0,"$TEX$\\MainMenu2\\mmLevelZal1.png");
	styleSet->Add(style);
	style = new CGUIStyle("MainMenu_Level_Zal2",1,0,"$TEX$\\MainMenu2\\mmLevelZal2.png");
	styleSet->Add(style);
	style = new CGUIStyle("MainMenu_Level_ZobL",1,0,"$TEX$\\MainMenu2\\mmLevelZobL.png");
	styleSet->Add(style);
	style = new CGUIStyle("MainMenu_Level_ZobR",1,0,"$TEX$\\MainMenu2\\mmLevelZobR.png");
	styleSet->Add(style);


}

void CGUI::CreateCursor()
{
	cursor = new CGUICursor(rootwnd);

	//cursor->Load(0,"$TEX$\\cursors\\kurzor.png",7,7);
	cursor->Load(0,"$TEX$\\cursors\\std_kurzor.png",1,1);
	
	/*
	cursor->Load(1,"$TEX$\\cursors\\0.png",7,7);
	cursor->Load(2,"$TEX$\\cursors\\1.png",7,7);
	cursor->Load(3,"$TEX$\\cursors\\2.png",7,7);
	cursor->Load(4,"$TEX$\\cursors\\3.png",7,7);
	*/
	cursor->Load(1,"$TEX$\\cursors\\resizers\\0.png",7,7);
	cursor->Load(2,"$TEX$\\cursors\\resizers\\1.png",5,9);
	cursor->Load(3,"$TEX$\\cursors\\resizers\\2.png",7,7);
	cursor->Load(4,"$TEX$\\cursors\\resizers\\3.png",9,5);

	cursor->Load(5,"$TEX$\\cursors\\editor\\select_free.png",7,7);
	cursor->Load(6,"$TEX$\\cursors\\editor\\select_cell.png",8,8);
	cursor->Load(7,"$TEX$\\cursors\\editor\\selectObj.png",7,7);
	cursor->Load(8,"$TEX$\\cursors\\editor\\delete.png",7,7);
	cursor->Load(9,"$TEX$\\cursors\\editor\\noConnect.png",7,7);
	// cursor 10 - vyhrazeno pro texturu vkladaneho objektu
	cursor->Load(10,"$TEX$\\cursors\\std_kurzor.png",1,1); // nez si vytvori vlastni
	
	cursor->Load(11,"$TEX$\\cursors\\editor\\pointSelector.png",14,14); // nez si vytvori vlastni

	cursor->Set(0,true);
}
