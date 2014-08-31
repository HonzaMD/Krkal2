/////////////////////////////////////////////////////////////////////////////////
///
///		GEnElement
///
///		obrazk pro GEn
///		A: Honza M.D. Krcek (viditelnostni trideni), Petr Altman (svetla)
///
/////////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"

#include "genelement.h"
#include "genmain.h"

#include "lightcache.h"
#include "register.h"
#include "KerConstants.h"

///////////////////////////////////////////////////////////////////
///
///		C G E n   E l e m e n t
///
///////////////////////////////////////////////////////////////////

/// Konstruktor
CGEnElement::CGEnElement(float x, float y, float z, CLightTex *tex, OPointer ptr) {
	UINT sx,sy;

	assert(!tex->Lazy());

	LightTex=tex;
	if(LightTex) LightTex->AddRef();

	CGEnElSource *els =	LightTex->GetElSource();

	LightTex->GetSize(sx,sy);
	float x1=x+els->pbx+GEnMain->GetXZTransform()*z;
	float y1=y+els->pby+GEnMain->GetYZTransform()*z;
	SetPos(x1,y1,(float)sx,(float)sy);
	zx=x; zy=y; zz=z;

	ObjPtr = ptr;

	mel=NULL;
	
	DasTepichFlag = 0;

	sortel = 0;
}

CGEnElement::~CGEnElement()
{
	if(GEnMain)
	{
		if(mel) //zrusim pohyb a animaci
			GEnMain->DeleteMovingElem(mel); //smaze moving element

		if(GEnMain->lightcache)
			GEnMain->lightcache->DeleteElem(this); //vyhodim se z lightcache
	}
	SAFE_RELEASE(LightTex);
}

int CGEnElement::CmpTex()
{
	if(!LightTex) return 0;

	CTexture *t;

	GEnMain->lightcache->GetTex(this,&t);
	
	{
		if(tex) tex->Release();
		tex=t;
		tex->AddRef();
	}

	return 2;
}

void CGEnElement::Render(int drawall)
{
	if(!drawall&&!redraw) return;

	redraw=0;

	if(!visible) return;
	if(numvert<3) return;

	LPDIRECT3DDEVICE8 d3dDev=DXbliter->GetD3DDevice();

	DXbliter->numdr++;

	if(!tex) CmpTex();

	if(tex)
	{
		LPDIRECT3DTEXTURE8 t=tex->GetTexture();
		d3dDev->SetTexture(0,t);
	} else d3dDev->SetTexture(0,NULL);	

	d3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, numvert-2, vertices,sizeof(CBltVert));

	SAFE_RELEASE(tex);
}

///////////////////////////////////////////////////////////
///  Posunuti vykresleneho Objektu
void CGEnElement::GEnMove(float dx, float dy, float dz, int cancelmove) {

	if(cancelmove && mel)
	{
		if(mel->type & 1) 
			mel->type ^= 1; //zrusim move
	}

	if(dx==0&&dy==0&&dz==0) 
		return;

	GEnMain->lightcache->DeleteElem(this);

	zx+=dx; zy+=dy; zz+=dz;
	dx+=GEnMain->GetXZTransform()*dz;
	dy+=GEnMain->GetYZTransform()*dz;

	Move(dx,dy);

	GEnMain->lightcache->AddElem(this);
}

void CGEnElement::GEnSetPos(float x, float y, float z, int cancelmove) 
{
	GEnMove(x-zx,y-zy,z-zz,cancelmove);
/*
	UINT sx,sy;
	GEnMain->lightcache->DeleteElem(this);
	CGEnElSource *els =	LightTex->GetElSource();

	LightTex->GetSize(sx,sy);
	float x1=x+els->pbx+GEnMain->GetXZTransform()*(z+els->pbz);
	float y1=y+els->pby+GEnMain->GetYZTransform()*(z+els->pbz);
	SetPos(x1,y1,(float)sx,(float)sy);
	zx=x; zy=y; zz=z;

	GEnMain->lightcache->AddElem(this);
*/
}

