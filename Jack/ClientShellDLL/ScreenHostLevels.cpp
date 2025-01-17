// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostLevels.cpp
//
// PURPOSE : Interface screen for choosing levels for a hosted game
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostLevels.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "VarTrack.h"
#include "NetDefs.h"
#include "profileMgr.h"
#include "clientmultiplayermgr.h"
#include "WinUtil.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;



namespace
{
	uint8 nListFontSize = 12;
	int nAvailWidth = 0;
	int nSelWidth = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostLevels::CScreenHostLevels()
{
    m_pAvailMissions	= LTNULL;
    m_pSelMissions    = LTNULL;
	m_pAdd			= LTNULL;
	m_pRemove		= LTNULL;
	m_pAddAll		= LTNULL;
	m_pRemoveAll    = LTNULL;

}

CScreenHostLevels::~CScreenHostLevels()
{
	Term();
}

// Build the screen
LTBOOL CScreenHostLevels::Build()
{
	LTIntPt addPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"AddPos");
	LTIntPt removePos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"RemovePos");
	LTIntPt commandPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"CommandPos");

	if (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,"ListFontSize"))
		nListFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListFontSize");

	CreateTitle(IDS_TITLE_HOST_MISSIONS);

	m_pAdd = AddTextItem(IDS_HOST_ADD_MISSION, CMD_ADD_LEVEL, IDS_HELP_ADD_MISSION, addPos, LTTRUE);
	
	m_pAddAll = AddTextItem(IDS_HOST_ADD_ALL, CMD_ADD_ALL, IDS_HELP_ADD_ALL, commandPos);
	m_pRemoveAll = AddTextItem(IDS_HOST_REMOVE_ALL, CMD_REMOVE_ALL, IDS_HELP_REM_ALL);

	m_pRemove = AddTextItem(IDS_HOST_REMOVE_MISSION, CMD_REMOVE_LEVEL, IDS_HELP_REM_MISSION, removePos, LTTRUE);

	
	LTRect rcAvailRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"AvailRect");
	int nListHeight = (rcAvailRect.bottom - rcAvailRect.top);
	nAvailWidth = (rcAvailRect.right - rcAvailRect.left);

	m_pAvailMissions = AddList(LTIntPt(rcAvailRect.left,rcAvailRect.top),nListHeight,LTTRUE,nAvailWidth);
	m_pAvailMissions->SetIndent(LTIntPt(5,5));
	m_pAvailMissions->SetFrameWidth(2);
	m_pAvailMissions->SetScrollWrap(LTFALSE);

	LTRect rcSelRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"SelectRect");
	nListHeight = (rcSelRect.bottom - rcSelRect.top);
	nSelWidth = (rcSelRect.right - rcSelRect.left);

	m_pSelMissions = AddList(LTIntPt(rcSelRect.left,rcSelRect.top),nListHeight,LTTRUE,nSelWidth);
	m_pSelMissions->SetIndent(LTIntPt(5,5));
	m_pSelMissions->SetFrameWidth(2);
	m_pSelMissions->SetScrollWrap(LTFALSE);

	m_nextPos.y += nListHeight;
	uint16 nListWidth = nSelWidth-16;


 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	

	return LTTRUE;

}

void CScreenHostLevels::Escape()
{
	CBaseScreen::Escape();
}



