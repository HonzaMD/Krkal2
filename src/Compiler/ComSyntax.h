#pragma once
#include "ComPart.h"
#include "ComExpression.h"

class CComNameDepData
{
public:
	CComNameDepData() : d(0), n(0), id(cUnknown) { }
	CComNameDepData(CComOBBaseData* _d,	CKerName* _n, int _id) : d(_d), n(_n), id(_id) { }
	CComNameDepData(CComNameDepData& nd) : d(nd.d), n(nd.n), id(nd.id) { }
	~CComNameDepData() {};

	CComNameDepData& operator=(const CComNameDepData& nd) { d = nd.d; n = nd.n; id = nd.id;  return *this; }

	CComOBBaseData* d;
	CKerName* n;
	int id;
};


//*******************************************************
// CComSyntax - syntakticky analyzator
//*******************************************************

class CComSyntax :	public CComPart
{
public:
	CComSyntax(CComObjectBrowser* _objBrowser, int _queueSize);
	~CComSyntax();

	void DoPart();
	void DoPanicPickDirties();

	bool slowMode;				// pomaly mod, ve kterem se pri kazdem definovani zavislosti testuje, zda mezi jmeny nevznikl cyklus
	
	CComSymbolTable* toRemove;	// tabulka entit, ktere je treba necistou operaci odebrat
	CComSymbolTable* toModify;	// tabulka entit, ktere je treba necistou operaci modifikovat
	CComSymbolTable* removed;	// tabulka odstranenych entit
	CComSymbolTable* modified;	// tabulka modifikovanych entit
	int removedNr;				// pocet odstranenych entit
	int modifiedNr;				// pocet modifikovanych entit
	CListK<CComOBAttribute*> inheritedAttributes;	// atributy, ktere se budou dedit
	bool secondPass;			// true, pokud se jedna o druhy pruchod (kvuli necistym operacim)

private:
	enum enmDirtyOpType {dotRemove, dotModify, dotInherit};

	int look;			// token, ktery vidim
	int old;			// minuly token

	int  constType;		// typ nalezene konstanty
	int  reqType;		// pozadovany typ kostanty
	int  constID;		// ID nalezene konstanty
	CComOBConstant* constConstant;	// pointer na nalezenou konstantu
	CComConstStack cs;	// zasobnik pro pocitani konstantnich vyrazu

	int blockDepth;		// v kolika blocich jsem vnoren
	int breakDepth;		// do jake urovne vnoreni budu delat brake
	int continueDepth;	// do jake urovne vnoreni budu delat continue
	int currentBlock;	// index prave kompilovaneho bloku
	int switchBlock;	// index bloku switche
	int totalBlockNr;	// celkovy pocet bloku v prave kompilovane metode

	bool singleCommand;	// provedl se jediny prikaz (ne ve slozenych zavorkach) -> ocekava se strednik
	bool doingParameters;	// true, pokud prave parsuji seznam parametru ve volani metody)
	bool inheritAll;	// ma se zdedit vse v celem objektu
	bool groupInherit;	// ma se zdidit vse ve skupine
	bool noSeparator;	// nyni se neocekava strednik
	int attributeGroup;	// ID skupiny atributu, kterou prave parsuji, pripadne cUnknown

	CListK<int>* inputTokens;	// nahradni vstup, ktery se vyuzije pri necistych operacich (pri modifikaci)

	CComShortPosition identPos;	// pozice posledniho identifikatoru
	CListK<CComNSENode> ident;	// posledni identifikator
	CListK<CComNSENode> inObjIdent;	// identifikator prave kompilovaneho objektu
	CListK<CComNSENode> inMethodIdent;	// identifikator prave kompilovane metody
	CDynamicArrayV<int,32> refObjID;	// mozny identifikator objektu, ke kteremu se bude vztahovat volani funkce
	CComOBBaseData* defaultEdit;	// defaultni nastaveni edit-tagu pro skupinu

	CComNSENode* tmpArray;	// pomocne pole
	int  tmpArraySize;		// a jeho velikost

	char* nameStr;			// posledni rozpoznane jmeno
	int maxNameLn;			// aktualni maximum delky rozpoznaneho jmena

	int inMethodID;			// ID prave kompilovane metody

	CComSymbolTable* MDtestTab;	// pomocna tabulka pro nedokumentovanou funkci kompilatoru

	void Step();			// prechod na dalsi token
	void RedirectInput(CListK<int>* tokens);	// presmerovani vstupu na seznam tokenu (u necistych operaci)

	void MatchLexical(int token, int note=0);	// prectu lexikalni token nebo ohlasim chybu
	void MatchKeyword(int token, int note=0);	// prectu klicove slovo nebo ohlasim chybu
	void MatchIdentifier(CComOBBase** ute, int note=0);	// prectu identifikator nebo ohlasim chybu
	CComOBConstant* MatchConstant(int type, int note=0, int* cid = 0);	// prectu konstantu nebo ohlasim chybu

	void Expected(int token, int error, int note=0);	// hlasim chybu expected...

	void LocalizeIdent(CListK<CComNSENode>* what, CListK<CComNSENode>* towhich);	// lokalizuje identifikator what k towhich
	void SetIdent(CListK<CComNSENode>* what, CListK<CComNSENode>* towhich);	// nastavi identifikator whar na towhich

