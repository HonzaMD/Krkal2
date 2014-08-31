//////////////////////////////////////////////////////////////////////
//
// DXtext.cpp
//
// zobrazi formatovany text
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "dx.h"

#include "dxtext.h"
#include "dxhighlight.h"

int CBltTextVertices::AddChar(int ch, float xx, float yy, CDXFont *font, DWORD color, const CMatrix2D *m, float *cl)
{

	int sx=font->charABC[ch].abcB+1;
	int sy=font->metric.tmHeight+1;

	if(numchar==maxchar)
	{
		CBltVert *vert = new CBltVert[maxchar*6 * 2];
		for(int i=0;i<curvert;i++)
		{
			vert[i]=vertices[i];
		}
		delete[] vertices;
		vertices=vert;
		maxchar*=2;
	}

		
	float x1,y1,x2,y2;
	float u1,v1,u2,v2;

	u1 = font->texcoord[ch][0];
	if(xx<cl[0]) {
		u1+=(cl[0]-xx)*font->texaddx; 
		x1=cl[0]; 
	}
	else 	
		x1=xx;
	

	v1 = font->texcoord[ch][1];
	if(yy<cl[1]) {
		v1+=(cl[1]-yy)*font->texaddy;
		y1=cl[1];
	}
	else
		y1=yy;

	x2 = xx+sx;
	u2 = font->texcoord[ch][2];
	if(x2>cl[2]){
		u2-=(x2-cl[2])*font->texaddx;
		x2=cl[2];
	}

	y2 = yy+sy;
	v2 = font->texcoord[ch][3];
	if(y2>cl[3]) {
		v2-=(y2-cl[3])*font->texaddy;
		y2=cl[3];
	}

	x1-=0.5f;x2-=0.5f;
	y1-=0.5f;y2-=0.5f;

	vertices[curvert].x=x1; 
	vertices[curvert].y=y1; 
	vertices[curvert].z = vertices[curvert].rhw = 1;
	vertices[curvert].col = color;
	vertices[curvert].u=u1;
	vertices[curvert].v=v1;	
	curvert++;
	vertices[curvert].x=x2; 
	vertices[curvert].y=y1; 
	vertices[curvert].z = vertices[curvert].rhw = 1;
	vertices[curvert].col = color;
	vertices[curvert].u=u2;
	vertices[curvert].v=v1;
	curvert++;
	vertices[curvert].x=x1; 
	vertices[curvert].y=y2; 
	vertices[curvert].z = vertices[curvert].rhw = 1;
	vertices[curvert].col = color;
	vertices[curvert].u=u1;
	vertices[curvert].v=v2;
	curvert++;
	vertices[curvert].x=x1; 
	vertices[curvert].y=y2; 
	vertices[curvert].z = vertices[curvert].rhw = 1;
	vertices[curvert].col = color;
	vertices[curvert].u=u1;
	vertices[curvert].v=v2;
	curvert++;
	vertices[curvert].x=x2; 
	vertices[curvert].y=y1; 
	vertices[curvert].z = vertices[curvert].rhw = 1;
	vertices[curvert].col = color;
	vertices[curvert].u=u2;
	vertices[curvert].v=v1;
	curvert++;
	vertices[curvert].x=x2; 
	vertices[curvert].y=y2; 
	vertices[curvert].z = vertices[curvert].rhw = 1;
	vertices[curvert].col = color;
	vertices[curvert].u=u2;
	vertices[curvert].v=v2;
	curvert++;

	CVec2 v;
	if(m){
		for(int i=curvert-6;i<curvert;i++)
		{
			v = m->TransPoint(vertices[i].x,vertices[i].y);
			vertices[i].x=v.x;
			vertices[i].y=v.y;
		}
	}

    numchar++;

	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


CBltFormatedText::CBltFormatedText(CFormatedText *_text, int xmargin, int ymargin)
{
	text = _text;

	vertices = NULL;

	drawcursor = 0; cursorblink=1;
	showmark = 0;

	xshift = xmargin;
	yshift = ymargin;

	clipout=1;
}

int CBltFormatedText::Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc)
{

	if(parentrecalc) needrecalc=dxrcAll;

	if(needrecalc==dxrcTextCursorBlink)
		needrecalc=0;

	if(needrecalc)
	{

		drawmark = 0;
		cursclout = 1;

		needrecalc = 0;


		CTextCursor cur(text);
		UC c;
		CTextFormat *format=cur.GetFormat();

		SAFE_DELETE(vertices);

		float cl[4]; //0-minx, 1-miny, 2-maxx 3-maxy

		const CMatrix2D *m=NULL;
		CMatrix2D im;

		if(parent)
		{			
			parent->GetGlobalTransPtr(&m);
			if(parentclip){
				m->CmpInverse(im);
			}
		}

		clipout=0;

		if(parentclip){
			if(parentclip->clippedout) {clipout=1;return 1;}
			if(m)
			{
				m->CmpInverse(im);
				CVec2 tmv;

				tmv.x = parentclip->minx;
				tmv.y = parentclip->miny;
				tmv = im.TransPoint(tmv);
				cl[0]=cl[2]=tmv.x;
				cl[1]=cl[3]=tmv.y;

				tmv.x = parentclip->maxx;
				tmv.y = parentclip->maxy;
				tmv = im.TransPoint(tmv);
				cl[0]=MIN(cl[0],tmv.x);
				cl[2]=MAX(cl[2],tmv.x);
				cl[1]=MIN(cl[1],tmv.y);
				cl[3]=MAX(cl[3],tmv.y);

				if(cl[0]>cl[2] || cl[1]>cl[3]) {clipout=1;return 1;}

			}else{
				if(parentclip->clippedout) {clipout=1;return 1;}
				cl[0] = parentclip->minx;
				cl[1] = parentclip->miny;
				cl[2] = parentclip->maxx;
				cl[3] = parentclip->maxy;
			}
		}

		CBltTextVertices *vv;
		float xx=0,yy=0;

		int v=0;

		int sx;
		int h=0;
		int desc,asc;

		int cnt;
		int height=0;
		int curw=2, curp=0;
		int mark=0,setmark=0,mm=0;

		if(text->IsMarked()&&showmark) mark=1;
		

		CTextBlock *block = text->GetFirstBlock();
		int drc;

		int insmode = text->cursor->GetInsertMode();
        
		if(mark) insmode=0;

		while(block&&block->text && block->starty+block->height+yshift < cl[1])
		{
			if(mark && (text->markstart->GetBlock()==block || text->markend->GetBlock()==block )  )
			{
				if(mm==0) {
					if(text->markend->GetBlock()==block) mm=2;
					else
					{
						SetMarkStart(0,(float)block->starty,(float)block->starty); mm=1; 
					}
				}
				else
				if(mm==1) {/*nic markovat nebudu*/ mm=2; }
			}
			block=block->next;
		}
		
		yy=(float)block->starty;


		while(block&&block->text)
		{
			cur = CTextCursor(text,block);
			format = cur.GetFormat();

//			block->starty = (int)yy;

			if(block->starty+yshift > cl[3]) break;

			if(text->cursor && text->cursor->GetBlock()==block )
			{
				if(text->cursor->DrawAtEndOfLine()) 
				{
					drc=2; 
					if(text->cursor->GetPos()==0) drc=1;
				}else drc=1;				
			}else drc=0;

			setmark=0;
			if(mark)
			{
				if(text->markstart->GetBlock()==block) setmark=1;
				if(text->markend->GetBlock()==block)   setmark|=2;
			}
			
			int lcnt=0,lasc=cur.CharAscent();
			int bcnt=0,line=0;

			while (line<block->numlines)
			{

				xx=0;

				cnt = block->lineslen[line];

				cur.CmpHeight(cnt - lcnt,h,height);
				
				lcnt = cnt;
				
				yy+=h;

				int sl=1;

				while( bcnt < cnt  )
				{
					c = cur.GetChar();


					sx = cur.CharWidth();
					asc = cur.CharAscent();
					desc = cur.CharDescent();

					if(sl) lasc=asc;

					if(c!=VK_TAB)
						xx+=format->font->charABC[c].abcA;
                    
					if(drc==1 && text->cursor->GetPos()==bcnt)
					{
						if(insmode)
						{
							if(c!=VK_TAB)
							{
								curw = cur.CharTotalWidth();
								curp=-format->font->charABC[c].abcA;
							}else{
								curw= (( ((int)xx/text->tabsize)+1 ) * text->tabsize) - (int)xx;
							}
						}
						SetCursorPos(xx+curp,xx+curp+curw,yy-lasc-1,yy-lasc+height,m,cl);
						drc=0;
					}

					if(setmark)
					{
						if(setmark&1)
						{
							if(text->markstart->GetPos()==bcnt)
							{
								if(mm==0) 
									{ SetMarkStart(xx,yy-lasc-1,yy-lasc+height); mm=1; }
								else 
									if(mm==1) {SetMarkEnd(xx,yy-lasc-1,yy-lasc+height,m,cl);mm=2;}
								
							}

						}
						if(setmark&2)
						{

							if(text->markend->GetPos()==bcnt)
							{
								if(mm==0) 
									{ SetMarkStart(xx,yy-lasc-1,yy-lasc+height); mm=1; }
								else 
									if(mm==1) {SetMarkEnd(xx,yy-lasc-1,yy-lasc+height,m,cl); mm=2;}
								
							}

						}
					}
					
					if(c!=VK_TAB)
					{

						if(xshift+xx+sx>cl[0] && xshift+xx<cl[2] && yshift+yy+desc>cl[1] && yshift+yy-asc<cl[3])
						{
							if(!format->font->vertices)
							{
								vv = format->font->vertices = new CBltTextVertices(6);
								vv->font = format->font;
								vv->next = vertices;
								vertices = vv;
							}else{
								vv = format->font->vertices;
							}
							vv->AddChar(c,xshift + xx, yshift + yy-asc,format->font,format->color,m,cl);
						}

						xx+=format->font->charABC[c].abcB + format->font->charABC[c].abcC;

					}else{
						xx = (float) (( ((int)xx/text->tabsize)+1 ) * text->tabsize);
					}

					cur.MoveRight(&format);
					//cnt--;

					bcnt++;

					if(drc==2 && text->cursor->GetPos()==bcnt)
					{
						SetCursorPos(xx+curp,xx+curp+curw,yy-lasc-1,yy-lasc+height,m,cl);
						//SetCursorPos(xx,xx+2,yy-lasc,yy,m,cl);
						drc=0;
					}

					lasc=asc;
					sl=0;
				}

				line++;

				yy+=height-h;

			}
			if(drc && text->cursor->GetPos()==bcnt)
			{
				SetCursorPos(xx+curp,xx+curp+curw,yy-height+h-lasc-1,yy-lasc+h,m,cl);
				//SetCursorPos(xx,xx+2,yy-height+h-lasc,yy-height+h,m,cl);
			}


			if(setmark)
			{
				if(setmark&1)
				{
					if(text->markstart->GetPos()==bcnt)
					{
						if(mm==0) { SetMarkStart(xx,yy-height+h-lasc-1,yy-lasc+h); mm=1; }
						else 
							if(mm==1) {SetMarkEnd(xx,yy-height+h-lasc-1,yy-lasc+h,m,cl); mm=2;}
						
					}

				}
				if(setmark&2)
				{

					if(text->markend->GetPos()==bcnt)
					{
						if(mm==0) { SetMarkStart(xx,yy-height+h-lasc-1,yy-lasc+h); mm=1; }
						else 
							if(mm==1) {SetMarkEnd(xx,yy-height+h-lasc-1,yy-lasc+h,m,cl);mm=2;}
						
					}

				}
			}
			

			block=block->next;
		}

		if(mark&&mm==1&&block->text)
		{
			SetMarkEnd(xx,(float)block->starty,(float)block->starty,m,cl);
		}

		vv = vertices;
		while(vv)
		{
			vv->font->vertices=NULL;
			vv=vv->next;
		}
		
	}

	if(!cached)
	{
		if(parent->AddToCache(this)) cached=1;
	}


	return 1;
}