uint32 CScreenHostLevels::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_BACK:
		{
			m_pScreenMgr->EscapeCurrentScreen();
			break;
		}

	case CMD_ADD_LEVEL:
		{
			char sMission[256] = "";

			if (m_pAvailMissions->GetSelectedIndex() >= 0 && (m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS))
			{
				CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailMissions->GetSelectedControl();
				if (pCtrl)
				{
					AddMissionToList(pCtrl->GetParam1());
					m_pSelMissions->ClearSelection();
				}
			}

			UpdateButtons();
		} break;
	case CMD_ADD_ALL:
		{
			if (m_pAvailMissions->GetNumControls())
			{
				for (int i = 0; i < m_pAvailMissions->GetNumControls() && (m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS); i++)
				{
					char sMission[256] = "";
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailMissions->GetControl(i);
					if (pCtrl)
					{
						AddMissionToList(pCtrl->GetParam1());
					}
				}
			}
			m_pSelMissions->ClearSelection();
			UpdateButtons();
		} break;
	case CMD_REMOVE_LEVEL:
		{
			int nIndex = m_pSelMissions->GetSelectedIndex();
			if (nIndex >= 0)
			{
				m_pSelMissions->ClearSelection();
				m_pSelMissions->RemoveControl(nIndex);
				int numLeft = m_pSelMissions->GetNumControls();
				if (numLeft > 0)
				{
					if (nIndex >= numLeft)
						nIndex = numLeft-1;
					m_pSelMissions->SetSelection(nIndex);
				}
			}
			UpdateButtons();

		} break;
	case CMD_REMOVE_ALL:
		{
			if (m_pSelMissions->GetNumControls() > 0)
			{
				m_pSelMissions->ClearSelection();
				m_pSelMissions->RemoveAll();
			}
			UpdateButtons();
		} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenHostLevels::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{

		CUserProfile* pUserProfile = g_pProfileMgr->GetCurrentProfile( );
		m_sCampaignFile = GetCampaignFile( pUserProfile->m_ServerGameOptions );
		
		if(!CWinUtil::FileExist( m_sCampaignFile.c_str() ) )
		{
			//TODO handle more cleanly
			g_pLTClient->CPrint("Could not load campaign file %s.",  m_sCampaignFile.c_str() );
			m_sCampaignFile = "";
		}



		FillAvailList();

		LoadMissionList();

		if (!m_pSelMissions->GetNumControls())
		{
			MakeDefaultMissionList();
		}

		m_bLoopMissions = LTTRUE;

		UpdateButtons();
        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		if (m_sCampaignFile.length())
		{
			SaveMissionList();
		}
		m_pAvailMissions->RemoveAll();
		m_pSelMissions->RemoveAll();
	}
	CBaseScreen::OnFocus(bFocus);

}

LTBOOL CScreenHostLevels::FillAvailList()
{
	// Sanity checks...

    if (!m_pAvailMissions) return(LTFALSE);


	for (int nMission = 0; nMission < g_pMissionButeMgr->GetNumMissions(); nMission++)
	{
		MISSION* pMission = g_pMissionButeMgr->GetMission(nMission);
		if (pMission)
		{
			char szWorldTitle[MAX_PATH] = "";
			_splitpath( pMission->aLevels[0].szLevel, NULL, NULL, szWorldTitle, NULL );

			bool bAdd = false;
			switch (g_pGameClientShell->GetGameType())
			{
			case eGameTypeDeathmatch:
			case eGameTypeTeamDeathmatch:
				bAdd = true;
				break;
			case eGameTypeDemolition:
				if (strnicmp(szWorldTitle,"DE_",3) == 0)
				{
					bAdd = true;
				}
				break;
			case eGameTypeDoomsDay:
				if (strnicmp(szWorldTitle,"DD_",3) == 0)
				{
					bAdd = true;
				}
				break;
			default:
				bAdd = false;
				break;
			}


			if (bAdd)
			{
				CLTGUITextCtrl *pCtrl = NULL;
				if (pMission->nNameId > 0)
					pCtrl = CreateTextItem(LoadTempString(pMission->nNameId),CMD_ADD_LEVEL,pMission->nDescId);
				else if (!pMission->sName.empty())
					pCtrl = CreateTextItem((char *)pMission->sName.c_str(),CMD_ADD_LEVEL,pMission->nDescId);
				else
				{
					pCtrl = CreateTextItem(szWorldTitle,CMD_ADD_LEVEL,0);
				}
				pCtrl->SetFont(LTNULL,nListFontSize);
				pCtrl->SetParam1(nMission);
				pCtrl->SetFixedWidth(nAvailWidth,LTTRUE);
				m_pAvailMissions->AddControl(pCtrl);

			}
		}
	}


    return (LTTRUE);
}

void CScreenHostLevels::LoadMissionList()
{
	// Sanity checks...

	if (!m_pSelMissions) return;

	char szString[256];
	CWinUtil::WinGetPrivateProfileString( "MissionList", "LoopMissions", "0", szString, ARRAY_LEN( szString ), m_sCampaignFile.c_str());

	m_bLoopMissions = ( atoi(szString) > 0 );

	char szKey[64];
	uint8 numMissions = 0;
	do
	{
		sprintf(szKey,"Mission%d",numMissions);
		CWinUtil::WinGetPrivateProfileString( "MissionList", szKey, "", szString, ARRAY_LEN( szString ), m_sCampaignFile.c_str());
		if (strlen(szString))
		{
			int nMissionId = atoi(szString);
			AddMissionToList(nMissionId);
			numMissions++;
		}

	} while (strlen(szString));

}

