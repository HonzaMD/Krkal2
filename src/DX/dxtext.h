//////////////////////////////////////////////////////////////////////
//
// DXtext.h
//
// zobrazi formatovany text
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef DXTEXT_H
#define DXTEXT_H

#include "types.h"
#include "dxbliter.h"
#include "dxfont.h"

class CDXHighlighter;
class CFormatedText;

typedef CDynamicArrayV<char,1> CDynChar;

////////////////////////////////////////////////////////////////////////////////////////////

//vertexy pro vypis textu
struct CBltTextVertices
{
	CDXFont *font; //font

	int numchar; //# znaku
	int maxchar; //maximalni # znaku
	int curvert; //aktualni cislo vertexu
	CBltVert *vertices; //pole vertexu (velikosti maxchar*6) - kazdy znak je tvoren 2 trojuhelniky

	CBltTextVertices(int _maxchar){ //alokuje vertexy
		maxchar = _maxchar;
		vertices = new CBltVert[maxchar * 6];
		numchar = 0;
		curvert = 0;
		next=0;
	}
	~CBltTextVertices(){
		SAFE_DELETE_ARRAY(vertices);
		SAFE_DELETE(next);
	}

	//prida 1 znak, oclipuje ho (v pripade potreby realokuje pole vertexu - 2x zvetsi)
	int AddChar(int ch, float x, float y, CDXFont *font, DWORD color, const CMatrix2D *m, float *cl);

	CBltTextVertices *next;
};

////////////////////////////////////////////////////////////////////////////////////////////

//vykresli formatovany text (vcetne kurzoru a oznaceni)
class CBltFormatedText: public CBltElem
{
public:

    CBltFormatedText(CFormatedText *text, int xmargin=4, int ymargin=0);
	~CBltFormatedText(){SAFE_DELETE(vertices);}

	virtual void Render(int drawall=1); //vykreslovaci fce

	virtual dxBltType GetType(){return dxFormatedText;} //vrati typ elementu

	virtual int Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc=1); //vytvori vertexy

	virtual void RemoveCache(CCache *cache){}
	virtual void DeleteDXObjects(){};

	void SetCursor(int draw) {drawcursor = draw;} //nastavi kurzor

	void BlinkCursor(int bl=-1); //blikani kurzoru

	void CmpCursorPos(int &x, int &y, int &lnheight); //spocita souradnice kurzoru

	void TransformCoords(int &x, int &y); //transformuje souradnice (odecte okraj)

	void SetWidth(int width); //nastavi sirku textu
	int CmpTextSize(int &xs, int &ys); //spocita rozmery textu

	int CmpEditBoxHeight(); //spocita vysku jednoradkoveho editboxu

	void SetShowMark(int sh); //nastavi jestli se bude zobrazovat oznaceni

protected:

	void SetCursorPos(float x1, float x2,float y1,float y2, const CMatrix2D *m, float *cl); //nastavi pozici kurzoru pro vykresleni
	
	void SetMarkStart(float x, float y1,float y2); //nstavi pozici zacatku oznaceni
	void SetMarkEnd(float x, float y1,float y2, const CMatrix2D *m, float *cl); //nastavi pozici konce oznaceni
	int AddMarkSq(int sq, float x1, float x2, float y1, float y2, const CMatrix2D *m, float *cl); //prida vertexy 1 oznacovaciho ctverce (max. 3 ctverce - 1.: konec 1. radky, 2.: druha az predposledni radka 3.: zacatek posledni radky)

	CFormatedText *text; //formatovany text, ktery vykresluje
	CBltTextVertices *vertices; //vertexy textu

	int drawcursor, cursorblink; //mam zobrazovat kurzor?, blikani kurzoru
	int cursclout; //1=cursor je cely oclipovan -> neni videt
	CBltVert cursorvert[4]; //vertexy kurzoru

	int showmark; //mam zobrazovat oznaceni?
	int drawmark; //mam vykreslit oznaceni?
	CBltVert markvert[12]; //vertexy oznaceni (max. 3 obdelniky)
	float msx,msy1,msy2; //pomocne promenne pro oznaceni
	int nummarksq; //pocet oznacovacich ctvercu (max 3)

	int xshift,yshift; //okraje - x a y posun

	int clipout; //cely text je oclipovan pryc
};

////////////////////////////////////////////////////////////////////////////////////////////

