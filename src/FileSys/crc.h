/////////////////////////////////////////////////////////////////////////////
//
// crc.h
//
// Vypocet CRC32
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////

/*

pouziti:

a)	
	//Spocita CRC z retezce "123456789" (bez koncovy nuly):
	DWORD CRC = CCRC32("123456789",9) 

b)	//Spocita CRC z tri 32bit. integeru
	int a=1,b=2,c=3;

	CCRC32 crc;
	crc.Cmp(&a,4); //zalezi na poradi volani
	crc.Cmp(&b,4);
	crc.Cmp(&c,4);
	DWORD CRC=crc;

	//stejne CRC vyjde i takto:
	int a[3]={1,2,3}
	DWORD CRC=CCRC32(a,12);


*/

/////////////////////////////////////////////////////////////////////////////

#ifndef FSCRC_H
#define FSCRC_H


class CCRC32{
public:
	CCRC32(){CRC=0xFFFFFFFF;}
	CCRC32(const void *data, int size){CRC=0xFFFFFFFF;Cmp(data,size);}
	
	void Cmp(const void *data, int size);
	DWORD GetCRC(){return ~CRC;}

	operator DWORD(){return ~CRC;}

private:
	DWORD CRC;
};

#endif