void CBltFormatedText::BlinkCursor(int bl)
{
	if(bl==-1)
		cursorblink=!cursorblink;
	else
		cursorblink=bl;
	SetNeedRecalc(dxrcTextCursorBlink);
}
void CBltFormatedText::SetShowMark(int bl)
{
	showmark=bl;
	SetNeedRecalc();
}

void CBltFormatedText::SetCursorPos(float x1, float x2,float y1,float y2, const CMatrix2D *m, float *cl)
{
	cursclout = 0;

	x1+=xshift;x2+=xshift;
	y1+=yshift;y2+=yshift;

	if(x2<cl[0] || x1>cl[2] || y1>cl[3] || y2<cl[1] ) {
		cursclout = 1; return;
	}

	if(x1<cl[0]) x1=cl[0];
	if(x2>cl[2]) x2=cl[2];
	
	if(y1<cl[1]) y1=cl[1];
	if(y2>cl[3]) y2=cl[3];
	

	cursorvert[0].x = x1;
	cursorvert[0].y = y1;
	cursorvert[0].z = cursorvert[0].rhw = 1;
	cursorvert[0].col = 0xFFFFFFFF;

	cursorvert[1].x = x2;
	cursorvert[1].y = y1;
	cursorvert[1].z = cursorvert[1].rhw = 1;
	cursorvert[1].col = 0xFFFFFFFF;

	cursorvert[2].x = x1;
	cursorvert[2].y = y2;
	cursorvert[2].z = cursorvert[2].rhw = 1;
	cursorvert[2].col = 0xFFFFFFFF;

	cursorvert[3].x = x2;
	cursorvert[3].y = y2;
	cursorvert[3].z = cursorvert[3].rhw = 1;
	cursorvert[3].col = 0xFFFFFFFF;

	CVec2 v;
	if(m){
		for(int i=0;i<4;i++)
		{
			v = m->TransPoint(cursorvert[i].x,cursorvert[i].y);
			cursorvert[i].x=v.x;
			cursorvert[i].y=v.y;
		}
	}

}

void CBltFormatedText::SetMarkStart(float x, float y1,float y2)
{
	msx=x+xshift;msy1=y1+yshift;msy2=y2+yshift;
}

void CBltFormatedText::SetMarkEnd(float x, float y1,float y2, const CMatrix2D *m, float *cl)
{
	int sq=0;
	x+=xshift; y1+=yshift; y2+=yshift;

	if(msy1==y1)
	{
		sq+=AddMarkSq(sq,msx,x,y1,y2,m,cl);
	}else{
		if(y1>msy2){
			sq+=AddMarkSq(sq,msx,cl[2],msy1,msy2,m,cl);
			sq+=AddMarkSq(sq,cl[0],cl[2],msy2,y1,m,cl);
			sq+=AddMarkSq(sq,cl[0],x,y1,y2,m,cl);
		}else
		{
			sq+=AddMarkSq(sq,msx,cl[2],msy1,msy2,m,cl);
			sq+=AddMarkSq(sq,cl[0],x,msy2,y2,m,cl);
		}
	}
	
	if(sq) drawmark=1; else drawmark=0;

	nummarksq=sq;
}

int CBltFormatedText::AddMarkSq(int sq, float x1, float x2, float y1, float y2, const CMatrix2D *m, float *cl)
{

	sq=sq*4;

	if(x2<cl[0] || x1>cl[2] || y1>cl[3] || y2<cl[1] ) 
		return 0;
	

	if(x1<cl[0]) x1=cl[0];
	if(x2>cl[2]) x2=cl[2];
	
	if(y1<cl[1]) y1=cl[1];
	if(y2>cl[3]) y2=cl[3];
	
	markvert[sq+0].x = x1;
	markvert[sq+0].y = y1;
	markvert[sq+0].z = markvert[sq+0].rhw = 1;
	markvert[sq+0].col = 0xFFFFFFFF;

	markvert[sq+1].x = x2;
	markvert[sq+1].y = y1;
	markvert[sq+1].z = markvert[sq+1].rhw = 1;
	markvert[sq+1].col = 0xFFFFFFFF;

	markvert[sq+2].x = x1;
	markvert[sq+2].y = y2;
	markvert[sq+2].z = markvert[sq+2].rhw = 1;
	markvert[sq+2].col = 0xFFFFFFFF;

	markvert[sq+3].x = x2;
	markvert[sq+3].y = y2;
	markvert[sq+3].z = markvert[sq+3].rhw = 1;
	markvert[sq+3].col = 0xFFFFFFFF;

	CVec2 v;
	if(m){
		for(int i=sq;i<sq+4;i++)
		{
			v = m->TransPoint(markvert[i].x,markvert[i].y);
			markvert[i].x=v.x;
			markvert[i].y=v.y;
		}
	}

	return 1;

}

void CBltFormatedText::Render(int drawall)
{

	if(clipout) return;

	LPDIRECT3DDEVICE8 d3dDev=DXbliter->GetD3DDevice();

	CBltTextVertices *vv = vertices;

	while(vv)
	{
		d3dDev->SetTexture(0,vv->font->tex);
		d3dDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,2*vv->numchar,vv->vertices,sizeof(CBltVert));
		vv=vv->next;
	}

	d3dDev->SetTexture(0,NULL);

	if(drawcursor&&!cursclout&&cursorblink)
	{
		
		d3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_INVDESTCOLOR);
		d3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,cursorvert,sizeof(CBltVert));
		d3dDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	}

	if(drawmark)
	{
		d3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_INVDESTCOLOR);

        for(int i=0;i<nummarksq;i++)
			d3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,markvert+4*i,sizeof(CBltVert));

		d3dDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	}


}

void CBltFormatedText::CmpCursorPos(int &x, int &y, int &lnheight)
{
	int ln,cx,cy;
	
	text->GetCursor()->CmpPos(ln,cx,cy,lnheight);
	
	x=cx + xshift;
	y=cy+text->GetCursor()->GetBlock()->starty + yshift;

}

void CBltFormatedText::TransformCoords(int &x, int &y)
{
	x = x - xshift;
	y = y - yshift;
}

void CBltFormatedText::SetWidth(int width)
{
	text->SetWidth(width-xshift*2);
}

int CBltFormatedText::CmpTextSize(int &xs, int &ys)
{
	text->CmpTextSize(xs,ys);
	xs += xshift*2;
	ys += yshift*2;
	return 1;
}

int CBltFormatedText::CmpEditBoxHeight()
{
	CTextFormat *f = text->GetDefaultFormat();

	int sz = f->font->metric.tmAscent;

	sz+=2*yshift;

	return sz;

}

/////////////////////////////////////////////////////////////////////

