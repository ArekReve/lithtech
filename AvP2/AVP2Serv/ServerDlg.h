#if !defined(AFX_SERVERDLG_H__CB06D3C1_2B86_11D2_860A_00609719A842__INCLUDED_)
#define AFX_SERVERDLG_H__CB06D3C1_2B86_11D2_860A_00609719A842__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ServerDlg.h : header file
//


// Includes...

#include <afxtempl.h>
#include "Resource.h"
#include "lithtech.h"
#include "server_interface.h"
#include <strstrea.h>
#include "MultiplayerMgrDefs.h"

struct CPlayerInfo
{
	CPlayerInfo( )
	{
		m_nClientId = -1;
		m_szPlayerHandle[0] = '\0';
		m_nKills = 0;
		m_nScore = 0;
		m_nLives = 0;
		m_nRace = 0;
		m_bController = 0;
		m_timeOnServer.GetCurrentTime( );
	}

	CPlayerInfo( const CPlayerInfo &playerInfo )
	{
		CopyData( playerInfo );
	}

	CPlayerInfo &operator=( const CPlayerInfo &playerInfo )
	{
		CopyData( playerInfo );
		return *this;
	}

	void CopyData( const CPlayerInfo &playerInfo )
	{
		m_nClientId = playerInfo.m_nClientId;
		strcpy( m_szPlayerHandle, playerInfo.m_szPlayerHandle );
		m_nKills = playerInfo.m_nKills;
		m_nScore = playerInfo.m_nScore;
		m_nLives = playerInfo.m_nLives;
		m_nRace = playerInfo.m_nRace;
		m_bController = playerInfo.m_bController;
		m_timeOnServer = playerInfo.m_timeOnServer;
	}

	int			m_nClientId;
	char		m_szPlayerHandle[30];
	int			m_nKills;
	int			m_nScore;
	int			m_nLives;
	char		m_nRace;
	char		m_bController;
	CTime		m_timeOnServer;
};


class CGameServerAppHandler : public ServerAppHandler
{
public:
	LTRESULT ShellMessageFn( char* pInfo, uint32 nLen );
	LTRESULT ConsoleOutputFn(char* pInfo);
	LTRESULT OutOfMemory();
	LTRESULT ProcessPacket(char *pData, uint32 dataLen, uint8 senderAddr[4], uint16 senderPort);
};

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog

class CServerDlg : public CDialog
{
public:

// Construction
public:
	CServerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerDlg();

// Dialog Data
	//{{AFX_DATA(CServerDlg)
	enum { IDD = IDD_GAMESERVER };
	CButton	m_SelectLevel;
	CButton	m_PlayerBoot;
	CListCtrl	m_Players;
	CListCtrl	m_Levels;
	CEdit	m_edConsole;
	CString	m_sServerName;
	CString m_sGameType;
	CString	m_sServerTime;
	CString	m_sTimeInLevel;
	CString	m_sNumPlayers;
	DWORD	m_nPeakPlayers;
	DWORD	m_nTotalPlayers;
	DWORD	m_nAveragePing;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerDlg)
	public:
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnConsoleSend();
	afx_msg void OnConsoleClear();
	afx_msg void OnCommandsNextLevel();
	afx_msg void OnCommandsSelectLevel();
	afx_msg void OnPlayersBoot();
	afx_msg void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDblclkLevels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnStopserver();
	afx_msg void OnItemchangedLevels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedPlayers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStartup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	
	// Member functions...

public:

	BOOL				IsLoaded() { return( m_pServerMgr != NULL ); }
	BOOL				IsRunning() { return( m_pServerMgr != NULL && m_hThread != NULL ); }

	void				RemoveMessage(int nMsg, int nMax);

	void				WriteConsoleString(LPCTSTR pMsg, ...);
	void				WriteConsoleString(int nStringID);

	CRITICAL_SECTION*	GetCS() {return &m_CS;}

	BOOL				SetSelectedWorlds( );

	ServerInterface *	GetServer( ) { return m_pServerMgr; }

	// Messages handlers from the shell.  Called within the shell's thread.
	void				OnShellMessage(char* pMsg, uint32 nLen);
	void				OnShellConsoleOutput(char* pMsg);
	void				OnShellFatalError( int nStringId );
	void				OnShellProcessNetPacket(char* sData, DWORD nLen, BYTE senderAddr[4], DWORD senderPort);

private:

	static void			ThreadUpdate( void *pParam );
	
	// Update function called by thread.
	void				ServerUpdate();

	// Message handlers from the shell.  Called within the shell's thread.
	void				OnShellAddClient( istrstream &msg );
	void				OnShellRemoveClient( istrstream &msg );
	void				OnShellUpdate( istrstream &msg );
	void				OnShellPreLoadWorld( istrstream &msg );
	void				OnShellPostLoadWorld( istrstream &msg );

	void				KillThread( );
	BOOL				SetService( );
	DDWORD				DoHost( );
	BOOL				ConfirmStop( );
	void				UpdateUI();

	BOOL				AddResources( );
	BOOL				LoadServer( );
	BOOL				RunServer( );
	BOOL				StopServer( );
	BOOL				SelectLevel( int nLevelIndex );

	CString				FormatTime( CTimeSpan &timeSpan );

	// Parses command line for certain parameters.  Set to NULL if not needed.
	void				ParseCommandLine( CString* pConfig, MPGameInfo* pGameInfo );


	// Member variables...

private:
	CRITICAL_SECTION	m_CS; // So we don't switch levels and update at the same time.
	BOOL				m_bCSInitted;

	BOOL				m_bFirstShow;

	CTime				m_serverStartTime;
	CTimeSpan			m_serverRunTime;
	CTime				m_levelStartTime;
	CTimeSpan			m_levelRunTime;
	HANDLE				m_hThread;

	int					m_nCurLevel;

	DWORD				m_nMaxPlayers;
	DWORD				m_nGamePlayers;

	// These parameters are directly updated by the server thread.
	int					m_nCurLevel_Shell;
	CStringList			m_lstShellMsgs_Shell;
	BOOL				m_bStopServer_Shell;
	BOOL				m_bPlayerUpdate_Shell;
	BOOL				m_bChangingLevels_Shell;
	BOOL				m_bChangedLevel_Shell;
	CMapWordToPtr		m_mapPlayerInfo_Shell;

	CString				m_sFullTcpIpAddress;
	BOOL				m_bConfirmExit;

	ServerInterface *	m_pServerMgr;
	CGameServerAppHandler m_AppHandler;

	CImageList *		m_pImageList;

	// Event which signals that the server should stop
	HANDLE				m_hStopEvent;
};

extern CServerDlg *g_pDialog;


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERDLG_H__CB06D3C1_2B86_11D2_860A_00609719A842__INCLUDED_)
