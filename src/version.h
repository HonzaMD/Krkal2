///////////////////////////////////////////////////////////////////////////////////
///
///		Defines version of System Krkal
///
///		A: M.D.
///
///////////////////////////////////////////////////////////////////////////////////


#include "KScripts/ks_version.h"


#define KRKAL_MAINVERSION 2


#define QUOTE(x) #x
#define MAKE_KRKAL_VERSION_STRING(main,kernel) QUOTE(main.kernel)
#ifdef _DEBUG
	#define KRKAL_VERSION_STRING MAKE_KRKAL_VERSION_STRING(KRKAL_MAINVERSION, KS_VERSION) " (debug)"
#else
#ifdef RELEASE_DEBUG
	#define KRKAL_VERSION_STRING MAKE_KRKAL_VERSION_STRING(KRKAL_MAINVERSION, KS_VERSION) " (rdebug)"
#else
	#define KRKAL_VERSION_STRING MAKE_KRKAL_VERSION_STRING(KRKAL_MAINVERSION, KS_VERSION) ".1"
#endif
#endif
#define KRKAL_NAME_AND_VERSION_STRING "KRKAL v" KRKAL_VERSION_STRING


