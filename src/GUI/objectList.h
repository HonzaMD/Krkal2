///////////////////////////////////////////////
//
// objectList.h: sablona na vytvareni seznamu objektu
//					- obousmerny spojak s hlavou a patou
//					- umoznujici pridavat na zacatek i konec (FIFO / LIFO)
//
//		-- pro pouziti seznamu musi mit zarazovana trida definovanou pratelskou tridu CGUIListElement
//		-- a prvek "CGUIList<T>* list", ktery odkazuje na seznam, ve kterem je objekt zarazeny
//		-- implementace nepouziva (nevyhazuje) vyjimky
//		-- objekt muze byt max. v jednom seznamu se zapnutou kontrolou (listChecking)
//		-- objekt muze byt v neomezene seznamech s vypnutou kontrolou, funkce na mazani (delete) se ovsem muzou volat jen jednou (v ostatnich seznamech se musi volat pouze remove)
//
//	A: Jan Poduska
//
//////////////////////////////////////////////////////////////////////

#ifndef OBJECTLIST_H
#define OBJECTLIST_H

#include "types.h"

//////////////////////////////////////////////////////////////////////
// CGUIListElement
//////////////////////////////////////////////////////////////////////

template <class T> class CGUIList;

template <class T, class P>	T *Get(CGUIList<T> *list, P prm);		
							// vrati objekt ze seznamu, ktery odpovida zadanemu parametru (prm)
							// vyhledava se podle typu "prm" a podle pretizeneho operatoru ==
							// u frontovaneho objektu
							// hleda se napr. podle jmena, pointeru, cisla atp.
							// proste podle toho, jak si to frontovany objekt preje a 
							// jak si to pretizenim operatoru == zaridi
							// stejny princip je u ostatnich fci teto kategorie, viz dale:
template <class T, class P>	bool IsMember(CGUIList<T> *list, P prm);
template <class T, class P> T *Remove(CGUIList<T> *list, P prm);
template <class T, class P> void Delete(CGUIList<T> *list, P prm);	// (zrusi objekt i s vypnutym listChecking, proste kazdopadne smaze objekt !!!) - vice viz. DeleteFromBack()


template <class T> class CGUIListElement		// element seznamu objekty typu T
												// musi byt jako pratelska trida v objektech, ktere se maji radit do seznamu
												// realizuje propojeni prvku seznamu
												// pri destrukci ListElementu se nerusi jeho objekt, dela se pripadne zvlast
{
public:
	T* object;
	CGUIListElement* prev;
	CGUIListElement* next;

	CGUIListElement() { prev = next = 0; object = 0;}
	void DeleteListElementObject();

	bool IsNotObjectInList(CGUIList<T>* list) { return (object && object->list != list); }	// vrati zda objekt neni v seznamu "list" 
	bool IsObjectInAnyList() { return (object && object->list !=0); }	// vrati zda objekt neni v zadnem seznamu
	void SetObjectList(CGUIList<T> *list) { if(object) object->list=list; }	// nastavi odkaz na seznam objektu na "list"
};

template <class T> void CGUIListElement<T>::DeleteListElementObject()	// smaze objekt, prvek seznamu zustane zachovan
{
	if(object)
	{
		object->list = 0;
		delete object;
	}
}


//////////////////////////////////////////////////////////////////////
// CGUIList
//////////////////////////////////////////////////////////////////////

