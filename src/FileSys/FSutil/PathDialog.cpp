// PathDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FSutil.h"
#include "PathDialog.h"


// CPathDialog dialog

IMPLEMENT_DYNAMIC(CPathDialog, CDialog)
CPathDialog::CPathDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPathDialog::IDD, pParent)
	, Path(_T(""))
{
}

CPathDialog::~CPathDialog()
{
}

void CPathDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITPATH, Path);
}


BEGIN_MESSAGE_MAP(CPathDialog, CDialog)
END_MESSAGE_MAP()


// CPathDialog message handlers
