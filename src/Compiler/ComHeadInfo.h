/////////////////////////////////////////////////////////////////////////////
//
// ComHeadInfo.h
//
// Struktura SComHeadInfo - popis hlavicky skriptu - vyuziva ji editor skriptu
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined COM_COMHEADINFO_H__INCLUDED

#define COM_COMHEADINFO_H__INCLUDED

struct SComHeadInfo
{
	~SComHeadInfo()
	{
		char* ifName;
		while(!importedFiles.IsEmpty())
		{
			ifName = importedFiles.RemoveHead();
			SAFE_DELETE_ARRAY(ifName);
		}
	}

	char version[32];					// verze souboru (cosi jako 0000_FFFF_0001_0001)
	char author[cMaxFileNameLn+1];		// autor skriptu
	char game[cMaxFileNameLn+1];		// nazev hry
	CListK<char*> importedFiles;			// seznam importovanych souboru - pouze z teto hlavicky
	int behindHeadIndex;				// index prvniho neprazdneho znaku za hlavickou
	int headLineCount;					// pocet radku hlavicky
};

#endif