template <class T> class CGUIList
{

public:
	CGUIListElement<T> *GetNextListElement(CGUIListElement<T> *le);	// nebezpecna fce	(le==0 => vrati prvni prvek seznamu)
	CGUIListElement<T> *GetPrevListElement(CGUIListElement<T> *le);	// nebezpecna fce	(le==0 => vrati posledni prvek seznamu)
										// (nepracuje s objekty, ale s celym ListElementem)
										// jen pro urychleni vyhledavani objektu v seznamu podle specialnich (neobecnych) parametru
	CGUIListElement<T> *GetNextListElementObj(T *obj);		// nebezpecna fce (to samy, ale vychazi z objektu a ne ListElementu)
	CGUIListElement<T> *GetPrevListElementObj(T *obj);		// nebezpecna fce (to samy, ale vychazi z objektu a ne ListElementu)
	T* GetObjectFromListElement(CGUIListElement<T> *le);	// nebezpecna fce (vrati objekt z ListElementu)
	T* RemoveListElement(CGUIListElement<T> *le);	// nebezpecna fce 
													// zrusi ListElement a upravi seznam do konzistentni podoby
													// nezrusi hlavu ani zadek seznamu
													// vrati objekt z ruseneho ListElementu
	
	CGUIList(bool _listChecking = true);	// vytvori seznam s kontrolou / bez kontroly
											// kontrola znamena, ze se kontroluje odkaz na seznam u objektu (obj->list) s aktualne zpracovavanym seznamem
											// objekt muze byt jen v jednom kontrolovanem seznamu
	~CGUIList();					// pri ruseni seznamu se zrusi i vsechny objekty v nem obsazene !!!
									// pokud je vypnuty listChecking => objekty zustanou zachovany !!!
	void AddToHead(T* object);
	void AddToBack(T* object);
	void AddToBehindObj(T* which, T* object);	// umisti objekt do seznamu za objekt "which" (kdyz which v seznamu neni, umisti objekt za hlavu seznamu)
	void AddToFrontObj(T* which, T* object);	// umisti objekt do seznamu pred objekt "which" (kdyz which v seznamu neni, umisti objekt pred zadek seznamu)
	T *GetFromHead();
	T *GetFromBack();
	T *RemoveFromHead();		// odstrani prvni prvek ze seznamu a vrati ho 
	T *RemoveFromBack();
	void DeleteFromHead();		// odstrani prvni prvek ze seznamu a smaze ho (i s vypnutym listChecking, proste kazdopadne smaze objekt !!!)
	void DeleteFromBack();		// (i s vypnutym listChecking, proste kazdopadne smaze objekt !!!)
								// pokud se ovsem pokousim smazat objekt ze seznamu s vypnutym listCheckingem
								// a prijdu na to, ze objekt je obsazen jeste v jinem seznamu se
								// zapnutym listCheckingem => chyba (assert(false)), do budoucna jen nemazat objekt,
								// ale samozrejme vyradit ho ze seznamu s vypnutym listCheckingem

	T* GetNext(T* obj);			// vrati nasledujici prvek v seznamu (0 = vrati prvni prvek)
	T* GetPrev(T* obj);			// vrati predchozi prvek v seznamu (0 = vrati posledni prvek)
	T* Remove(T* obj);			// odstrani prvek ze seznamu a vrati ho
	void Delete(T* obj);		// odstrani prvek ze seznamu a smaze ho (i s vypnutym listChecking, proste kazdopadne smaze objekt !!!) - vice viz. DeleteFromBack()
	bool IsMember(T* obj);		// vrati zda je objekt obsazen v seznamu

	CGUIListElement<T> *head, *back;

	bool listChecking;		// zda se pri praci se seznamem kontroluje, nastavuje atp. promenna list,
							// kterou obsahuje frontovany objekt
							// promenna list ukazuje na seznam, ve kterem je objekt obsazen
							// pri zapnute kontrole listu muze byt objekt obsazen max. v jenom seznamu
							// pri vypnute kontrole muze byt v neomezene seznamech, ovsem musi
							// si sam davat pozor, aby se z nich korektne vyndaval atp. !!!
};


//////////////////////////////////////////////////////////////////////
// CGUIGrowingArray
// automaticky rostouci pole dle potrebne velikosti
// umi pridavat, vracet, odstranovat objekt 
// pri destrukci pole zustanou objekty zachovany, nevola se jejich destruktor
// umi zjistit zda zadany objekt je obsazen v poli a vrati index na nej
// umi setrast prvky pole, aby obsazovaly nejnizsi pozice pole
// v pripade potreby lze dodelat i zmensovani pole
// pristupy k prvkum jsou kontrolovane, tzn. nikdy se nepristupuje na pozici, kde nic nebylo ulozeno nebo mimo oblast pole (v techto pripadech vraci 0)
// kvuli kontrolovanemu pristupu obsahuje 2 pole - jedno pro objekty, druhe rikajici zda je prvek vyuzit nebo ne
// pole je realizovano jako sablona - lze do nej pridavat jak cele objekty tak i pointery na objekty
//////////////////////////////////////////////////////////////////////

