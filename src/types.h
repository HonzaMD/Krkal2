/////////////////////////////////////////////////////////////////////////////
//
// types.h
//
// typy a makra pouzivane v KRKALovi
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////

#ifndef KRKALTYPES_H
#define KRKALTYPES_H

#include <assert.h>

#define SAFE_RELEASE(p) {if(p){(p)->Release();(p)=NULL;}}
#define RELEASE(p) {(p)->Release();(p)=NULL;}
#define SAFE_DELETE(p) {if(p){delete (p);(p)=NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p){delete[](p);(p)=NULL;}}

#define RO_CLEAR(file) {FILE*f=fopen(file,"wt");fclose(f);}
#define RO_TEXT(file,text) {FILE*f=fopen(file,"at");fprintf(f,(text));fclose(f);}
#define RO_INT(file,n) {FILE*f=fopen(file,"at");fprintf(f,"%d\n",(n));fclose(f);}

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

#define UI unsigned int
#define UC unsigned char

char *newstrdup(const char *str);

///////////////////////////////////////////////////////////////////////////////////////////////
//
// CRefCnt
//
// pocitani referenci
//
///////////////////////////////////////////////////////////////////////////////////////////////

void MyDebugTab(char* file, int tabs);
void MyDebugStr(char* file, char* str);
void MyDebug(char* file, char* str, int i, int info = -1);

class CRefCnt{

private:
	unsigned int RefCnt;

public:

	CRefCnt() {RefCnt=0;}
	virtual ~CRefCnt() {
		//vyhodit error, kdyz refcnt>0
		assert(RefCnt==0);
	}

	void AddRef(){RefCnt++;}
	void Release(){

		if(RefCnt==0)
			delete this;
		else
			RefCnt--;

	}
	int GetRefCnt(){return RefCnt;}
};

///////////////////////////////////////////////////////////////////////////////////////////////
//
// CExc
//
// objekt na vyjimky
//
///////////////////////////////////////////////////////////////////////////////////////////////


enum eExcType{
	eDXblitter,
	eFS,
	eHash,
	eKernel,
	eGEn,
	eGUI
};

class CExc
{
public:

	CExc ( int etype, int param, char *format,...); //pouziva se jako printf, maximalni delka retezce je 256

	char errstr[256];
	int  errnum;
	int	 param;
};

///////////////////////////////////////////////////////////////////////////////////////////////
//
// CListK
//
// jednosmerny seznam
//
///////////////////////////////////////////////////////////////////////////////////////////////

//pri ruseni prvku seznamu se nedestruuje objekt T - ani nemuze - nevi se jak byl vytvoren


//prverk seznamu
template <class T> class CListKElem{

public:
	CListKElem(const T &dat){data=dat;next=NULL;}

	T data; 

	CListKElem<T> *next; //pointr na dalsi
};

//seznam
template <class T> class CListK{

public:
	CListK() {el=ellast=NULL; num = 0; }
	~CListK() { DeleteAll(); }

	void DeleteAll() //smaze vsechny prvky seznamu
	{
		CListKElem<T> *e=el,*f;
		while(e)
		{
			f=e;
			e=e->next;
			delete f;
		}
		el=ellast=NULL;
		num = 0;
	}

	int Add(const T &dat) //prida prvek na zacatek seznamu
	{
		CListKElem<T> *e;
		e=new CListKElem<T>(dat);
		e->next=el;
		el=e;
		if(!ellast)
			ellast = e;
		num++;
		return 1;
	}

	int AddTail(const T &dat) //prida prvek na konec seznamu
	{
		CListKElem<T> *e;
		e=new CListKElem<T>(dat);
		e->next=0;
		if(!ellast)
			ellast = el = e;
		else
			ellast = ellast->next=e;
		num++;
		return 1;
	}

	int AddUnique(const T &dat){ //prida prvek, jen pokud v seznamu jeste neni
		CListKElem<T> *e;

		num++;
		if(!el) {
			el=new CListKElem<T>(dat);
			return 1;
		}
		e=el;
		while(e){
			if(e->data==dat) 
			{
				num--;
				return 0;
			}

			e=e->next;
		}
		e=new CListKElem<T>(dat);
		e->next=el;
		el=e;

		return 1;

	}

	int Delete(const T &dat) //smaze prvek
	{
		CListKElem<T> *e,*f;

		if(!el) return 0;
		num--;

		if(el->data==dat){
			e=el;
			if(el == ellast)
				ellast = el->next;
			el=el->next;
			
			delete e;
			
			return 1;
		}

		e=el;f=el->next;
		while(f){
			if(f->data==dat){
				if(f == ellast)
					ellast = e;
				e->next=f->next;
				delete f;
				return 1;
			}
			e=f;f=f->next;
		}

		num++;
		return 0;
	}

	bool IsEmpty() { return el == 0; }

	T RemoveHead()
	{
		assert(!IsEmpty());
		CListKElem<T>* p = el;
		T ret = p->data;
		if(el == ellast)
			ellast = el->next;
		el = el->next;
		SAFE_DELETE(p);
		num--;
		return ret;
	}


	CListKElem<T> *el; //pointr na prvni prvek (pokud zadny neni =NULL)
	CListKElem<T> *ellast; 
	int num;

};


