////////////////////////////////////////////////////////////////////////////////////////////////////
//
// lights.cpp
//
// Definice svetel
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include "lights.h"

#include "normalizer.h"
#include "genmain.h"

CTopLight::CTopLight(float _dx,float _dy,float _dz,float _ir,float _ig,float _ib)
{

	float ln;

	ln=(float)sqrt(_dx*_dx+_dy*_dy+_dz*_dz);

	dx=(char)((-_dx/ln)*127);
	dy=(char)((-_dy/ln)*127);
	dz=(char)((-_dz/ln)*127);

	if(_ir>=1) ir=255; else ir=(UC)(_ir*255);
	if(_ig>=1) ig=255; else ig=(UC)(_ig*255);
	if(_ib>=1) ib=255; else ib=(UC)(_ib*255);

	CmpID();
}

//////////////////////////////////

CLight::CLight(float px,float py,float pz, float ir, float ig, float ib, float radius)
{
	tmtag=0;
	PointLight(px,py,pz,ir,ig,ib,radius);
}

void CLight::PointLight(float _px,float _py,float _pz, float _ir, float _ig, float _ib, float _radius)
{

	px=(int)(_px);
	py=(int)(_py);
	pz=(int)(_pz);

	if(_ir>=1) ir=255; else ir=(UC)(_ir*255);
	if(_ig>=1) ig=255; else ig=(UC)(_ig*255);
	if(_ib>=1) ib=255; else ib=(UC)(_ib*255);

	imax=ir;
	if(ig>imax) imax=ig;
	if(ib>imax) imax=ib;

	r=(int)_radius;
	radius=(int)(1./(r*r) * (1<<24) + .5);
}

void CLight::PointLightInt(int _px,int _py,int _pz, UC _ir, UC _ig, UC _ib, int _radius)
{
	px=_px;py=_py;pz=_pz;

	ir=_ir;ig=_ig;ib=_ib;
	imax=ir;
	if(ig>imax) imax=ig;
	if(ib>imax) imax=ib;

	r=_radius;
	radius=(int)(1./(r*r) * (1<<24) + .5);
}

int CLight::Compare(CLight &l, int *f) //porovna 2 svetla: vraci 0=stejny, 1=jsem vetsi nez l, -1=jsem mensi nez l
{

	/* usporadani:
			podle px,py,pz, radiusu, intenzity	
	*/


	int lpx,lpy,lpz;
	if(f)
	{
		lpx=l.px-f[0];lpy=l.py-f[1];lpz=l.pz-f[2];
	}else{
		lpx=l.px;lpy=l.py;lpz=l.pz;
	}

	if(px!=lpx) { if(px>lpx) return 1; else return -1; } 
	if(py!=lpy) { if(py>lpy) return 1; else return -1; }
	if(pz!=lpz) { if(pz>lpz) return 1; else return -1; }

	if(radius!=l.radius) {if(radius>l.radius) return 1; else return -1; }

	if(ir!=l.ir) { if(ir>l.ir) return 1; else return -1; }
	if(ig!=l.ig) { if(ig>l.ig) return 1; else return -1; }
	if(ib!=l.ib) { if(ib>l.ib) return 1; else return -1; }

	return 0;
}

int CLight::TestLight(int x1, int y1, int z1,int x2, int y2, int z2)
{

#define sqr(x) (x)*(x)
#define test3D(x,y,z) if( sqr(x-px) + sqr(y-py) + sqr(z-pz) < sqr(r) ) return 1; else return 0;
#define testXY(x,y)   if( sqr(x-px) + sqr(y-py)             < sqr(r) ) return 1; else return 0;
#define testXZ(x,z)   if( sqr(x-px)             + sqr(z-pz) < sqr(r) ) return 1; else return 0;
#define testYZ(y,z)   if(             sqr(y-py) + sqr(z-pz) < sqr(r) ) return 1; else return 0;

	if(px>=x2){

		if(py>=y2){

			if(pz>=z2){
				test3D(x2,y2,z2);
			}else if(pz<=z1){
				test3D(x2,y2,z1);
			}else{
				testXY(x2,y2);
			}

		}else if(py<=y1){

			if(pz>=z2){
				test3D(x2,y1,z2);
			}else if(pz<=z1){
				test3D(x2,y1,z1);
			}else{
				testXY(x2,y1);
			}

		}else{

			if(pz>=z2){
				testXZ(x2,z2);
			}else if(pz<=z1){
				testXZ(x2,z1);
			}else{
				if( px-r < x2 ) return 1; else return 0;
			}

		}

	}else if(px<=x1){

		if(py>=y2){

			if(pz>=z2){
				test3D(x1,y2,z2);
			}else if(pz<=z1){
				test3D(x1,y2,z1);
			}else{
				testXY(x1,y2);
			}

		}else if(py<=y1){

			if(pz>=z2){
				test3D(x1,y1,z2);
			}else if(pz<=z1){
				test3D(x1,y1,z1);
			}else{
				testXY(x1,y1);
			}

		}else{

			if(pz>=z2){
				testXZ(x1,z2);
			}else if(pz<=z1){
				testXZ(x1,z1);
			}else{
				if( px+r > x1 ) return 1; else return 0;
			}

		}

	}else{

		if(py>=y2){

			if(pz>=z2){
				testYZ(y2,z2);
			}else if(pz<=z1){
				testYZ(y2,z1);
			}else{
				if( py-r < y2 ) return 1; else return 0;
			}

		}else if(py<=y1){

			if(pz>=z2){
				testYZ(y1,z2);
			}else if(pz<=z1){
				testYZ(y1,z1);
			}else{
				if( py+r > y1 ) return 1; else return 0;
			}

		}else{

			if(pz>=z2){
				if( pz-r < z2 ) return 1; else return 0;
			}else if(pz<=z1){
				if( pz+r > z1 ) return 1; else return 0;
			}else{
				return 1;
			}

		}
	}
	

	return 0;

}

int CLight::TestLightPlane(int x1, int y1, int x2, int y2, int z)
{
	int x,y;

	if(px<=x1) x=x1; else
	if(px>=x2) x=x2; else
			   x=px;
	if(py<=y1) y=y1; else
	if(py>=y2) y=y2; else
			   y=py;


	int llx,lly,llz;
	int ll,li,nl;

	llx = px - x; 
	lly =-py + y; 
	llz = pz - z; 

	ll=llx*llx+lly*lly+llz*llz;
	li=(ll*radius)>>16; 
	li=255-li; 

	if(li<=0) return 0;
	
	normalizer.CmpNormal(llx,lly,llz);	
	nl=(127*llz);

	if(nl<=0) return 0;

	if((li*(nl*imax))>>22) return 1;

	return 0;
}

int CLight::GetBBox(int &x1, int &y1, int &z1, int &x2, int &y2, int &z2)
{

	x1=px-r; x2=px+r;
	y1=py-r; y2=py+r;
	z1=pz-r; z2=pz+r;

	return 1;
}