template <class T> class CGrowingArray
{
public:
	CGrowingArray(int n=0)		// vytvori pole o zadane pocatecni velikosti
		{
			if(n)
			{
				array = new T[n];
				arrayFull = new char[n];
				for(int i=0;i<n;i++)
					arrayFull[i]=0;
			}else{
				array=0;
				arrayFull=0;
			}
			arraySize=n;
			startFinding=0;
		};

	~CGrowingArray()
		{
			SAFE_DELETE_ARRAY(array);
			SAFE_DELETE_ARRAY(arrayFull);
		};

	void Rebuild(int n=0)		// smaze puvodni obsah pole a vytvori nove prazdne pole o n prvcich
		{
			SAFE_DELETE_ARRAY(array);
			SAFE_DELETE_ARRAY(arrayFull);

			if(n)
			{
				array = new T[n];
				arrayFull = new char[n];
				for(int i=0;i<n;i++)
					arrayFull[i]=0;
			}else{
				array=0;
				arrayFull=0;
			}
			arraySize=n;
			startFinding=0;
		};

	int Add(T obj)		// prida objekt do pole a vrati index na nej
		{
			if(!array)
			{
				array = new T[1];
				arrayFull = new char[1];
				for(int i=0;i<1;i++)
					arrayFull[i]=0;
				arraySize=1;
				startFinding=0;
			}
			for(int i=startFinding;i<arraySize;i++)
				if(!arrayFull[i])
				{
					arrayFull[i]=1;
					array[i]=obj;
					startFinding=i+1;
					return i;
				}

			startFinding=arraySize;
			Grow();
			return Add(obj);
		};

	int Set(T obj, int index)		// prida objekt do pole na pozici index
									// pokud je jiz tato pozice obsazena, vrati -1 a nic neudela
									// jinak vrati index pozice
		{
			if(!array)
			{
				arraySize = (int)exp(2*log((double)index)+1);
				array = new T[arraySize];
				arrayFull = new char[arraySize];
				for(int i=0;i<1;i++)
					arrayFull[i]=0;
				startFinding=0;
			}
			else if(index>=arraySize)
			{
				while(index>=arraySize)
					Grow();
			}

			if(arrayFull[index])
				return -1;

			arrayFull[index]=1;
			array[index]=obj;
			return index;
		};


	T Get(int i)	// vrati objekt na indexu "i", pokud tam nic neni vrati 0
		{
			if(i>=0 && i<arraySize)
			{
				if(arrayFull[i])
					return array[i];
				else
					return 0;
			}else
				return 0;
		};

	T Remove(int i)	// odstrani objekt na indexu "i" z pole a vrati ho, pokud tam nic neni vrati 0
		{
			if(i>=0 && i<arraySize)
			{
				if(arrayFull[i])
				{
					arrayFull[i]=0;
					if(i<startFinding)
						startFinding=i;
					return array[i];
				}else
					return 0;
			}else
				return 0;
		};

	int GetSize() { return arraySize;};	// vrati velikost pole (nemusi byt vsechny polozky obsazene)

	T& operator [] (const int i)	// vrati objekt na indexu "i", nekontrolovany pristup ! (muze pristupovat i mimo oblast pole, nebo na prvky kde nebylo nic ulozeno)
		{
			return array[i];
		};

	int FindObj(T obj)		// projde obsazene prvky pole a vrati index na prvni objekt, ktery se rovna zadanemu objektu (operator ==), pokud zadny nenalzene, vrati -1
		{
			for(int i=0;i<arraySize;i++)
				if(arrayFull[i])
					if(obj==array[i])
						return i;
			return -1;
		}
	int ShakeDown()	// setrese objekty v poli tak, aby zabiraly nejnizzsi indexy pole
					// vrati pocet obsazenych polozek
		{
			int write=0;
			for(int i=0;i<arraySize;i++)
			{
				if(arrayFull[i])
				{
					if(write!=i)
					{
						array[write]=array[i];
						arrayFull[write]=1;
					}
					write++;
				}
			}
			for(int i=write;i<arraySize;i++)
			{
				arrayFull[i]=0;
			}
			startFinding=write;
			return write;
		}
protected:
	void Grow()		// zvetsi pole na dvonasobek
		{
			T* arrayBig = new T[arraySize*2];
			char *arrayFullBig = new char[arraySize*2];

			for(int i=0;i<arraySize;i++)
				arrayFullBig[i]=arrayFull[i];
			for(int i=arraySize;i<arraySize*2;i++)
				arrayFullBig[i]=0;

			for(int i=0;i<arraySize;i++)
				if(arrayFull[i])
					arrayBig[i]=array[i];
			
			SAFE_DELETE_ARRAY(array);
			SAFE_DELETE_ARRAY(arrayFull);

			arraySize=arraySize*2;
			array=arrayBig;
			arrayFull=arrayFullBig;
		};
	

	T* array;
	char *arrayFull;	// pole "booleanu" - zda je polozka obsazena
	int arraySize;
	int startFinding;
};