//popisuje format
struct CTextFormat
{
	int numchar; //pocet znaku 
	CDXFont *font; //font
	DWORD color; //barva
};

////////////////////////////////////////////////////////////////////////////////////////////

//jeden odstavec textu (uvnitr neni zadny enter)
class CTextBlock
{
	friend class CFormatedText;
	friend class CBltFormatedText;
	friend class CTextCursor;
	friend class CDXHighlighter;
public:
	CTextBlock(); //head constructor
	CTextBlock(char *_text, int numchar=-1);

	void SetFormat(CTextFormat *format, int formatcnt); //nastavi formatovani bloku - format je pole formatu, po zavolani nerusit!
	void ResetFormat(){formatcnt=0;} //zrusi formatovani bloku
	void FormatChars(int formatpos, int formatcharpos, int numchars, CDXFont *font, DWORD color, CTextFormat *defaultformat); //naformatuje zadany pocet znaku od zadane pozice

	~CTextBlock();

	void AddBlockBehind( CTextBlock *block ); //prida dalsi blok

	void WordWrap(CFormatedText *ftext, int width); //zalame cely text

	CTextBlock* GenNextBlock(){if(next->text) return next; else return 0;} //vrati dalsi blok
	const char* GetText(){return text;}

	int GetBlockNum(){return blocknum;}

protected:

	void ResetNumLines(){numlines=0;} //nastavi pocet radek
	void AddLine(int linelen); //prida radku

	void InsertChar(int pos,UC ch); //prida znak
	void DeleteChar(int pos); //smaze snak

	int DeleteFormat(int pos, int &jfp); //smaze format

	void SplitBlock(int pos, int formatpos, int formatcharpos, CTextFormat *defaultfmt); //rozdeli blok na dva
	void JoinBlock(int formatcharpos, CTextFormat *defaultfmt); //spoji dva bloky do jednoho

	int CmpBlockSize(CFormatedText *text, int calcystarts=1); //spocita velikost bloku (pokud je calcystarts=1 - prepocita zacatky bloku, pokud se zmenila velikost bloku); vraci 1 pokud se zmenila vyska bloku
	void CmpYStarts(); //spocita zacatky bloku (pro vsechny bloky od tohoto bloku)
	void CmpBlockNums(); //spocita cislo bloku (pro vsechny bloky od tohoto bloku)

	char *text; //=NULL -> head; jinak neformatovany text ukonceny 0
	int textlen; //delka text (bez 0)
	int textbufferlen; //delka textoveho bufferu

	CTextFormat *format; //format bloku; pokud=NULL - cely blok ma default format
	int formatcnt; //pocet format. prikazu
	int formatbuffercnt; //velikost format. bufferu

	int numlines; //0=unknown
	int *lineslen; //kumulativni delky radek (index konec radky)
	int lineslenbufferlen; //velikost bufferu

	int height,width; //vyska a sirka bloku
	int starty; //pocatecni y-souradnice bloku

	int blocknum; //cislo bloku
	int blockcomment; //uroven zanoreni viceradkoveho komentare

	CTextBlock *prev,*next; //predchozi, dalsi blok
};

////////////////////////////////////////////////////////////////////////////////////////////

//formatovany text
class CFormatedText
{
	friend CBltFormatedText;
	friend CTextBlock;
	friend CTextCursor;
	
public:

	CFormatedText(CDXFont *font, DWORD color, int _wordwrap, int _tabsize=32);
	~CFormatedText();

	int AddTextBlockToEnd( CTextBlock *block ); //prida blok na konec

	CTextBlock* GetFirstBlock(); //vrati prvni blok
	CTextBlock* GetLastBlock(); //vrati posledni blok
	CTextFormat* GetDefaultFormat(){return &format;} //vrati defaultformat

	void SetWidth(int _width); //nastavi sirku
	int GetWidth(){return width;} //vrati sirku

	CTextCursor* GetCursor(){return cursor;} //vrati cursor
	void CreateCursor(); //vytvori cursor

	int CmpTextSize(int &xx, int &yy); //spocita velikost textu