// Iterator umoznuje pohodlne prolejzani seznamem
//
// operator ++ :posune iterator na dalsi
// do iteratoru je mozne priradit typ T, je mozne iterator pretypovat na typ T
// pokud je T poitr na tridu je mozne pristupovat k polozkam tridy pomoci operatoru ->
//
// vyhazovani prvku, pridani prvku
//

template <class T> class CListIterator{
public:
	CListIterator() {el = NULL; }
	CListIterator(CListK<T>& list){ el = &list.el; }
	CListIterator(CListK<T>* list){ el = &list->el; }

	void SetList(CListK<T>& list){el = &list.el; }
	void SetList(CListK<T>* list){el = &list->el; }

	CListIterator<T> operator++() { //postincrement
		CListIterator<T> temp = *this;
		if(*el) el=&(*el)->next;
		return temp;
	}
	CListIterator<T>& operator++(int) { //preincrement
		if(*el) el=&(*el)->next;
		return *this;
	}

	const T& operator=(const T& data){ //prirazeni
		if(*el) (*el)->data=data;
		return data;
	}

	operator const T&(){ //pretypovani
		return (*el)->data;
	}
    
	T& operator ->(){ //pokud je T pointr na strukturu, tak umoznuje lizt k polozkam T pomoci ->
		return (*el)->data;
	}

	int operator==(const T& data){ //porovnani
		if((*el)->data==data) return 1; else return 0;
	}
	int operator==(const CListIterator<T>& it){
		if((*el)==*it.el) return 1; else return 0;
	}

	int End(){return !(*el);} //konec?

	int Remove(){ //vyhodi prvek ze seznamu
		if(!*el) return 0;
		CListKElem<T> *e=*el;
		*el=e->next;
		e->next=NULL;
		delete e;
		return 1;
	}
	int AddBefore(const T& data) //prida prvek pred (iterator zustane na stejnem prvku)
	{
		CListKElem<T> *elm = new CListKElem<T>(data);
		elm->next=*el;
		*el=elm;
		el=&elm->next;
		return 1;
	}
	int AddBehind(const T& data) //prida prvek za (iterator zustane na stejnem prvku)
	{
		CListKElem<T> *elm = new CListKElem<T>(data);
		if(!*el) //jsem nakonci - pridam prvek nakonec
		{
			*el=elm;
		}else{
			elm->next=(*el)->next;
			(*el)->next=elm;
		}
		return 1;
	}

protected:
	CListKElem<T> **el;
};


//zrusi vsechny prvky seznamu a zavola delete
template <class T> void DeleteListElems(CListK<T>& list)
{
	CListIterator<T> it(list);
	T el;
	while(!it.End()){
		el = it;
		SAFE_DELETE( el );
		it++;
	}
	list.DeleteAll();
};

//zrusi vsechny prvky seznamu a zavola release
template <class T> void ReleaseListElems(CListK<T>& list)
{
	CListIterator<T> it(list);
	T el;
	while(!it.End()){
		el = it;
		SAFE_RELEASE( el );
		it++;
	}
	list.DeleteAll();
};