CTextBlock::CTextBlock() //head constructor
{
	text = NULL; //head

	next=prev=this;

	format = NULL;
	formatcnt = 0;
	formatbuffercnt = 0;
	
	lineslen = NULL;
	numlines = 0;
	lineslenbufferlen = 0;

	height=0;width=0;

	blocknum=-1;
	
}

CTextBlock::CTextBlock(char *_text, int ln)
{
	if(ln==-1)
		ln=strlen(_text);

	text = new char[ln+1];
	memcpy(text,_text,ln);
	text[ln]=0;
	textlen = ln;
	textbufferlen = ln+1;

	format = NULL;
	formatcnt = 0;
	formatbuffercnt = 0;

	lineslen = NULL;
	numlines = 0;
	lineslenbufferlen = 0;

	height=0;width=0;
	blocknum=-1;

	blockcomment=-1;

	next=prev=NULL;
}

CTextBlock::~CTextBlock()
{
	SAFE_DELETE_ARRAY(text);
	SAFE_DELETE_ARRAY(format);
	SAFE_DELETE_ARRAY(lineslen);
}

void CTextBlock::SetFormat(CTextFormat *_format, int _formatcnt)
{
	SAFE_DELETE_ARRAY(format);
	
	format = _format;
	formatcnt = _formatcnt;
	formatbuffercnt = _formatcnt;

}

void CTextBlock::AddBlockBehind(CTextBlock *block)
{
	block -> next = next;
	block -> prev = this;
	next -> prev = block;
	next = block;
}

void CTextBlock::AddLine(int linelen)
{
	if(numlines>=lineslenbufferlen)
	{
		if(lineslen)
		{
			int *tm = new int[lineslenbufferlen*2];
			for(int i=0;i<lineslenbufferlen;i++)
				tm[i]=lineslen[i];
			delete[] lineslen;
			lineslen = tm;

			lineslenbufferlen*=2;
			
		}else{
			lineslenbufferlen=1;
			lineslen = new int[lineslenbufferlen];
		}
	}

	lineslen[numlines] = linelen;
	numlines++;
	
}
void CTextBlock::WordWrap(CFormatedText *ftext, int _width)
{
	int xx=0;
	UC c;

	int cnt=0,cumcnt=0;

	CTextCursor cur(ftext,this); 

	if(!ftext->wordwrap)
	{
		ResetNumLines();
		AddLine(textlen);
		while( (c=cur.GetChar())!=0 )
		{
			if(c!=VK_TAB)
				xx+=cur.CharTotalWidth();
			else
				xx = ( (xx/ftext->tabsize)+1 ) * ftext->tabsize;
			cur.MoveRight();
		}
		width = xx;
		if(ftext->maxwidth<width || (!prev->text && !next->text) ) {
			ftext->maxwidth=width;
		}
		return;
	}else{
		width=_width;
	}

	CTextFormat *form=cur.GetFormat();

	int lv=0;
	int lxx=0;

	ResetNumLines();

	cnt=0;
	lv=0;
	xx=0;

	int cs=0;

	if(textlen==0)
		AddLine(0);
	else
	while( (c=cur.GetChar())!=0 )
	{
			if(c!=VK_TAB)
				xx+=cur.CharTotalWidth();
			else
				xx = ( (xx/ftext->tabsize)+1 ) * ftext->tabsize;

			if(c == ' ' || c == VK_TAB)
			{
				lv=cnt+1;
				lxx = xx;
			}
			
			cnt++;

			if(xx>=width)
			{

				if(!lv)
				{
					lv=cnt-1;
					xx = cur.CharTotalWidth();
				}else
					xx = xx-lxx;

				lxx = 0;

				cumcnt+=lv;
				AddLine(cumcnt);

				cnt = cnt - lv;
				lv=0;

			}	
			cur.MoveRight(&form);
	}

	if(cnt)
	{
		cumcnt+=cnt;
		AddLine(cumcnt);
	}

}

void CTextBlock::InsertChar(int pos,UC ch)
{
	if(textlen+1==textbufferlen)
	{
		char *tm=new char[textbufferlen*2];
		memcpy(tm,text,pos);
		tm[pos]=ch;
		strcpy(tm+pos+1,text+pos);
		textbufferlen*=2;
		delete[] text;
		text=tm;
	}else{
		for(int i=textlen;i>=pos;i--)
		{
			text[i+1]=text[i];
		}
		text[pos]=ch;
	}
	textlen++;
}

void CTextBlock::DeleteChar(int pos)
{
	char *c=text+pos;

	do{
		*c=c[1];
		c++;
	}while(*c);

	textlen--;
}

int CTextBlock::DeleteFormat(int pos, int &jfp)
{
	CTextFormat *f=format+pos;

	formatcnt--;

	int joinfmt=0;

	if(pos>0 && pos<formatcnt)
	{
		if( format[pos-1].color==format[pos+1].color && 
			format[pos-1].font==format[pos+1].font )
		{
			joinfmt=1;
			formatcnt--;
			jfp = format[pos-1].numchar;
			format[pos-1].numchar+=format[pos+1].numchar;
		}
	}

	for(int i=pos;i<formatcnt;i++){
		*f=f[1+joinfmt];
		f++;
	}
	return joinfmt;
}

void CTextBlock::SplitBlock(int pos, int formatpos, int formatcharpos, CTextFormat *defaultfmt)
{
	CTextBlock *b = new CTextBlock(text+pos);

	text[pos]=0;
	textlen=pos;

	b->next=next;
	b->prev=this;

    next->prev=b;
	next = b;

	if( formatcnt!=0 &&  
		( formatpos<formatcnt-1 || 
		  format[formatcnt-1].color != defaultfmt->color || format[formatcnt-1].font != defaultfmt->font) 
	  )
	{
		
		CTextFormat *f;
		if(formatpos != formatcnt)
		{
			f = new CTextFormat[formatcnt-formatpos];
			for(int i=formatpos,j=0;i<formatcnt;i++,j++)
				f[j]=format[i];
			f[0].numchar-=formatcharpos;
			b->SetFormat(f,formatcnt-formatpos);
		}else
		{
			f = new CTextFormat[1];
			f[0].font = format[formatcnt-1].font;
			f[0].color = format[formatcnt-1].color;
			f[0].numchar = 0;
			b->SetFormat(f,1);
		}

		if(formatcharpos==0)
			formatcnt=formatpos;
		else
		{
			formatcnt=formatpos+1;
			format[formatpos].numchar=formatcharpos;
		}

	}
	
	
}

void CTextBlock::JoinBlock(int formatcharpos, CTextFormat *defaultfmt)
{
	CTextBlock *n=next;

	int i;

	int hn = n->height;

	if(n->textlen)
	{
		if(formatcnt || n->formatcnt)
		{
			int fc,ff;
			if(formatcnt) fc = formatcnt; else fc = 1;			
			if(n->formatcnt) fc+=n->formatcnt; else fc++;

			if(fc>formatbuffercnt)
			{
				CTextFormat *f=new CTextFormat[fc];

				for(i=0;i<formatcnt;i++)
					f[i] = format[i];
				
				delete[] format;
				format = f;
				formatbuffercnt = fc;
			}

			if(!formatcnt && textlen)
			{
				format[0] = *defaultfmt;
				format[0].numchar = textlen;
				ff=1;
			}else
			{
				if(!formatcnt) fc=n->formatcnt;
				ff=formatcnt;
			}

			if(!n->formatcnt)
			{
				if( ff && format[ff-1].color==defaultfmt->color && format[ff-1].font==defaultfmt->font )
				{
					format[ff-1].numchar += n->textlen;
					formatcnt = fc-1;

				}else{
					format[ff] = *defaultfmt;
					format[ff].numchar = n->textlen;
					formatcnt = fc;
				}
			}else{
				if( ff && format[ff-1].color== n->format[0].color && format[ff-1].font==n->format[0].font )
				{
					format[ff-1].numchar += n->format[0].numchar;
					formatcnt = fc-1;
					for(i=1;i<n->formatcnt;i++,ff++)
						format[ff] = n->format[i];
				}else
				{
					formatcnt = fc;
					for(i=0;i<n->formatcnt;i++,ff++)
						format[ff] = n->format[i];
				}

			}
		}
		if(textbufferlen<textlen+n->textlen+1)
		{
			char *t = new char[textlen+n->textlen+1];

			textbufferlen = textlen+n->textlen+1;

			memcpy(t,text,textlen);

			delete[] text;
			text = t;

		}
		memcpy(text+textlen,n->text,n->textlen+1);
		textlen+=n->textlen;
	}

	next->next->prev = this;
	next=n->next;

	n->next=n->prev=NULL;
	delete n;

	height+=hn; //aby se v CmpBlockSize odecetla delka nasledujiciho bloku

}

int CTextBlock::CmpBlockSize(CFormatedText *text, int calcystarts)
{

	int xx=0,yy=0;
	int hh = 0;
	int ln=0;

	CTextCursor c2(text,this);

	int oh=height;
	int set=0;

	UC c;

	if(c2.GetChar()==0) 
		height=c2.CharHeight();
	else
	{
		while(c=c2.GetChar())
		{

			if(c2.GetChar()!=' ')
			{
				hh = MAX(hh,c2.CharHeight());
				set=1;
			}

			if(c!=VK_TAB)
				xx+=c2.CharTotalWidth();
			else
				xx = ( (xx/text->tabsize)+1 ) * text->tabsize;
			
			if(c2.GetPos()==lineslen[ln]-1)
			{

				ln++; 
				yy+=hh;
				hh=0;
				xx=0;
			}
			c2.MoveRight();
		}

		yy-=hh;
		height = yy;

		if(set==0) height=c2.CharHeight();

	}

	if( oh!=height)
	{
		text->height += height - oh;
		if(calcystarts ) CmpYStarts();
		return 1;
	}else
		return 0;
}