//////////////////////////////////////////////////////////////////////
// CGUIList
//////////////////////////////////////////////////////////////////////

template <class T> CGUIList<T>::CGUIList(bool _listChecking)
{
	head = new CGUIListElement<T>;
	back = new CGUIListElement<T>;
	head->next = back;
	back->prev = head;
	listChecking = _listChecking;
}


template <class T> CGUIList<T>::~CGUIList()
{
	CGUIListElement<T> *le;
	while(head)
	{
		le = head;
		head = head->next;

		if(listChecking && le->IsNotObjectInList(this))	// !@#$ jen prozatim pro kontrolu (object musi byt v tomto seznamu
			assert(false);			// kdyz ho z neho mazu a musi mit nastaven svuj seznam (list) na 
									// tento seznam)
									// (pokud je zapnuty listChecking)
		if(listChecking)
			le->DeleteListElementObject();
		delete le;
	}
}

template <class T> void CGUIList<T>::AddToHead(T* object)
{
	if(object)
	{
		CGUIListElement<T> *listElement = new CGUIListElement<T>;	// vytvoreni a nastaveni noveho elementu seznamu
		listElement->object = object;
		listElement->next = head->next;
		listElement->prev = head;
		
		// propojeni z puvodnimi prvky seznamu
		head->next->prev = listElement;
		head->next = listElement;

		if(listChecking && listElement->IsObjectInAnyList())
			assert(false);		// !@#$ object uz v nejakem seznamu je a ja ho vkladam do dalsiho
								// toto zakazuji !!! (objekt smi byt jen v jednom seznamu)
								// (pokud je zapnuty listChecking)
								// do budoucna predelat na vyjimku ???
		if(listChecking)
			listElement->SetObjectList(this);	// nastaveni seznamu, ve kterem se nachazi objekt na tento seznam (pokud je zapnuty listChecking)
	}
}

template <class T> void CGUIList<T>::AddToBack(T* object)
{
	if(object)
	{
		CGUIListElement<T> *listElement = new CGUIListElement<T>;	// vytvoreni a nastaveni noveho elementu seznamu
		listElement->object = object;
		listElement->next = back;
		listElement->prev = back->prev;
		
		// propojeni z puvodnimi prvky seznamu
		back->prev->next = listElement;
		back->prev = listElement;

		if(listChecking && listElement->IsObjectInAnyList())
			assert(false);		// !@#$ object uz v nejakem seznamu je a ja ho vkladam do dalsiho
								// toto zakazuji !!! (objekt smi byt jen v jednom seznamu)
								// (pokud je zapnuty listChecking)
								// mozna predelat na vyjimku ???
		if(listChecking)
			listElement->SetObjectList(this);	// nastaveni seznamu, ve kterem se nachazi objekt na tento seznam (pokud je zapnuty listChecking)

	}
}