int CGEnElement::SetLightTex(CLightTex *tex)
{
	CGEnElSource *els;
	if(!tex||!tex->ForceLoad()) return 0;

	els = tex->GetElSource();
	if(!els) return 0;

	GEnMain->lightcache->DeleteElem(this);
	
	if(LightTex&&LightTex->GetElSource()==els)
	{
		InvalidCache();
		SetNeedRecalc(dxrcTex);

	}else{

		UINT sx,sy;
		tex->GetSize(sx,sy);
		float x1=zx+els->pbx+GEnMain->GetXZTransform()*zz;
		float y1=zy+els->pby+GEnMain->GetYZTransform()*zz;
		SetPos(x1,y1,(float)sx,(float)sy);

	}

	SAFE_RELEASE(LightTex);
	LightTex = tex;
	LightTex->AddRef();

	GEnMain->lightcache->AddElem(this);

	return 1;
}

//////////////////////////////////////////////////////////////////////
///
///		C G E n   E l S o u r c e
///
//////////////////////////////////////////////////////////////////////

int CGEnElSource::SetParams( float _pbx, float _pby, float _pbz, float _zdz, int _rampaflag, int _flugflag)
{
	pbx=_pbx;pby=_pby;pbz=_pbz;
	zdz=_zdz;
	DerRampaFlag=_rampaflag;
	DerFlugFlag=_flugflag;
	return 1;
}

int CGEnElSource::SetNumPoints(int pointsnum)
{
	if(pointsnum==BasePointsCount) return 1;

	if(pointsnum<3) return 0;

	SAFE_DELETE_ARRAY(BasePoints);
	BasePoints = new CVec2[pointsnum];

	BasePointsCount = pointsnum;

	return 1;
}

///  Vypocet Krajnich bodu ze zakladny pro viditelnostni trideni
///  Zavisi na uhlu pohledu (zvolene perspektive)
void CGEnElSource::CalculateBorderPoints() {
	CVec2 vv=RotateVR(GEnMain->GetZTransform()); // zjistuji kolmy vektor na perspektivni vektor
	float a;
	int f;
	if (!BasePoints) return;
	float minf, maxf;
	int mini=0, maxi=0;
	a=minf=maxf=vv*BasePoints[0];
	for (f=0;f<BasePointsCount;f++) {
		a=vv*BasePoints[f];
		if (a<minf) {mini=f; minf=a; }
		if (a>=maxf) {maxi=f; maxf=a; }
	}
	RPoint=maxi;
	LPoint=mini;

	CmpBBox();
}


void CGEnElSource::CmpBBox()
{

	if(BasePointsCount<0) return;

	minx = maxx = BasePoints[0].x;
	miny = maxy = BasePoints[0].y;

	for(int i=1;i<BasePointsCount;i++)
	{
		if(BasePoints[i].x<minx) minx=BasePoints[i].x;
		if(BasePoints[i].x>maxx) maxx=BasePoints[i].x;
		if(BasePoints[i].y<miny) miny=BasePoints[i].y;
		if(BasePoints[i].y>maxy) maxy=BasePoints[i].y;
	}

}

int CGEnElSource::Save(char *name, CGEnPackage *pkg)
{
	CFSRegister reg(name,"KRKAL_ELSOURCE",1);

	CFSRegKey *k,*kx,*ky;

	k = reg.AddKey("numPoints",FSRTint);
	k->writei(BasePointsCount);

	kx = reg.AddKey("Px",FSRTdouble);
	ky = reg.AddKey("Py",FSRTdouble);

	for(int i=0;i<BasePointsCount;i++)
	{
		kx->writed(BasePoints[i].x);
		ky->writed(BasePoints[i].y);
	}

	k = reg.AddKey("zdz",FSRTdouble);
	k->writed(zdz);
	k = reg.AddKey("pb",FSRTdouble);
	k->writed(pbx);
	k->writed(pby);
	k->writed(pbz);

	k = reg.AddKey("Flags",FSRTint);
	k->writei(DerRampaFlag);
	k->writei(DerFlugFlag);

	reg.WriteFile();

	return 1;

}

