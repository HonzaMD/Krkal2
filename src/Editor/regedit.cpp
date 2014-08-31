///////////////////////////////////////////////
//
// regedit.cpp
//
// Implementation pro editor registru
//
// A: Jan Poduska
//
///////////////////////////////////////////////


#include "stdafx.h"
#include "regedit.h"
#include "widgets.h"
#include "primitives.h"

#include "fs.h"

//////////////////////////////////////////////////////////////////////
// CEDRegEdit
//////////////////////////////////////////////////////////////////////

CEDRegEdit::CEDRegEdit(float _x, float _y, float _sx, float _sy, char *_dir, char *_file)
: CGUITree(_x,_y,_sx,_sy,"Registers Viewer",new CGUIRectHost(0,0,styleSet->Get("GOTB_GlobalVars")->GetTexture(0)))
{
	/*
	treeTextSize=8;
	treeRootItemHeight=25;
	treeItemHeight=15;
	treeRootNormalItemGap=5;
	treeItemStartY=0;
	treeEndGapY=treeEndGapX=0;
	*/

	CGUIFont *font = new CGUIFont();
	font->CreateFont(ARIAL,8);
	SetTextFont(font);
	SetRootTextFont(font);
	font->Release();

	treeRootItemHeight=15;
	treeItemHeight=15;
	treeRootNormalItemGap=0;
	treeItemStartY=0;
	treeEndGapY=treeEndGapX=0;

	dir = new char[strlen(_dir)+1];
	strcpy(dir,_dir);
	file = new char[strlen(_file)+1];
	strcpy(file,_file);

	reg=0;

	FS->ChangeDir(dir);
	if(CFSRegister::VerifyRegisterFile(file)==FSREGOK)
	{
		reg = new CFSRegister(file,0);
		if(reg->GetOpenError()==FSREGOK)
		{
			AddRegister(reg,0);
		}
		// delete reg;
	}

	UpdateTree();
	//CGUITreeItem* ti = AddTreeItem(0,label,new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)));
}

void CEDRegEdit::AddRegister(CFSRegister *r, CGUITreeItem* parentItem)
{
	//if(parentItem)
	//	parentItem->SetPicture(new CGUIRectHost(0,0,styleSet->Get("LebkyTB1")->GetTexture(0)));

	CFSRegKey* key  = r->GetFirstKey();
	while(key)
	{
		CGUITreeItem* ti;
		if(parentItem)
			ti = parentItem->AddTreeItem(0,key->GetName(),new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)),0,true);	
		else
			ti = AddTreeItem(0,key->GetName(),new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)),0,true,false); // false,true	

		ti->userTagType = eUTregister;

		if(key->CFSGetKeyType()==FSRTregister)
		{
			ti->ut.registerFS = key->GetSubRegister();
			ti->state=1;
			ti->ChangeRoller();
			ti->SetPicture(new CGUIRectHost(0,0,styleSet->Get("LebkyTB1")->GetTexture(0)));
			//AddRegister(key->GetSubRegister(),ti);
		}
		else
		{
			ti->ut.registerFS = 0;
			AddKey(key,ti);
		}
		key=key->GetNextKey();
	}
}

