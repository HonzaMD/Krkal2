/////////////////////////////////////////////////////////////////////////////
//
// ComString.h
//
// Interface CComString - trida pro praci se stringy v kompilovanych skriptech
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined COM_COMSTRING_H__INCLUDED

#define COM_COMSTRING_H__INCLUDED

const int csUnknown = 1;
const int csMaxStringLn = 250;

class CComString
{
public:
	CComString(int _ln, int line);
	CComString(const char* str, int line = csUnknown);
	CComString(int _ln, void* mem, int line = csUnknown);
	CComString(CComString& str);
	~CComString();

	void SafeDelete(int line);

	CComString& operator=(CComString str);
	char operator[](int n);

	CComString operator+(CComString& str);
	char* operator+(int i);
	operator char*() { return data; }

	int GetLn() const { return ln; }
	int lineContext;		// radka, na ktere byl string vytvoren

private:
	int ln;			// delka stringu
	char* data;		// data stringu
	bool owner;		// true, pokud se ma pointer data v destruktoru odalokovat
};

#endif