int CGEnElSource::Load(char *name, CGEnPackage *pkg, int lazy, int myindex )
{
	CFSRegister reg(name,"KRKAL_ELSOURCE");
	if(reg.GetOpenError()!=1) return 0;

	CFSRegKey *k,*kx,*ky;

	k = reg.FindKey("numPoints");
	BasePointsCount = k->readi();

	SAFE_DELETE_ARRAY(BasePoints);
	BasePoints = new CVec2[BasePointsCount];

	kx = reg.FindKey("Px");
	ky = reg.FindKey("Py");

	for(int i=0;i<BasePointsCount;i++)
	{
		BasePoints[i].x = (float) kx->readd();
		BasePoints[i].y = (float) ky->readd();
	}

	k = reg.FindKey("zdz");
	zdz = (float) k->readd();
	k = reg.FindKey("pb");
	pbx = (float) k->readd();
	pby = (float) k->readd();
	pbz = (float) k->readd();

	k = reg.FindKey("Flags");
	DerRampaFlag = k->readi();
	DerFlugFlag = k->readi();

	CalculateBorderPoints();

	return 1;

}


//////////////////////////////////////////////////////////////////
//
//		CGEnLine
//
//////////////////////////////////////////////////////////////////


CGEnLine::CGEnLine(float x1, float y1, float z1, float x2, float y2, float z2, DWORD color, int _TopFlag)
{

	if(z1<=z2) //prehodim body, aby zz1<=zz2
	{
		zx1=x1;zy1=y1;zz1=z1;
		zx2=x2;zy2=y2;zz2=z2;
	}else{
		zx1=x2;zy1=y2;zz1=z2;
		zx2=x1;zy2=y1;zz2=z1;
	}

	float xx1=zx1+GEnMain->GetXZTransform()*zz1;
	float yy1=zy1+GEnMain->GetYZTransform()*zz1;
	float xx2=zx2+GEnMain->GetXZTransform()*zz2;
	float yy2=zy2+GEnMain->GetYZTransform()*zz2;

	SetPoints(xx1,yy1,xx2,yy2);
	SetCol(color);

	TopFlag=_TopFlag;

	sortel = 0;

}

void CGEnLine::GEnSetPos(float x1, float y1, float z1,float x2, float y2, float z2)
{
	if(z1<=z2) //prehodim body, aby zz1<=zz2
	{
		zx1=x1;zy1=y1;zz1=z1;
		zx2=x2;zy2=y2;zz2=z2;
	}else{
		zx1=x2;zy1=y2;zz1=z2;
		zx2=x1;zy2=y1;zz2=z1;
	}

	float xx1=zx1+GEnMain->GetXZTransform()*zz1;
	float yy1=zy1+GEnMain->GetYZTransform()*zz1;
	float xx2=zx2+GEnMain->GetXZTransform()*zz2;
	float yy2=zy2+GEnMain->GetYZTransform()*zz2;

	SetPoints(xx1,yy1,xx2,yy2);
}

void CGEnLine::GEnMove(float dx, float dy, float dz)
{
	zx1+=dx;zy1+=dy;zz1+=dz;
	zx2+=dx;zy2+=dy;zz2+=dz;

	float xx1=zx1+GEnMain->GetXZTransform()*zz1;
	float yy1=zy1+GEnMain->GetYZTransform()*zz1;
	float xx2=zx2+GEnMain->GetXZTransform()*zz2;
	float yy2=zy2+GEnMain->GetYZTransform()*zz2;

	SetPoints(xx1,yy1,xx2,yy2);
}

///////////////////////////////////////////////////////////////////////////////////////

CGEnCursor::CGEnCursor()
{
	type=0;
	numlines=0;
	lines = NULL;

	inengine=-1;
}

CGEnCursor::~CGEnCursor()
{
	DeleteCursor();
}

