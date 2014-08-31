//////////////////////////////////////////////////////////////////////////
///
///		R E G I S T R Y   - Prace se strukturovanym souborem
///
///		A: M.D.
///
//////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "register.h"

// Velikosti polozek jednotlivych typu
int FSRTypeSizes[FSRNumTypes] = {1,4,8,8,1,1};

CFSRegHT *RegActualHT = 0;

char *FSRCommonHead = "KrkalReg";



///////////////////////////////////////////////////////////////////////////////////
///
///			K L I C		- CFSRegKey
///
///////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// Konstruktor Pro vytvoreni noveho klice
CFSRegKey::CFSRegKey(CFSRegister *reg,char Type, char *Name, CFSRegister *SubReg) {
	next = 0;
	if (reg->lastkey) reg->lastkey->next = this;
	else reg->keys = this;
	reg->lastkey = this;
	reg->NumKeys++;
	MyRegister = reg;
	type = Type;
	state = 3;
	size = FSRTypeSizes[type];
	size2 = 1;
	top = 0;
	data = new char[size];
	name = new char[strlen(Name)+1]; 
	strcpy(name,Name);
	pos=0;
	AddToHashTable();
	if (type==FSRTregister) {
		if (SubReg) { 
			subregister = SubReg;
			if (SubReg->RootInfo) {
				SAFE_DELETE_ARRAY(SubReg->RootInfo->Path);
				SAFE_DELETE_ARRAY(SubReg->RootInfo->Head);
			}
		} else subregister = new CFSRegister(this); 
	} else subregister=0;
}


////////////////////////////////////////////////////////////////////////
/// Konstruktor pro nahrani Klice z Dat (ze souboru)
CFSRegKey::CFSRegKey(CFSRegister *reg, unsigned int *datasource,char *buffer, int &POS, int vel) {
	int f=0;
	int p=(*datasource)&0x1fffffff;
	next = 0;
	if (reg->lastkey) reg->lastkey->next = this;
	else reg->keys = this;
	reg->lastkey = this;
	MyRegister = reg;
	name = buffer+POS;
	while(POS<vel&&name[f]) f++,POS++;
	if (POS>=vel||name[f]) reg->OpenError=0; 
	POS++;
	data = buffer+POS;
	type = (*datasource)>>29;
	size = p;
	if (size<0) reg->OpenError=0;
	POS+=size;
	size2 = size / FSRTypeSizes[type];
	top = size2;
	state = 0;
	pos=0;
	AddToHashTable();
	if (reg->OpenError==1) {
		if (type==FSRTregister) subregister = new CFSRegister(this); else subregister=0;
	} else subregister=0;
}


/////////////////////////////////////////////////////////////////////////////////
/// Destruktor
CFSRegKey::~CFSRegKey() {
	if (subregister) SAFE_DELETE(subregister);
	if (MyRegister->HashTable) {
		MyRegister->HashTable->NumKeys--;
		HFindAndDelete(&(MyRegister->HashTable->HT[MyRegister->HashTable->HashFunction(name,MyRegister)]),this);
	}
	if (state & 1) delete[] data;
	if (state & 2) delete[] name;
}


// najde jmeno key v seznamu p2 a odstrani ho
void CFSRegKey::HFindAndDelete(CFSRegKey **p2,CFSRegKey *key) { 		
	while (*p2 && *p2 != key) p2 = &(**p2).HTnext;
	if (*p2) {
		*p2=key->HTnext;
	}
}


/////////////////////////////////////////////////////////////////////////////
/// Funkce pro prejmenovani klice
void CFSRegKey::rename(char *Name) {
	if (MyRegister->HashTable) {
		MyRegister->HashTable->NumKeys--;
		HFindAndDelete(&(MyRegister->HashTable->HT[MyRegister->HashTable->HashFunction(name,MyRegister)]),this);
	}
	if (state & 2) delete[] name;
	name = new char[strlen(Name)+1]; 
	strcpy(name,Name);
	state |= 2;
	AddToHashTable();
}