	int GetText(char *buf, int bufsize, int add13=0, int addendzero=1); //vrati cely text; konce radky oddeluje 10 (pokud je add13=1 tak i 13); pokud se text nevejde do buferu - vraci potrebnou velikost bufferu, jinak vraci 0
	int GetTextInt(int &i); //prevede text na int; v pripade chyby vraci 1, jinak 0
	int GetTextDouble(double &i); //prevede text na double; v pripade chyby vraci 1, jinak 0

	int SetText(char *buf, int size); //nastavi text

	void SetMarkStart(CTextCursor *c=NULL); //nastavi zacatek oznaceni
	void SetMarkEnd(CTextCursor *c=NULL); //nastavi konec oznaceni
	void UnMark(); //zrusi oznaceni
	int IsMarked(); //1=neco je oznaceno; 0=nic neni oznaceno

	CTextCursor* GetMarkStart(){return markstart;} //vrati zacatek oznaceni
	CTextCursor* GetMarkEnd(){return markend;} //vrati konec oznaceni

	CTextCursor* FindFirstMark(); //vrati prvni oznacovaci zarazku (zacatek oznaceni muze byt az za koncem oznaceni)
	CTextCursor* FindSecondMark(); //vrati druhou oznacovaci zarazku

	int DeleteSelectedText(int undo=1, int link=0); //smaze oznaceny text
	int CopyToClipboard(); //kopiruje oznaceny text do clipboardu (neformatovany)
	int PasteFromClipboard(int multiline=0, CDXHighlighter *highlighter=NULL, int undo=1); //pastuje z clipboardu (jen neformatovany text)

	int DoIdent(); //vlozi taby na zacatek radky podle prechoziho bloku
	int IdentBlock(int left); //vlozi/sebere jeden tab na zacatku kazde radky bloku

	int Undo(CDXHighlighter *highlighter=NULL); //undo
	int Redo(CDXHighlighter *highlighter=NULL); //redo
	void DoneUndo(); //uzavre posledni undo krok

	int CanUndo();
	int CanRedo();

protected:

	void CmpFirstMark(); //zjisti, ktera z oznacovacich zarazek je prvni
	
	void DeleteAllTextBlocks(); //smaze vsechny odstavce

	CTextBlock *head; //seznam odstavcu - dvojity kruhovy spojak s hlavou
	CTextFormat format; //default format
	
	CTextCursor *cursor; //cursor

	int width; //sirka (sirka vnitrku okna - podle ni se zalamuji radky)
	int height,maxwidth; //vyska, maxsirka (maximalni sirka radku)

	CTextCursor *markstart, *markend; //oznacovaci zarazky
	int firstmark; //flag, ktary urcuje, ktera zarazka je prvni
	
	int wordwrap; //0 = bez zalamovani radek, 1 = zalamovani radek
	int tabsize; //velikost tabu

	class CTextUndo *undo; //undo

};

////////////////////////////////////////////////////////////////////////////////////////////

//textovy kurzor
class CTextCursor
{
	friend class CDXHighlighter;
public:
	CTextCursor(CFormatedText *_text, CTextBlock *_block=NULL); //nastavi kurzor na zacatek bloku (pokud je block=NULL - nastavi kurzor na zacatek 1.bloku)

	int SetCursorToStartOfBlock(CTextBlock *_block); //nastavi kurzor na zacatek bloku
	int SetCursorToEndOfBlock(CTextBlock *_block); //nastavi kurzor na konec bloku
	int SetCursorPos(int block, int blockpos); //nastavi kurzor na blok cislo 'block' a znak 'blockpos' - pouziva se pri undo

	UC GetChar(){ //vrati znak za kurzorem
		if(!block) return 0;
		return block->text[textpos];
	}
	
	CTextFormat* GetFormat(){return format;} //vrati format zanku za kurzorem

	int MoveRight(CTextFormat **_format=NULL); //posune kurzor doprava
	int MoveLeft(CTextFormat **_format=NULL); //posune kurzor doleva
	int MoveUp(); //posune kurzor nahoru
	int MoveDown(); //posune kurzor dolu
	int MoveHome(); //posune kurzor na zacatek radky
	int MoveEnd(); //posune kurzor na konec radky
	int MoveTextHome(); //posune kurzor na zacatek textu
	int MoveTextEnd(); //posune kurzor na konec textu
	int MoveWordRight(); //posune kurzor na zacatek dalsiho slova
	int MoveWordLeft(); //posune kurzor na zacatek predchoziho slova
	int MovePageUp(int dy); //posune kurzor o stranku nahoru
	int MovePageDown(int dy); //posune kurzor o stranku dolu
	int MoveXY(int x, int y, int me=1); //nastavi kurzor na zadamou pozici (me=1 ovlivnuje chovani u prvni a posledni radky)