int CGEnCursor::CreateCursor(int _type, float _sx, float _sy, float _sz, DWORD color, DWORD color2)
{

	DeleteCursor();

	type=_type;
	sx=_sx;sy=_sy;sz=_sz;
	px=py=pz=0;

	numlines=0;

	inengine=-1;
	lines=NULL;

	int ln=0;

	float x2=sx/2,y2=sy/2;

	switch(type)
	{
	case eKCTctverce:
		lines = new CGEnLine*[24];

		lines[ln++] = new CGEnLine(-x2,-y2, 0, x2,-y2, 0,color,0);
		lines[ln++] = new CGEnLine( x2,-y2, 0, x2, y2, 0,color,0);
		lines[ln++] = new CGEnLine( x2, y2, 0,-x2, y2, 0,color,0);
		lines[ln++] = new CGEnLine(-x2, y2, 0,-x2,-y2, 0,color,0);

		lines[ln++] = new CGEnLine(-x2,-y2,sz, x2,-y2,sz,color,0);
		lines[ln++] = new CGEnLine( x2,-y2,sz, x2, y2,sz,color,0);
		lines[ln++] = new CGEnLine( x2, y2,sz,-x2, y2,sz,color,0);
		lines[ln++] = new CGEnLine(-x2, y2,sz,-x2,-y2,sz,color,0);

		lines[ln++] = new CGEnLine(-x2,-y2,sz,-x2,-y2,0,color,0);
		lines[ln++] = new CGEnLine( x2,-y2,sz, x2,-y2,0,color,0);
		lines[ln++] = new CGEnLine( x2, y2,sz, x2, y2,0,color,0);
		lines[ln++] = new CGEnLine(-x2, y2,sz,-x2, y2,0,color,0);

        //---
		lines[ln++] = new CGEnLine(-x2,-y2, 0, x2,-y2, 0,color2,1);
		lines[ln++] = new CGEnLine( x2,-y2, 0, x2, y2, 0,color2,1);
		lines[ln++] = new CGEnLine( x2, y2, 0,-x2, y2, 0,color2,1);
		lines[ln++] = new CGEnLine(-x2, y2, 0,-x2,-y2, 0,color2,1);

		lines[ln++] = new CGEnLine(-x2,-y2,sz, x2,-y2,sz,color2,1);
		lines[ln++] = new CGEnLine( x2,-y2,sz, x2, y2,sz,color2,1);
		lines[ln++] = new CGEnLine( x2, y2,sz,-x2, y2,sz,color2,1);
		lines[ln++] = new CGEnLine(-x2, y2,sz,-x2,-y2,sz,color2,1);

		lines[ln++] = new CGEnLine(-x2,-y2,sz,-x2,-y2,0,color2,1);
		lines[ln++] = new CGEnLine( x2,-y2,sz, x2,-y2,0,color2,1);
		lines[ln++] = new CGEnLine( x2, y2,sz, x2, y2,0,color2,1);
		lines[ln++] = new CGEnLine(-x2, y2,sz,-x2, y2,0,color2,1);

		break;
	default:
		return 0;
	}

	inengine = 0;
	numlines=ln;

	return 1;
}

int CGEnCursor::DeleteCursor()
{
	if(inengine==-1) return 0;

	if(inengine==1){
		for(int i=0;i<numlines;i++){
			if(lines[i]){
				GEnMain->DeleteLine(lines[i]);
				lines[i]=NULL;
			}
		}
	}else{
		for(int i=0;i<numlines;i++){
			SAFE_DELETE(lines[i]);
		}
	}

	SAFE_DELETE_ARRAY(lines);
	numlines=0;

	inengine = -1;

	return 1;
}
int CGEnCursor::AddToGEn(float x,float y,float z)
{
	if(inengine!=0) return 0;

	SetCursorPos(x,y,z);

	for(int i=0;i<numlines;i++)
	{
		GEnMain->DrawLine(lines[i]);
	}

	inengine=1;
	return 1;
}

int CGEnCursor::MoveCursor(float dx,float dy,float dz)
{
	if(inengine==-1) return 0;

	for(int i=0;i<numlines;i++)
	{
		lines[i]->GEnMove(dx,dy,dz);
	}
	px+=dx;py+=dy;pz+=dz;
	return 1;
}

int CGEnCursor::SetCursorPos(float x, float y, float z)
{
	if(inengine==-1) return 0;
	x=x-px;y=y-py;z=z-pz;
	return MoveCursor(x,y,z);
}

int CGEnCursor::SetVisible(int visible)
{
	if(inengine==-1) return 0;

	if(inengine==visible) return 1;

	if(visible)
	{
		for(int i=0;i<numlines;i++)
			GEnMain->DrawLine(lines[i]);
		inengine = 1;
	}else{
		for(int i=0;i<numlines;i++)
			GEnMain->RemoveLine(lines[i]);
		inengine = 0;
	}

	return 1;

}

int CGEnCursor::SetColor(DWORD color, DWORD color2)
{
	if(inengine==-1) return 0;
	for(int i=0;i<numlines/2;i++)
	{
		lines[i]->SetCol(color); lines[i]->SetNeedRecalc();
	}
	for(int i=numlines/2;i<numlines;i++)
	{
		lines[i]->SetCol(color2); lines[i]->SetNeedRecalc();
	}
	return 1;
}