void CScreenHostLevels::MakeDefaultMissionList()
{
	// Sanity checks...

	if (!m_pAvailMissions) return;
	if (!m_pSelMissions) return;

	for (int nMission = 0; nMission < g_pMissionButeMgr->GetNumMissions(); nMission++)
	{
		MISSION *pMission = g_pMissionButeMgr->GetMission(nMission);
		if (pMission)
		{
			char szWorldTitle[MAX_PATH] = "";
			_splitpath( pMission->aLevels[0].szLevel, NULL, NULL, szWorldTitle, NULL );

			switch (g_pGameClientShell->GetGameType())
			{
			case eGameTypeDeathmatch:
			case eGameTypeTeamDeathmatch:
				if (strnicmp(szWorldTitle,"DM_",3) == 0 || 
					strnicmp(szWorldTitle,"DE_",3) == 0 )
				{
					AddMissionToList(nMission);
				}
				break;
			case eGameTypeDemolition:
				if (strnicmp(szWorldTitle,"DE_",3) == 0)
				{
					AddMissionToList(nMission);
				}
				break;
			case eGameTypeDoomsDay:
				if (strnicmp(szWorldTitle,"DD_",3) == 0)
				{
					AddMissionToList(nMission);
				}
				break;
			default:
				AddMissionToList(nMission);
				break;
			}
		}
		
	}

}

void CScreenHostLevels::SaveMissionList()
{
	// Sanity checks...

	if (!m_pSelMissions) return;

	remove(m_sCampaignFile.c_str());

	char szString[256];
	char szNum[4];

	sprintf(szNum, "%d", (m_bLoopMissions ? 1 : 0) );
	CWinUtil::WinWritePrivateProfileString( "MissionList", "LoopMissions", szNum, m_sCampaignFile.c_str());

	CWinUtil::WinWritePrivateProfileString( "MissionList", "MissionSourceFile", g_pMissionButeMgr->GetAttributeFile(), m_sCampaignFile.c_str());

	for (int n = 0; n < m_pSelMissions->GetNumControls(); n++)
	{
		CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSelMissions->GetControl(n);
		if (pCtrl)
		{
			sprintf(szString,"Mission%d",n);
			sprintf(szNum,"%d",pCtrl->GetParam1());
			CWinUtil::WinWritePrivateProfileString( "MissionList", szString, szNum, m_sCampaignFile.c_str());
		}
	}

	// Flush the file.
	CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, m_sCampaignFile.c_str());
}			
		



void CScreenHostLevels::UpdateButtons()
{
	m_pAddAll->Enable( m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS && m_pAvailMissions->GetNumControls() > 0);
	m_pRemoveAll->Enable(m_pSelMissions->GetNumControls() > 0);
		
}

void CScreenHostLevels::AddMissionToList(int nMissionId)
{
	// Sanity checks...

	if (!m_pSelMissions) return;
	if (m_pSelMissions->GetNumControls() == MAX_GAME_LEVELS) return;


	// Add the level to the list...

	MISSION *pMission = g_pMissionButeMgr->GetMission(nMissionId);

	if (pMission)
	{
		
		CLTGUITextCtrl *pCtrl = NULL;
		if (pMission->nNameId > 0)
			pCtrl = CreateTextItem(LoadTempString(pMission->nNameId),CMD_REMOVE_LEVEL,pMission->nDescId);
		else if (!pMission->sName.empty())
			pCtrl = CreateTextItem((char *)pMission->sName.c_str(),CMD_REMOVE_LEVEL,pMission->nDescId);
		else
		{
			char szWorldTitle[MAX_PATH] = "";
			_splitpath( pMission->aLevels[0].szLevel, NULL, NULL, szWorldTitle, NULL );
			pCtrl = CreateTextItem(szWorldTitle,CMD_REMOVE_LEVEL,0);
		}
		pCtrl->SetFont(LTNULL,nListFontSize);
		pCtrl->SetParam1(nMissionId);
		pCtrl->SetFixedWidth(nSelWidth,LTTRUE);
		m_pSelMissions->AddControl(pCtrl);
		
	}
	else
	{
		ASSERT(!"Invalid mission id");
	}

}

