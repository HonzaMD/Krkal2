////////////////////////////////////////////////////////////////////////////////
//
//		GEnPackage
//
//		balicek obrazku(CLightTex), ElSourcu a animaci pro GEn
//		A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "genpackage.h"
#include "fs.h"
#include "register.h"

#include "genmain.h"
#include "musicmod.h"

#define PACKAGE_HASHTABLE_SIZE 128

#define KRKAL_PACKAGE "KRKAL package"

int PackageLazyLoad=1;

/////////////////////////////////////////////////////////////////////////////

CGEnPackage::CGEnPackage()
{

	PackageName = NULL;

	numResources = 0;
	ResourceNames = NULL;
	ResourceTypes = NULL;
	Resources = NULL;
}

CGEnPackage::~CGEnPackage()
{
	DestroyPackage();
}

void CGEnPackage::DestroyPackage()
{

	int i;
	for(i=0;i<numResources;i++){
		SAFE_DELETE_ARRAY(ResourceNames[i]);
		SAFE_RELEASE(Resources[i]);
	}

	SAFE_DELETE_ARRAY(ResourceNames);
	SAFE_DELETE_ARRAY(ResourceTypes);
	SAFE_DELETE_ARRAY(Resources);

	numResources = 0;

	SAFE_DELETE_ARRAY(PackageName);
}

int CGEnPackage::CreatePackage(char *name, int createarchive)
{

	int ok=1;

	DestroyPackage();

	if( !FS->GetFullPath(name,&PackageName) ) 
	{
		SAFE_DELETE_ARRAY(PackageName);
		return 0;
	}

	if(createarchive)
	{
		if( !FS->CreateArchive(PackageName) )
		{
			SAFE_DELETE_ARRAY(PackageName);
			return 0;
		}
	}else{
		if( !FS->CreateDir(PackageName) )
		{
			SAFE_DELETE_ARRAY(PackageName);
			return 0;
		}
	}

	char *curdir=NULL;
	FS->GetCurDir(&curdir);

	if(FS->ChangeDir(PackageName))
	{
		if(!WriteIndex()) ok=0;
	}else
		ok=0;

	if(curdir){
		FS->ChangeDir(curdir);
		delete[] curdir;
	}

	if(!ok) DestroyPackage();

	return ok;
}

int CGEnPackage::LoadPackage(char *name)
{
	int ok=1;

	DestroyPackage();

	if( !FS->GetFullPath(name,&PackageName) ) 
	{
		SAFE_DELETE_ARRAY(PackageName);
		return 0;
	}

	char *curdir=NULL;
	FS->GetCurDir(&curdir);

	if(FS->ChangeDir(PackageName))
	{
		if(!ReadIndex()) ok=0;
	}else
		ok=0;

	if(curdir){
		FS->ChangeDir(curdir);
		delete[] curdir;
	}

	if(!ok) DestroyPackage();

	return ok;
}

int CGEnPackage::WriteIndex()
{

	//predpoklada se, ze je nastaven akt. adresar do rootu package

	CFSRegister reg = CFSRegister("index",KRKAL_PACKAGE,1);
	CFSRegKey *key;

	key = reg.AddKey("numResources",FSRTint);
	key->writei(numResources);

	CFSRegKey *keyname, *keytype;
	int i;

	if(numResources)
	{
		keytype = reg.AddKey("ResType",FSRTint);
		keyname = reg.AddKey("ResName",FSRTstring);

		for(i=0;i<numResources;i++)
		{
			keytype->writei(ResourceTypes[i]);
			if(ResourceTypes[i]!=resNone)
				keyname->stringwrite(ResourceNames[i]);
		}
	}

	reg.WriteFile();

	return 1;

}

int CGEnPackage::ReadIndex()
{

	//predpoklada se, ze je nastaven akt. adresar do rootu package

	CFSRegister reg = CFSRegister("index",KRKAL_PACKAGE);
	if(reg.GetOpenError()!=1) return 0;

	CFSRegKey *key;

	key = reg.FindKey("numResources");
	numResources = key->readi();

	CFSRegKey *keyname, *keytype;
	int i;

	if(numResources)
	{

		Resources = new CGEnResource*[numResources];
		ResourceTypes = new eResourceType[numResources];
		ResourceNames = new char*[numResources];

		keytype = reg.FindKey("ResType");
		keyname = reg.FindKey("ResName");

		for(i=0;i<numResources;i++)
		{
			Resources[i] = NULL;
			ResourceTypes[i] = (eResourceType)keytype->readi();
			if(ResourceTypes[i] != resNone)
				ResourceNames[i] = keyname->stringread();
			else
				ResourceNames[i] = NULL;
		}
	}

	return 1;

}

