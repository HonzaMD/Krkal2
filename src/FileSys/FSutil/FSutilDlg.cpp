// FSutilDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FSutil.h"
#include "FSutilDlg.h"
#include "NewDirDlg.h"
#include "PathDialog.h"
#include <io.h>
#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFSutilDlg dialog



CFSutilDlg::CFSutilDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFSutilDlg::IDD, pParent)
	, ArchiveName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFSutilDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ARCHIVENAME, ArchiveName);
	DDX_Control(pDX, IDC_ARCHIVELIST, ArchiveList);
	DDX_Control(pDX, IDC_ARCHIVELIST, ArchiveList);
}

BEGIN_MESSAGE_MAP(CFSutilDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTOPENARCHIVE, OnBnClickedButOpenArchive)
//	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_LBN_DBLCLK(IDC_ARCHIVELIST, OnLbnDblclkArchivelist)
	ON_BN_CLICKED(IDC_BUTROOT, OnBnClickedButroot)
	ON_BN_CLICKED(IDC_BUTMKDIR, OnBnClickedButmkdir)
	ON_BN_CLICKED(IDC_BUTPARRENT, OnBnClickedButparrent)
	ON_BN_CLICKED(IDC_BUTADDFILES, OnBnClickedButaddfiles)
	ON_BN_CLICKED(IDC_BUTDEL, OnBnClickedButdel)
	ON_BN_CLICKED(IDC_BUTCREATEARCHIVE, OnBnClickedButcreatearchive)
	ON_BN_CLICKED(IDC_BUTRENAME, OnBnClickedButrename)
	ON_BN_CLICKED(IDC_BUTDEFRAG, OnBnClickedButdefrag)
	ON_BN_CLICKED(IDC_BUTADDCOMPRFILES, OnBnClickedButaddcomprfiles)
END_MESSAGE_MAP()


// CFSutilDlg message handlers

BOOL CFSutilDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	int ts[]={150,200,280};
	ArchiveList.SetTabStops(3,ts);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFSutilDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFSutilDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFSutilDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFSutilDlg::OnBnClickedButOpenArchive()
{
	char szFilters[]="All files (*.*)|*.*";

	CFileDialog fileDlg(TRUE,0,NULL,OFN_FILEMUSTEXIST| OFN_HIDEREADONLY|OFN_EXPLORER, szFilters, this);

	if( fileDlg.DoModal ()==IDOK )
	{
		CString arname = fileDlg.GetFileName();

		ArchiveName="";

		ArchiveList.ResetContent();

		archive.Close();
		if(archive.Open(arname,0))
		{
			ArchiveName=arname;

			FillArchiveList();

			
		}
		UpdateData(FALSE);

	}

}

void CFSutilDlg::OnDestroy()
{
	CDialog::OnDestroy();

	archive.Close();
}

void CFSutilDlg::OnLbnDblclkArchivelist()
{
	int it=ArchiveList.GetCurSel();
	int itm;
	if(it>=0)
	{

		itm=(int)ArchiveList.GetItemData(it);
		int dir;
		const char *name;
		archive.GetCurDirItem(itm,&name,dir);
		
		if(dir)
		{
			archive.ChangeDir(name);
			FillArchiveList();

		}
	}
}

int CFSutilDlg::FillArchiveList(void)
{
	int ni=archive.GetCurDirNumItems();
	const char *name;
	int dir,it,sz;
	CString s;
	FILETIME time,ltime;
	SYSTEMTIME time2;

	ArchiveList.ResetContent();

	for(int i=0;i<ni;i++)
	{
		archive.GetCurDirItem(i,&name,dir);

		archive.GetTime(name,time);


		FileTimeToLocalFileTime(&time,&ltime);
		FileTimeToSystemTime(&ltime,&time2);
	
		if(dir) s.Format("%s\t<DIR>\t%02i.%02i.%04i %02i:%02i:%02i",name,time2.wDay,time2.wMonth,time2.wYear,time2.wHour,time2.wMinute,time2.wSecond);
		else{
			sz=archive.GetFileSize(name);
			if(archive.IsCompressed(name))
				s.Format("%s\t%i\t%02i.%02i.%04i %02i:%02i:%02i\tcompressed",name,sz,time2.wDay,time2.wMonth,time2.wYear,time2.wHour,time2.wMinute,time2.wSecond);
			else
				s.Format("%s\t%i\t%02i.%02i.%04i %02i:%02i:%02i",name,sz,time2.wDay,time2.wMonth,time2.wYear,time2.wHour,time2.wMinute,time2.wSecond);
		}

		it=ArchiveList.AddString(s);
		ArchiveList.SetItemData(it,i);
	}

	ArchiveList.SetCurSel(0);

	return 1;
}

