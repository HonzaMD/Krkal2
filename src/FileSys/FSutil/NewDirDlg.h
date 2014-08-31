#pragma once


// CNewDirDlg dialog

class CNewDirDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewDirDlg)

public:
	CNewDirDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewDirDlg();

// Dialog Data
	enum { IDD = IDD_NEWDIRDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString editname;
};
