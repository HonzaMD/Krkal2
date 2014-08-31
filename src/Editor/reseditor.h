////////////////////////////////////////////////////////////////////////////////////
//
// reseditor.cpp
//
// Editor resourcu
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////


#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

#include "widget.h"
#include "dialogs.h"
#include "tree.h"

class CGUIButton;
class CGUIEditWindow;
class CGUICheckBox;
class CGUIStaticPicture;
class CGUIStaticText;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Editor resourcu
//
// globalni objekt
// cliboard na elsourcy
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CEDResourceEditor
{
public:
	CEDResourceEditor();
	~CEDResourceEditor();

	class CPackageBrowser* ShowPackageBrowser();
	class CPackageBrowser* GetPackageBrowser();

	struct CEDElSourceCopy *elsclipboard;
private:
	typeID pkgbrowID;
};

extern CEDResourceEditor* resourceEditor;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Package browser
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CPackageBrowser : public CGUIFileBrowser
{
	friend class CPackageBrowserEventHandler;
public:
	CPackageBrowser(float _x, float _y, float _sx, float _sy, char* path="$GAMES$", char* label="GAMES");
	~CPackageBrowser();

protected:

	virtual int TreeHandler(typeID treeItem, int index, int state);
	virtual int Filter(CGUITreeItem* parent, const char *name, int dir, int first);
	virtual CGUIFileBrowserItem* AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir);
	static int Compare(CGUITreeItem *aa, CGUITreeItem *bb);

	class CGUIMenu *dirmenu,*packagemenu,*resourcemenu,*musicmenu,*picturemenu;
	class CPackageBrowserEventHandler *eventhandler;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PackageBrowserEventHandler
//
// obsluha context menu, newdialogu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CPackageBrowserEventHandler : public CGUIEventProgram
{
public:
	CPackageBrowserEventHandler(CPackageBrowser* pb);
	~CPackageBrowserEventHandler();
private:
	virtual void EventHandler(CGUIEvent *event);
	CPackageBrowser* pkgbrow;

	typeID newresnamedlgID;
	
	class CGUIDlgOkCancel* GetDeleteDlg();
	typeID deletedlgID;
	char *deletename;
	int deletetype; //1=package 2=resource
	
	typeID filebrowID;
	int module_sample; //1=module 0=sample
	char *filebrowpkg;
	int newrestype; //-1=package, 0=els, 1=lighttex, 2=anim
	char *newresname;
	char *selectedpackage;
	
};



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CEDElSourceEditor
//
// editor ElSourcu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_ELS_POINTS 10
class CEDElSourceEditor: public CGUIDlgOkCancel
{
	
public:
	CEDElSourceEditor(float _x, float _y);
	~CEDElSourceEditor();

	int LoadElSource(char *packagename, char *resource);
	int CreateElSource(char *packagename, char *resource);

	virtual int Ok();

	virtual int FocusChanged(UINT focus, typeID dualID); //zmenil se focus

protected:

	double pbx,pby,pbz,zdz;
	int flug,rampa;

	int tmpnumpoints;
	double pointsx[MAX_ELS_POINTS],pointsy[MAX_ELS_POINTS];

	int numpoints;
	int ChangeNumPoints(int num);

	float bwnsx,bwnsy;

	CGUIEditWindow *edpbx, *edpby, *edpbz, *edzdz, *edflug, *ednumpoints;
	CGUICheckBox *edrampa;
	CGUIButton *copy,*paste;

	CGUIEditWindow *edpointsx[MAX_ELS_POINTS], *edpointsy[MAX_ELS_POINTS];

	class CGEnElSource *els;
	char *resourcename;
	int newresource;

	virtual void EventHandler(CGUIEvent *event);

	int SaveElSource();

};

