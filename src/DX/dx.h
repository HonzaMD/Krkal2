//////////////////////////////////////////////////////////////////////
//
// DX.h
//
// initace directu, obsluha win. zprav,...
//
// upraveno ze vzorovych souboru k DirectX
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef DX_H
#define DX_H

#include "d3dfont.h"

#include "dxbliter.h"
#include "gui.h"
#include "input.h"
#include "genmain.h"

void DebugMessage(int row,DWORD color, const char *string,...);
void DebugMessage(int row, const char *string,...);

#define DEBUG_OUTPUT_ROWS 20
#define DEBUG_STRMAXLEN 256

class CDebugOutput{
public:

	CDebugOutput(){
		for(int i=0;i<DEBUG_OUTPUT_ROWS;i++) string[i]=0;
	}
	~CDebugOutput()
	{
		for(int i=0;i<DEBUG_OUTPUT_ROWS;i++) SAFE_DELETE_ARRAY(string[i]);
	}

	int GetNumRow(){return DEBUG_OUTPUT_ROWS;}
	char* GetString(int r){
		if(r<0||r>=DEBUG_OUTPUT_ROWS) return 0;
		return string[r];
	}
	char* AllocString(int r){
		if(r<0||r>=DEBUG_OUTPUT_ROWS) return 0;
		if(!string[r]) string[r]=new char[DEBUG_STRMAXLEN+1];
		return string[r];
	}
	DWORD GetStringColor(int r){return cols[r];}
	int SetColor(int r,DWORD color=0xFFFFFFFF){
		if(r<0||r>=DEBUG_OUTPUT_ROWS) return 0;

		cols[r]=color;
		
		return 1;
	}
protected:
	char *string[DEBUG_OUTPUT_ROWS];
	DWORD cols[DEBUG_OUTPUT_ROWS];
};

extern CDebugOutput *DebugOutput;


enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF };

#define D3DAPPERR_NODIRECT3D          0x82000001
#define D3DAPPERR_NOWINDOW            0x82000002
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
#define D3DAPPERR_NODESKTOPHAL        0x82000008
#define D3DAPPERR_NOHALTHISMODE       0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_MEDIANOTFOUND       0x8200000b
#define D3DAPPERR_RESIZEFAILED        0x8200000c


//-----------------------------------------------------------------------------
// Name: struct D3DModeInfo
// Desc: Structure for holding information about a display mode
//-----------------------------------------------------------------------------
struct D3DModeInfo
{
    DWORD      Width;      // Screen width in this mode
    DWORD      Height;     // Screen height in this mode
    D3DFORMAT  Format;     // Pixel format in this mode
    DWORD      dwBehavior; // Hardware / Software / Mixed vertex processing
    D3DFORMAT  DepthStencilFormat; // Which depth/stencil format to use with this mode
};




//-----------------------------------------------------------------------------
// Name: struct D3DDeviceInfo
// Desc: Structure for holding information about a Direct3D device, including
//       a list of modes compatible with this device
//-----------------------------------------------------------------------------
struct D3DDeviceInfo
{
    // Device data
    D3DDEVTYPE   DeviceType;      // Reference, HAL, etc.
    D3DCAPS8     d3dCaps;         // Capabilities of this device
    const TCHAR* strDesc;         // Name of this device
    BOOL         bCanDoWindowed;  // Whether this device can work in windowed mode

    // Modes for this device
    DWORD        dwNumModes;
    D3DModeInfo  modes[150];

    // Current state
    DWORD        dwCurrentMode;
    BOOL         bWindowed;
    D3DMULTISAMPLE_TYPE MultiSampleType;
};




//-----------------------------------------------------------------------------
// Name: struct D3DAdapterInfo
// Desc: Structure for holding information about an adapter, including a list
//       of devices available on this adapter
//-----------------------------------------------------------------------------
struct D3DAdapterInfo
{
    // Adapter data
    D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
    D3DDISPLAYMODE d3ddmDesktop;      // Desktop display mode for this adapter

    // Devices for this adapter
    DWORD          dwNumDevices;
    D3DDeviceInfo  devices[5];

    // Current state
    DWORD          dwCurrentDevice;
};

class CDXapp{
friend class CKrkal;
public:
	D3DCAPS8          m_d3dCaps;           // Caps for the device

protected:

