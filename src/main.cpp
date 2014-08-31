/////////////////////////////////////////////////////////////////////////////
//
// KRKAL.cpp
//
// KRKAL main
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dx.h"

#include <crtdbg.h>
#include <time.h>

#include "krkal.h"


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

    #ifdef _DEBUG
//		#define new DEBUG_NEW
		_CrtMemState s1,s2,s3;
		_CrtMemCheckpoint( &s1 );
	#endif

		
	//srand( (unsigned)time( NULL ) );

	setlocale( LC_CTYPE, "" ); 
	setlocale( LC_COLLATE, "" );

	KRKAL=new CKrkal;

	if(KRKAL->Init(hInstance))
	{
		KRKAL->SetTimes(33,33);
		KRKAL->SetTimeOuts(300,600);

		KRKAL->Run();
	}

	delete KRKAL;


	#ifdef _DEBUG
		_CrtMemCheckpoint( &s2 );

		if ( _CrtMemDifference( &s3, &s1, &s2 ) )
		{
//			_CrtDumpMemoryLeaks();
			MessageBox( NULL, "Detected memory leak!", "KRKAL", MB_ICONERROR|MB_OK );
		}
	#endif

	return 1;

}

