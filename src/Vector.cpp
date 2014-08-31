//////////////////////////////////////////////////////////////////
//
// Vector.cpp
//
// Operace s 2D vektory a maticema
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "vector.h"


CMatrix2D CMatrix2D::operator * (const CMatrix2D &m) const
{
	CMatrix2D r;
	int i,j,k;

	if(type&&m.type){
		if(type==1&&m.type==1) r.Identity(); else
		r.Translate(a[2][0]+r.a[2][0],a[2][1]+r.a[2][1]);
	}	

	for(i=0;i<2;i++)
		for(j=0;j<2;j++)
		{
			r.a[i][j]=0;
			for(k=0;k<2;k++) r.a[i][j]+=a[i][k]*m.a[k][j];

		}
	//i=2;
	for(j=0;j<2;j++)
	{
		r.a[2][j]=m.a[2][j];
		for(k=0;k<2;k++) r.a[2][j]+=a[2][k]*m.a[k][j];
	}


	return r;
}

int CMatrix2D::CmpInverse(CMatrix2D &m) const
{


	float D;

	float a00,a01,a02,a10,a11,a12,a20,a21,a22;
	
	a00 = a[0][0];  a01 = a[0][1];  a02 = 0;//a[0][2]; 
	a10 = a[1][0];  a11 = a[1][1];  a12 = 0;//a[1][2]; 
	a20 = a[2][0];  a21 = a[2][1];  a22 = 1;//a[2][2]; 

	D = a00*a11*a22 + a01*a12*a20 + a02*a10*a21 - a20*a11*a02 - a21*a12*a00 - a22*a10*a01;
	
	if (D == 0) return 0;

	m.a[0][0] = (a11*a22-a21*a12)/D; m.a[1][0] =-(a10*a22-a20*a12)/D; m.a[2][0] = (a10*a21-a20*a11)/D;
	m.a[0][1] =-(a01*a22-a21*a02)/D; m.a[1][1] = (a00*a22-a20*a02)/D; m.a[2][1] =-(a00*a21-a20*a01)/D;
//	m.a[0][2] = (a01*a12-a11*a02)/D; m.a[1][2] =-(a00*a12-a10*a02)/D; m.a[2][2] = (a00*a11-a10*a01)/D;

	return 1;
}


float len(const CVec2 &v)  //delka vektoru
{ 
	return (float)sqrt(v.x*v.x+v.y*v.y); 
}