int CGEnPackage::GetResourceIndex( CGEnResource *els)
{
	for(int i=0;i<numResources;i++)
	{
		if(Resources[i] == els) return i;
	}
	return -1;
}

int CGEnPackage::GetResourceIndex( char *name )
{
	int index;
	for(index=0;index<numResources;index++)
		if(ResourceNames[index] && _stricmp(name, ResourceNames[index]) == 0) break;
	

	if(index==numResources) return -1; //neni v packagi

	return index;
}

int CGEnPackage::AddResource( char *name, CGEnResource *res, int newresource )
{
	int st=0;

	int newptr=0;

	if(!res) return 0;

	if(!FS->IsValidFilename(name)) return 0;

	if(_stricmp(name,"index")==0) return 0; //resource se nemuze jmenovat 'index'

	int index;
	for(index=0;index<numResources;index++)
	{
		if(ResourceNames[index] && _stricmp(name, ResourceNames[index]) == 0) break;
	}
	
	if(newresource)
	{
		if(index!=numResources) return 0; //uz je v packagi stejny jmeno -> error
	}else
	{
		if(index==numResources) return 0; //nebyl v packagi -> error
		if(res->GetType() != ResourceTypes[index] ) return 0; //nesouhlasi typy
		
		if( res != Resources[index] ) newptr=1; //zmenila se adresa
	}

	char *curdir=NULL;
	FS->GetCurDir(&curdir);

	if(FS->ChangeDir(PackageName))
	{
		st = res->Save(name,this);

		if(st){
			int i=0;
			
			if(newresource) //novy resource
			{
				for(i=0;i<numResources;i++)
					if(ResourceTypes[i] == resNone ) break;

				if(i>=numResources) {
					numResources++;
					char* *newresnames = new char*[numResources];
					eResourceType *newtypes = new eResourceType[numResources];
					CGEnResource* *newres = new CGEnResource*[numResources];

					for(int j=0;j<numResources-1;j++)
					{
						newres[j]=Resources[j];
						newtypes[j]=ResourceTypes[j];
						newresnames[j]=ResourceNames[j];
					}
					newres[i]=NULL;newtypes[i]=resNone;newresnames[i]=NULL;
					SAFE_DELETE_ARRAY(Resources);
					SAFE_DELETE_ARRAY(ResourceTypes);
					SAFE_DELETE_ARRAY(ResourceNames);

					Resources=newres;
					ResourceTypes=newtypes;
					ResourceNames=newresnames;
				}

				SAFE_DELETE_ARRAY(ResourceNames[i]);
				ResourceNames[i] = new char[strlen(name)+1];
				strcpy(ResourceNames[i], name);

			}else
			{
				i=index; //ulozim resource pod stejnym indexem
			}
			
			if(newresource||newptr)
			{
				res->AddRef();
				SAFE_RELEASE(Resources[i]);
			}

			Resources[i] = res;
			ResourceTypes[i] = res->GetType();

			WriteIndex();

		}
	
	}

	if(curdir){
		FS->ChangeDir(curdir);
		delete[] curdir;
	}

	return st;
}

