////////////////////////////////////////////////////////////////////////////////
//
//		GEnResource
//
//		resource, ktery se ukladaji do packagu
//		A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "genresource.h"

#include "genelement.h"
#include "lighttex.h"
#include "genanim.h"
#include "musicmod.h"

//vytvori instanci spravneho typu resourcu
CGEnResource* CGEnResource::CreateResource(eResourceType type)
{
	CGEnResource *res=NULL;

	switch( type ) 
	{
	case resElSource:
		res = new CGEnElSource(); //vytvori ElSource
		break;

	case resLightTex:
		res = new CLightTex(); //vytvori LightTex
		break;

	case resAnim:
		res = new CGEnAnim(); //vytvori Animaci
		break;

	case resTex:
		res = new CTexture(); //vytvori Texturu
		break;

	case resMusicModule:
		res = new CMusicModule(); //vytvori MusicModule
		break;

	case resMusicSample:
		res = new CMusicSample(); //vytvori MusicSample
		break;

	/*
	case res????:
		res = new ?????(); //vytvori ?????
		break;
	*/

	}

	return res;
}
