////////////////////////////////////////////////////////////////////////////////////////////////////
//
// normalizer.h
//
// Normalizace 3D vektoru; ZShift
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NORMALIZER_H
#define NORMALIZER_H

// CNormalizer 
// umoznuje rychlou normalizaci vektoru pomoci predpocitanych tabulek
//
// myslenka:
// ---------
// krychle, jejiz stred je umisteny v pocatku 
// v kazdem bode na plasti krychle je ulozen normalizovany vektor vedouci od pocatku do tohoto bodu
// ve skutecnosti staci ulozit pouze 1/4 jedne steny - ostatni se dostane pomoci symetrii
// (stena krychle je ulozena v poli nvec)
//
// normalizovany vektor, ziskame kdyz zjistime prusecik vektoru se stenou krychle

class CNormalizer
{
public:
	CNormalizer();
	void CmpNormal(int &nx, int &ny, int &nz);
private:
	char nvec[128][128][3];
};
extern CNormalizer normalizer;

// CLightZShift
// 1) vypocet posunuti v (x,y) podle 'z'; zavisi "pohledove" transformaci
//    posuny jsou predpocitany v tabulce pro z~0..255
// 2) vypocet posunuti stinu v (x,y) podle 'z'; smeru paprsku svetla shora
//    predpocitany

struct CLightZShift
{
public:
	CLightZShift(float ztransformx, float ztransformy, float toplight_zx, float toplight_zy);
	void Init(float ztransformx, float ztransformy, float toplight_zx, float toplight_zy);

	void CmpShift(int z, float &x, float &y);
	void CmpShadowShift(int z, int &x, int &y);

	struct{
		int x,y; //posuny "pohledove" transformace
		int tl_x,tl_y; //posuny stinu
	}xyshift[256];

	float zx,zy;
	float shadow_zx,shadow_zy;

};


extern CLightZShift *zshift;

#endif