int CGEnPackage::AddResourceFromFile( char *name, char *filename, eResourceType type, int newresource)
{
	int st=0;
	int compr=-1;

	switch(type)
	{
	case resTex:
		compr=0;
		break;
	case resElSource:
		break;
	case resMusicModule:
		break;
	case resMusicSample:
		break;
	default:
		return 0;
	}


	if(!filename) return 0;

	if(!FS->IsValidFilename(name)) return 0;

	if(_stricmp(name,"index")==0) return 0; //resource se nemuze jmenovat 'index'

	int index;
	for(index=0;index<numResources;index++)
	{
		if(ResourceNames[index] && _stricmp(name, ResourceNames[index]) == 0) break;
	}
	
	if(newresource)
	{
		if(index!=numResources) return 0; //uz je v packagi stejny jmeno -> error
	}else
	{
		if(index==numResources) return 0; //nebyl v packagi -> error
		if(type != ResourceTypes[index] ) return 0; //nesouhlasi typy
	}

	char *curdir=NULL;
	FS->GetCurDir(&curdir);

	if(FS->ChangeDir(PackageName))
	{
		if(type==resMusicModule)
			st = CMusicModule::SaveFromFile(name,filename,this,compr);
		else if(type==resMusicSample)
			st = CMusicSample::SaveFromFile(name,filename,this,compr);
		else{
			int sz = FS->GetFileSize(filename);		
			char *buf;
			if(sz>0){
				buf = new char[sz];
				st = FS->ReadFile(filename,buf,sz);
				if(st) st = FS->WriteFile(name,buf,sz,compr);
				delete[] buf;
			}
		}
		if(st){
			int i=0;
			
			if(newresource) //novy resource
			{
				for(i=0;i<numResources;i++)
					if(ResourceTypes[i] == resNone ) break;

				if(i>=numResources) {
					numResources++;
					char* *newresnames = new char*[numResources];
					eResourceType *newtypes = new eResourceType[numResources];
					CGEnResource* *newres = new CGEnResource*[numResources];

					for(int j=0;j<numResources-1;j++)
					{
						newres[j]=Resources[j];
						newtypes[j]=ResourceTypes[j];
						newresnames[j]=ResourceNames[j];
					}
					newres[i]=NULL;newtypes[i]=resNone;newresnames[i]=NULL;
					SAFE_DELETE_ARRAY(Resources);
					SAFE_DELETE_ARRAY(ResourceTypes);
					SAFE_DELETE_ARRAY(ResourceNames);

					Resources=newres;
					ResourceTypes=newtypes;
					ResourceNames=newresnames;
				}

				SAFE_DELETE_ARRAY(ResourceNames[i]);
				ResourceNames[i] = new char[strlen(name)+1];
				strcpy(ResourceNames[i], name);

			}else
			{
				i=index; //ulozim resource pod stejnym indexem
			}
			

			SAFE_RELEASE(Resources[i]);
			Resources[i] = NULL;
			ResourceTypes[i] = type;

			WriteIndex();

		}
	
	}

	if(curdir){
		FS->ChangeDir(curdir);
		delete[] curdir;
	}

	return st;
}

int CGEnPackage::DeleteResource( char *name )
{
	int ok=0;
	int index = GetResourceIndex(name);

	if(index==-1) return 0; //neni v packagi

	char *curdir=NULL;
	FS->GetCurDir(&curdir);

	if(FS->ChangeDir(PackageName) )
	{
		ok = FS->Delete(name);
		if(ok)
		{
			SAFE_DELETE_ARRAY(ResourceNames[index]);
			SAFE_RELEASE(Resources[index]);
			ResourceTypes[index] = resNone;

			ok=WriteIndex();
		}
	}

	if(curdir){
		FS->ChangeDir(curdir);
		delete[] curdir;
	}

	return ok;
}

int CGEnPackage::IsValidResourceName(char *name)
{
	if(!name) return 0;
	if(!FS->IsValidFilename(name)) return 0;

	if(_stricmp(name,"index")==0) return 0; //resource se nemuze jmenovat 'index'

	if(GetResourceIndex(name)!=-1) return 2; //resource je v packagi

	return 1;
}

CGEnResource* CGEnPackage::LoadResource( int index )
{
	int ok=0;
	if(index<0||index>=numResources) return NULL;

	if(Resources[index]) {
		if(!PackageLazyLoad) Resources[index]->ForceLoad();
		return Resources[index];
	}
	if(!ResourceNames[index] || ResourceTypes[index] == resNone ) return NULL;

	CGEnResource *res=NULL;
	
	res = CGEnResource::CreateResource( ResourceTypes[index] );	
	if(!res) return NULL;

	if(PackageLazyLoad && res->CanLazyLoad())
	{
		ok = res->Load(ResourceNames[index],this,PackageLazyLoad,index);
	}else
	{
		char *curdir=NULL;
		FS->GetCurDir(&curdir);

		if(FS->ChangeDir(PackageName) && res -> Load(ResourceNames[index],this,0) )
			ok=1;

		if(curdir){
			FS->ChangeDir(curdir);
			delete[] curdir;
		}
	}

	if(!ok) {res->Release();res=NULL;}
	else
		Resources[index]=res;

	return res;
}

