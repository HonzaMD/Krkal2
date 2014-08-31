//////////////////////////////////////////////////////////////////////
//
// Vector.h
//
// operace s 2D vektory a maticema
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

const float PI = 3.1415926535f;

struct CVec2 {
	float x,y;
	CVec2(){};
	CVec2(float xx,float yy) {x=xx;y=yy;}

	CVec2 operator + (const CVec2 &B) const //soucet vektoru
	{ CVec2 C; C.x=x+B.x; C.y=y+B.y; return C; }
	CVec2 operator - (const CVec2 &B) const //rozdil vektoru
	{ CVec2 C; C.x=x-B.x; C.y=y-B.y; return C; }
	
	float operator * (const CVec2 &b)  const //skalarni soucin
	{ return x*b.x+y*b.y; }
	
	CVec2 operator * (float k) const //nasobeni vektoru skalarem
	{ CVec2 C; C.x=x*k; C.y=y*k; return C; }
	CVec2 operator / (float kk) const //deleni vektoru skalarem
	{ CVec2 C; float k=1/kk; C.x=x*k; C.y=y*k; return C; }
	
	CVec2 operator - (void) const //otoceni vektoru
	{ return CVec2(-x,-y); }
	
	void operator ~ (void) //normalizace vektoru
	{
		float ll=1/len();
		x*=ll;y*=ll;
	}

	int operator == (const CVec2 &v) const	//porovnani vektoru
	{	return (v.x==x)&&(v.y==y); }

	int operator != (const CVec2 &v) const	//porovnani vektoru
	{	return (v.x!=x)||(v.y!=y); }

	float len(void) const{ //delka vektoru
		return (float)sqrt(x*x+y*y); 
	}

};

float len(const CVec2 &v); //delka vektoru



struct CMatrix2D {		//transformacni matice 3x3

	CMatrix2D(){type=0;}

	void Identity(){	//jednotkova matice
		int x,y; 
		for(y=0;y<3;y++)
			for(x=0;x<2;x++)
				a[y][x]=(x==y?1.f:0);
		
		type=1;
	}
	int IsIdentity(){	//testuje jestli je matice jednotkova
		int x,y; 
		for(y=0;y<3;y++)
			for(x=0;x<2;x++)
				if(a[y][x]!=(x==y?1:0)) return 0;
		type=1;
		return 1;
	}
	void Translate(float xx,float yy) //vytvori matici posunu
	{
		int x,y; 
		for(y=0;y<2;y++)
			for(x=0;x<2;x++)
				a[y][x]=(x==y?1.f:0);
		a[2][0]=xx;a[2][1]=yy;
		type=2;
	}
	void Rotate(float alfa) //vytvori matici rotace
	{
		Identity();
		alfa=alfa*PI/180;
		float cx=(float)cos(alfa),sx=(float)sin(alfa);
		a[0][0]= cx; a[0][1]=sx;
		a[1][0]=-sx; a[1][1]=cx;
		type=0;
	}
	void Scale(float x,float y) //vytvori matici zvetseni
	{
		Identity();
		a[0][0]=x;a[1][1]=y;
		type=0;
	}
	int CmpInverse(CMatrix2D &m) const; //spocte inverzni matici

	CMatrix2D operator * (const CMatrix2D &m) const; //nasobeni matic

	CVec2 TransPoint(const CVec2 &v) const //transformace bodu
	{
		CVec2 vv;
		switch(type)
		{
		case 1: //id
			vv=v;
			break;
		case 2: //translate
			vv.x=v.x+a[2][0];
			vv.y=v.y+a[2][1];
			break;
		default:
			vv.x=v.x*a[0][0]+v.y*a[1][0]+a[2][0];
			vv.y=v.x*a[0][1]+v.y*a[1][1]+a[2][1];
		}
    	return vv;
	}

	CVec2 TransPoint(float x,float y) const //transformace bodu
	{
		CVec2 vv;
		switch(type)
		{
		case 1: //id
			vv.x=x;vv.y=y;
			break;
		case 2: //translate
			vv.x=x+a[2][0];
			vv.y=y+a[2][1];
			break;
		default:
			vv.x=x*a[0][0]+y*a[1][0]+a[2][0];
			vv.y=x*a[0][1]+y*a[1][1]+a[2][1];
		}
		return vv;
	}

	CVec2 TransVector(const CVec2 &v) const //transformace vektoru
	{
		CVec2 vv;
		if(type){
			vv.x=v.x;vv.y=v.y;
		}else
		{
			vv.x=v.x*a[0][0]+v.y*a[1][0];
			vv.y=v.x*a[0][1]+v.y*a[1][1];
		}
		return vv;
	}

	float Get_a(int row, int col) const {return a[row][col];}

protected:
	
	float a[3][2];
	char type; //0=unknown, 1=identity, 2=translation

};




#endif
