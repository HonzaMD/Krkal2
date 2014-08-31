// NewDirDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FSutil.h"
#include "NewDirDlg.h"


// CNewDirDlg dialog

IMPLEMENT_DYNAMIC(CNewDirDlg, CDialog)
CNewDirDlg::CNewDirDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewDirDlg::IDD, pParent)
	, editname(_T(""))
{
}

CNewDirDlg::~CNewDirDlg()
{
}

void CNewDirDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT, editname);
}


BEGIN_MESSAGE_MAP(CNewDirDlg, CDialog)
END_MESSAGE_MAP()


// CNewDirDlg message handlers