void CFSutilDlg::OnBnClickedButroot()
{
	archive.ChangeDir("/");
	FillArchiveList();
}

void CFSutilDlg::OnBnClickedButparrent()
{
	archive.ChangeDir("..");
	FillArchiveList();
}

void CFSutilDlg::OnBnClickedButmkdir()
{
	CNewDirDlg dlg;

	if(dlg.DoModal()==IDOK)
	{
		archive.CreateDir(dlg.editname);
		FillArchiveList();
	}
}

void CFSutilDlg::OnOK(void)
{
}

void CFSutilDlg::AddFiles(int compress)
{

	char szFilters[]="All files (*.*)|*.*";

	CFileDialog fileDlg(TRUE,NULL,NULL,OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT|OFN_EXPLORER, szFilters, this);

	char szFile[32000]="";

	fileDlg.m_ofn.lpstrFile=szFile;
	fileDlg.m_ofn.nMaxFile=sizeof(szFile);

	FILETIME time;
	HANDLE hFile;

	if( fileDlg.DoModal ()==IDOK )
	{
		CString pathName = fileDlg.GetPathName();
		CString fn,fn2;
		int i;
		int len;
		char *buf=NULL;
		FILE *f;

		POSITION p=fileDlg.GetStartPosition();
		while(p)
		{
			fn=fileDlg.GetNextPathName(p);
			for(i=fn.GetLength()-1;i>=0;i--)
				if(fn[i]=='\\'||fn[i]=='/') break;

			i++;
			fn2=fn.Right(fn.GetLength()-i);

			f=fopen(fn,"rb");
			if(f)
			{
				len=_filelength(_fileno(f));
				buf=new char[len];
				if(buf)
				{
					fread(buf,len,1,f);					
					archive.WriteFile(fn2,buf,len,compress);
				}
				delete[] buf;
				fclose(f);

				hFile = CreateFile (
				    fn,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_BACKUP_SEMANTICS,
					NULL
				);

				if(hFile!=INVALID_HANDLE_VALUE)
				{
					GetFileTime(hFile, NULL, NULL, &time);
					CloseHandle(hFile); 	
					archive.SetTime(fn2,time);
				}

			}
		}

		FillArchiveList();

	}

	
}

void CFSutilDlg::OnBnClickedButdel()
{
	int num=ArchiveList.GetCount();
	int itm,dir;
	const char *name;

	for(int i=0;i<num;i++)
		if(ArchiveList.GetSel(i))
		{
			itm=(int)ArchiveList.GetItemData(i);
			archive.GetCurDirItem(itm,&name,dir);
			if(strcmp(name,"..")!=0)
				archive.Delete(name);
		}

	FillArchiveList();

}

void CFSutilDlg::OnBnClickedButcreatearchive()
{
	char szFilters[]="All files (*.*)|*.*";

	CFileDialog fileDlg(FALSE,0,NULL,OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY|OFN_EXPLORER, szFilters, this);

	if( fileDlg.DoModal ()==IDOK )
	{
		CString arname = fileDlg.GetFileName();

		ArchiveName="";

		ArchiveList.ResetContent();

		archive.Close();
		if(archive.Create(arname))
		{
			ArchiveName=arname;

			FillArchiveList();

			
		}
		UpdateData(FALSE);

	}
}

void CFSutilDlg::OnBnClickedButrename()
{
	CPathDialog dlg;

	if(ArchiveList.GetSelCount()!=1) return;

	int it=ArchiveList.GetCurSel();
	
	if(it>=0)
	{

		int itm=(int)ArchiveList.GetItemData(it);
		int dir;
		const char *name;
		archive.GetCurDirItem(itm,&name,dir);

		if(strcmp(name,"..")==0) return;

		dlg.Path=name;

		if(dlg.DoModal()==IDOK)
		{
			if(archive.Rename(name,dlg.Path))
				FillArchiveList();
			else
				MessageBox("Can't rename !","Error!");
		}
		
	}

}

void CFSutilDlg::OnBnClickedButdefrag()
{
	if(!archive.Defragment())
		MessageBox("Error in defrag!","Fatal error!");
}


void CFSutilDlg::OnBnClickedButaddfiles()
{
	AddFiles(0);
}

void CFSutilDlg::OnBnClickedButaddcomprfiles()
{
	AddFiles(1);
}