void CTextBlock::CmpYStarts()
{
	if(prev->text)
	{
		starty=prev->starty+prev->height;
		blocknum=prev->blocknum+1;
	}else 
	{
		starty=0;
		blocknum=0;
	}

	int nb=blocknum+1;

	CTextBlock *b=next;
	while(b->text)
	{		
		b->starty=b->prev->starty + b->prev->height;
		b->blocknum=nb;
		nb++;
		b=b->next;
	}
}

void CTextBlock::CmpBlockNums()
{
	if(prev->text)
	{
		blocknum=prev->blocknum+1;
	}else 
		blocknum=0;

	int nb=blocknum+1;

	CTextBlock *b=next;
	while(b->text)
	{		
		b->blocknum=nb;
		nb++;
		b=b->next;
	}
}

void CTextBlock::FormatChars(int formatpos, int formatcharpos, int numchar, CDXFont *font, DWORD color, CTextFormat *defaultformat)
{
	int ff=0,ffs=-1,ffss,i;

	if(numchar==0) return;

	if(formatpos==-1)
	{
		if(font == defaultformat->font && color == defaultformat->color) return;

		if(formatbuffercnt<3)
		{
			SAFE_DELETE_ARRAY(format);
			formatbuffercnt=3;
			format=new CTextFormat[formatbuffercnt];
		}

		if(formatcharpos)
		{
			format[0].font=defaultformat->font;
			format[0].color=defaultformat->color;
			format[0].numchar=formatcharpos;
			ff++;
		}
		
		format[ff].font=font;
		format[ff].color=color;
		format[ff].numchar=numchar;
		ff++;

		if(formatcharpos+numchar<textlen)
		{
			format[ff].font=defaultformat->font;
			format[ff].color=defaultformat->color;
			format[ff].numchar=textlen - formatcharpos-numchar;
			ff++;
		}
		formatcnt=ff;

	}else{
		if(formatcharpos==0 && formatpos>0 && format[formatpos-1].font == font && format[formatpos-1].color==color)
		{
			format[formatpos-1].numchar+=numchar;
			ffs=formatpos;
			ffss=formatpos-1;
		}else
		if(format[formatpos].font==font && format[formatpos].color==color)
		{
			int n=format[formatpos].numchar - formatcharpos;
			if(numchar<=n)
				return;
			format[formatpos].numchar+=numchar-n;
			numchar-=n;
			ffs=formatpos+1;
			ffss=formatpos;
		}else
		{
			if(formatbuffercnt<formatcnt+2)
			{
				formatbuffercnt*=2;
				if(formatbuffercnt<formatcnt+2) formatbuffercnt=formatcnt+2;
				CTextFormat *f2 = new CTextFormat[formatbuffercnt];

				for(i=0;i<=formatpos;i++)
					f2[i]=format[i];

				for(i=formatpos;i<formatcnt;i++)
					f2[i+2]=format[i];

				delete[] format;
				format=f2;

			}else
			{
				for(i=formatcnt-1;i>=formatpos;i--)
					format[i+2]=format[i];
			}
			format[formatpos+2].numchar=format[formatpos].numchar-formatcharpos;
			format[formatpos].numchar=formatcharpos;

			format[formatpos+1].font=font;
			format[formatpos+1].color=color;
			format[formatpos+1].numchar=numchar;
			formatcnt+=2;
			ffs=formatpos+2;
			ffss=formatpos+1;
		}

		if(ffs!=-1&&numchar)
		{
			ff=ffs;
			//odectu zbyvajici delku
			while(numchar)
			{
				if(format[ff].numchar<=numchar)
				{
					numchar-=format[ff].numchar;
					i=ff+1;
					format[ff].numchar=0;
				}else{
					format[ff].numchar-=numchar;
					i=ff;
					numchar=0;					
				}
				ff++;
			}

			if(format[ffss].font==format[i].font && format[ffss].color==format[i].color )
			{
				format[ffss].numchar+=format[i].numchar;
				format[i].numchar=0;
			}
			
			ff=ffs=formatpos;
			i=0;
			//zrusim formaty s nulovou delkou
			while(ff<formatcnt)
			{
				if(format[ff].numchar)
				{
					if(ffs!=ff)
						format[ffs] = format[ff];
					
					ffs++;
				}else i++;
				ff++;
			}
			formatcnt-=i;

		}

	}
}


///////////////////////////////////////////////////////////////////////////

CFormatedText::CFormatedText(CDXFont *font, DWORD color, int _wordwrap, int _tabsize)
{
	head = new CTextBlock;

	wordwrap=_wordwrap;

	width = 200;

	format.font = font;
	format.color=color;
//	format.numcharbefore=0;

	height = 0;

	tabsize = _tabsize;

	cursor = NULL;

	markstart = markend = NULL;
	firstmark=0;

	undo = new CTextUndo;

}

CFormatedText::~CFormatedText()
{

	SAFE_DELETE(undo);

	SAFE_DELETE(markstart);
	SAFE_DELETE(markend);
	SAFE_DELETE(cursor);

	if(!head) return;

	CTextBlock *b=head, *tm;

	do{
		tm=b;
		b=b->next;
		delete tm;
	}while(b!=head);

}

void CFormatedText::DeleteAllTextBlocks()
{
	if(!head) return;

	CTextBlock *b=head->next, *tm;

	while(b!=head){
		tm=b;
		b=b->next;
		delete tm;
	};

	head->next=head->prev=head;

	height=0;
	maxwidth=0;
}

int CFormatedText::AddTextBlockToEnd( CTextBlock *block )
{
	if(!block) return 0;
	head->prev->AddBlockBehind(block);

	block->WordWrap(this,width);
	block->CmpBlockSize(this);

	return 1;
}

CTextBlock* CFormatedText::GetFirstBlock()
{
	if(head->next==head) return 0;
	return head->next;
}

CTextBlock* CFormatedText::GetLastBlock()
{
	if(head->prev==head) return 0;
	return head->prev;
}

void CFormatedText::CreateCursor()
{
	SAFE_DELETE(cursor);
	cursor = new CTextCursor(this);
}

void CFormatedText::SetWidth(int _width)
{
	width=_width;
	maxwidth = 0;
	CTextBlock *b=GetFirstBlock();
	while(b->text)
	{
		b->WordWrap(this,width);
		b->CmpBlockSize(this,0);
		b=b->next;
	}
	GetFirstBlock()->CmpYStarts();
	if(cursor)cursor->InvalidPos();
}

int CFormatedText::CmpTextSize(int &xs, int &ys)
{
	if(wordwrap) 
		xs = width;
	else
		xs = maxwidth;
	ys = height;
	return 1;
}

int CFormatedText::GetText(char *buf, int bufsize, int add13, int addendzero)
{
	CTextBlock *b=GetFirstBlock();
	int sz=0,csz;
	int ee=0;
	int az=1;

	if(addendzero) bufsize--; //misto na konecnou 0
	if(bufsize<0) {bufsize=0;ee=1;az=0;}

	while(b->text)
	{
		if(bufsize>=b->textlen) 
			csz=b->textlen; 
		else 
			{ csz=bufsize; ee=1; }

		memcpy(buf,b->text,csz);
		buf+=csz; bufsize-=csz;
		sz+=b->textlen;
		
		b=b->next;
		if(b->text)
		{
			if(add13){
				if(bufsize>1)
				{
					*buf++ = 13;
					*buf++ = 10;
					bufsize-=2;				
				}else {ee=1;bufsize=0;}
				sz+=2;
			}else
			{
				if(bufsize>0)
				{
					*buf++ = 10;
					bufsize--;				
				}else ee=1;
				sz++;
			}
		}

	}
	if(addendzero) {
		if(az) *buf=0;
		sz++;
	}

	if(ee) return sz;
	return 0;
}

int CFormatedText::SetText(char *buf, int size)
{

	int endln=0;

	if(undo) undo->DeleteAllUndoSteps();

	DeleteAllTextBlocks();
	CTextBlock *b;

	if(!buf) {
		b=new CTextBlock("");
		return 0;
	}

	char *bb=buf;
	int sz=0;

	if(!*buf || size==0)
	{ //zadny text -> pridam prazdny blok
		b=new CTextBlock("",0);
		AddTextBlockToEnd(b);
		b->WordWrap(this,width);
		b->CmpBlockSize(this,0);
	}else
	{
		while(*buf && size!=0)
		{
			endln=0;

			while(*buf!=10 && *buf!=13 && *buf && size!=0)
			{
				sz++;
				buf++;size--;
			}
			if(*buf==13) {buf++;size--;endln=1;}
			if(*buf==10) {buf++;size--;endln=1;}

			b=new CTextBlock(bb,sz);
			sz=0;
			bb=buf;

			AddTextBlockToEnd(b);
			b->WordWrap(this,width);
			b->CmpBlockSize(this,0);
		}
		if(endln)
		{
			b=new CTextBlock("",0);
			AddTextBlockToEnd(b);
			b->WordWrap(this,width);
			b->CmpBlockSize(this,0);
		}
	}
	if(cursor)
		cursor->SetCursorToStartOfBlock(GetFirstBlock());

	GetFirstBlock()->CmpYStarts();

	return 0;

}

