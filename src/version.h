///////////////////////////////////////////////////////////////////////////////////
///
///		Defines version of System Krkal
///
///		A: M.D.
///
///////////////////////////////////////////////////////////////////////////////////


#include "KScripts/ks_version.h"


#define KRKAL_SUBVERSION "en"
#define KRKAL_MAINVERSION 2


#define QUOTE(x) #x
#define MAKE_KRKAL_VERSION_STRING(main,kernel) QUOTE(main.kernel)
#ifdef _DEBUG
	#define KRKAL_VERSION_STRING MAKE_KRKAL_VERSION_STRING(KRKAL_MAINVERSION, KS_VERSION) KRKAL_SUBVERSION " (debug)"
#else
#ifdef RELEASE_DEBUG
	#define KRKAL_VERSION_STRING MAKE_KRKAL_VERSION_STRING(KRKAL_MAINVERSION, KS_VERSION) KRKAL_SUBVERSION " (rdebug)"
#else
	#define KRKAL_VERSION_STRING MAKE_KRKAL_VERSION_STRING(KRKAL_MAINVERSION, KS_VERSION) KRKAL_SUBVERSION
#endif
#endif
#define KRKAL_NAME_AND_VERSION_STRING "KRKAL v" KRKAL_VERSION_STRING