/////////////////////////////////////////////////////////////////////////////
/// Zvetsi velikost pole s daty na dvojnasobek nebo na vel. Podle toho ce je vic
/// Vytvori se nove pole. Stare bude okopirovano a pripadne zruseno.
/// Vraci 0, kdyz by nove pole melo byt priliz velke
int CFSRegKey::resize(int vel) {
	int	sz2 = vel>size2*2 ? vel : size2*2;
	int sz = sz2*FSRTypeSizes[type];
	if (sz > FSRMAXREGKEYVEL) return 0;
	char *dt = new char[sz];
	memcpy(dt,data,size);
	if (state&1) delete[] data;
	state |= 1;
	data = dt;
	size = sz;
	size2 = sz2;
	return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// 
///  FUNKCE PRO ZAPIS A CTENI DAT Z POZICE pos
///
///////////////////////////////////////////////////////////////////////////////

void CFSRegKey::writec(char a) {
	assert(type==FSRTchar||type==FSRTstring);
	if (pos>=size2) if (!resize(pos+1)) return;
	((char*)data)[pos] = a;
	pos++;
	if (pos>top) top=pos;
}
void CFSRegKey::writei(int a) {
	assert(type==FSRTint);
	if (pos>=size2) if (!resize(pos+1)) return;
	if (pos>top) top=pos;
	((int*)data)[pos] = a;
	pos++;
	if (pos>top) top=pos;
}
void CFSRegKey::writed(double a) {
	assert(type==FSRTdouble);
	if (pos>=size2) if (!resize(pos+1)) return;
	if (pos>top) top=pos;
	((double*)data)[pos] = a;
	pos++;
	if (pos>top) top=pos;
}
void CFSRegKey::write64(_int64 a) {
	assert(type==FSRTint64);
	if (pos>=size2) if (!resize(pos+1)) return;
	if (pos>top) top=pos;
	((_int64*)data)[pos] = a;
	pos++;
	if (pos>top) top=pos;
}

/////////////////////////////////////////////////////////////////////////////////
///		CTENI
char CFSRegKey::readc() {
	assert(type==FSRTchar||type==FSRTstring);
	if (pos>=top||pos>=size2) return 0;
	pos++;
	return ((char*)data)[pos-1];
}
int CFSRegKey::readi() {
	assert(type==FSRTint);
	if (pos>=top||pos>=size2) return 0;
	pos++;
	return ((int*)data)[pos-1];
}
double CFSRegKey::readd() {
	assert(type==FSRTdouble);
	if (pos>=top||pos>=size2) return 0;
	pos++;
	return ((double*)data)[pos-1];
}
_int64 CFSRegKey::read64() {
	assert(type==FSRTint64);
	if (pos>=top||pos>=size2) return 0;
	pos++;
	return ((_int64*)data)[pos-1];
}


///////////////////////////////////////////////////////////////////////////////
/// FUNKCE BLOKOVEHO ZAPISU A CTENI
void CFSRegKey::blockwrite(const void *source, int Size) {
	if (pos+Size>size2) if (!resize(pos+Size)) return;
	memcpy(data+pos*FSRTypeSizes[type],source,Size*FSRTypeSizes[type]);
	pos+=Size;
	if (pos>top) top=pos;
}
int CFSRegKey::blockread(void *dest, int Size) {
	int b=Size<top-pos?Size:top-pos;
	if (pos<size2) memcpy(dest,data+pos*FSRTypeSizes[type],b<size2-pos?b*FSRTypeSizes[type]:(size2-pos)*FSRTypeSizes[type]);
	pos+=b;
	return b;
}

char * CFSRegKey::stringread() {
	assert(type==FSRTchar||type==FSRTstring);
	char *string;
	int oldpos=pos;
	int f;
	if (pos>=top) return 0;
	while(pos<size2&&data[pos]) pos++;
	string = new char[pos-oldpos+1];
	for(f=oldpos;f<pos;f++) string[f-oldpos] = data[f];
	string[pos-oldpos]=0;
	pos++;
	return string;
}


/////////////////////////////////////////////////////////////////////////////////
/// Vyhleda prvni nulovou polozku a za ni nastavi pos. Nebo na konec dat.
void CFSRegKey::SetPosToNextString() {
	if (pos>=top) return;
	if (pos>=size2) {pos=top; return;}
	switch (type) {
		case FSRTchar:
		case FSRTstring:
		case FSRTregister:
			while (pos<size2&&pos<top&&data[pos]) pos++;
			break;
		case FSRTint:
			while (pos<size2&&pos<top&&((int*)data)[pos]) pos++;
			break;
		case FSRTdouble:
			while (pos<size2&&pos<top&&((double*)data)[pos]) pos++;
			break;
		case FSRTint64:
			while (pos<size2&&pos<top&&((_int64*)data)[pos]) pos++;
	}
	if (pos>=top||pos>=size2) pos=top; else pos++;
}




///////////////////////////////////////////////////////////////////////////////
///		Primy Pristup - Celkova zmena vsech dat
void CFSRegKey::SetAllData(char *Data, int Size) { // Data v klici budou zahozena a nahrazena temito daty
	size = Size;
	if (state & 1) delete[] data;
	data = Data;
	size2 = size / FSRTypeSizes[type];
	top = size2;
	pos = 0;
}



////////////////////////////////////////////////////////////////////////////////
///		
///   Metody Pro Ulozeni Registru
///
////////////////////////////////////////////////////////////////////////////////

void CFSRegKey::zjistivelikostazmenu(int &vel, int &changed) {
	if (state&3) changed=1;
	if (type==FSRTregister) {
		subregister->zjistivelikostazmenu(vel,changed);
		vel+=4+strlen(name)+1;
	} else {
		vel+=4+top*FSRTypeSizes[type]+strlen(name)+1;
		if (top!=size2) changed=1;
	}
}


//////////////////////////////////////////////////////////////////////////////
/// Posbira data klice a ulozi je
void CFSRegKey::collectall(char *buff, int &pos, int &tablepos) {
	int tpos2,pos2;
	char *bf2;
	unsigned int *buff2 = (unsigned int*)(buff+tablepos);
	tablepos+=4; // tabulku vyplnuju pozpatku
	strcpy(buff+pos,name);  
	if (state & 2) SAFE_DELETE_ARRAY(name);
	name=buff+pos;
	state &= (255-2);
	pos+=strlen(name)+1;
	if (type==FSRTregister) {  // ulozeni dat pro typ register:
		bf2=buff+pos;
		pos2=tpos2=0;
		subregister->collectall(bf2,pos2,tpos2);
		size=pos2;
	} else {   // ulozeni dat pro ostatni typy:
		memcpy(buff+pos,data,size2<top?size:top*FSRTypeSizes[type]);
		size=top*FSRTypeSizes[type];
	}
	if (state & 1) SAFE_DELETE_ARRAY(data); // nastaveni na novou datovou oblast:
	buff2[0]=size+(type<<29);	// Ulozeni jmena:
	state &= (255-1);
	data = buff+pos;
	pos+=size;
	size2=size/FSRTypeSizes[type];
}







////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
///
///			R E G I S T E R		- CFSRegister
///
/////////////////////////////////////////////////////////////////////////////////

///   Metody Pro Ulozeni Registru
void CFSRegister::zjistivelikostazmenu(int &vel, int &changed) {
	CFSRegKey *p=keys;
	vel+=2;
	if (RegChanged) changed=1;
	while(p) {
		p->zjistivelikostazmenu(vel,changed);
		p=p->next;
	}
}


//////////////////////////////////////////////////////////////////////////////
/// Posbira data vsech klicu a ulozi je
void CFSRegister::collectall(char *buff, int &pos, int &tablepos) {
	CFSRegKey *p=keys;
	*((short*)(buff+tablepos)) = NumKeys; 
	pos+=2+NumKeys*4;
	tablepos+=2;
	while(p) {
		p->collectall(buff,pos,tablepos);
		p=p->next;
	}
}



////////////////////////////////////////////////////////////////////////////////
/// Ulozeni registru na Disk
/// Pokud je to vnoreny egistr, tak dojde jen k setreseni jeho dat do jednoho bufferu
int CFSRegister::WriteFile() {
	if (!RootInfo) return 0;
	int vel=0;
	int changed=RegChanged;
	int pos=0;
	int tablepos;
	int headvel=0;
	int headvel2=0;
	char *buff;
	if (RootInfo->Head&&RootInfo->Path) {
		headvel2 = strlen(FSRCommonHead)+1;
		vel=headvel=strlen(RootInfo->Head)+1+headvel2;  
	}
		// zjisteni velikosti a jestli doslo ke zmenam:
	zjistivelikostazmenu(vel,changed); 
	if (changed) {
		RegChanged=1;
		buff = new char[vel]; // tvorim novy buffer, do ktereho data setresu
		if (RootInfo->Head&&RootInfo->Path) {
			strcpy(buff,FSRCommonHead);
			strcpy(buff+headvel2,RootInfo->Head); // ulozeni hlavicky
			pos=headvel;
		}
		tablepos=pos;
		collectall(buff,pos,tablepos); // setreseni
		if (RootInfo->deletebuff) SAFE_DELETE_ARRAY(RootInfo->buffer); //zmazani stareho bufferu a vymena bufferu:
		RootInfo->buffer=buff;
		RootInfo->deletebuff=1;
	}
	if (RootInfo->Path) {
		RegChanged = 0;
		return FS->WriteFile(RootInfo->Path,RootInfo->buffer,vel,RootInfo->CompressMode); //zapis na disk
	}
	return 0;
}




/////////////////////////////////////////////////////////
/// precte ze souboru hlavicku a overi zda sedi. Vraci OpenError.
int CFSRegister::VerifyRegisterFile(const char *path, char *head) {
	int vel = strlen(FSRCommonHead)+1;
	if (head) vel += strlen(head)+1;
	char *buffer = 0;
	int f,g;
	int OpenError;

	buffer = new char[vel];
	if (FS->ReadFile(path,buffer,vel)) {
		OpenError=FSREGOK;
		f=0;
		while (f<vel && buffer[f]==FSRCommonHead[f] && FSRCommonHead[f]) f++;
		if (FSRCommonHead[f]) OpenError=FSREGCONTENTERROR; /*Hlavicka nesouhlasi*/ 
		else if (head) { 
			f++;
			g=0;
			while (f<vel && buffer[f]==head[g] && head[g]) f++, g++;
			if (head[g]) OpenError=FSREGCONTENTERROR; /*Hlavicka nesouhlasi*/ 
		}
	} else OpenError = FSREGFILEERROR;
	SAFE_DELETE_ARRAY(buffer);
	return OpenError;
}


//////////////////////////////////////////////////
// zmeni cestu k souboru (save as .. nekam jinam)
int CFSRegister::ChangePath(const char *path) {
	char *p;
	if (!RootInfo) return 0;
	if (FS->GetFullPath(path,&p)) {
		SAFE_DELETE_ARRAY(RootInfo->Path);
		RootInfo->Path = p;
		return 1;
	}
	return 0;
}



////////////////////////////////////////////////////////////////////////////////
///		 KONSTRUKTOR  pro nahrani registru ze souboru
CFSRegister::CFSRegister(const char *path, char *head, int ClearIt) {
	int f,g,aa;
	int vel;
	if (ClearIt) vel = 0; else vel = FS->GetFileSize(path);
	RootInfo = new CFSRootRegInfo();
	keys=0;
	lastkey=0;
	NumKeys = 0;
	HashTable = 0;
	FS->GetFullPath(path,&RootInfo->Path);
	if (head) {
		RootInfo->Head = new char[strlen(head)+1];
		strcpy(RootInfo->Head,head);
	} 
	

	if (vel>0 && !ClearIt) {
		RootInfo->buffer = new char[vel];
		if (FS->ReadFile(path,RootInfo->buffer,vel)) {
			OpenError=FSREGOK;
			f=0;
			while (f<vel && RootInfo->buffer[f]==FSRCommonHead[f] && FSRCommonHead[f]) f++;
			if (FSRCommonHead[f]) OpenError=FSREGCONTENTERROR; /*Hlavicka nesouhlasi*/ 
			else { 
				f++;
				if (head) {
					g=0;
					while (f<vel && RootInfo->buffer[f]==head[g] && head[g]) f++, g++;
					if (head[g]) OpenError=FSREGCONTENTERROR; /*Hlavicka nesouhlasi*/ 
					f++;
				} else {
					g = f;
					while (f<vel && f<FSMAXHEADSIZE && RootInfo->buffer[f]) f++;
					if (f>=vel || f>=FSMAXHEADSIZE) OpenError=FSREGCONTENTERROR; /*Hlavicka nenactena spravne*/ 
					else {
						RootInfo->Head = new char[f-g+1];	strcpy(RootInfo->Head,RootInfo->buffer+g);
					}
					f++;
				}
			}
			if (OpenError==FSREGOK) {
				NumKeys = *((short*)(RootInfo->buffer+f)); f+=2;
				if (vel<f+NumKeys*4) OpenError=FSREGCONTENTERROR; /*neni tu cala tabulka klicu*/ else {
					g=0; aa=f+NumKeys*4;
					while(OpenError&&g<NumKeys) {
						new CFSRegKey(this,(unsigned int*)(RootInfo->buffer+f),RootInfo->buffer,aa,vel); // load klicu
						f+=4;g++;
					}
				}
			}
		} else OpenError = FSREGFILEERROR;
	} else OpenError = FSREGFILEERROR;
	if (OpenError==FSREGOK) {
		RootInfo->deletebuff=1; 
		RegChanged=0;
	}else {
		RegChanged=1; // Vytvoreni prazdneho registru
		DeleteAllKeys();
		NumKeys=0;
		RootInfo->deletebuff=0;
		SAFE_DELETE_ARRAY(RootInfo->buffer);
		vel=0;
	}
	if (ClearIt) OpenError = FSREGOK;
	if (!RootInfo->Head) {
		RootInfo->Head = new char[7]; 	strcpy(RootInfo->Head,"Error!");
	}
}



////////////////////////////////////////////////////////////////////////////////
///		 KONSTRUKTOR  pro vytvoreni prazdneho subregistru
CFSRegister::CFSRegister() {
	keys=0;
	lastkey=0;
	NumKeys = 0;
	RootInfo = 0;
	HashTable = 0;
	RegChanged=1; // Vytvoreni prazdneho registru
	NumKeys=0;
}



////////////////////////////////////////////////////////////////////////////////
///		 KONSTRUKTOR  pro nahrani registru z klice
CFSRegister::CFSRegister(CFSRegKey *key) {
	int f,g,aa;
	int vel = key->size;
	keys=0;
	RootInfo=0;
	lastkey=0;
	NumKeys = 0;
	HashTable = 0;
	key->subregister=this;
	
	OpenError=0;
	if (key->top>4) {
		OpenError=1;
		NumKeys = *((short*)(key->data)); f=2;
		if (key->top<f+NumKeys*4||key->size2<f+NumKeys*4) OpenError=0; /*neni tu cala tabulka klicu*/
		else {
			g=0; aa=f+NumKeys*4;
			while(OpenError&&g<NumKeys) {
				new CFSRegKey(this,(unsigned int*)(key->data+f),key->data,aa,vel); // load klicu
				f+=4;g++;
			}
		}
		if (!OpenError) {
			DeleteAllKeys(); // vytvoreni prazdneho registru
			NumKeys=0;
			vel=0;
		}
	} else OpenError = 2;
	if (OpenError==1) RegChanged=0; else RegChanged=1;
}




// Registr se zrusi, zmeny se neulozi
CFSRegister::~CFSRegister() { 
	DeleteAllKeys();
	if (HashTable) HashTable->Release();
	if (RootInfo) {
		if (RootInfo->deletebuff) SAFE_DELETE_ARRAY(RootInfo->buffer);
		SAFE_DELETE_ARRAY(RootInfo->Path);
		SAFE_DELETE_ARRAY(RootInfo->Head);
		SAFE_DELETE(RootInfo);
	}
}


///////////////////////////////////////////////////////////////////////////////
/// Smaze vsechny klice
void CFSRegister::DeleteAllKeys() {
	CFSRegKey *p1, *p2;
	p1=keys;
	while (p1) {
		p2=p1;
		p1=p1->next;
		delete p2;
	}
	keys=0;
	lastkey=0;
	NumKeys=0;
	RegChanged=1;
}


///////////////////////////////////////////////////////////////////////////////
///  Nastavi u vsech klicu v registru a podregistrech seek na 0
void CFSRegister::SeekAllTo0() {
	CFSRegKey *key = keys;
	while (key) {
		if (key->type==FSRTregister) key->GetSubRegister()->SeekAllTo0();
		else key->pos = 0;
		key = key->next;
	}
}


///////////////////////////////////////////////////////////////////////////////
/// Smaze konkretni klic
void CFSRegister::DeleteKey(CFSRegKey *key) {
	CFSRegKey *p1=keys, *p2=0;
	while (p1 && p1!=key) { 
		p2 = p1; p1 = p1->next;
	}
	if (p1) {
		if (p2) p2->next = p1->next; else keys = p1->next;
		if (p1==lastkey) lastkey = p2;
		delete p1;
		NumKeys--;
		RegChanged=1;
	}
}



///////////////////////////////////////////////////
// postupnym prohledavanim zjisti pozici klice. vrati pozici nebo -1 v pripade neuspechu
int CFSRegister::FindKeyPos(CFSRegKey *key) {
	CFSRegKey *k = GetFirstKey();
	int count = 0;
	while (k) {
		if (k==key) return count;
		count++;
		k=k->GetNextKey();
	}
	return -1;
}







//////////////////////////////////////////////////////////////
////
////		H A S H   T A B L E
////
//////////////////////////////////////////////////////////////


// Hashovaci funkce ze stringu
int CFSRegHT::HashFunction(const char *name, CFSRegister *r) {
	int f=0,a=(int)r;
	while (name[f]) {
		a+=name[f];
		f++;
	}
	return a % KER_REGISTER_HTS ;
}




// Vyhleda nasledujici vyskyt, nebo prvni vyskyt, pokud je PrevKey ponechan na 0
CFSRegKey *CFSRegHT::Member(const char *name, CFSRegister *r,CFSRegKey *PrevKey) {
	CFSRegKey *p;
	if (PrevKey) p = PrevKey->HTnext;
	else p = HT[HashFunction(name,r)];
	
	int f;
	while (p) {
		if (p->MyRegister == r) {
			f=0;
			while(name[f]==p->name[f]&&name[f]) f++; // porovnani stringu
			if (name[f]==p->name[f]) return p;
		}
		p=p->HTnext;
	}
	return 0;
}


//////////////////////////////////////////////////////

void CFSRegister::CreateHT() {
	if (RegActualHT && RegActualHT->NumKeys + NumKeys < KER_REGISTER_HTS) {
		HashTable = RegActualHT;
	} else {
		RegActualHT = HashTable = new CFSRegHT();
	}
	RegActualHT->NumRefs++;

	CFSRegKey *k = keys;
	while (k) {
		k->AddToHashTable();
		k = k->next;
	}
}


// Pridej do hashTable
void CFSRegKey::AddToHashTable() {
	HTnext = 0;
	if (!MyRegister->HashTable) return;
	CFSRegHT *ht = MyRegister->HashTable;
	ht->NumKeys++;
	CFSRegKey **list = ht->HT + ht->HashFunction(name,MyRegister);
	while (*list) list = &(**list).HTnext; // umistuju na konec
	*list = this;
}
