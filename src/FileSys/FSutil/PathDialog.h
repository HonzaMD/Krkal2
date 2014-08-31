#pragma once


// CPathDialog dialog

class CPathDialog : public CDialog
{
	DECLARE_DYNAMIC(CPathDialog)

public:
	CPathDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPathDialog();

// Dialog Data
	enum { IDD = IDD_PATHDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString Path;
};
