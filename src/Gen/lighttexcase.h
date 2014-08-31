//vypocet nasvicene textury
//
//vetveni podle:
//	ISALPHA - v texture je alpha
//  ISAMBIENT - v texture je ambient
//	ISZDEPTH - v texture je zdepth
//  ISSHADOW - na texturu je vrzen stin
//	ISPOINTLIGHT - na texturu sviti bodove svetlo
//	ISNORMAL - v texture je normala
//  ISATMOSPHERE - v texture je atmosphere

if(LightComponets&LgC_ATMOSPHERE)
{
	#define ISATMOSPHERE
	if(isnormal)
	{
		#define ISNORMAL
		if(isalpha)
		{
			#define ISALPHA
			if(isambient){
				#define ISAMBIENT
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
				#undef ISAMBIENT
			}else{ //ambient
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
			}
			#undef ISALPHA
		}else{ //alpha
			if(isambient){
				#define ISAMBIENT
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
				#undef ISAMBIENT
			}else{ //ambient
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
			}
		}
		#undef ISNORMAL
	}else{ //normal
		if(isalpha)
		{
			#define ISALPHA
			if(isambient){
				#define ISAMBIENT
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
				#undef ISAMBIENT
			}else{ //ambient
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
			}
			#undef ISALPHA
		}else{ //alpha
			if(isambient){
				#define ISAMBIENT
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
				#undef ISAMBIENT
			}else{ //ambient
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
			}
		}
	}
	#undef ISATMOSPHERE
}else{ //atmosphere
	if(isnormal)
	{
		#define ISNORMAL
		if(isalpha)
		{
			#define ISALPHA
			if(isambient){
				#define ISAMBIENT
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
				#undef ISAMBIENT
			}else{ //ambient
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
			}
			#undef ISALPHA
		}else{ //alpha
			if(isambient){
				#define ISAMBIENT
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
				#undef ISAMBIENT
			}else{ //ambient
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
			}
		}
		#undef ISNORMAL
	}else{ //normal
		if(isalpha)
		{
			#define ISALPHA
			if(isambient){
				#define ISAMBIENT
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
				#undef ISAMBIENT
			}else{ //ambient
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
			}
			#undef ISALPHA
		}else{ //alpha
			if(isambient){
				#define ISAMBIENT
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
				#undef ISAMBIENT
			}else{ //ambient
				if(iszdepth){
					#define ISZDEPTH
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
					#undef ISZDEPTH
				}else{ //zdepth
					if(numshadows){
						#define ISSHADOW
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
						#undef ISSHADOW
					}else{ //shadow
						if(numlight){
							#define ISPOINTLIGHT
							#include "lighttexcmp.h"
							#undef ISPOINTLIGHT
						}else{ //pointlight
							#include "lighttexcmp.h"
						}
					}
				}
			}
		}
	}
}
