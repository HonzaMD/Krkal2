// FSutilDlg.h : header file
//

#pragma once

#include "fs.h"
#include "afxwin.h"

// CFSutilDlg dialog
class CFSutilDlg : public CDialog
{
// Construction
public:
	CFSutilDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FSUTIL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButOpenArchive();

protected:
	CString ArchiveName;
	CFSArchive archive;

public:
	afx_msg void OnDestroy();
public:
	afx_msg void OnLbnDblclkArchivelist();
protected:
	int FillArchiveList(void);
public:
	afx_msg void OnBnClickedButroot();
	afx_msg void OnBnClickedButmkdir();
	afx_msg void OnBnClickedButparrent();
protected:
	virtual void OnOK(void);
	void AddFiles(int compress);
public:
	CListBox ArchiveList;
	afx_msg void OnBnClickedButaddfiles();
	afx_msg void OnBnClickedButdel();
	afx_msg void OnBnClickedButcreatearchive();
	afx_msg void OnBnClickedButrename();
	afx_msg void OnBnClickedButdefrag();
	afx_msg void OnBnClickedButaddcomprfiles();

};