////////////////////////////////////////////////////////////////////////////////////////
///
///  HASH TABLE
///
///  A: M.D.
///
////////////////////////////////////////////////////////////////////////////////////////

/*
	Hashovacim klicem je promenna name - cos je string ukonceny nulou

	POUZITI
	-------
	Vytvorte si hashovaci tabulku pozadovane velikosti (jen mocnina 2)
	A ukladejde do ni (hledejte, odebirejte) objekty oddedene od objektu typu CHashElem
	POZOR HashElem nerusi svou promennou name!!! To naprogramujte v destruktoru potomka!
*/

class CHashElem;
class CHashList;



////////////////////////////////////////////////////////////////////////////////////////
///
///		CHashTable - HASHOVACI TABULKA
///
////////////////////////////////////////////////////////////////////////////////////////
class CHashTable {
friend CHashElem;
public:
	CHashTable(int hashsize) { // hashsize (= velikost tabulky) musi byt mocniny 2
		int f;
		HashSize = hashsize-1;
		HashTable = new CHashList*[hashsize];
		for (f=0;f<hashsize;f++) HashTable[f]=0;
	};
	CHashElem *Member(const char *name); // Vyhleda v HashTabulce podle jmena
	CHashElem *MemberFirst(const char *name, CHashList **ptr); // Vyhleda prvni vyskyt
	CHashElem *MemberNext(const char *name, CHashList **ptr); // Vyhleda nasledujici vyskyt
	CHashElem *MemberLast(const char *name, CHashList **ptr); // Vyhleda posledni vyskyt
	CHashElem *MemberPrev(const char *name, CHashList **ptr); // Vyhleda predchozi vyskyt
	~CHashTable(); // odebere vsechny polozky z tabulky a zrusi tabulku (polozky neodstrani)
	void DeleteAllMembers(); // odebere a odstrani vsechny polozky
private:
	int HashFunction(const char *name);
protected:	// Jirka
	CHashList **HashTable; // Hash Table 
	int HashSize;
};


//////////////////////////////////////////////////////////////////////////////////
/// Spoljovy seznam Hash Elementu
class CHashList {
friend CHashElem;
friend CHashTable;
public:
	CHashList(CHashElem * Name, CHashList *Next) {
		name=Name;
		next=Next;
	}
//private:
public:
	CHashElem *name;
	CHashList *next;
};




/////////////////////////////////////////////////////////////////////////////////
///
///		Hash Element
///
///		Muze byt pridan do jedne hashovaci tabulky. Hashuje se podle jmena (string)
///
/////////////////////////////////////////////////////////////////////////////////
class CHashElem {
friend CHashTable;
public:
	CHashElem(){
		name = 0;
		HashTable = 0;
	}
	void SetName(char *Name) { // Nastaveni jmena. Pozor JMENO NENI RUSENO. Je treba doprogramovat u potomka
		name = new char[strlen(Name)+1]; // zkopiruju si string se jmenem
		strcpy(name,Name);
	}
	void AddToHashTable(CHashTable *ht) { // Pridej do hashTable
		if (HashTable) throw CExc(eHash,0,"Hashed in more than one HashTables!");
		HashTable=ht;
		CHashList **list = HashTable->HashTable+HashTable->HashFunction(name);
		*list = new CHashList(this,*list);
	}
	int AddToHashTableUnique(CHashTable *ht) { // Pridej do hashTable Pokud tam neni
		if (HashTable) throw CExc(eHash,0,"Hashed in more than one HashTables!");
		HashTable=ht;
		CHashList **list = HashTable->HashTable+HashTable->HashFunction(name);
		if (ht->MemberNext(name,list)) return 0;
		*list = new CHashList(this,*list);
		return 1;
	}
	void RemoveFromHashTable() {  // Odeber z HashTable
		if (HashTable) HFindAndDelete(&(HashTable->HashTable[HashTable->HashFunction(name)]),this);
		HashTable = 0;
	}
	virtual ~CHashElem(){ // Prvek se odebere z Hashovaci tabulky
		if (HashTable) HFindAndDelete(&(HashTable->HashTable[HashTable->HashFunction(name)]),this);
		// Pozor JMENO name NENI RUSENO. Je treba doprogramovat u potomka
	}
private:
	void HFindAndDelete(CHashList **p2,CHashElem *name); // najde jmeno v seznamu a odstrani ho
//protected:
public:
	char *name; // string jmena
	CHashTable *HashTable; // Hash Table, kde jsem; 0 - nejsem v zadne HashTable
};