template <class T> void CGUIList<T>::AddToBehindObj(T* which, T* object)
{
	if(object)
	{
		CGUIListElement<T> *le=head;

		if(which)
		{
			while(le->next && le->object!=which)
			{
				le = le->next;
			}
			if(le->object==which)
			{
				if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
					assert(false);
			}else
				le=head;
		}


		CGUIListElement<T> *listElement = new CGUIListElement<T>;	// vytvoreni a nastaveni noveho elementu seznamu
		listElement->object = object;
		listElement->next = le->next;
		listElement->prev = le;
		
		// propojeni z puvodnimi prvky seznamu
		le->next->prev = listElement;
		le->next = listElement;

		if(listChecking && listElement->IsObjectInAnyList())
			assert(false);		// !@#$ object uz v nejakem seznamu je a ja ho vkladam do dalsiho
								// toto zakazuji !!! (objekt smi byt jen v jednom seznamu)
								// (pokud je zapnuty listChecking)
								// do budoucna predelat na vyjimku ???
		if(listChecking)
			listElement->SetObjectList(this);	// nastaveni seznamu, ve kterem se nachazi objekt na tento seznam (pokud je zapnuty listChecking)
	}
}

template <class T> void CGUIList<T>::AddToFrontObj(T* which, T* object)
{
	if(object)
	{
		CGUIListElement<T> *le=back;

		if(which)
		{
			while(le->prev && le->object!=which)
			{
				le = le->prev;
			}
			if(le->object==which)
			{
				if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
					assert(false);
			}else
				le=back;
		}


		CGUIListElement<T> *listElement = new CGUIListElement<T>;	// vytvoreni a nastaveni noveho elementu seznamu
		listElement->object = object;
		listElement->next = le;
		listElement->prev = le->prev;
		
		// propojeni z puvodnimi prvky seznamu
		le->prev->next = listElement;
		le->prev = listElement;

		if(listChecking && listElement->IsObjectInAnyList())
			assert(false);		// !@#$ object uz v nejakem seznamu je a ja ho vkladam do dalsiho
								// toto zakazuji !!! (objekt smi byt jen v jednom seznamu)
								// (pokud je zapnuty listChecking)
								// mozna predelat na vyjimku ???
		if(listChecking)
			listElement->SetObjectList(this);	// nastaveni seznamu, ve kterem se nachazi objekt na tento seznam (pokud je zapnuty listChecking)
	}
}

template <class T> T *CGUIList<T>::GetFromHead(void)
{
	return head->next->object;	// head->next v seznamu vzdy je (alespon jako back)
								// pokud je seznam prazdny vrati 0 (back) jinak vrati prvni objekt
}

template <class T> T *CGUIList<T>::GetFromBack(void)
{
	return back->prev->object;	// head->next v seznamu vzdy je (alespon jako back)
								// pokud je seznam prazdny vrati 0 (back) jinak vrati prvni objekt
}


template <class T> T *CGUIList<T>::RemoveFromHead(void)
{
	if(head->next->object)
	{						// je to normalni prvek (nejedna se o hlavu nebo zadek)
		CGUIListElement<T> *le;
		
		le = head->next;

		if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba) (pokud je zapnuty listChecking)
			assert(false);

		head->next=le->next;
		le->next->prev=head;

		T* object = le->object;
		if(listChecking)
			le->SetObjectList(0);
		delete le;

		return object;
	}else
		return 0;	
}

template <class T> T *CGUIList<T>::RemoveFromBack(void)
{
	if(back->prev->object)
	{						// je to normalni prvek (nejedna se o hlavu nebo zadek)
		CGUIListElement<T> *le;
		
		le = back->prev;

		if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba) (pokud je zapnuty listChecking)
			assert(false);

		back->prev=le->prev;
		le->prev->next=back;

		T* object = le->object;
		if(listChecking)
			le->SetObjectList(0);
		delete le;
		
		return object;
	}else
		return 0;	
}