int CFormatedText::GetTextInt(int &i)
{
	CTextBlock *b=GetFirstBlock();
	if(!b) return 1;
	
	char *c;

	int ii = strtol(b->text,&c,10);

	if(b->next->text) return 1;
	if(*c) return 1;
	
	i=ii;

	return 0;
}

int CFormatedText::GetTextDouble(double &i)
{
	CTextBlock *b=GetFirstBlock();
	if(!b) return 1;
	
	char *c;

	double ii = strtod(b->text,&c);

	if(b->next->text) return 1;
	if(*c) return 1;

	i=ii;

	return 0;
}

void CFormatedText::SetMarkStart(CTextCursor *cur)
{
	SAFE_DELETE(markstart);
	if(!cur) cur=cursor;
	markstart = new CTextCursor(*cur);
	firstmark=0;
}

void CFormatedText::SetMarkEnd(CTextCursor *cur)
{
	SAFE_DELETE(markend);
	if(!cur) cur=cursor;
	markend = new CTextCursor(*cur);
	firstmark=0;
}

void CFormatedText::UnMark()
{
	SAFE_DELETE(markstart);
	SAFE_DELETE(markend);
	firstmark=0;
}

CTextCursor* CFormatedText::FindFirstMark()
{
	if(!markstart||!markend) return 0;
    
	if(!firstmark) CmpFirstMark();

	if(firstmark==1) return markstart;
	if(firstmark==2) return markend;
	return 0;
}
CTextCursor* CFormatedText::FindSecondMark()
{
	if(!markstart||!markend) return 0;
    
	if(!firstmark) CmpFirstMark();

	if(firstmark==2) return markstart;
	if(firstmark==1) return markend;
	return 0;
}

void CFormatedText::CmpFirstMark()
{
	if(!markstart||!markend) return;

	if(firstmark) return;

	CTextBlock *b1,*b2, *bb1, *bb2;
	b1 = markstart->GetBlock(); 
	b2 = markend->GetBlock(); 

	if(b1==b2)
	{
		if(markstart->GetPos()>markend->GetPos())
			firstmark=2;			
		else
			firstmark=1;
	}
	else
	{
		
		bb1 = b1->prev;
		bb2 = b1->next;

		while(bb1->text && bb2->text)
		{
			if(bb1 == b2){
				firstmark=2;
				break;
			}else
			if(bb2 == b2){
				firstmark=1;
				break;
			}
			bb1=bb1->prev;bb2=bb2->next;
		}
		if(!firstmark){
			if(!bb1->text) firstmark=1;
			if(!bb2->text) firstmark=2;
		}
	}

}

int CFormatedText::IsMarked()
{
	if(!markstart||!markend) return 0;

	if(markstart->GetBlock()==markend->GetBlock() && markstart->GetPos()==markend->GetPos())
		return 0;

	return 1;
}

int CFormatedText::DeleteSelectedText(int saveundo, int link)
{
	if(!IsMarked()) return 0;

	CTextCursor *c1,*c2;
	if(!(c1=FindFirstMark())) return 0;
	if(!(c2=FindSecondMark())) return 0;

	*cursor = *c1;

	CTextBlock *b1=c1->GetBlock();
	CTextBlock *b2=c2->GetBlock();

	char *tt;

	int ln;

	if(saveundo)
	{
		undo->CreateNewStep(eUdelsel,b1->blocknum,c1->GetPos(),link);
	}

	if(b1==b2)
	{
		ln=c2->GetPos()-c1->GetPos();
		while(ln){
			if(saveundo) undo->AddChar(cursor->GetChar());
			cursor->Delete(1,0);
			ln--;
		}
	}
	else
	{
		CTextBlock *b=b1->next,*tmb;

		int hh=0;

		//mazu do konce radky
		while(cursor->GetChar())
		{
			if(saveundo) undo->AddChar(cursor->GetChar());
			cursor->Delete(0,0);
		}

		if(saveundo) undo->AddChar(0); //pridam konec 1. radky do undo

		//zrusim bloky mezi
		while(b!=b2)
		{
			if(saveundo)
			{
				tt=b->text;
				while(*tt){
					undo->AddChar(*(UC*)tt);
					tt++;
				}
				undo->AddChar(0);
			}
			tmb = b;
			hh+=b->height;
			b=b->next;
			delete tmb;
		}
		b1->next=b2;
		b2->prev=b1;

		ln=c2->GetPos();

		cursor->Delete(0,0); //smazu konec 1.radky!

		while(ln) {  //smazu zbytek
			if(saveundo) undo->AddChar(cursor->GetChar());
			cursor->Delete(0,0);ln--;
		} 

		b1->WordWrap(this,width);
		b1->CmpBlockSize(this,0);
        
		height-=hh;

		b1->CmpYStarts();

		
	}

	if(saveundo) undo->DoneCurStep();

	UnMark();

	return 1;
	
}


int CFormatedText::CopyToClipboard()
{

	CTextBlock *b,*b1,*b2;
	int p1,p2;
	int insent=0;

	if(IsMarked())
	{
		CTextCursor *c1,*c2;
		if(!(c1=FindFirstMark())) return 0;
		if(!(c2=FindSecondMark())) return 0;
		b1=c1->GetBlock();
		b2=c2->GetBlock();
		p1=c1->GetPos();
		p2=c2->GetPos();
	}else{
		b1=cursor->GetBlock();
		b2=b1;
		p1=0;
		p2=b1->textlen;
		insent=1;
	}

	int ln,ln1,ln2;

	if(b1==b2){
		ln = ln1 = p2-p1;
		ln2=0;
	}else{
		b=b1->next;
		ln1 = b1->textlen - p1;
		ln = ln1+2;
		while(b!=b2)
		{
			ln+=b->textlen + 2;
			b=b->next;
		}
		ln2=p2;
		ln+=ln2;
	}
	if(insent) ln+=2; //cr+lf
	ln++;//konecna nula

	if (!OpenClipboard(NULL)) return 0; 
    
	EmptyClipboard(); 

	HANDLE hglbCopy = GlobalAlloc(GMEM_MOVEABLE, ln); 
	
	if(hglbCopy)
	{

		// Lock the handle and copy the text to the buffer. 
		char* clp = (char*)GlobalLock(hglbCopy); 

		if(clp)
		{

			memcpy(clp,b1->text+p1,ln1);
			clp+=ln1;
			if(b1!=b2)
			{
				*clp++=13;*clp++=10; //cr+lf
				b=b1->next;
				while(b!=b2)
				{
					memcpy(clp,b->text,b->textlen); clp+=b->textlen;
					*clp++=13;*clp++=10;
					b=b->next;
				}
				memcpy(clp,b2->text,ln2); clp+=ln2;
			}
			if(insent){
				*clp++=13;*clp++=10; //cr+lf
			}
			*clp = 0;    // null character 
		    
			//Unlock
			GlobalUnlock(hglbCopy); 
			
			// Place the handle on the clipboard. 
			SetClipboardData(CF_TEXT, hglbCopy); 
			
		}

	}

	CloseClipboard(); 
	 
	return 1;
}


int CFormatedText::PasteFromClipboard(int multiline, CDXHighlighter *highlighter, int saveundo)
{
	if (!IsClipboardFormatAvailable(CF_TEXT)) 
            return 0; 

	if (!OpenClipboard(NULL)) return 0; 

	HGLOBAL hglb = GetClipboardData(CF_TEXT);

	if (hglb != NULL) 
	{ 
		UC *str = (UC*)GlobalLock(hglb); 
		if (str != NULL) 
		{ 
			int insm = cursor->GetInsertMode();
			cursor->SetInsertMode(0);
            
			CTextBlock *b=cursor->GetBlock(),*bs,*be;

			if(saveundo)
			{
				undo->CreateNewStep(eUpaste,b->blocknum,cursor->GetPos());				
			}

            while(*str)
			{
				if(*str==10)
				{
					if(multiline) 
					{
						cursor->Return(0,0);					
						if(saveundo) undo->AddChar(0);
					}
					else
						break;
				}else
				if(*str==13);
				else
				if(isprint(*str)){
					cursor->InsertChar(*str,0,0);
					if(saveundo) undo->AddChar(*str);
				}

				str++;
			}

			if(saveundo) undo->DoneCurStep();

			bs=b;
			be=cursor->GetBlock();
			if(highlighter)
			{
				highlighter->HighlightFromBlock(bs);
				cursor->CalcFormat();
			}else
			{
				int chngw=0;
				while(b!=be)
				{
					b->WordWrap(this,width);
					chngw|= b->CmpBlockSize(this,0);
					b=b->next;
				}
				be->WordWrap(this,width);
				chngw|= be->CmpBlockSize(this,0);
					
				if(chngw) bs->CmpYStarts();
			}

			cursor->SetInsertMode(insm);

			GlobalUnlock(hglb); 
		} 
    } 
    
	CloseClipboard(); 
	 
	return 1;
}