	LPDIRECT3D8 m_pD3D;				 // The main D3D object
	LPDIRECT3DDEVICE8 m_pd3dDevice;  // The D3D rendering device
	D3DPRESENT_PARAMETERS m_d3dpp;   // Parameters for CreateDevice/Reset
    D3DSURFACE_DESC   m_d3dsdBackBuffer;   // Surface desc of the backbuffer

    D3DAdapterInfo    m_Adapters[10];
    DWORD             m_dwNumAdapters;
    DWORD             m_dwAdapter;
    
	HWND	m_hWnd;					// The main app window
	char*	m_strWindowTitle;		// Title for the app's window

	DWORD	width;		// Width used to create window
    DWORD	height;		// Height used to create window

	DWORD   m_dwCreateFlags;		// Indicate sw or hw vertex processing
	DWORD   m_dwWindowStyle;        // Saved window style for mode switches
	RECT    m_rcWindowBounds;		// Saved window bounds for mode switches
    RECT    m_rcWindowClient;		// Saved client area size for mode switches
	DWORD   m_dwMinWidth,m_dwMinHeight;

    BOOL    m_bActive;
    BOOL    m_bReady;
	BOOL	m_bWindowed;
	BOOL	m_bShowCursorWhenFullscreen;
    BOOL    m_bUseDepthBuffer;   // Whether to autocreate depthbuffer
    DWORD   m_dwMinDepthBits;    // Minimum number of bits needed in depth buffer
    DWORD   m_dwMinStencilBits;  // Minimum number of bits needed in stencil buffer
	
    // Variables for timing
    FLOAT             m_fTime;             // Current time in seconds
    FLOAT             m_fElapsedTime;      // Time elapsed since last frame
    FLOAT             m_fFPS;              // Instanteous frame rate
    TCHAR             m_strDeviceStats[90];// String to hold D3D device stats
    TCHAR             m_strFrameStats[40]; // String to hold frame stats
    
	CD3DFont*		  m_pFont;              // Font for drawing text

	CDXbliter* DXbliter; //DXbliter

	// Internal variables used for timing
    BOOL    m_bFrameMoving;
    BOOL    m_bSingleStep;

	HRESULT DisplayErrorMsg( HRESULT hr, DWORD dwType );
	HRESULT Initialize3DEnvironment();
	HRESULT AdjustWindowForChange();
	HRESULT BuildDeviceList();
	HRESULT ToggleFullscreen();
	HRESULT ForceWindowed();
	HRESULT Resize3DEnvironment();
    BOOL    FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );

	HRESULT ConfirmDevice(D3DCAPS8*,DWORD,D3DFORMAT fmt)   { 
		//if(fmt!=D3DFMT_X8R8G8B8) return E_FAIL; 
		return S_OK; 
	}
	HRESULT InitDeviceObjects()                        { 
		m_pFont->InitDeviceObjects( m_pd3dDevice );
		DXbliter->InitDeviceObjects(m_pd3dDevice);return S_OK; 
	}
	HRESULT InvalidateDeviceObjects()                  { 
		m_pFont->InvalidateDeviceObjects();
		DXbliter->InvalidateDeviceObjects();
		return S_OK; 
	}
    HRESULT DeleteDeviceObjects()                      { 
		 m_pFont->DeleteDeviceObjects();
		DXbliter->DeleteDeviceObjects();return S_OK; }
	HRESULT RestoreDeviceObjects() { 
	    m_pFont->RestoreDeviceObjects();
		DXbliter->RestoreDeviceObjects();return S_OK; 
	}

	HRESULT Render();

	void Cleanup();

	int showwincur;

	TRACKMOUSEEVENT mousetrack;

public:

	CDXapp();
	~CDXapp(){Cleanup();};

	HRESULT Create( HINSTANCE hInstance );

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	int GetScreenRes(int &sx,int &sy);
	int ChangeScreenRes(int sx,int sy);

	CInput* input;

	LPDIRECT3DDEVICE8 GetD3DDevice() {return m_pd3dDevice;}
 
};

extern CDXapp* g_pDXapp;


enum TIMER_COMMAND { TIMER_RESET, TIMER_START, TIMER_STOP, TIMER_ADVANCE,
                     TIMER_GETABSOLUTETIME, TIMER_GETAPPTIME, TIMER_GETELAPSEDTIME };
FLOAT DXUtil_Timer( TIMER_COMMAND command );

HRESULT D3DUtil_SetDeviceCursor( LPDIRECT3DDEVICE8 pd3dDevice, HCURSOR hCursor );

#endif