	void FormatChars(int numchar, CDXFont *font, DWORD color); //naformatuje zadany pocet znaku
	
	void Return(int recalcblock=1, int undo=1); //vlozi enter
	void InsertChar(UC ch, int recalcblock=1, int undo=1); //vlozi znak
	void Delete(int recalcblock=1, int undo=1); //smaze nasledujici znak
	void Backspace(int recalcblock=1, int undo=1); //smaze predchozi znak

	int CharWidth(); //vrati sirku znaku (bez mezer)
	int CharTotalWidth(); //vrati celkovou sirku znaku (vcetne mezer)
	int CharHeight(); //vrati vysku fontu
	int CharAscent(); //vrati ascent
	int CharDescent(); //vrati descent

	int CmpHeight( int cnt, int &maxAscent, int &BaselineDist ); //spocita vysku zadaneho postu znaku

	CTextBlock* GetBlock(){return block;} //vrati blok, kde je kurzor
	int GetPos(){return textpos;} //vrati pozici v bloku

	int DrawAtEndOfLine(){return drawatendofln;} //vraci 1, kdyz se ma kurzor nakresit nakonci radku (jinak by se nakreslil nazacatku dalsiho radku)

	int GetBlockLine(); //vrati cislo radku v bloku
	int CmpXPos(); //spocita x-souradnice
	void CmpPos(int &line, int &cx, int &cy, int &lnheight); //spocita x,y souradnici, cislo radku, vysku radky

	void InvalidPos(){xpos=-1;} //zrusi xpos

	int GetInsertMode(){return insertmode;} //vrati typ prepisovaci rezimu
	void SetInsertMode(int ins){insertmode=ins;} //nastavi typ prepisovaci rezimu
	int ChangeInsertMode(){insertmode=!insertmode; return insertmode;} //zmeni prepisovaci rezim

	void CalcFormat(); //spocita format

protected:

	int MoveBlockXY(int x, int y);

	CFormatedText *text;
	CTextBlock *block;
	
	int textpos; //index do textu

	int formatpos; //index do formatu
	int formatcharpos; // znaky ve fmt. cislovany od 0-numchar
	
	CTextFormat *format; //akt. format
	
	int drawatendofln; //1, kdyz se ma kurzor nakresit nakonci radku (jinak by se nakreslil nazacatku dalsiho radku)

	int insertmode;
	
	int xpos; //-1=unknown

};

////////////////////////////////////////////////////////////////////////

enum eUndoType{eUhead,eUnone,eUwrite,eUoverwrite,eUdel,eUbackspace,eUreturn,eUdelsel,eUpaste,eUidentblock,eUidentblockleft};

class CTextUndoStep{
	friend class CTextUndo;
	friend CFormatedText;
public:
	CTextUndoStep(); //head constructor
	CTextUndoStep(eUndoType _type, int block, int blockpos, int link=0);
	~CTextUndoStep();

	int AddChar(UC c);
	void SetPos(int _block,int _blockpos){block=_block;blockpos=_blockpos;}

	int GetCount(){if(text) return text->GetCount();else return 0;}

	int SetLink(int link){linkwithnext=link;}

protected:

	CDynChar *text;
	int block,blockpos;

	eUndoType type;

	int linkwithnext;

	CTextUndoStep *prev, *next;
};

class CTextUndo{
public:

	CTextUndo(int maxsteps=100);
	~CTextUndo();

	int CreateNewStep(eUndoType _type, int block, int blockpos, int link=0);
	int AddChar(UC ch);
	int SetPos(int block,int blockpos){if(!curundo)return 0;curundo->SetPos(block,blockpos);return 1;}
	void DoneCurStep();

	void DeleteAllUndoSteps();

	eUndoType GetCurType();

	CTextUndoStep* GetLastUndoStep(){return lastundo;}

	int GetNumSteps(){return numundosteps;}

	void Undo();
	void Redo();

protected:

	CTextUndoStep *head;
	CTextUndoStep *curundo;
	CTextUndoStep *lastundo;
	int maxundosteps;
	int numundosteps;
};


#endif

