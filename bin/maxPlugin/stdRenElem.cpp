////////////////////////////////////////////////////////////////////////
//
//	Standard Render Elements 	
//
//	Created: Kells Elmquist, 24, June 2000
//
//	Copyright (c) 2000, All Rights Reserved.
//

// maxsdk includes
#include <iparamm2.h>

// local includes
#include "renElemPch.h"
#include "stdRenElems.h"

Class_ID specularRenderElementClassID( SPECULAR_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID specularSelectRenderElementClassID( SPECULAR_SELECT_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID specularCompRenderElementClassID( SPECULAR_COMP_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID diffuseRenderElementClassID( DIFFUSE_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID emissionRenderElementClassID( EMISSION_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID reflectionRenderElementClassID( REFLECTION_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID refractionRenderElementClassID( REFRACTION_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID shadowRenderElementClassID( SHADOWS_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID atmosphereRenderElementClassID( ATMOSPHERE_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID atmosphere2RenderElementClassID( ATMOSPHERE2_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID blendRenderElementClassID( BLEND_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID ZRenderElementClassID( Z_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID alphaRenderElementClassID( ALPHA_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID clrShadowRenderElementClassID( CLR_SHADOW_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID bgndRenderElementClassID( BGND_RENDER_ELEMENT_CLASS_ID , 0);
Class_ID normalRenderElementClassID( NORMAL_RENDER_ELEMENT_CLASS_ID, 0);
Class_ID ambientRenderElementClassID( AMBIENT_RENDER_ELEMENT_CLASS_ID, 0);

////////////////////////////////////
// color utility
inline void ClampMax( Color& c ){
	if( c.r > 1.0f ) c.r = 1.0f;
	if( c.g > 1.0f ) c.g = 1.0f;
	if( c.b > 1.0f ) c.b = 1.0f;
}

inline void ClampMax( AColor& c ){
	if( c.r > 1.0f ) c.r = 1.0f;
	if( c.g > 1.0f ) c.g = 1.0f;
	if( c.b > 1.0f ) c.b = 1.0f;
	if( c.a > 1.0f ) c.a = 1.0f;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Diffuse render element
//
class DiffuseRenderElement : public BaseRenderElement {
public:
		DiffuseRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return diffuseRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_DIFFUSE_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// Diffuse element class descriptor
// --------------------------------------------------
class DiffuseElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new DiffuseRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_DIFFUSE_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return diffuseRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("diffuseRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static DiffuseElementClassDesc diffuseCD;
ClassDesc* GetDiffuseElementDesc() { return &diffuseCD; }


// ------------------------------------------------------
// Diffuse parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 diffuseParamBlk(PBLOCK_NUMBER, _T("Diffuse render element parameters"), 0, &diffuseCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Diffuse Render Element ------------------------------------------------
DiffuseRenderElement::DiffuseRenderElement()
{
	diffuseCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle DiffuseRenderElement::Clone( RemapDir &remap )
{
	DiffuseRenderElement*	newEle = new DiffuseRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void DiffuseRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		//sc.out.elementVals[ mShadeOutputIndex ] = ip.finalAttenuation * (ip.diffIllumOut+ip.ambIllumOut);

		/*Point3 pp = sc.Normal();
		Point3 tp = sc.globContext->camToWorld.VectorTransform(pp);
*/
		int cc=-1;
		
		if(ip.pShader) cc=ip.pShader->StdIDToChannel(ID_DI);

		if(cc>=0)
			sc.out.elementVals[ mShadeOutputIndex ] = ip.channels[cc];
		else
			sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Specular render element
//
class SpecularRenderElement : public BaseRenderElement {
public:
		SpecularRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return specularRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SPECULAR_RENDER_ELEMENT ); }

		BOOL AtmosphereApplied() const{ return TRUE; } //override, always on

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void SpecularRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class SpecularElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SpecularRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SPECULAR_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return specularRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("specularRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static SpecularElementClassDesc specularCD;
ClassDesc* GetSpecularElementDesc() { return &specularCD; }


// ---------------------------------------------
// Specular parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 specularParamBlk(PBLOCK_NUMBER, _T("Specular render element parameters"), 0, &specularCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Specular Render Element ------------------------------------------------
SpecularRenderElement::SpecularRenderElement()
{
	specularCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle SpecularRenderElement::Clone( RemapDir &remap )
{
	SpecularRenderElement*	newEle = new SpecularRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void SpecularRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
/*		Color c = ip.specIllumOut;
		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;
//		sc.out.elementVals[ mShadeOutputIndex ].a = Intens( ip.specIllumOut );
		sc.out.elementVals[ mShadeOutputIndex ].a = 0;*/
		
		int cc=-1;
		
		if(ip.pShader) cc=ip.pShader->StdIDToChannel(ID_SP);

		if(cc>=0)
			sc.out.elementVals[ mShadeOutputIndex ] = ip.channels[cc];
		else
			sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;


	}
}

void SpecularRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	// additive xparency can wipe out the a, so fill it in
	sc.out.elementVals[ mShadeOutputIndex ].a = Intens( sc.out.elementVals[ mShadeOutputIndex ] );
}

//-----------------------------------------------------------------------------

/*************
///////////////////////////////////////////////////////////////////////////////
//
//	Composited Specular render element
//
class SpecularCompRenderElement : public BaseRenderElement {
public:
		SpecularCompRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return specularCompRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SPECULAR_COMP_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class SpecularCompElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SpecularCompRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SPECULAR_COMP_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return specularCompRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("specularCompRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static SpecularCompElementClassDesc specularCompCD;
ClassDesc* GetSpecularCompElementDesc() { return &specularCompCD; }


// ---------------------------------------------
// SpecularComp parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 specularCompParamBlk(PBLOCK_NUMBER, _T("SpecularComp render element parameters"), 0, &specularCompCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Specular Comp Render Element ------------------------------------------------
SpecularCompRenderElement::SpecularCompRenderElement()
{
	specularCompCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle SpecularCompRenderElement::Clone( RemapDir &remap )
{
	SpecularCompRenderElement*	newEle = new SpecularCompRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void SpecularCompRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		sc.out.elementVals[ mShadeOutputIndex ] = ip.specIllumOut;
		sc.out.elementVals[ mShadeOutputIndex ].a = Intens( ip.specIllumOut );
	}
}


//-----------------------------------------------------------------------------
***********/

///////////////////////////////////////////////////////////////////////////////
//
//	Emission render element
//
class EmissionRenderElement : public BaseRenderElement {
public:
		EmissionRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return emissionRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_EMISSION_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class EmissionElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new EmissionRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_EMISSION_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return emissionRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("emissionRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static EmissionElementClassDesc emissionCD;
ClassDesc* GetEmissionElementDesc() { return &emissionCD; }


// ---------------------------------------------
// Emission parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 emissionParamBlk(PBLOCK_NUMBER, _T("Emission render element parameters"), 0, &emissionCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Emission Render Element ------------------------------------------------
EmissionRenderElement::EmissionRenderElement()
{
	emissionCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle EmissionRenderElement::Clone( RemapDir &remap )
{
	EmissionRenderElement*	newEle = new EmissionRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void EmissionRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
 		Color c = ip.selfIllumOut;
		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = ip.finalAttenuation * c;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}


//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Background render element
//
class BgndRenderElement : public BaseRenderElement {
public:
		BgndRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return bgndRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_BGND_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// Bgnd element class descriptor
// --------------------------------------------------
class BgndElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new BgndRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BGND_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return bgndRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("bgndRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static BgndElementClassDesc bgndCD;
ClassDesc* GetBgndElementDesc() { return &bgndCD; }


// ------------------------------------------------------
// Bgnd parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 bgndParamBlk(PBLOCK_NUMBER, _T("Bgnd render element parameters"), 0, &bgndCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Bgnd Render Element ------------------------------------------------
BgndRenderElement::BgndRenderElement()
{
	bgndCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle BgndRenderElement::Clone( RemapDir &remap )
{
	BgndRenderElement*	newEle = new BgndRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void BgndRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	float a;
	Color bgClr, bgTrans;
	sc.GetBGColor( bgClr, bgTrans, FALSE);
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = bgClr;
	} else {
		a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ] = bgClr * a;
	}
	a = 1.0f - Intens( bgTrans );
	if( a < 0.0f ) a = 0.0f;
	sc.out.elementVals[ mShadeOutputIndex ].a = a;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Reflection render element
//
class ReflectionRenderElement : public BaseRenderElement {
public:
		ReflectionRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return reflectionRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_REFLECTION_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class ReflectionElementClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ReflectionRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_REFLECTION_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return reflectionRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("reflectionRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ReflectionElementClassDesc reflectionCD;
ClassDesc* GetReflectionElementDesc() { return &reflectionCD; }


// ---------------------------------------------
// Reflection parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 reflectionParamBlk(PBLOCK_NUMBER, _T("Reflection render element parameters"), 0, &reflectionCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Reflection Render Element ------------------------------------------------
ReflectionRenderElement::ReflectionRenderElement()
{
	reflectionCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle ReflectionRenderElement::Clone( RemapDir &remap )
{
	ReflectionRenderElement*	newEle = new ReflectionRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void ReflectionRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
 		Color c = ip.reflIllumOut;
		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}


//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//
//	Refraction render element
//
class RefractionRenderElement : public BaseRenderElement {
public:
		RefractionRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return refractionRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_REFRACTION_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class RefractionElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new RefractionRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_REFRACTION_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return refractionRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("refractionRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static RefractionElementClassDesc refractionCD;
ClassDesc* GetRefractionElementDesc() { return &refractionCD; }


// ----------------------------------------------------------
// Refraction parameter block description - global instance
// ----------------------------------------------------------
static ParamBlockDesc2 refractionParamBlk(PBLOCK_NUMBER, _T("Refraction render element parameters"), 0, &refractionCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
/*
	BaseRenderElement::pathName, _T("pathName"), TYPE_STRING, 0, IDS_PATH_NAME,
		p_default, "",
		end,
*/
	end
	);

//--- Refraction Render Element ------------------------------------------------
RefractionRenderElement::RefractionRenderElement()
{
	refractionCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle RefractionRenderElement::Clone( RemapDir &remap )
{
	RefractionRenderElement*	newEle = new RefractionRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void RefractionRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
 		Color c = ip.transIllumOut;
		ClampMax( c );
		sc.out.elementVals[ mShadeOutputIndex ] = c;
//		sc.out.elementVals[ mShadeOutputIndex ] = ip.transIllumOut;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}


//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//
//	Shadow render element
//
class ShadowRenderElement : public BaseRenderElement {
public:
		ShadowRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		// note, we don't override the ShadowsApplied(){ return FALSE; }
		// we control it ourselves

		Class_ID ClassID() {return shadowRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_SHADOW_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class ShadowElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ShadowRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_SHADOW_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return shadowRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("shadowRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ShadowElementClassDesc shadowCD;
ClassDesc* GetShadowElementDesc() { return &shadowCD; }


// ---------------------------------------------
// Shadow parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 shadowParamBlk(PBLOCK_NUMBER, _T("Shadow render element parameters"), 0, &shadowCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
//	BaseRenderElement::colorOn, _T("colorOn"), TYPE_BOOL, 0, IDS_COLOR_ON,
//		p_default, TRUE,
//		end,
//	BaseRenderElement::alphaOn, _T("alphaOn"), TYPE_BOOL, 0, IDS_ALPHA_ON,
//		p_default, TRUE,
//		end,
	end
	);

//--- Shadow Render Element ------------------------------------------------
ShadowRenderElement::ShadowRenderElement()
{
	shadowCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle ShadowRenderElement::Clone( RemapDir &remap )
{
	ShadowRenderElement* newEle = new ShadowRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void ShadowRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		sc.shadow = 0;
		Color clrNS = sc.DiffuseIllum();
		sc.shadow = 1;
		Color clrS = sc.DiffuseIllum();
		float ns = Intens(clrNS);
//		float atten = (ns > 0.0f)? Intens(clrS)/ns : 1.0f;
		float atten = (ns > 0.01f)? Intens(clrS)/ns : 1.0f;
		if (atten>1.0f) atten = 1.0f/atten;	// correction for negative lights

		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0.0f,0.0f,0.0f,1.0f-atten);
	}
}


//-----------------------------------------------------------------------------

/*******************************************************

///////////////////////////////////////////////////////////////////////////////
//
//	Colored Shadow render element
//
class ClrShadowRenderElement : public BaseRenderElement {
public:
		ClrShadowRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		// note, we don't override the ShadowsApplied(){ return FALSE; }
		// we want unshadowed diffuse for scaling

		Class_ID ClassID() {return clrShadowRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_CLR_SHADOW_RENDER_ELEMENT ); }

		BOOL ShadowsApplied() const{ return TRUE; }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};



// ----------------------------------------------
// ClrShadow element class descriptor 
// ----------------------------------------------
class ClrShadowElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ClrShadowRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CLR_SHADOW_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return clrShadowRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("clrShadowRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ClrShadowElementClassDesc clrShadowCD;
ClassDesc* GetClrShadowElementDesc() { return &clrShadowCD; }


// ---------------------------------------------
// ClrShadow parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 clrShadowParamBlk(PBLOCK_NUMBER, _T("ClrShadow render element parameters"), 0, &clrShadowCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
//	BaseRenderElement::colorOn, _T("colorOn"), TYPE_BOOL, 0, IDS_COLOR_ON,
//		p_default, TRUE,
//		end,
//	BaseRenderElement::alphaOn, _T("alphaOn"), TYPE_BOOL, 0, IDS_ALPHA_ON,
//		p_default, TRUE,
//		end,
	end
	);

//--- ClrShadow Render Element ------------------------------------------------
ClrShadowRenderElement::ClrShadowRenderElement()
{
	clrShadowCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle ClrShadowRenderElement::Clone( RemapDir &remap )
{
	ClrShadowRenderElement* newEle = new ClrShadowRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


inline Clamp( Color& c ){
	if( c.r > 1.0f ) c.r = 1.0f; else if( c.r < 0.0f ) c.r = 0.0f;
	if( c.g > 1.0f ) c.g = 1.0f; else if( c.g < 0.0f ) c.g = 0.0f;
	if( c.b > 1.0f ) c.b = 1.0f; else if( c.b < 0.0f ) c.b = 0.0f;
}

inline CopyInputs( IllumParams& ipOut, IllumParams& ipIn ){
		ipOut.pShader = ipIn.pShader;
		ipOut.pMtl = ipIn.pMtl;
		ipOut.stdIDToChannel = ipIn.stdIDToChannel;
		ipOut.refractAmt = ipIn.refractAmt;
		ipOut.reflectAmt = ipIn.reflectAmt;
		ipOut.finalOpac = ipIn.finalOpac;
		ipOut.hasComponents = ipIn.hasComponents;
		ipOut.stdParams = ipIn.stdParams;
		for( int i=0; i < STD2_NMAX_TEXMAPS; ++i )
			ipOut.channels[ i ] = ipIn.channels[ i ];
}

void ClrShadowRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
 	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		/*********************
		Color diffClr;
		// use textured diffuse if it's there
		if( ip.stdIDToChannel )
			diffClr = ip.channels[ ip.stdIDToChannel[ ID_DI ] ];
		else
			diffClr = ip.pMtl->GetDiffuse();

		sc.shadow = 0;
		Color clrNS = sc.DiffuseIllum() * diffClr;
		Clamp( clrNS );
		sc.shadow = 1;
		Color clrS = sc.DiffuseIllum() * diffClr;
		Clamp( clrS );
		clrNS -= clrS;

		// permanent bug: negative colors will eventually be lost
		// can happen with colored shadows
		DbgAssert( clrNS.r >= 0.0f && clrNS.g >= 0.0f && clrNS.b >= 0.0f );

		sc.out.elementVals[ mShadeOutputIndex ] = clrNS;
		sc.out.elementVals[ mShadeOutputIndex ].a = 0.0f;
		***********************/
/**************
		Color clrNS, clrS;
		clrS = ip.finalC;
		BOOL saveShadow = sc.shadow;
		sc.shadow = 0;
 
		if( ip.pShader ){
			IllumParams ipNS;
			CopyInputs( ipNS, ip );
			ipNS.ClearOutputs();
			ip.pShader->Illum( sc, ipNS );
			clrNS = ipNS.finalC;
		} else {
			// prevent recursion to the elements by eliminating the mgr
			IRenderElementMgr* saveMgr = sc.globContext->GetRenderElementMgr();
			sc.globContext->SetRenderElementMgr(NULL);
			// save
			Color saveC = sc.out.c;
			Color saveT = sc.out.t;
			// get unshadowed illumination
			ip.pMtl->Shade( sc );
			clrNS = sc.out.c;
			// restore original values
			sc.out.c = saveC ;
			sc.out.t = saveT;
			sc.globContext->SetRenderElementMgr(saveMgr);	
		}
		// Clamp the colors before subtracting.
		Clamp( clrNS );
		Clamp( clrS );
		clrNS -= clrS;
		sc.out.elementVals[ mShadeOutputIndex ] = clrNS;

		sc.shadow = saveShadow;
	}
}
*************************************************/

///////////////////////////////////////////////////////////////////////////////
//
//	Atmosphere render element
//
class AtmosphereRenderElement : public BaseRenderElement {
public:
		AtmosphereRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return atmosphereRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_ATMOS_RENDER_ELEMENT ); }

		BOOL AtmosphereApplied() const{ return TRUE; } //override, always on

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class AtmosphereElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AtmosphereRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_ATMOS_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return atmosphereRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("atmosphereRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static AtmosphereElementClassDesc atmosphereCD;
ClassDesc* GetAtmosphereElementDesc() { return &atmosphereCD; }


// ---------------------------------------------
// Atmosphere parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 atmosphereParamBlk(PBLOCK_NUMBER, _T("Atmosphere render element parameters"), 0, &atmosphereCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Atmosphere Render Element ------------------------------------------------
AtmosphereRenderElement::AtmosphereRenderElement()
{
	atmosphereCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle AtmosphereRenderElement::Clone( RemapDir &remap )
{
	AtmosphereRenderElement*	newEle = new AtmosphereRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void AtmosphereRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
}

void AtmosphereRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	sc.out.elementVals[ mShadeOutputIndex ] = sc.out.c;
	sc.out.elementVals[ mShadeOutputIndex ].a = 1.0f - Intens( sc.out.t );
}

//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Alpha render element
//
class AlphaRenderElement : public BaseRenderElement {
public:
		AlphaRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return alphaRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_ALPHA_RENDER_ELEMENT ); }
		BOOL AtmosphereApplied() const{ return TRUE; } //we get z info from atmosphere

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// alpha element class descriptor 
class AlphaElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AlphaRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_ALPHA_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return alphaRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("alphaRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static AlphaElementClassDesc alphaCD;
ClassDesc* GetAlphaElementDesc() { return &alphaCD; }


// ---------------------------------------------
// Alpha parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 alphaParamBlk(PBLOCK_NUMBER, _T("Alpha render element parameters"), 0, &alphaCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Alpha Render Element ------------------------------------------------
AlphaRenderElement::AlphaRenderElement()
{
	alphaCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle AlphaRenderElement::Clone( RemapDir &remap )
{
	AlphaRenderElement* newEle = new AlphaRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void AlphaRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);

	} else {
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f; else if( a > 1.0f ) a = 1.0f;
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0.0f,0.0f,0.0f,a);
//		sc.out.elementVals[ mShadeOutputIndex ].Black();
	}
}

void AlphaRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
		float a = 1.0f - Intens( sc.out.t );
		sc.out.elementVals[ mShadeOutputIndex ] = AColor( a, a, a, a );
}



///////////////////////////////////////////////////////////////////////////////
//
//	Blend render element
//
class BlendRenderElement : public BaseRenderElement {
public:
		enum{
//			diffuseOn = BaseRenderElement::pathName+1, 
			diffuseOn = BaseRenderElement::pbBitmap+1, 
		    atmosphereOn, shadowOn,
			ambientOn, specularOn, emissionOn, reflectionOn, refractionOn
		};

		BlendRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return blendRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_BLEND_RENDER_ELEMENT ); }

		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		//attributes of the blend
		void SetApplyAtmosphere(BOOL on){ mpParamBlk->SetValue( atmosphereOn, 0, on ); }
		BOOL AtmosphereApplied() const{
			int	on;
			mpParamBlk->GetValue( atmosphereOn, 0, on, FOREVER );
			return on;
		}

		void SetApplyShadows(BOOL on){ mpParamBlk->SetValue( shadowOn, 0, on ); }
		BOOL ShadowsApplied() const{
			int	on;
			mpParamBlk->GetValue( shadowOn, 0, on, FOREVER );
			return on;
		}

		void SetDiffuseOn(BOOL on){ mpParamBlk->SetValue( diffuseOn, 0, on ); }
		BOOL IsDiffuseOn() const{
			int	on;
			mpParamBlk->GetValue( diffuseOn, 0, on, FOREVER );
			return on;
		}

		void SetAmbientOn(BOOL on){ mpParamBlk->SetValue( ambientOn, 0, on ); }
		BOOL IsAmbientOn() const{
			int	on;
			mpParamBlk->GetValue( ambientOn, 0, on, FOREVER );
			return on;
		}

		void SetSpecularOn(BOOL on){ mpParamBlk->SetValue( specularOn, 0, on ); }
		BOOL IsSpecularOn() const{
			int	on;
			mpParamBlk->GetValue( specularOn, 0, on, FOREVER );
			return on;
		}

		void SetEmissionOn(BOOL on){ mpParamBlk->SetValue( emissionOn, 0, on ); }
		BOOL IsEmissionOn() const{
			int	on;
			mpParamBlk->GetValue( emissionOn, 0, on, FOREVER );
			return on;
		}

		void SetReflectionOn(BOOL on){ mpParamBlk->SetValue( reflectionOn, 0, on ); }
		BOOL IsReflectionOn() const{
			int	on;
			mpParamBlk->GetValue( reflectionOn, 0, on, FOREVER );
			return on;
		}
		void SetRefractionOn(BOOL on){ mpParamBlk->SetValue( refractionOn, 0, on ); }
		BOOL IsRefractionOn() const{
			int	on;
			mpParamBlk->GetValue( refractionOn, 0, on, FOREVER );
			return on;
		}

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// element class descriptor - class declaration
// --------------------------------------------------
class BlendElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new BlendRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLEND_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return blendRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("blendRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static BlendElementClassDesc BlendCD;
ClassDesc* GetBlendElementDesc() { return &BlendCD; }

IRenderElementParamDlg *BlendRenderElement::CreateParamDialog(IRendParams *ip)
{
	return (IRenderElementParamDlg *)BlendCD.CreateParamDialogs(ip, this);
}

// ---------------------------------------------
// Blend parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 BlendParamBlk(PBLOCK_NUMBER, _T("Blend render element parameters"), 0, &BlendCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_BLEND_ELEMENT, IDS_BLEND_ELEMENT_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	BlendRenderElement::atmosphereOn, _T("atmosphereOn"), TYPE_BOOL, 0, IDS_ATMOSPHERE_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_ATMOS_ON,
		end,
	BlendRenderElement::shadowOn, _T("shadowOn"), TYPE_BOOL, 0, IDS_SHADOW_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SHADOW_ON,
		end,
	BlendRenderElement::diffuseOn, _T("diffuseOn"), TYPE_BOOL, 0, IDS_DIFFUSE_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_DIFFUSE_ON,
		end,
	BlendRenderElement::ambientOn, _T("ambientOn"), TYPE_BOOL, 0, IDS_AMBIENT_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_AMBIENT_ON,
		end,
	BlendRenderElement::specularOn, _T("specularOn"), TYPE_BOOL, 0, IDS_SPECULAR_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SPECULAR_ON,
		end,
	BlendRenderElement::emissionOn, _T("emissionOn"), TYPE_BOOL, 0, IDS_EMISSION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_EMISSION_ON,
		end,
	BlendRenderElement::reflectionOn, _T("reflectionOn"), TYPE_BOOL, 0, IDS_REFLECTION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_REFLECTION_ON,
		end,
	BlendRenderElement::refractionOn, _T("refractionOn"), TYPE_BOOL, 0, IDS_REFRACTION_ON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_REFRACTION_ON,
		end,
	end
	); 

//--- Blend Render Element ------------------------------------------------
BlendRenderElement::BlendRenderElement()
{
	BlendCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle BlendRenderElement::Clone( RemapDir &remap )
{
	BlendRenderElement*	newEle = new BlendRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void BlendRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	AColor c(0,0,0,0);
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = c;
	} else {
		if( IsDiffuseOn() ) c += ip.diffIllumOut;
		if( IsAmbientOn() ) c += ip.ambIllumOut;
		if( IsEmissionOn() ) c += ip.selfIllumOut;
		c *= ip.finalAttenuation;
		if( IsSpecularOn() ) c += ip.specIllumOut;
		if( IsReflectionOn() ) c += ip.reflIllumOut;
		if( IsRefractionOn() ) c += ip.transIllumOut;
		c.a = Intens( sc.out.t );
		ClampMax( c );

		sc.out.elementVals[ mShadeOutputIndex ] = c;
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

void BlendRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	sc.out.elementVals[ mShadeOutputIndex ] = sc.out.c;
	sc.out.elementVals[ mShadeOutputIndex ].a = 1.0f - Intens( sc.out.t );
}

//-----------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//	Z render element
//
class ZRenderElement : public BaseRenderElement {
public:
		enum{
			ZMin = BaseRenderElement::pbBitmap+1, 
		    ZMax,
			ZMult
		};

		ZRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return ZRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_Z_RENDER_ELEMENT ); }

		SFXParamDlg *CreateParamDialog(IRendParams *ip);

		// override shared attributes
		BOOL IsFilterEnabled() const{ return TRUE; } // filter z
		BOOL AtmosphereApplied() const{ return TRUE; } //we get z info from atmosphere
		BOOL BlendOnMultipass() const { return FALSE; } // don't blend z's

		//attributes of the z element
		void SetZMin(float z){
			mpParamBlk->SetValue( ZMin, 0, z ); 
		}
		float zMin() const {
			float z;
			mpParamBlk->GetValue( ZMin, 0, z, FOREVER );
			return z;
		}

		void SetZMax(float z){
			mpParamBlk->SetValue( ZMax, 0, z ); 
		}
		float zMax() const {
			float z;
			mpParamBlk->GetValue( ZMax, 0, z, FOREVER );
			return z;
		}

		void SetZMult(float z){
			mpParamBlk->SetValue( ZMult, 0, z ); 
		}
		float zMult() const {
			float z;
			mpParamBlk->GetValue( ZMult, 0, z, FOREVER );
			return z;
		}

		// the compute functions
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};
 
// --------------------------------------------------
// element class descriptor - class declaration
//
class ZElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ZRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_Z_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return ZRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("ZRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static ZElementClassDesc zElementCD;
ClassDesc* GetZElementDesc() { return &zElementCD; }

IRenderElementParamDlg* ZRenderElement::CreateParamDialog(IRendParams *irp)
{
	return (IRenderElementParamDlg*) zElementCD.CreateParamDialogs(irp, this);
}

// ---------------------------------------------
// z parameter block description - global instance
// ---------------------------------------------
static ParamBlockDesc2 ZParamBlk(PBLOCK_NUMBER, _T("Z render element parameters"), 0, &zElementCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_Z_ELEMENT, IDS_Z_ELEMENT_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	ZRenderElement::ZMult, _T("zMult"), TYPE_FLOAT, 0, IDS_ZMULT,
		p_default,		0.819,
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ZMULT_EDIT, IDC_ZMULT_SPIN, SPIN_AUTOSCALE,
		end,
	ZRenderElement::ZMin, _T("zMin"), TYPE_FLOAT, 0, IDS_ZMIN,
		p_default,		0.0,
		p_range, 		-10000.0, 10000.0, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ZMIN_EDIT, IDC_ZMIN_SPIN, SPIN_AUTOSCALE, 
//		p_accessor,		&extendedPBAccessor,
		end, 
	ZRenderElement::ZMax, _T("zMax"), TYPE_FLOAT, 0, IDS_ZMAX,
		p_default,		40.0,
		p_range, 		-10000.0, 10000.0, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ZMAX_EDIT, IDC_ZMAX_SPIN, SPIN_AUTOSCALE, 
		end,
	end
	);

//--- Z Render Element ------------------------------------------------
ZRenderElement::ZRenderElement()
{
	zElementCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle ZRenderElement::Clone( RemapDir &remap )
{
	ZRenderElement*	newEle = new ZRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void ZRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	sc.out.elementVals[ mShadeOutputIndex ].Black();
	sc.out.elementVals[ mShadeOutputIndex ].a = 1.0f;
}

void ZRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	if( zPrev <= 0.0f ){
		// first level z
		float zMin, zMax, zOut, zMult;
		mpParamBlk->GetValue( ZMin, 0, zMin, FOREVER );
		mpParamBlk->GetValue( ZMax, 0, zMax, FOREVER );
		mpParamBlk->GetValue( ZMult, 0, zMult, FOREVER );

		if(z<-1e9) return;


		
		Point3 p = sc.P();
		p = sc.globContext->camToWorld.PointTransform( p );
		z= p.z * zMult;


		//z = (float)((p.z+89.263664)*0.89622148442324401);

		if( z <= zMin ) 
			zOut = 0.0f;
		else if( z >= zMax ) 
			zOut = 1.0f;	
		else 
			zOut = ((z - zMin) / (zMax - zMin )) ;
	
		sc.out.elementVals[ mShadeOutputIndex ].a = 1;
		sc.out.elementVals[ mShadeOutputIndex ].r = 
		sc.out.elementVals[ mShadeOutputIndex ].g = 
		sc.out.elementVals[ mShadeOutputIndex ].b = zOut;
	}

}

//-----------------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//	Normal render element
//
class NormalRenderElement : public BaseRenderElement {
public:
		NormalRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return normalRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_NORMAL_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
};

// --------------------------------------------------
// Normal element class descriptor
// --------------------------------------------------
class NormalElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new NormalRenderElement; }
	const TCHAR *	ClassName() { return GetString(IDS_KE_NORMAL_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return normalRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("normalRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static NormalElementClassDesc normalCD;
ClassDesc* GetNormalElementDesc() { return &normalCD; }


// ------------------------------------------------------
// Normal parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 normalParamBlk(PBLOCK_NUMBER, _T("Normal render element parameters"), 0, &normalCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Normal Render Element ------------------------------------------------
NormalRenderElement::NormalRenderElement()
{
	normalCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle NormalRenderElement::Clone( RemapDir &remap )
{
	NormalRenderElement*	newEle = new NormalRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void NormalRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		//sc.out.elementVals[ mShadeOutputIndex ] = ip.finalAttenuation * (ip.diffIllumOut+ip.ambIllumOut);

		Point3 n = sc.Normal();

		float x,y,z;
		x=n.x;y=n.y;z=n.z;
		Point3 v1(0,0,0),v2(0,0,0);
		int max;
		double absx = fabs(x), absy = fabs(y), absz = fabs(z);
		if (absx > absy)
			max = (absx > absz ? 0 : 2);
		else
			max = (absy > absz ? 1 : 2);
		switch (max) {
		case 0:
			if (x > 0) v1.x=-y,v1.y=x; else v1.x=y,v1.y=-x;
			v2.x=-z;v2.z=x;
			break;
		case 1:
			if (n.y > 0) v1.x=-y,v1.y=x; else v1.x=y,v1.y=-x;
			v2.y=-z;v2.z=y;
			break;
		case 2:
			if (n.z > 0) v1.x=-z,v1.z=x; else v1.x=z,v1.z=-x;
			v2.y=-z,v2.z=y;
			break;
		}
		v1 = sc.globContext->camToWorld.VectorTransform(v1);
		v2 = sc.globContext->camToWorld.VectorTransform(v2);

		Point3 tp;

		tp=v1^v2;
	
		tp=tp.Normalize();

		sc.out.elementVals[ mShadeOutputIndex ].r = .5f+tp.x*.5f;
		sc.out.elementVals[ mShadeOutputIndex ].g = .5f+tp.y*.5f;
		sc.out.elementVals[ mShadeOutputIndex ].b = .5f+tp.z*.5f;
		sc.out.elementVals[ mShadeOutputIndex ].a = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Ambient render element
//
class AmbientRenderElement : public BaseRenderElement {
public:
		AmbientRenderElement();
		RefTargetHandle Clone( RemapDir &remap );

		Class_ID ClassID() {return ambientRenderElementClassID;}
		TSTR GetName() { return GetString( IDS_KE_AMBIENT_RENDER_ELEMENT ); }

		// the compute function
		void PostIllum(ShadeContext& sc, IllumParams& ip);
		//void PostAtmosphere(ShadeContext& sc, float z, float zPrev);
};

// --------------------------------------------------
// Ambient element class descriptor
// --------------------------------------------------
class AmbientElementClassDesc:public ClassDesc2
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AmbientRenderElement; }										   
	const TCHAR *	ClassName() { return GetString(IDS_KE_AMBIENT_RENDER_ELEMENT); }
	SClass_ID		SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }
	Class_ID 		ClassID() { return ambientRenderElementClassID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("ambientRenderElement"); } // hard-coded for scripter
	HINSTANCE		HInstance() { return hInstance; }
};

// global instance
static AmbientElementClassDesc ambientCD;
ClassDesc* GetAmbientElementDesc() { return &ambientCD; }


// ------------------------------------------------------
// Diffuse parameter block description - global instance
// ------------------------------------------------------
static ParamBlockDesc2 ambientParamBlk(PBLOCK_NUMBER, _T("Ambient render element parameters"), 0, &ambientCD, P_AUTO_CONSTRUCT, PBLOCK_REF,
	//rollout
//	IDD_COL_BAL_EFFECT, IDS_COL_BAL_PARAMS, 0, 0, NULL,
	// params
	BaseRenderElement::enableOn, _T("enabled"), TYPE_BOOL, 0, IDS_ENABLED,
		p_default, TRUE,
		end,
	BaseRenderElement::filterOn, _T("filterOn"), TYPE_BOOL, 0, IDS_FILTER_ON,
		p_default, TRUE,
		end,
	BaseRenderElement::eleName, _T("elementName"), TYPE_STRING, 0, IDS_ELEMENT_NAME,
		p_default, "",
		end,
	BaseRenderElement::pbBitmap, _T("bitmap"), TYPE_BITMAP, 0, IDS_BITMAP,
		end,
	end
	);

//--- Ambient Render Element ------------------------------------------------
AmbientRenderElement::AmbientRenderElement()
{
	ambientCD.MakeAutoParamBlocks(this);
	DbgAssert(mpParamBlk);
	SetElementName( GetName() );
}


RefTargetHandle AmbientRenderElement::Clone( RemapDir &remap )
{
	AmbientRenderElement*	newEle = new AmbientRenderElement();
	newEle->ReplaceReference(0,remap.CloneRef(mpParamBlk));
	return (RefTargetHandle)newEle;
}


void AmbientRenderElement::PostIllum(ShadeContext& sc, IllumParams& ip)
{
	if( ip.pMtl == NULL ){ // bgnd
		sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
	} else {
		//sc.out.elementVals[ mShadeOutputIndex ] = ip.finalAttenuation * (ip.diffIllumOut+ip.ambIllumOut);

		/*Point3 pp = sc.Normal();
		Point3 tp = sc.globContext->camToWorld.VectorTransform(pp);
*/
		int cc=-1;
		
		if(ip.pShader) cc=ip.pShader->StdIDToChannel(ID_AM);

		if(cc>=0)
			sc.out.elementVals[ mShadeOutputIndex ] = ip.channels[cc];
		else
			sc.out.elementVals[ mShadeOutputIndex ] = AColor(0,0,0,0);
		float a = 1.0f - Intens( sc.out.t );
		if( a < 0.0f ) a = 0.0f;
		sc.out.elementVals[ mShadeOutputIndex ].a = a;
	}
}

/*void AmbientRenderElement::PostAtmosphere(ShadeContext& sc, float z, float zPrev)
{
	sc.out.elementVals[ mShadeOutputIndex ] = sc.out.c;
	sc.out.elementVals[ mShadeOutputIndex ].a = 1.0f - Intens( sc.out.t );
}*/
//-----------------------------------------------------------------------------