///////////////////////////////////////////////////////////////////////////////////////////////
//
// CDynamicArray
//
// dynamicke pole
//
///////////////////////////////////////////////////////////////////////////////////////////////

template <class T, int initSize=128> 
class CDynamicArray
{
public:
	CDynamicArray() : size(initSize), count(0)
	{ 
		array = new T*[size];
		assert(array);
		for(int i=0; i<initSize; i++)
			array[i] = 0;
	}

	~CDynamicArray() { RemoveAll(); SAFE_DELETE_ARRAY(array); }

	int GetCount() const { return count; }
	T*  operator[](int index) { return Get(index); }
	T*	Get(int index) { return (index>=0 && index<GetCount()) ? array[index] : 0; }

	int Add(T* item) 
	{ 
		if(GetCount() == size) 
			Expand(); 
		array[count] = item;
		return count++;
	}

	int Find(T* item)
	{
		int i = 0;
		while(i < GetCount())
		{
			if(Get(i) == item)
				return i;
			i++;
		}
		return -1;
	}

	void RemoveAll() { for(int i=0; i<GetCount(); i++) Remove(i); count = 0;}


private:
	T** array;
	int count;
	int size;

	void Remove(int index) { T* p=Get(index); SAFE_DELETE(p);  }

	void Expand()
	{
		int newSize = 2*size;
		T** newArray = new T*[newSize];
		assert(newArray);
		int i;
		for(i=0; i<GetCount(); i++)
			newArray[i] = Get(i);
		for(i=GetCount(); i<newSize; i++)
			newArray[i] = 0;
		SAFE_DELETE_ARRAY(array);
		array = newArray;
		size = newSize;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////
//
// CDynamicArrayV
//
// dynamicke pole hodnot
//
///////////////////////////////////////////////////////////////////////////////////////////////

template <class T, int initSize=128> 
class CDynamicArrayV
{
public:
	CDynamicArrayV() : size(initSize), count(0)
	{ 
		array = new T[size];
		assert(array);
	}

	~CDynamicArrayV() { RemoveAll(); SAFE_DELETE_ARRAY(array); }

	int GetCount() const { return count; }
	T&  operator[](int index) const { return Get(index); }
	T&	Get(int index) const { assert(index>=0 && index<GetCount()); return array[index]; }

	int Add(T& item) 
	{ 
		if(GetCount() == size) 
			Expand(); 
		array[count] = item;
		return count++;
	}

	int Insert(int index, T& item)
	{
		int i;
		assert(index < count);
		if(GetCount() == size) 
			Expand(); 

		for(i = count; i > index; i--)
			array[i] = array[i-1];

		array[index] = item;
		return count++;
	}

	int Find(T& item)
	{
		int i = 0;
		while(i < GetCount())
		{
			if(Get(i) == item)
				return i;
			i++;
		}
		return -1;
	}

	void RemoveAll() { count = 0;}

	void SetCount(int Count) {	// jen pro zmensovani
		assert(Count>=0&&Count<=size) ;
		count = Count;
	}

	T Remove(int k) 
	{ 
		assert(k < count);
		T ret = array[k];

		count--;
		for(int i=k; i<count; i++)
			array[i] = array[i+1];
		
		return ret;
	}

	T RemoveHead() { return Remove(0); }

	T RemoveLast()	
	{ 
		assert(count);
		return array[--count];
	}

	T GetLast() { return array[count>0 ? count-1 : 0]; }

private:
	T* array;
	int count;
	int size;

	void Expand()
	{
		int newSize = 2*size;
		T* newArray = new T[newSize];
		assert(newArray);
		for(int i=0; i<GetCount(); i++)
			newArray[i] = Get(i);
		SAFE_DELETE_ARRAY(array);
		array = newArray;
		size = newSize;
	}
};


#endif