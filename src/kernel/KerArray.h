///////////////////////////////////////////////////////////////////////////////////////////////
//
//		K e r n e l A r r a y
//
//		dynamicke pole hodnot. 
//		pouziva se ve skriptech a muze existovat jen tehdy pokud bezi Kernel
//		A: M.D.
//
///////////////////////////////////////////////////////////////////////////////////////////////

#ifndef KERARRAYS_H
#define KERARRAYS_H


#include "kernel.h"
#include "interpret.h"



#define CKERNELARRAY(_classname,_type,_GCname,_vtag) \
class _classname {\
	friend CKerGarbageCollector;\
public:\
	_classname() {\
		ValidTag = _vtag;\
		array = 0; size = 0; count = 0;\
		next = KerMain->GarbageCollector._GCname; \
		if (next) next->prev = this; \
		KerMain->GarbageCollector._GCname = this; \
		prev = 0; \
	} \
	~_classname() {  \
		if (this==0 || ValidTag != _vtag) { KerMain->Errors->LogError(eKRTEdeletingBadArray,(int)this); return; } \
		ValidTag = 0; \
		if (array&&KerInterpret) KerInterpret->ISFree(array); \
		if (!prev) KerMain->GarbageCollector._GCname = next; \
		else prev->next = next; \
		if (next) next->prev = prev; \
	} \
\
	int CheckIntegrity() { \
		if (this==0 || ValidTag != _vtag) return 0; \
		return 1; \
	} \
\
	int GetCount() const { \
		if (this==0 || ValidTag != _vtag) { KerMain->Errors->LogError(eKRTEaccessingNEarray,(int)this); return 0; } \
		return count; \
	} \
	void SetCount(int _count) { \
		if (this==0 || ValidTag != _vtag) { KerMain->Errors->LogError(eKRTEaccessingNEarray,(int)this); return; } \
		if (_count<0) _count=0; \
		count = _count; \
		if (count>size) Expand(count); \
	} \
\
	_type & operator[](int index) { \
		if (this==0 || ValidTag != _vtag) { KerMain->Errors->LogError(eKRTEaccessingNEarray,(int)this); *KRValue##_classname=0; return *KRValue##_classname; } \
		if (index<0) { \
			KerMain->Errors->LogError(eKRTEarrayAccErr,index); \
			*KRValue##_classname=0; return *KRValue##_classname;\
		} else { \
			if (index>count) KerMain->Errors->LogError(eKRTEarrAddedNDef); \
			if (index>=size) Expand(index+1); \
			if (index>=count) count = index+1; \
			return array[index]; \
		} \
	} \
	_type &	Get(int index) { \
		if (this==0 || ValidTag != _vtag) { KerMain->Errors->LogError(eKRTEaccessingNEarray,(int)this); *KRValue##_classname=0; return *KRValue##_classname; } \
		if (index<0) { \
			KerMain->Errors->LogError(eKRTEarrayAccErr,index); \
			*KRValue##_classname=0; return *KRValue##_classname;\
		} else { \
			if (index>count) KerMain->Errors->LogError(eKRTEarrAddedNDef); \
			if (index>=size) Expand(index+1); \
			if (index>=count) count = index+1; \
			return array[index]; \
		} \
	} \
\
	_type &	Read(int index) { \
		if (this==0 || ValidTag != _vtag) { KerMain->Errors->LogError(eKRTEaccessingNEarray,(int)this); *KRValue##_classname=0; return *KRValue##_classname; } \
		if (index<0||index>=count) { \
			KerMain->Errors->LogError(eKRTEarrayAccErr,index); \
			*KRValue##_classname=0; return *KRValue##_classname;\
		} else { \
			return array[index]; \
		} \
	} \
\
	int Add(_type item) \
	{ \
		if (this==0 || ValidTag != _vtag) { KerMain->Errors->LogError(eKRTEaccessingNEarray,(int)this); return 0; } \
		if(GetCount() == size) \
			Expand(size+1); \
		array[count] = item; \
		return count++; \
	} \
\
private: \
	_type* array; \
	int count; \
	int size; \
	UI ValidTag;\
	_classname *next, *prev; \
\
	void Expand(int _size) \
	{ \
		if (_size<8) _size=8; \
		int newSize = 2*size>_size?2*size:_size; \
		_type* newArray = KER_NEW2(_type,newSize); \
		if (array) { \
			for(int i=0; i<GetCount(); i++) \
				newArray[i] = array[i]; \
			KerInterpret->ISFree(array); \
		} \
		array = newArray; \
		size = newSize; \
	} \
};


extern char* KRValueCKerArrChar;
extern double* KRValueCKerArrDouble;
extern int* KRValueCKerArrInt;
extern void** KRValueCKerArrPointer;
extern OPointer* KRValueCKerArrObject;
extern CKerName** KRValueCKerArrName;

CKERNELARRAY(CKerArrChar, char,GCArrChar,0xcf5c1370)
CKERNELARRAY(CKerArrDouble, double,GCArrDouble,0xcf5c1371)
CKERNELARRAY(CKerArrInt, int,GCArrInt,0xcf5c1372)
CKERNELARRAY(CKerArrPointer, void*,GCArrPointer,0xcf5c1373)
CKERNELARRAY(CKerArrObject, OPointer,GCArrObject,0xcf5c1374)
CKERNELARRAY(CKerArrName, CKerName*,GCArrName,0xcf5c1375)


#endif