int CFormatedText::Undo(CDXHighlighter *highlighter)
{
	if(!undo||undo->GetNumSteps()==0) return 0;

	undo->DoneCurStep();

	CTextUndoStep *s=undo->GetLastUndoStep();
	if(!s || s->type==eUhead) return 0;

	UnMark();

	int i,cnt=s->GetCount();
	int im=cursor->GetInsertMode();
	UC ch;

	cursor->SetInsertMode(0);

	CTextBlock *b,*bb;

	switch(s->type)
	{
	case eUpaste:
	case eUwrite:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
			cursor->Delete(1,0);
		break;
	case eUoverwrite:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		
		for(i=0;i<cnt;i+=2)
		{
			cursor->Delete(1,0);
			ch=(*s->text)[i];
			if(ch)
				cursor->InsertChar(ch,1,0);
		}
		break;		
	case eUreturn:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		cursor->Delete(1,0);
		break;
	case eUdelsel:
	case eUdel:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
		{
			ch=(*s->text)[i];
			if(ch)
				cursor->InsertChar(ch,1,0);
			else
				cursor->Return(1,0);
		}
		cursor->SetCursorPos(s->block,s->blockpos);
		break;
	case eUbackspace:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
		{
			ch=(*s->text)[i];
			if(ch)
				cursor->InsertChar(ch,1,0);				
			else
				cursor->Return(1,0);

			cursor->MoveLeft();

		}
		for(i=0;i<cnt;i++)
			cursor->MoveRight();
		break;
	case eUidentblock:
		cursor->SetCursorPos(s->block,0);bb=b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
		{
			cursor->SetCursorToStartOfBlock(bb);
			cursor->Delete(1,0);
			bb=bb->next;
		}
		cursor->SetCursorPos(s->block,s->blockpos);
		break;
	case eUidentblockleft:
		cursor->SetCursorPos(s->block,0);bb=b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
		{
			if((*s->text)[i]=='\t')
			{
				cursor->SetCursorToStartOfBlock(bb);
				cursor->InsertChar('\t',1,0);
			}
			bb=bb->next;
		}
		cursor->SetCursorPos(s->block,s->blockpos);
		break;
	default:
		return 0;
	}

	cursor->SetInsertMode(im);


	if(highlighter)
	{
		highlighter->HighlightFromBlock(b);
		cursor->CalcFormat();
	}

	undo->Undo();

	s=undo->GetLastUndoStep();
	if(s&&s->type!=eUhead&&s->linkwithnext) return Undo(highlighter);

	return 1;
}

int CFormatedText::Redo(CDXHighlighter *highlighter)
{
	if(!undo||undo->GetNumSteps()==0) return 0;

	undo->DoneCurStep();

	CTextUndoStep *s=undo->GetLastUndoStep();
	s=s->next;
	if(!s || s->type==eUhead) return 0;

	UnMark();

	int i,cnt=s->GetCount();
	int im=cursor->GetInsertMode();
	UC ch;

	cursor->SetInsertMode(0);

	CTextBlock *b,*bb;

	switch(s->type)
	{
	case eUwrite:
	case eUpaste:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
		{
			ch=(*s->text)[i];
			if(ch)
				cursor->InsertChar(ch,1,0);
			else
				cursor->Return(1,0);
		}
		break;
	case eUoverwrite:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		
		for(i=0;i<cnt;i+=2)
		{
			cursor->Delete(1,0);
			ch=(*s->text)[i+1];
			if(ch)
				cursor->InsertChar(ch,1,0);
		}  
		break;		
	case eUreturn:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		cursor->Return(1,0);
		break;
	case eUdelsel:
	case eUdel:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
			cursor->Delete(1,0);
		break;
	case eUbackspace:
		cursor->SetCursorPos(s->block,s->blockpos);b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
			cursor->Delete(1,0);
		break;
	case eUidentblock:
		cursor->SetCursorPos(s->block,0);bb=b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
		{
			cursor->SetCursorToStartOfBlock(bb);
			cursor->InsertChar('\t',1,0);
			bb=bb->next;
		}
		cursor->SetCursorPos(s->block,s->blockpos);
		break;
	case eUidentblockleft:
		cursor->SetCursorPos(s->block,0);bb=b=cursor->GetBlock();
		for(i=0;i<cnt;i++)
		{
			if((*s->text)[i]=='\t')
			{
				cursor->SetCursorToStartOfBlock(bb);
				cursor->Delete(1,0);
			}
			bb=bb->next;
		}
		cursor->SetCursorPos(s->block,s->blockpos);
		break;
	default:
		return 0;
	}

	cursor->SetInsertMode(im);


	if(highlighter)
	{
		highlighter->HighlightFromBlock(b);
		cursor->CalcFormat();
	}

	int lnk = s->linkwithnext;

	undo->Redo();

	if(lnk) return Redo(highlighter);

	return 1;
}

void CFormatedText::DoneUndo()
{
	if(undo)
		undo->DoneCurStep();
}

int CFormatedText::CanUndo()
{
	if(!undo) return 0;
	CTextUndoStep *s=undo->GetLastUndoStep();
	if(!s || s->type==eUhead) return 0;
	return 1;
}

int CFormatedText::CanRedo()
{
	if(!undo) return 0;
	CTextUndoStep *s=undo->GetLastUndoStep();
	if(!s) return 0;
	s=s->next;
	if(!s || s->type==eUhead) return 0;
	return 1;
}


//---

int CFormatedText::DoIdent()
{
	
	CTextBlock *b=cursor->GetBlock(),*bprev;

	bprev = b->prev;
	if(bprev->text==0) return 0;

	char *c=bprev->text;
	int uu=0;

	if(*c=='\t' || *c==' ')
	{
		uu=1;
		DoneUndo();
	}

	while(*c=='\t' || *c==' ')
	{
		cursor->InsertChar(*c);
		c++;
	}

	if(uu) DoneUndo();

	return 0;
}