int CGEnPackage::ForceLoad( int index )
{
	int ok=0;
	if(index<0||index>=numResources) return 0;

	if(!Resources[index]) return 0;		
	if(!ResourceNames[index] || ResourceTypes[index] == resNone ) return 0;

	CGEnResource *res=Resources[index];
	
	char *curdir=NULL;
	FS->GetCurDir(&curdir);

	if(FS->ChangeDir(PackageName) && res -> Load(ResourceNames[index],this,0) )
	{
		ok=1;
	}

	if(curdir){
		FS->ChangeDir(curdir);
		delete[] curdir;
	}

	return ok;
}


CGEnResource* CGEnPackage::LoadResource( char *name )
{

	int index = GetResourceIndex(name);

	if(index==-1) return 0; //neni v packagi

	return LoadResource(index);
}

eResourceType CGEnPackage::GetResourceType( char *name )
{
	int index = GetResourceIndex(name);

	if(index==-1) return resNone; //neni v packagi

	return ResourceTypes[index];
}

eResourceType CGEnPackage::GetResourceType( int index )
{
	if(index<0||index>=numResources) return resNone;
	return ResourceTypes[index];
}

const char* CGEnPackage::GetResourceName( int index )
{
	if(index<0||index>=numResources) return 0;
	return ResourceNames[index];
}

int CGEnPackage::UnloadResources()
{
	int empty;
	int del;

	do
	{
		del=0;
		empty=1;
		for(int i=0;i<numResources;i++)
		{
			if(Resources[i]) 
			{
				if(Resources[i]->GetRefCnt()==0)
				{
					SAFE_RELEASE(Resources[i]);
					del=1;
				}else
					empty = 0;
			}
		}
	}while(del);

	return empty;
}

//////////////////////////////////////////////////////////////////////////////

CPackageMgr::CPackageMgr()
{
	PackageHashTable = new CHashTable(PACKAGE_HASHTABLE_SIZE);
}


CPackageMgr::~CPackageMgr()
{
	UnloadAllPackages();
	SAFE_DELETE(PackageHashTable);
}

void CPackageMgr::UnloadAllPackages()
{
	if(PackageHashTable)
	{
		PackageHashTable->DeleteAllMembers();
	}
	DeleteListElems(PackageList);
}

void CPackageMgr::UnloadResources()
{
	CPGEnPackageListIter it(PackageList);
	CGEnPackage *pkg;

	int empty;

	while(!it.End())
	{
		pkg=it;
		empty = it->UnloadResources();
		if(empty)
		{
			CHashElem *hel = PackageHashTable->Member(pkg->GetName());
			if(hel)
			{
				hel->RemoveFromHashTable();
				delete hel;
			}
			delete pkg;
			it.Remove();
		}else
			it++;
	}
}

CGEnPackage* CPackageMgr::GetPackage(char *packagename)
{

	if(!packagename) return 0;

	CPackageHashEl* hel = (CPackageHashEl*) PackageHashTable->Member(packagename);
	
	if(hel) {
		return hel->GetPackage();
	}

	CGEnPackage *pkg = new CGEnPackage();

	if(!pkg->LoadPackage(packagename))
	{
		delete pkg; return 0;
	}


	hel = new CPackageHashEl(pkg);

	hel->AddToHashTable(PackageHashTable);
	PackageList.Add(pkg);

	return pkg;

}

int CPackageMgr::DeletePackage(char *packagename)
{

	if(!packagename) return 0;

	CPackageHashEl* hel = (CPackageHashEl*) PackageHashTable->Member(packagename);
	
	if(hel) {
		CGEnPackage *pkg = hel->GetPackage();
		PackageList.Delete(pkg);
		delete hel;
		delete pkg;
	}

	return FS->Delete(packagename);

}