	// Metody rekurzivniho top-down parseru - funkce je obvykle zrejma ze jmena metody :-)
	void DoTopDecl();

		// parsing definic objektu, atributu a hlavicek metod
		void DoObjectDecl();
			void DoObjectEditTags();
			void DoInObject();
				void DoScripted(bool inherit);
				void DoType(int& type, int* storage = 0, int* ret = 0, bool* decl = 0, bool *inherit = 0);
				void DoPointer(int& pointer, int type);
				void DoStruct(int dType);
					void DoStructMember(CComOBStructure* newStruct, CComOBBase* ute, int type, int pointer);
				void DoMemberDecl(int storage,int type, bool decl, bool inherit, int ret);
					void BaseTypeTest(int val,int& comp,int error);
					void DoMethod(const char* idName,int storage, int type, int pointer, int knownName, bool global, bool decl, bool inherit, int ret, int version);
						void SkipMethodFromArgumentList();
						void DoArgumentList();
						void DoOneArgument(int* ret,int& type,int& pointer,char** name, int& version, int& knownName, bool& global, bool* decl);
							void DoSafeArgumentCheck(CComOBArgument* newArg);
					void DoAttribute(CComOBBase* foundRec, int storage, int type, int pointer, int knownName, bool inherit, int version);
						void CheckMethodAttributeName(const char* name);
						void DoDataEditTags(CComOBBaseData* dat);
							void DoDataETDefaultValue(CComOBBaseData* dat);
							void DoDataETList(CComOBBaseData* dat);
							void DoDataETEditType(CComOBBaseData* dat);
							void DoDataETIs(CComOBBaseData* dat);
								void DoNameLater(int type, int index);


	void DoMethodBody();

	// parsing prikazu
	void DoCommand();
		void DoIf();
		void DoWhile();
		void DoDo();
		void DoFor();
		void DoReturn();
		void DoBreak();
		void DoContinue();
		void DoSwitch();
		void DoWith();

	// parsing vyrazu
	void DoExpression(bool putStartExp = true);
		void DoAssign();
		void DoConditional();
		void DoLogicOr();
		void DoLogicAnd();
		void DoBitOr();
		void DoBitXor();
		void DoBitAnd();
		void DoEqual();
		void DoRelation();
		void DoShift();
		void DoAdd();
		void DoSub();
		void DoMultiply();
		void DoDivide();
		void DoUnary();
			void DoPrefixPostfix();
			void DoPrefix(int& op);
		void DoSizeof();
		void DoAssigned();
		void DoTypeOf();
		void DoAlloc();
		void DoDealloc();
		 // vraci true, pokud je zpracovano volani funkce
		bool DoPostfix(int chainedMemberAccess = cUnknown, CComShortPosition* chainedPos = 0);

		void DoPrimary();
		void DoPrimaryMain();
		void DoIdentifier();

		void DoMethodCall(CComShortPosition* opPos);
			void DoNameArg();
	
		void DoLocalDef();
			void DoOneLocal(int type);

		// parsing dalsich globalnich deklaraci
		void DoNameDef();
			void DoOneName(int type);
				void DoNameParts(char** name, int& maxNameLn, int* version);
					void DoReadVersion(int& version);
					int ResolveVersionAmbiguity(CListK<CComNSEEntry>* nList, int localVersion, CComShortPosition* versionPos);

				void DoNameCompleteParse(CComOBBase** name, int* nameID);
			void CheckGlobalName(const char* name, const char* nameToShow);

		void DoNameDepend();
			void DoOneDepend(CComOBBaseData** d, CKerName** n, int& id);
			CComOBBaseData* GetDefinedName(const char* name, int& id);


	void DoGlobalDef();
		void DoOneGlobal(int type);


	// parsing necistych operaci
	void DoModifyOrRemove(enmDirtyOpType type);

	// parsing konstantiho vyrazu
	void DoConstExpression();
		void DoConstLogicOr();
		void DoConstLogicAnd();
		void DoConstBitOr();
		void DoConstBitXor();
		void DoConstBitAnd();
		void DoConstEqual();
		void DoConstRelation();
		void DoConstShift();
		void DoConstAdd();
		void DoConstMultiply();
		void DoConstUnary();
		void DoConstPrefix(int& op);
		void DoConstPrimary();
			void DefineKnownName(int type, const char* idName);
		void DoOp(enmOperators op, CComShortPosition pos);
	
	// prace po pruchodu celym zdrojakem -> urceni dedicnosti
	void DoPostPass();
	void Inherit(CComOBObject* o, CComOBMethod* m, int fromIndex);
	void Inherit(CComOBObject* o, CComOBAttribute* a, int fromIndex, bool scripted);
	void Inherit(CComOBObject* o, CComOBAttributeGroup* ag, int fromIndex);

	// manipulace s obsahem vystupni fronty
	void CopyQueueFromPos(int ref);					// zkopiruje frontu od ref jeste jednou na konec fronty
	void RenameMethodOnPos(int ref, int newID);		// najde na pozice ref ID metody a prepise ho na newID
	void SetLastOpState(enmTokens state);			// najde posledni operator v aktualnim vyrazu (jinak skonci) a pred nej da stavovy token state

};