void CEDRegEdit::AddKey(CFSRegKey* key, CGUITreeItem* parentItem, int val_index)
{
	bool only_one=false;

	if(key->top==0)
		return;

	if(key->top==1)
		only_one=true;
		
	if(key->CFSGetKeyType()==FSRTstring)
	{
		key->SetPosToNextString();
		if(key->pos>=key->top)
			only_one=true;
		key->pos=0;
	}

	if(only_one)
	{
		char *buf;
		switch(key->CFSGetKeyType())
		{
			case FSRTchar	:	{buf = new char[25];
								unsigned char c = key->readc();
								if(isprint((int)c))
									_snprintf(buf,24,"%3d  0x%02X  '%c'",c,c,c);
								else
									_snprintf(buf,24,"%3d  0x%02X",c,c);
								break;}
			case FSRTdouble	:	buf = new char[35];
								_snprintf(buf,34,"%.10g",key->readd());
								break;
			case FSRTint	:	buf = new char[15];
								_snprintf(buf,14,"%d",key->readi());
								break;
			case FSRTint64	:	buf = new char[30];
								_snprintf(buf,29,"%I64d",key->read64());
								break;
			case FSRTstring	:	buf=0;
								break;
		}
		CGUIStaticText* st;
		if(buf)
		{
			st = new CGUIStaticText(buf,treeTextFont,0,0,STD_REGEDIT_VALUE_COLOR,300);
			//st = new CGUIStaticText(0,0,buf,ARIAL,treeTextSize,0,STD_REGEDIT_VALUE_COLOR);
			delete[] buf;
		}else
			//st = new CGUIStaticText(0,0,key->GetDirectAccess(),ARIAL,treeTextSize,0,STD_REGEDIT_VALUE_COLOR);
			st = new CGUIStaticText(key->GetDirectAccess(),treeTextFont,0,0,STD_REGEDIT_VALUE_COLOR,300);
			
		parentItem->SetItemElement(st,true);
	}else{
		if(val_index)
		{
			if(key->CFSGetKeyType()==FSRTstring)
			{
				int i = val_index;
				while(i--)
					key->SetPosToNextString();
			}else
			{
				key->seek(val_index);
			}
		}

		int max=STD_REGEDIT_MAX_KEYVALUES;
		char *buf = new char[35];
		char *index = new char[35];
		int ind=val_index;
		bool string=(key->CFSGetKeyType()==FSRTstring) ? true : false;
		while(key->pos<key->top && max--)
		{
			_snprintf(index,34,"%d. value:",ind++);
			switch(key->CFSGetKeyType())
			{
				case FSRTchar	:	{
										unsigned char c = key->readc();
										if(isprint((int)c))
											_snprintf(buf,34,"%3d  0x%02X  '%c'",c,c,c);
										else
											_snprintf(buf,34,"%3d  0x%02X",c,c);
										break;
									}
				case FSRTdouble	:	_snprintf(buf,34,"%.10g",key->readd());break;
				case FSRTint	:	_snprintf(buf,34,"%d",key->readi());break;
				case FSRTint64	:	_snprintf(buf,34,"%I64d",key->read64());break;
			}
			
			CGUITreeItem* ti = parentItem->AddTreeItem(0,index,new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));	
			if(val_index && ind==val_index+1)
			{
				ti->tree->UpdateTree();
				ti->Mark();
			}

			/*
			if(string)
			{
				parentItem->AddTreeItem(0,key->GetDirectAccessFromPos(),new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));	
				key->SetPosToNextString();
			}else
				parentItem->AddTreeItem(0,buf,new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));	
				*/
			CGUIStaticText* st;
			if(string)
			{
				st = new CGUIStaticText(key->GetDirectAccessFromPos(),treeTextFont,0,0,STD_REGEDIT_VALUE_COLOR,300);
				//st = new CGUIStaticText(0,0,key->GetDirectAccessFromPos(),ARIAL,treeTextSize,0,STD_REGEDIT_VALUE_COLOR);
				key->SetPosToNextString();
			}else
				st = new CGUIStaticText(buf,treeTextFont,0,0,STD_REGEDIT_VALUE_COLOR,300);
				//st = new CGUIStaticText(0,0,buf,ARIAL,treeTextSize,0,STD_REGEDIT_VALUE_COLOR);

			ti->SetItemElement(st,true);
		}

		if(max==-1 && key->pos<key->top)
		{
			parentItem->AddTreeItem(0,"...        ",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
		}
		delete[] buf;
		delete[] index;
	}

	parentItem->prepared=true;
}

int CEDRegEdit::TreeHandler(typeID treeItem, int index, int state)
{
	CFSRegister *reg;

	if(state!=0)
		return 1;

	CGUITreeItem* parentItem;
	CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(treeItem));
	if(ti)
	{
		if(ti->label)
		{
			if(!strcmp(ti->label->GetText(),"...        "))
			{
				
				FS->ChangeDir(dir);
				if(CFSRegister::VerifyRegisterFile(file)==FSREGOK)
				{
					CFSRegister *r = new CFSRegister(file,0);
					if(r->GetOpenError()==FSREGOK)
					{
						int ind=0;
						int depth;
						int *path = ti->GetTreePath(depth);
						reg = r;
						CFSRegKey* key=0;
						
						while(depth--)
						{
							int index=1;
							key  = reg->GetFirstKey();
							while(index<=path[ind])
							{
								key=key->GetNextKey();
								index++;
							}
							if(key->CFSGetKeyType()==FSRTregister)
							{
								reg = key->GetSubRegister();
								ind++;
							}
						}
						parentItem=ti->parentItem;
						parentItem->DeleteTreeItem(ti);
						AddKey(key,parentItem,path[ind+1]);
						UpdateTree();
						delete[] path;
					}
					delete r;
				}
				return 0;
			}
		}
	}

	return 1;
}

void CEDRegEdit::TreeUpdateHandler(typeID treeItem, int index)
{
	CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(treeItem));
	if(ti && !ti->prepared && ti->userTagType == eUTregister)
	{
		if(ti->ut.registerFS)
		{
			AddRegister(ti->ut.registerFS,ti);
			ti->prepared=true;
		}
	}

	UpdateTree();
}


CEDRegEdit::~CEDRegEdit()
{
	SAFE_DELETE(reg);
	delete[] dir;
	delete[] file;
}