template <class T> void CGUIList<T>::DeleteFromHead(void)
{
	if(head->next->object)
	{						// je to normalni prvek (nejedna se o hlavu nebo zadek)
		CGUIListElement<T> *le;
		
		le = head->next;
		head->next=le->next;
		le->next->prev=head;

		if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba) (pokud je zapnuty listChecking)
			assert(false);
		if(!listChecking && listElement->IsObjectInAnyList())
			assert(false);		// !@#$ object je jeste v nejakem jinem seznamu se zapnutym listCheckingem
								// promenna list je totiz nastavena 
								// objekt musi byt odstranen z tohoto seznamu nejdriv
								// do budoucna jen nemazat objekt, nebo ho odstranit i ze seznamu se zapnutym listCheckingem


		le->DeleteListElementObject();
		delete le;
	}	
}

template <class T> void CGUIList<T>::DeleteFromBack(void)
{
	if(back->prev->object)
	{						// je to normalni prvek (nejedna se o hlavu nebo zadek)
		CGUIListElement<T> *le;
		
		le = back->prev;
		back->prev=le->prev;
		le->prev->next=back;

		if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
			assert(false);
		if(!listChecking && listElement->IsObjectInAnyList())
			assert(false);		// !@#$ object je jeste v nejakem jinem seznamu se zapnutym listCheckingem
								// promenna list je totiz nastavena 
								// objekt musi byt odstranen z tohoto seznamu nejdriv
								// do budoucna jen nemazat objekt, nebo ho odstranit i ze seznamu se zapnutym listCheckingem

		le->DeleteListElementObject();
		delete le;
	}	
}

template <class T> T *CGUIList<T>::GetNext(T* obj)
{
	CGUIListElement<T> *le=head;

	while(le->next && le->object!=obj)
	{
		le = le->next;
	}
	if(le->next)
		return le->next->object;
	else
		return 0;
}

template <class T> T *CGUIList<T>::GetPrev(T* obj)
{
	CGUIListElement<T> *le=back;

	while(le->prev && le->object!=obj)
	{
		le = le->prev;
	}
	if(le->prev)
		return le->prev->object;
	else
		return 0;
}

template <class T> T *CGUIList<T>::Remove(T* obj)
{
	if(obj)
	{
		CGUIListElement<T> *le=head;

		while(le->next && le->object!=obj)
		{
			le = le->next;
		}
		if(le->object==obj)
		{
			if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
				assert(false);
			if(listChecking)
				le->SetObjectList(0);	// vyrazeni ze seznamu

			le->prev->next=le->next;
			le->next->prev=le->prev;
			delete le;

			return obj;			
		}else
			return 0;
	}else
		return 0;
}

template <class T> void CGUIList<T>::Delete(T* obj)
{
	if(obj)
	{
		CGUIListElement<T> *le=head;

		while(le->next && le->object!=obj)
		{
			le = le->next;
		}
		if(le->object==obj)
		{
			if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
				assert(false);

			le->prev->next=le->next;
			le->next->prev=le->prev;

			if(!listChecking && le->IsObjectInAnyList())
				assert(false);		// !@#$ object je jeste v nejakem jinem seznamu se zapnutym listCheckingem
									// promenna list je totiz nastavena 
									// objekt musi byt odstranen z tohoto seznamu nejdriv
									// do budoucna jen nemazat objekt, nebo ho odstranit i ze seznamu se zapnutym listCheckingem

			le->DeleteListElementObject();
			delete le;
		}
	}
}

template <class T>	bool CGUIList<T>::IsMember(T* obj)
{
	if(obj)
	{
		CGUIListElement<T> *le=head;

		while(le->next && le->object!=obj)
		{
			le = le->next;
		}
		if(le->object==obj)
		{
			if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
				assert(false);
			return true;
		}else
			return false;
	}else
		return false;
}

template <class T, class P>	T *Get(CGUIList<T> *list, P prm)
{
	if(!list)
		return 0;
	CGUIListElement<T> *le=list->head;

	while(le->next && !(le->object && (*(le->object)==prm)))
	{
		le = le->next;
	}
	if(le->object && *(le->object)==prm)
	{
		if(list->listChecking && le->IsNotObjectInList(list))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
			assert(false);
		return le->object;
	}else
		return 0;
}


