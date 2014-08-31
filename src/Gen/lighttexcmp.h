//vypocet nasvicene textury
//
//vetveni podle:
//	ISALPHA - v texture je alpha
//  ISAMBIENT - v texture je ambient
//	ISNORMAL - v texture je normala
//	ISZDEPTH - v texture je zdepth
//  ISSHADOW - na texturu je vrzen stin
//	ISPOINTLIGHT - na texturu sviti bodove svetlo
//  ISATMOSPHERE - v texture je atmosphere

yy=yys; 
shyb= (int)ElSource->pby; 
for(UI y=0;y<sy;y++)
{ 
	xx=xxs; 
	shxb= (int)ElSource->pbx; 
	for(UI x=0;x<sx;x++)
	{ 
		#ifdef ISALPHA
			a=*lt++; //alpha 
		#endif
		#ifdef ISAMBIENT
			amr=*lt++;amg=*lt++;amb=*lt++; //ambient
		#endif
		dr=*lt++;dg=*lt++;db=*lt++; //diffuse
		#ifdef ISNORMAL
			nx=*(char*)lt++;ny=*(char*)lt++;nz=*(char*)lt++; //normal
		#endif
		#ifdef ISAMBIENT
			cr=(amr * ar)>>7; //ambientni slozka 
			cg=(amg * ag)>>7; 
			cb=(amb * ab)>>7;			
		#else
			cr=(dr * ar)>>7; /*ambientni slozka*/ 
			cg=(dg * ag)>>7; 
			cb=(db * ab)>>7;
		#endif

		#ifdef ISZDEPTH
			z=*lt++; //zdepth
		#endif
		
		#ifdef ISATMOSPHERE
			cr+=*lt++; //atmosphere
			cg+=*lt++;
			cb+=*lt++;
		#endif

		#ifdef ISALPHA
			if(a)
		#endif
			{
				#ifdef ISZDEPTH
					zshiftx = zshift->xyshift[z].x; 
					zshifty = zshift->xyshift[z].y; 
					px=xx-zshiftx; py=yy-zshifty; pz=zz+z;
				#endif				
				
				#ifdef ISSHADOW

					shadow_intall = 256; 
					for(int sh=0;sh<numshadows;sh++){
						shadow = &shadows[sh]; 
						shadowtex=shadow->shadowtex; 
						#ifdef ISZDEPTH
							shx=shxb + zshift->xyshift[z].tl_x-zshiftx-shadow->dx;
							shy=shyb + zshift->xyshift[z].tl_y-zshifty-shadow->dy;
						#else
							shx=shxb - shadow->dx;
							shy=shyb - shadow->dy;
						#endif
						if(shx>=0 && shx<shadowtex->shadow_sx && shy>=0 && shy<shadowtex->shadow_sy){ 
								shadow_intenzity = shadowtex->shadowimg[ shx + shy * shadowtex->shadow_sx ] + 1; 
						}else 
								shadow_intenzity = 256; 
							shadow_intall *= shadow_intenzity; 
							shadow_intall>>=8; 
					}
					if(shadow_intall>0){
						#ifdef ISNORMAL
							nl=(nx*toplight->dx + ny*toplight->dy + nz*toplight->dz); /*>>7;*/ 
						#else
							nl=(127*toplight->dz); /*>>7;*/ 
						#endif
						if(nl>0) 
						{ 
							r=(( ((dr * shadow_intall)>>8) * (nl* toplight->ir))>>22); /* >>7 >>7 >>8 */ 
							g=(( ((dg * shadow_intall)>>8) * (nl* toplight->ig))>>22); 
							b=(( ((db * shadow_intall)>>8) * (nl* toplight->ib))>>22); 
							cr+=r;cg+=g;cb+=b; 
						}
					}

				#else
					#ifdef ISNORMAL
						nl=(nx*toplight->dx + ny*toplight->dy + nz*toplight->dz); /*>>7;*/ 
					#else
						nl=(127*toplight->dz); /*>>7;*/ 
					#endif
					if(nl>0) { 
						r=(( dr * (nl* toplight->ir))>>22); /* >>7 >>7 >>8 */ 
						g=(( dg * (nl* toplight->ig))>>22); 
						b=(( db * (nl* toplight->ib))>>22); 
						cr+=r;cg+=g;cb+=b; 
					}
				#endif

				#ifdef ISPOINTLIGHT
					for(lg=0; lg<numlight; lg++){ 
						l=lights[lg]; 
						#ifdef ISZDEPTH
							llx = l->px - px; 
							lly =-l->py + py; 
							llz = l->pz - pz; 
						#else
							llx = l->px - xx; 
							lly =-l->py + yy; 
							llz = l->pz - zz; 
						#endif
						ll=llx*llx+lly*lly+llz*llz;
						li=(ll*l->radius)>>16; 
						li=255-li; 

						if(li>0){ 
							normalizer.CmpNormal(llx,lly,llz);	//lx=llx;ly=lly;lz=llz; 
							#ifdef ISNORMAL
								nl=(nx*llx+ny*lly+nz*llz); /*>>7*/ 
							#else
								nl=(127*llz); /*>>7*/ 
							#endif
						
							if(nl>0){ 
								r=((dr*(nl*l->ir))>>22); // >>7 >>7 >>8
								g=((dg*(nl*l->ig))>>22); 
								b=((db*(nl*l->ib))>>22); 
								r=(r*li)>>8; 
								g=(g*li)>>8; 
								b=(b*li)>>8; 
								cr+=r;cg+=g;cb+=b; 
							} 
						}
					}
				#endif
			}

		if(cr>0xff) cr=0xff; 
		if(cg>0xff) cg=0xff; 
		if(cb>0xff) cb=0xff; 
		*buf++=cb; //b
		*buf++=cg; //g
		*buf++=cr; //r
		*buf++=a; //a
		xx++; 
		shxb++; 
	} 
	buf+=dp; 
	yy++; 
	shyb++; 
}