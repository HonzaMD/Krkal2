////////////////////////////////////////////////////////////////////////////////////
//
// scriptedit.h
//
// editor scriptu
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////

#ifndef SCRIPTEDIT_H
#define SCRIPTEDIT_H

#include "textedit.h"

class CGUIEditWindow;

class CSEdScriptEdit: public CGUIEditWindow{
public:
	CSEdScriptEdit();	
	virtual ~CSEdScriptEdit();

	int Load(const char *dir, const char *file);
	int Load(const char *fullpathfile);

	int CompileForOB();
	int Compile();

	int Save(int updfb=1);	
	int SaveAll();

	int IsChanged(){return chng;}

	//fce pro kompilator:
	int SeekToStart(); //naseekuje na zacatek (pro fci GetNextChar) (nevraci prvni znak - jen 0-err, 1-ok)
	int GetNextChar(); //vrati dalsi znak; konec fajlu - vraci EOF (=-1); pri chybe vraci -2
	char *GetPath(){return dir;} //vrati adresar, kde je editovany soubor
	char *GetFileName(){return file;} //vrati jmeno souboru (bez cesty)

	char* GetFileTitle(){return filetitle;}
	void ShowCurCompFile();


	virtual int CloseWindow();

protected:

	typeID timerID;

	void DeleteData();
	
	virtual void EventHandler(CGUIEvent *event);
	virtual int Keyboard(UINT ch, UINT state); //klavesnice
	virtual int FocusChanged(UINT focus, typeID dualID); //zmenil se focus
	virtual int TimerImpulse(typeID timerID, float time);

	virtual int ChangedEvent();
	virtual void ClipboardEvent();
	virtual void CursorMoved();

	CGUIButton *butsave,*butsaveall,*butundo,*butredo,*butcut,*butcopy,*butpaste,*butcompile,*butsetcomp;
	CGUIStaticText *stlinenum,*stcurcompfile;
	int linenum;

	void ShowLinenum();

	int chng;
	void SetTitleChng(int _chng);
	char *titletext;
	char *filetitle;
	char *fullpath;
	char *dir,*file;

	int comMode; //0-nic 1-head 2-body 3-eof
	CTextBlock *comBlock;
	const char *comChar;

	CSEdScriptEdit *compfile;

	typeID saveMsgID;

};

#endif