CGEnResource* CPackageMgr::LoadResource( char *resourcename, eResourceType type, CGEnPackage **_pkg )
{

	if(!resourcename) return 0;

	char *name=NULL;
	char *resnm;

	if(!FS->GetFullPath(resourcename,&name)) return 0;

	if(!SplitResource( name, &resnm ) )
	{
		delete[] name;
		return 0;
	}
	
	CGEnPackage *pkg = GetPackage(name);
	if(!pkg){
		delete[] name;
		return 0;
	}

	CGEnResource *res=NULL;
	int index = pkg->GetResourceIndex(resnm); //zjistim index

	if(index!=-1 && pkg->GetResourceType(index)==type) //pokud je v pakazi a souhlasi typ -> naloaduju
		res = pkg->LoadResource(index);

	delete[] name;

	if(_pkg) *_pkg = pkg;

	return res;

}

int CPackageMgr::AddResource( char *resourcename, CGEnResource *res, int newresource )
{
	if(!resourcename) return 0;

	char *name=NULL;
	char *resnm;
	int st;

	if(!FS->GetFullPath(resourcename,&name)) return 0;

	if(!SplitResource( name, &resnm ) )
	{
		delete[] name;
		return 0;
	}
	
	CGEnPackage *pkg = GetPackage(name);
	if(!pkg){
		delete[] name;
		return 0;
	}

	st = pkg->AddResource(resnm,res,newresource);

	delete[] name;

	return st;
}

int CPackageMgr::AddResourceFromFile( char *resourcename, char *filename, eResourceType type, int newresource)
{
	if(!resourcename||!filename||type==resNone) return 0;

	char *name=NULL;
	char *resnm;
	int st;

	if(!FS->GetFullPath(resourcename,&name)) return 0;

	if(!SplitResource( name, &resnm ) )
	{
		delete[] name;
		return 0;
	}
	
	CGEnPackage *pkg = GetPackage(name);
	if(!pkg){
		delete[] name;
		return 0;
	}

	st = pkg->AddResourceFromFile(resnm,filename,type,newresource);

	delete[] name;

	return st;
}

int CPackageMgr::DeleteResource( char *resourcename )
{
	if(!resourcename) return 0;

	char *name=NULL;
	char *resnm;
	int st;

	if(!FS->GetFullPath(resourcename,&name)) return 0;

	if(!SplitResource( name, &resnm ) )
	{
		delete[] name;
		return 0;
	}
	
	CGEnPackage *pkg = GetPackage(name);
	if(!pkg){
		delete[] name;
		return 0;
	}

	st = pkg->DeleteResource(resnm);

	delete[] name;

	return st;
}


eResourceType CPackageMgr::GetResourceType( char *resourcename, CGEnPackage **_pkg )
{
	if(!resourcename) return resNone;

	char *name=NULL;
	char *resnm;

	if(!FS->GetFullPath(resourcename,&name)) return resNone;

	if(!SplitResource( name, &resnm ) )
	{
		delete[] name;
		return resNone;
	}
	
	CGEnPackage *pkg = GetPackage(name);
	if(!pkg){
		delete[] name;
		return resNone;
	}

	eResourceType type = pkg->GetResourceType(resnm);

	delete[] name;

	if(_pkg) *_pkg = pkg;

	return type;
}

eResourceType CPackageMgr::GetResourceType( char *packagename, char *resourcename, CGEnPackage **_pkg )
{
	if(!packagename || !resourcename) return resNone;

	char *name=NULL;

	if(!FS->GetFullPath(packagename,&name)) return resNone;
	
	CGEnPackage *pkg = GetPackage(name);
	if(!pkg){
		delete[] name;
		return resNone;
	}

	eResourceType type = pkg->GetResourceType(resourcename);

	delete[] name;

	if(_pkg) *_pkg = pkg;

	return type;
}

int CPackageMgr::SplitResource(char *name, char **resourcename)
{

	int tl,len = strlen(name);

	tl=len;

	while(len>=0)
	{
		if(name[len]=='/' || name[len]=='\\' ) break;
		len--;
	}

	if(len==0 || len==tl ) 
		return 0;

	name[len] = 0;
	*resourcename = name+len+1;

	return 1;
}




///////////////////////////////////////////////////////////////////////////////


CPackageHashEl::CPackageHashEl(CGEnPackage *_package)
{	
	package=_package;
	name = package->GetName();
}