template <class T, class P>	bool IsMember(CGUIList<T> *list, P prm)
{
	if(!list)
		return false;

	CGUIListElement<T> *le=list->head;

	while(le->next && !(le->object && (*(le->object)==prm)))
	{
		le = le->next;
	}
	if(le->object && *(le->object)==prm)
	{
		if(list->listChecking && le->IsNotObjectInList(list))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
			assert(false);
		return true;
	}else
		return false;
}

template <class T, class P> T *Remove(CGUIList<T> *list, P prm)
{
	if(!list)
		return 0;
	CGUIListElement<T> *le=list->head;
	T* obj;

	while(le->next && !(le->object && (*(le->object)==prm)))
	{
		le = le->next;
	}
	if(le->object && *(le->object)==prm)
	{
		if(list->listChecking && le->IsNotObjectInList(list))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
			assert(false);
		if(list->listChecking)
			le->SetObjectList(0);	// vyrazeni ze seznamu
		obj = le->object;

		le->prev->next=le->next;
		le->next->prev=le->prev;
		delete le;

		return obj;			
	}else
		return 0;
}

template <class T, class P> void Delete(CGUIList<T> *list, P prm)
{
	if(!list)
		return;
	CGUIListElement<T> *le=list->head;

	while(le->next && !(le->object && (*(le->object)==prm)))
	{
		le = le->next;
	}
	if(le->object && *(le->object)==prm)
	{
		if(list->listChecking && le->IsNotObjectInList(list))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
			assert(false);

		le->prev->next=le->next;
		le->next->prev=le->prev;

		if(!list->listChecking && le->IsObjectInAnyList())
			assert(false);		// !@#$ object je jeste v nejakem jinem seznamu se zapnutym listCheckingem
								// promenna list je totiz nastavena 
								// objekt musi byt odstranen z tohoto seznamu nejdriv
								// do budoucna jen nemazat objekt, nebo ho odstranit i ze seznamu se zapnutym listCheckingem


		le->DeleteListElementObject();
		delete le;
	}
}



template <class T> CGUIListElement<T> *CGUIList<T>::GetNextListElement(CGUIListElement<T> *le)
{
	if(le)
	{
		return le->next;
	}else{
		return head->next;
	}
}

template <class T> CGUIListElement<T> *CGUIList<T>::GetPrevListElement(CGUIListElement<T> *le)
{
	if(le)
	{
		return le->prev;
	}else{
		return back->prev;
	}
}


template <class T> CGUIListElement<T> *CGUIList<T>::GetNextListElementObj(T *obj)
{
	if(obj)
	{
		CGUIListElement<T> *le=head;

		while(le->next && le->object!=obj)
		{
			le = le->next;
		}
		return le->next;
	}else{
		return head->next;
	}
}

template <class T> CGUIListElement<T> *CGUIList<T>::GetPrevListElementObj(T *obj)
{
	if(obj)
	{
		CGUIListElement<T> *le=back;

		while(le->prev && le->object!=obj)
		{
			le = le->prev;
		}
		return le->prev;
	}else{
		return back->prev;
	}
}


template <class T> T *CGUIList<T>::GetObjectFromListElement(CGUIListElement<T> *le)
{
	if(le)
		return le->object;
	else
		return 0;
}

template <class T> T *CGUIList<T>::RemoveListElement(CGUIListElement<T> *le)
{
	T* obj;
	if((obj=le->object))
	{
		if(listChecking && le->IsNotObjectInList(this))	// !@#$ prozatimni kontrola (objekt musi byt v tomto seznamu, jinak nastala chyba)
			assert(false);

		le->prev->next=le->next;
		le->next->prev=le->prev;
		
		if(listChecking)
			le->SetObjectList(0);	// vyrazeni ze seznamu
		delete le;				// maze listElement
		return obj;				// vraci objekt smazaneho ListElementu
	}else
		return 0;
}

#endif