int CFormatedText::IdentBlock(int left)
{
	CTextBlock *b1,*b2,*b;
	int p1;

	if(!IsMarked()) return 0;

	b1=FindFirstMark()->GetBlock(); 
	p1=FindFirstMark()->GetPos(); 
	b2=FindSecondMark()->GetBlock();

	UnMark();

	DoneUndo();

	if(left)
	{
		undo->CreateNewStep(eUidentblockleft,b1->blocknum,p1);

		b=b1;
		while(1){
			if(b->text[0]=='\t')
			{
				cursor->SetCursorToStartOfBlock(b);
				cursor->Delete(1,0);
				undo->AddChar('\t');
			}else
				undo->AddChar(0);
			if(b==b2) break;
			b=b->next;
		}
	}else
	{
		undo->CreateNewStep(eUidentblock,b1->blocknum,p1);
		b=b1;
		while(1){
			cursor->SetCursorToStartOfBlock(b);
			cursor->InsertChar('\t',1,0);
			undo->AddChar('\t');
			if(b==b2) break;
			b=b->next;
		}
	}
	CTextCursor cc(this);
	cc.SetCursorToStartOfBlock(b1);
	SetMarkStart(&cc);
	cc.SetCursorToEndOfBlock(b2);
	SetMarkEnd(&cc);

	cursor->SetCursorToStartOfBlock(b2);

	DoneUndo();

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CTextCursor
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

CTextCursor::CTextCursor(CFormatedText *_text, CTextBlock *_block)
{
	text = _text;
	SetCursorToStartOfBlock(_block);
	insertmode=0;
}


int CTextCursor::SetCursorToStartOfBlock(CTextBlock *_block)
{
	if(!_block)
		block = text->GetFirstBlock();
	else
		block = _block;

	textpos = 0;

	CalcFormat();
	
	drawatendofln=0;
	xpos = -1;

	return 1;
}

int CTextCursor::SetCursorToEndOfBlock(CTextBlock *_block)
{
	if(!_block)
		block = text->GetFirstBlock();
	else
		block = _block;

	textpos = block->textlen;
	
	CalcFormat();

	drawatendofln=1;
	xpos = -1;

	return 1;
}

int CTextCursor::SetCursorPos(int blocknum, int blockpos)
{
	CTextBlock *b=text->GetFirstBlock();

	while(b->blocknum!=blocknum&&b->text)
		b=b->next;

	if(b->blocknum!=blocknum) return 0;
	if(b->textlen<blockpos) return 0;

	block=b;
	textpos=blockpos;

	CalcFormat();

	drawatendofln=1;
	xpos = -1;

	return 1;
}

void CTextCursor::CalcFormat()
{
	if(block && block->formatcnt)
	{		
		int pos = 0;
		formatpos=0;
		if(block->format[formatpos].numchar)
		{
			while( formatpos < block->formatcnt && pos + block->format[formatpos].numchar <= textpos)
			{
				pos+=block->format[formatpos].numchar;
				formatpos++;
			}
		}

		formatcharpos = textpos-pos;
//		if(formatcharpos) formatpos++;

		format=&block->format[formatpos];
	}
	else
	{
		format = text->GetDefaultFormat();
		formatcharpos = 0;
		formatpos = -1;
	}

}

int CTextCursor::MoveRight(CTextFormat **_format)
{
	if(!block) return 0;

	if(block->text[textpos]!=0)
	{

		textpos++;
		if(formatpos!=-1)
		{

			formatcharpos++;

			assert(formatpos<block->formatcnt);

			if(formatcharpos==format->numchar)
			{
				formatpos++;
				formatcharpos=0;
				format++;
				if(_format) *_format=format;
			}
		}
	}else{
		if(block->next->text)
		{
			SetCursorToStartOfBlock(block->next);

		}else
			return 0;
	}

	drawatendofln=0;
	xpos=-1;

//	if(!_format) DebugMessage(17,"%i %i",block->formatbuffercnt,block->textbufferlen);

	return 1;

}

int CTextCursor::MoveLeft(CTextFormat **_format)
{
	if(!block) return 0;

	if(textpos!=0)
	{

		textpos--;
		if(formatpos!=-1)
		{

			formatcharpos--;

			if(formatcharpos==-1)
			{
				formatpos--;
				format--;
				formatcharpos = format->numchar-1;
				if(_format) *_format=format;
			}
			
		}
	}else{
		if(block->prev->text)
			SetCursorToEndOfBlock(block->prev);
		else
			return 0;
	}


	drawatendofln=0;
	
	xpos=-1;

//	if(!_format) DebugMessage(17,"%i %i",block->formatbuffercnt,block->textbufferlen);

	return 1;

}

int CTextCursor::MoveHome()
{
	if(!block) return 0;
	if(textpos==0) return 0;

	int line=GetBlockLine();
	line--;
	int pp;

	if(line>=0) pp=block->lineslen[line]; else pp=0;

	while(textpos>pp)
		MoveLeft(NULL);

	drawatendofln=0;

	return 1;
}

int CTextCursor::MoveEnd()
{
	if(!block) return 0;
	if(GetChar()==0) return 0;

	int line=GetBlockLine();

	int pp=block->lineslen[line]; 
	
	while(textpos<pp)
		MoveRight(NULL);

	if(GetChar()) drawatendofln=1; else drawatendofln=0;

	return 1;
}

int CTextCursor::MoveTextHome()
{
	if(!block) return 0;

	SetCursorToStartOfBlock(text->GetFirstBlock());
    
	return 1;
}

int CTextCursor::MoveTextEnd()
{
	if(!block) return 0;

	SetCursorToEndOfBlock(text->GetLastBlock());
    
	return 1;
}
int CTextCursor::MoveUp()
{
	if(!block) return 0;

	int line = GetBlockLine();
	if(block->prev->text==0 && line==0 ) return 0;
	
	if(xpos==-1) CmpXPos();
	int xp=xpos;

	int pp,pp2;

	if(line==0)
	{
		SetCursorToEndOfBlock(block->prev);
	
		line = block->numlines;
	}
	
	pp = block->lineslen[line-1];
	if(line>1) pp2 = block->lineslen[line-2]; else pp2=0;
	

	while(textpos>pp2) MoveLeft(NULL);
	int xx=0;
	if(xp>CharTotalWidth()-xp)
	{
		while(textpos<pp&&xx<xp){
			if(GetChar()!=VK_TAB)
				xx+=CharTotalWidth();
			else
				xx = ( (xx/text->tabsize)+1 ) * text->tabsize;
			MoveRight(NULL);
			if(xp-xx<=xx+CharTotalWidth()-xp) break;
		}
		if(textpos==pp) drawatendofln=1;
	}

	xpos=xp;

	return 1;
}

int CTextCursor::MoveDown()
{
	if(!block) return 0;

	int line = GetBlockLine();
	if(block->next->text==0 && line+1>=block->numlines ) return 0;
	
	if(xpos==-1) CmpXPos();
	int xp=xpos;

	int pp,pp2;

	if(line+1==block->numlines)
	{
		line=0;
		SetCursorToStartOfBlock(block->next);
		
		pp2 = block->lineslen[line];

	}else
	{

		pp = block->lineslen[line];
		pp2 = block->lineslen[line+1];

		while(textpos<pp) MoveRight(NULL);
	}

	int xx=0;
	if(xp>CharTotalWidth()-xp)
	{
		while(textpos<pp2&&xx<xp){
			if(GetChar()!=VK_TAB)
				xx+=CharTotalWidth();
			else
				xx = ( (xx/text->tabsize)+1 ) * text->tabsize;
			MoveRight(NULL);
			if(xp-xx<=xx+CharTotalWidth()-xp) break;
		}
		if(textpos==pp2) drawatendofln=1;
	}

	xpos=xp;

	return 1;
}

int CTextCursor::MoveWordRight()
{
	if(!block) return 0;

	if(block->next->text==0 && !GetChar()) return 0;

	UC c=GetChar();

	if(c==0) //jsem na konci bloku
	{
		MoveRight();
		return 1;
	}

	if(! ( isspace(c) || ispunct(c) ) )
	{
		//najsem na mezere - prejedu cely slovo
		do{
			MoveRight();
			c=GetChar();
			if(c==0) {
				MoveEnd();
				return 1;
			}
		}while(! ( isspace(c) || ispunct(c) ) );
	}else
	{
		if(!isspace(c))
		{
			MoveRight();
			c=GetChar();
		}
	}
	//dojedu na konec mezery
	while(isspace(c)) {MoveRight();c=GetChar();}

	return 1;
}

int CTextCursor::MoveWordLeft()
{
	if(!block) return 0;

	if(block->prev->text==0&&textpos==0) return 0;

	UC c;

	MoveLeft();
	c=GetChar();
	if(ispunct(c)) return 1;
	while(isspace(c)&&textpos) {MoveLeft();c=GetChar();}

	if(isspace(c)&&textpos==0) return 1;

	if(! ( isspace(c) || ispunct(c) ) )
	{
		//nejsem na mezere - prejedu cely slovo
		do{
			if(textpos==0) {
				return 1;
			}
			MoveLeft();
			c=GetChar();
		}while(! ( isspace(c) || ispunct(c) ) );
	}

	if(isspace(c)||ispunct(c)) MoveRight();

	return 1;
}

int CTextCursor::MovePageUp(int dy)
{
	int ln,cx,cy,lnh;
	int dd=0;
	int xp=xpos;

	CmpPos(ln,cx,cy,lnh);
	
	int oy = block->starty + cy;

	if(xp==-1) xp=cx;

	dy-=lnh;

	int ny = MoveXY(xp,block->starty+cy-dy,0);

	xpos = xp;

	return oy-ny;
}

int CTextCursor::MovePageDown(int dy)
{
	int ln,cx,cy,lnh;
	int dd=0;
	int xp=xpos;

	CmpPos(ln,cx,cy,lnh);

	int oy = block->starty + cy;

	if(xp==-1) xp=cx;

	int ny = MoveXY(xp,block->starty+cy+dy,0);

	xpos = xp;

	return ny-oy;
}

int CTextCursor::MoveXY(int x, int y, int me)
{
	CTextBlock *b=block;

	if(me && y<0)
	{
		MoveTextHome();
		return 0;
	}
	if(me && y>text->height)
	{
		MoveTextEnd();
		return text->height;
	}

	if(y<b->starty)
	{
		if(b->prev->text)
		{
			b=b->prev;
			while(y<b->starty && b->prev->text)
				b=b->prev;
		}
	}else
	if(y>b->starty)
	{
		while(b->starty + b->height<y && b->next->text)
			b=b->next;
	}

	SetCursorToStartOfBlock(b);
	int yy=b->starty;
	return yy + MoveBlockXY( x, y - yy);
}

int CTextCursor::MoveBlockXY(int xp, int y)
{
	int	ln = 0;
	int cnt=0,lcnt=0;
	int asc,h,th=0;

	if(!block->textlen) return 0;
	
	while(ln<block->numlines)
	{
		cnt = block->lineslen[ln];

		CmpHeight(cnt-lcnt,asc,h);

		lcnt=cnt;

		if(y>h){
            y-=h;
			th+=h;
		}else break;

		ln++;
	}

	if(ln==block->numlines)
		ln--;
	ln--;
	if(ln>=0)
	{
		textpos = block->lineslen[ln];
		CalcFormat();
	}
		
	int xx=0;
	int pp = block->lineslen[ln+1];
	if(xp>CharTotalWidth()-xp)
	{
		while(textpos<pp&&xx<xp){
			if(GetChar()!=VK_TAB)
				xx+=CharTotalWidth();
			else
				xx = ( (xx/text->tabsize)+1 ) * text->tabsize;
			MoveRight(NULL);
			if(xp-xx<=xx+CharTotalWidth()-xp) break;
		}
		if(textpos==pp) drawatendofln=1;
	}

	return th;
}

void CTextCursor::CmpPos(int &line, int &cx, int &cy, int &lnheight)
{
	line = 0;
	cx=cy=0;

	int xx=0,yy=0;
	int height = 0;
	int ln=0;
	int ee=0,slnh=0;

	CTextCursor c2(text,block);

	if(!c2.GetChar())
	{
		lnheight = c2.CharHeight();
		cx=cy=0;
		return;
	}

	UC c;

	while(c=c2.GetChar())
	{
		if(c2.textpos==textpos&&!ee)
		{
			line=ln; slnh=1;
			cx=xx;cy=yy;
		}

		if(c2.GetChar()!=' ')
			height = MAX(height,c2.CharHeight());

		if(c!=VK_TAB)
			xx+=c2.CharTotalWidth();
		else
			xx = ( (xx/text->tabsize)+1 ) * text->tabsize;
		
		if(c2.textpos==block->lineslen[ln]-1)
		{

			if(c2.textpos+1==textpos&&(drawatendofln||textpos==block->textlen))
			{
				ee=1; lnheight=height;
				line=ln;cx=xx;cy=yy;
			}
			if(slnh) { lnheight=height;slnh=0; }
			ln++; 
			yy+=height;
			height=0;
			xx=0;
		}
		c2.MoveRight();
	}
	if(slnh) { lnheight=height;slnh=0; }

	yy-=height;
}

int CTextCursor::CmpXPos()
{
	int line = GetBlockLine();
	int sp;
	line--;
	if(line>=0) sp = block->lineslen[line]; else sp=0;

	CTextCursor c2(*this);

	xpos=0;
	if(c2.textpos>sp)
	{
		while(c2.textpos>sp) 
			c2.MoveLeft();

		while(c2.textpos<textpos) 
		{	
			if(c2.GetChar()!=VK_TAB)
				xpos+=c2.CharTotalWidth();
			else
				xpos = ( (xpos/text->tabsize)+1 ) * text->tabsize;
			c2.MoveRight(NULL);
		}
	}
	return xpos;

}

int CTextCursor::GetBlockLine()
{
	if(!block) return 0;

	int line=0;
	while(block->lineslen[line]<=textpos&&line<block->numlines-1) line++;

	if(drawatendofln && line>0 && block->lineslen[line-1]==textpos) line--;

	return line;
}

int CTextCursor::CharWidth()
{
	if(formatpos==block->formatcnt) return 0;
	return format->font->charABC[(UC)(block->text[textpos])].abcB;
}

int CTextCursor::CharTotalWidth()
{
	UC c= block->text[textpos];
	if(formatpos==block->formatcnt) return 0;
	return format->font->charABC[c].abcA+format->font->charABC[c].abcB+format->font->charABC[c].abcC;
}

int CTextCursor::CharHeight()
{
	if(formatpos==block->formatcnt) return 0;
	return format->font->metric.tmHeight;
}

int CTextCursor::CharAscent()
{
	if(formatpos==block->formatcnt) return 0;
	return format->font->metric.tmAscent;
}

int CTextCursor::CharDescent()
{
	if(formatpos==block->formatcnt) return 0;
	return format->font->metric.tmDescent;
}


int CTextCursor::CmpHeight( int cnt, int &maxAscent, int &baselineDist )
{

	CTextCursor cur2(*this);
	CTextFormat *form=cur2.GetFormat();

	maxAscent = 0;
	baselineDist = 0;

	int set=0;

	while(cnt)
	{
		if(/*cnt!=1 ||*/ cur2.GetChar()!=' ')
		{
			maxAscent = MAX(maxAscent, cur2.CharAscent());
			baselineDist = MAX(baselineDist, form->font->metric.tmHeight);
			set=1;
		}
		cnt--;
		cur2.MoveRight(&form);
	}

	if(!set)
	{
		maxAscent = CharAscent();
		baselineDist = format->font->metric.tmHeight;
	}

	return 1;
}

void CTextCursor::InsertChar(UC ch,int recalcblock, int undo)
{

	UC och;

	if(insertmode)
	{
		och=GetChar();

        
		if(undo)
		{

			if(text->undo->GetCurType()!=eUoverwrite)
				text->undo->CreateNewStep(eUoverwrite,block->blocknum,textpos);

			text->undo->AddChar(och);
		}
		if(och!=0) Delete(recalcblock,0);

	}else{

		
		if(undo && text->undo->GetCurType()!=eUwrite)
			text->undo->CreateNewStep(eUwrite,block->blocknum,textpos);
	}

	if(undo) 
		text->undo->AddChar(ch);

	block->InsertChar(textpos,ch);

	if(formatpos!=-1)
	{
		if(formatpos>0 && formatcharpos==0)
			format[-1].numchar++;
		else
		{
			format->numchar++;
			formatcharpos++;
		}
	}
	textpos++;

	xpos = -1;

	if(recalcblock)
	{
		block->WordWrap(text,text->width);
		block->CmpBlockSize(text);
	}

}

void CTextCursor::Delete(int recalcblock, int undo)
{

	UC ch = GetChar();

	if(undo)
	{
		if(text->undo->GetCurType()!=eUdel)
			text->undo->CreateNewStep(eUdel,block->blocknum,textpos);
		
		text->undo->AddChar(ch);
	}

	if(ch==0) 
	{
		if(block->next->text==0) return;

		block->JoinBlock(formatcharpos,text->GetDefaultFormat());

        CalcFormat();

	}else
	{
		int jfp;

		block->DeleteChar(textpos);

		if(formatpos!=-1)
		{
			format->numchar--;
			if(format->numchar==0){
				if( block->DeleteFormat(formatpos,jfp) )
				{
					formatpos--;
					formatcharpos = jfp;
				}
				if(block->formatcnt)
					format = &block->format[formatpos];
				else
				{
					formatpos = -1;
					format = text->GetDefaultFormat();
				}
			}else
			if(formatcharpos==format->numchar)
			{
				formatpos++;
				formatcharpos=0;
				format++;
			}
		}
	
	}

	xpos=-1;

	if(recalcblock)
	{
		block->WordWrap(text,text->width);
		block->CmpBlockSize(text);
	}
	
}

void CTextCursor::Backspace(int recalcblock, int undo)
{
	if(textpos==0&&block->prev->text==0) return;

	if(undo)
	{
		if(text->undo->GetCurType()!=eUbackspace)
			text->undo->CreateNewStep(eUbackspace,block->blocknum,textpos);
	}

	MoveLeft();

	if(undo)
	{
		text->undo->AddChar(GetChar());
		text->undo->SetPos(block->blocknum,textpos);
	}
	
	Delete(recalcblock,0);
}

void CTextCursor::Return(int recalcblock, int undo)
{
	if(undo)
	{

		if(text->undo->GetCurType()!=eUreturn)
				text->undo->CreateNewStep(eUreturn,block->blocknum,textpos);			

		text->undo->AddChar(0);
		text->undo->DoneCurStep();
	}


	block->SplitBlock(textpos,formatpos,formatcharpos,text->GetDefaultFormat());

	if(recalcblock)
	{
		block->WordWrap(text,text->width);
   		block->next->WordWrap(text,text->width);

		int c=0;

		c = block->CmpBlockSize(text,0);
		c|= block->next->CmpBlockSize(text,0);
		if(c) block->CmpYStarts();
	}
	SetCursorToStartOfBlock(block->next);
}

void CTextCursor::FormatChars(int numchar, CDXFont *font, DWORD color)
{
	if(textpos+numchar>block->textlen)
		numchar=block->textlen-textpos;
	
	if(numchar<=0) return;

	if(formatpos==-1)
	{
		block->FormatChars(formatpos,textpos,numchar,font,color,text->GetDefaultFormat());
	}else{
		block->FormatChars(formatpos,formatcharpos,numchar,font,color,text->GetDefaultFormat());
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//
// Undo
//
/////////////////////////////////////////////////////////////////////////////////////


CTextUndoStep::CTextUndoStep()
{
	type = eUhead;

	next=prev=this;

	text = NULL;
}

CTextUndoStep::CTextUndoStep(eUndoType _type, int _block, int _blockpos, int link)
{
	assert(type!=eUhead);

	type = _type;

	block=_block;
	blockpos=_blockpos;
	
	next=prev=NULL;

	text = new CDynChar;
	linkwithnext = link;
}

CTextUndoStep::~CTextUndoStep()
{
	SAFE_DELETE(text);
}


int CTextUndoStep::AddChar(UC c)
{
	assert(text);
	char cc=c;
	text->Add(cc);
	return 1;
}

///////////////


CTextUndo::CTextUndo(int maxsteps)
{
	maxundosteps=maxsteps;
	numundosteps=0;

	head = new CTextUndoStep();
	curundo = NULL;
	lastundo = head;
}

CTextUndo::~CTextUndo()
{
	CTextUndoStep *s=head,*tmp;
	do{
		tmp=s;
		s=s->next;

		delete tmp;
	}while(s!=head);
}

void CTextUndo::DeleteAllUndoSteps()
{

	CTextUndoStep *s=head->next,*tmp;

	while(s!=head)
	{
		tmp=s;
		s=s->next;
		delete tmp;
	}

	head->next=head->prev=head;

	numundosteps=0;
	curundo=NULL;
	lastundo=head;
}

int CTextUndo::CreateNewStep(eUndoType _type, int block, int blockpos, int link)
{
	CTextUndoStep *s,*tmp;

	s=lastundo->next;
	while(s!=head)
	{
		tmp=s;
		s=s->next;
		delete tmp;
		numundosteps--;
	}

	s = new CTextUndoStep(_type,block,blockpos,link);

	s->prev=lastundo;
	s->next=head;

	lastundo->next=s;
	head->prev=s;

	numundosteps++;

	curundo=s;
	lastundo=s;

	if(numundosteps>maxundosteps)
	{
		//vyhodim prvni prvek
		s=head->next;

		s->next->prev=head;
		head->next=s->next;

		delete s;

		numundosteps--;
	}

	return 1;
}

int CTextUndo::AddChar(UC ch)
{
	assert(curundo);
	if(!curundo) return 0;

	curundo->AddChar(ch);

	return 1;
}

void CTextUndo::DoneCurStep()
{
	curundo=0;
}

eUndoType CTextUndo::GetCurType()
{
	if(!curundo) return eUnone;

	return curundo->type;
}

void CTextUndo::Undo()
{
	if(!lastundo||lastundo->type==eUhead) return;
	lastundo=lastundo->prev;
}

void CTextUndo::Redo()
{
	if(!lastundo||lastundo->next->type==eUhead) return;
	lastundo=lastundo->next;
}