struct CEDElSourceCopy{
	double pbx,pby,pbz,zdz;
	int flug,rampa;
	double pointsx[MAX_ELS_POINTS],pointsy[MAX_ELS_POINTS];
	int numpoints;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CEDLightTexEditor
//
// editor LightTexu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CEDLightTexEditor: public CGUIDlgOkCancel
{
	
public:
	CEDLightTexEditor(float _x, float _y);
	~CEDLightTexEditor();

	int LoadLightTex(char *packagename, char *resource);
	int CreateLightTex(char *packagename, char *resource);

protected:

	int SaveLightTex();
	virtual int Ok();

	virtual void EventHandler(CGUIEvent *event);

	void MarkComponents();
	void ArrangeDlg();

	int GenerateAllTexFilenames(char *dir, char *file);

	class CGEnPackage *pkg;

	char *resourcename;
	int newresource;

	class CLightTex *tex, *tex_pkg;
	CTexture *tex_ld;

	class CGUIEditWindow *edels,*edshdx,*edshdy;
	class CGUIButton *butels,*butcrop,*butloadall;
	class CGUICheckBox *cbalpha,*cbamb,*cbdiff,*cbspec,*cbspec_h,*cbnormal,*cbzdepth,*cbshadow,*cbatmosphere;
	class CGUIStaticPicture *spalpha,*spamb,*spdiff,*spspec,*spspec_h,*spnormal,*spzdepth, *spshadow, *spatmosphere, *splightdemo;
	class CGUIStaticText *stshdx,*stshdy;

	class CGUILine *ln1,*ln2;

	float compx[3],compy[3];
	float shy,shy2;

	int isalpha,isamb,isdiff,isspec,isspec_h,isnormal,iszdepth,isshadow,isatmosphere;

	UINT sx,sy;
	int shsx,shsy,shdx,shdy;

	char *elsname;

	typeID elsselID, importtexID,importtexFilesDlgID,importtexOKID;
	char *fnalpha,*fnamb,*fndiff,*fnspec,*fnspec_h,*fnnormal,*fnzdepth,*fnshadow,*fnatmosphere;
	
	void DeleteAllFn();
	int UpdateTex();

};

class CEDManyLightTexEditor: public CGUIDlgOkCancel
{
	
public:
	CEDManyLightTexEditor(float _x, float _y, char *pkgdir);
	~CEDManyLightTexEditor();
protected:

	virtual int Ok();
	int CreateLightTex(char *texname, char *fn);
	char* GenerateTexName(const char *fn);

	int FindAllTex( char *dir );
	int IsTex( const char *fn );

	void DeleteAllTexFn();
	void DeleteAllSTtexFn();

	void ShowTexNames();

	class CGUIEditWindow *edels;
	class CGUIButton *butels,*butseldir;
	char *elsname;
	class CGEnPackage *pkg;

	virtual void EventHandler(CGUIEvent *event);

	typeID elsselID,seldirID;

	char *texdir;
	CListK<char*> texfilenames,texnames;
	CListK<CGUIStaticText*> STtexFN;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CEDAnimEditor
//
// editor animaci
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAXFRAMES 200

class CEDAnimEditor: public CGUIDlgOkCancel
{
	
public:
	CEDAnimEditor(float _x, float _y);
	~CEDAnimEditor();

	int LoadAnim(char *packagename, char *resource);
	int CreateAnim(char *packagename, char *resource);

protected:

	int SaveAnim();
	virtual int Ok();

	virtual void EventHandler(CGUIEvent *event);
	virtual int TimerImpulse(typeID timerID, float time); 
	
	void ArrangeDlg();
	void UpdateCurFrame();


	void DeleteFrame(int frame);
	void DeleteAllFrames();
	
	void ShowAddFrame(int frame, CGUIElement *el, int newframe=1);
	int SetFrame(int lighttexindex);
	void CloseTexSelDlg();

	class CGEnPackage *pkg;

	char *resourcename;
	int newresource;

	class CGEnAnim *anim;

	CGUIStaticPicture *spanim;
	CGUIButton *butfirst,*butlast,*butprev,*butnext,*butplay,*butstop,*butaddfirst;
	CGUICheckBox *cbrepeat;

	class CGUILine *ln1,*ln2;
    
	int numframes,curframe;

	CLightTex *frames[MAXFRAMES];
	int times[MAXFRAMES];
	char *framenames[MAXFRAMES];
	CTexture *frametex[MAXFRAMES];

	CGUIStaticText *stframenum[MAXFRAMES],*stframetime[MAXFRAMES];
	CGUIButton *butadd[MAXFRAMES],*butdel[MAXFRAMES];
	CGUIEditWindow *edframename[MAXFRAMES],*edframetime[MAXFRAMES];
	CGUIStaticPicture *framepic[MAXFRAMES];

	int repeat;
	int playing;
	
	float maxsx,maxsy;

	typeID texseldlgID;
	int texselframe,addframe;

};

#endif
