// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicle.cpp
//
// PURPOSE : Implementation of the PlayerVehicle
//
// CREATED : 8/31/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerVehicle.h"
#include "PlayerButes.h"
#include "ServerButeMgr.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "PlayerObj.h"
#include "KeyMgr.h"
#include "VehicleButeMgr.h"
#include "VersionMgr.h"

#include <queue>


LINKFROM_MODULE( PlayerVehicle );


PlayerVehicle::PlayerVehicleList PlayerVehicle::m_lstPlayerVehicles;

float kBlinkTime = 10.0f;

// ----------------------------------------------------------------------- //
//
//	CLASS:		PlayerVehicle
//
//	PURPOSE:	An PlayerVehicle object
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(PlayerVehicle)
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "",  PF_HIDDEN | PF_FILENAME)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(1, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(VehicleType, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)
	ADD_BOOLPROP_FLAG(Locked, LTFALSE, 0)
	ADD_STRINGPROP_FLAG(LockedCommand, "", PF_NOTIFYCHANGE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(PlayerVehicle, Prop, NULL, NULL, 0, CPlayerVehiclePlugin)


CMDMGR_BEGIN_REGISTER_CLASS( PlayerVehicle )

	CMDMGR_ADD_MSG( ACTIVATE, 1, NULL, "ACTIVATE" )
	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )

CMDMGR_END_REGISTER_CLASS( PlayerVehicle, Prop )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::PlayerVehicle()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PlayerVehicle::PlayerVehicle() : Prop ()
{
	m_fRespawnTime = 0.0f;

	m_vOriginalPos.Init();
	m_vOriginalDims.Init(10, 10, 10);
    m_rOriginalRot.Init();

	m_pDebrisOverride = "Machinerysmall";

	m_bLocked = false;
	m_bRidden = false;
	m_hstrLockedCommand	= LTNULL;

	m_dwSavedFlags = 0;

	m_fLastRideTime = 0;
	m_bVirgin = true;
	m_bBlinking = false;

	m_nVehicleId = VBMGR_INVALID_ID;

	// Add this instance to a list of all bodies.
	m_lstPlayerVehicles.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::~PlayerVehicle()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

PlayerVehicle::~PlayerVehicle()
{
	// Erase this instance from the list of all playervehicles.
	PlayerVehicleList::iterator it = m_lstPlayerVehicles.begin( );
	while( it != m_lstPlayerVehicles.end( ))
	{
		if( *it == this )
		{
			m_lstPlayerVehicles.erase( it );
			break;
		}

		it++;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PlayerVehicle::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet =  Prop::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("VehicleType", &genProp) == LT_OK)
	{
		VEHICLE *pVehicle = g_pVehicleButeMgr->GetVehicle( genProp.m_String );
		if( pVehicle )
		{
			m_nVehicleId = pVehicle->nId;
		}
	}

	if( g_pLTServer->GetPropGeneric( "Locked", &genProp ) == LT_OK )
	{
		m_bLocked = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "LockedCommand", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_hstrLockedCommand = g_pLTServer->CreateString( genProp.m_String );
		}
	}

	m_PlayerVehicleStruct.nVehicleId = m_nVehicleId;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	if (!IsMultiplayerGame())
	{
		//vehicles do not respawn in SP
		m_fRespawnTime = 0.0f;
	}
	else
	{
		//vehicles must respawn in MP
		m_fRespawnTime = g_pServerButeMgr->GetVehicleRespawnTime();
	}
		
	m_bCanDeactivate = !IsMultiplayerGame();


	// Make us activateable...

	// Need rayhit to activate.
	// Need clientnonsolid because we like redundancies.
	// Need touchnotify so that we get put into containers.
	m_dwFlags |= (FLAG_RAYHIT | FLAG_CLIENTNONSOLID | FLAG_TOUCH_NOTIFY);
	m_dwFlags &= ~( FLAG_SOLID | FLAG_GRAVITY );

	m_dwSavedFlags = m_dwFlags;

	// Create the vehicle model from the attibute record...

	VEHICLE *pVehicle = g_pVehicleButeMgr->GetVehicle( m_nVehicleId );
	if( !pVehicle )
		return;

	LTStrCpy( pStruct->m_Filename, pVehicle->sModel.c_str(), ARRAY_LEN( pStruct->m_Filename ));
	
	pVehicle->blrSkins.CopyList( 0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );
	pVehicle->blrRenderStyles.CopyList( 0, pStruct->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

	if( pVehicle->bTranslucent )
	{
		pStruct->m_Flags2 |= FLAG2_FORCETRANSLUCENT;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool PlayerVehicle::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate( "ACTIVATE" );
	static CParsedMsg::CToken s_cTok_Lock( "LOCK" );
	static CParsedMsg::CToken s_cTok_UnLock( "UNLOCK" );

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		// Vehicle already being ridden.
		if( m_bRidden )
			return true;

		DoActivate(hSender);

		// Let Prop have a shot at it so activate commands are processed...
		if (!m_bLocked)
		{
			return Prop::OnTrigger(hSender, cMsg);
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Lock )
	{
		m_bLocked = true;
	}
	else if( cMsg.GetArg(0) == s_cTok_UnLock )
	{
		m_bLocked = false;
	}
	else
	{
		return Prop::OnTrigger(hSender, cMsg);
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::InitialUpdate()
{
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

	// Can't damage player vehicles...

    m_damage.SetCanDamage(LTFALSE);
	m_damage.SetMass(1000.0f);

	g_pLTServer->GetObjectPos(m_hObject, &m_vOriginalPos);
	g_pLTServer->GetObjectRotation(m_hObject, &m_rOriginalRot);
	g_pPhysicsLT->GetObjectDims(m_hObject, &m_vOriginalDims);

	CreateSFXMsg();

	CapNumberOfVehicles( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Respawn()
//
//	PURPOSE:	Start the respawn process...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Respawn()
{
	// Try to set our dims (make sure we fit where we were placed)...

	LTVector vDims = m_vOriginalDims;
    if (g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, SETDIMS_PUSHOBJECTS) == LT_ERROR)
	{
        g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);
	}

 	LTVector vVec(0, 0, 0);
	g_pPhysicsLT->SetVelocity(m_hObject, &vVec);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

    SetNextUpdate(UPDATE_NEXT_FRAME);

	MoveObjectToFloor(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Update()
//
//	PURPOSE:	Process updates if necessary...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Update()
{
	if (m_fRespawnTime <= 0.0f)
	{
		SetNextUpdate(UPDATE_NEVER);
		return;
	}

	bool bBlinking = m_bBlinking;
	if (m_RespawnTimer.On() && m_RespawnTimer.Stopped())
	{
		LTVector vCurPos;
		g_pLTServer->GetObjectPos(m_hObject, &vCurPos);

		if (m_bVirgin)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_SFX_MESSAGE);
			cMsg.Writeuint8(SFX_PLAYERVEHICLE_ID);
			cMsg.WriteObject(m_hObject);
			cMsg.Writeuint8(kPlayerVehicleStopFade);
		}
		else
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_SFX_MESSAGE);
			cMsg.Writeuint8(SFX_PLAYERVEHICLE_ID);
			cMsg.WriteObject(m_hObject);
			cMsg.Writeuint8(kPlayerVehicleRespawn);
			cMsg.WriteLTVector(vCurPos);
			cMsg.WriteLTVector(m_vOriginalPos);
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

		}

		g_pLTServer->SetObjectPos(m_hObject, &m_vOriginalPos);
		g_pLTServer->SetObjectRotation(m_hObject, &m_rOriginalRot);

		LTVector vZero(0, 0, 0);
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		MoveObjectToFloor(m_hObject);
		m_RespawnTimer.Stop();
        SetNextUpdate(UPDATE_NEVER);

		m_bBlinking = false;
	}
	else
	{
		if (m_RespawnTimer.On() && m_RespawnTimer.GetCountdownTime() < kBlinkTime)
		{
			if (!m_bBlinking)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_SFX_MESSAGE);
				cMsg.Writeuint8(SFX_PLAYERVEHICLE_ID);
				cMsg.WriteObject(m_hObject);
				cMsg.Writeuint8(kPlayerVehicleStartFade);
				g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

			}


			m_bBlinking = true;
		}
        SetNextUpdate(UPDATE_NEXT_FRAME);
	}

	//it changed state, notify the clients
	if (bBlinking != m_bBlinking)
	{
		CreateSFXMsg();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::DoActivate()
//
//	PURPOSE:	Handle Activate...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::DoActivate(HOBJECT hSender)
{
	// Check if vehicle already being ridden.
	if( m_bRidden )
		return;

	// Make sure we're visible (i.e., spawned in)...

    uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if ( !(dwFlags & FLAG_VISIBLE) || !hSender || !IsPlayer(hSender)) return;

	
	if( m_bLocked )
	{
		// Check to see if the player has the correct keys to operate a vehicle...

		if( !g_pKeyMgr->CanCharacterControlObject( hSender, m_hObject ))
		{
			// Send a command if we have one...

			if( m_hstrLockedCommand )
			{
				const char	*pCmd = g_pLTServer->GetStringData( m_hstrLockedCommand );

				if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
				{
					g_pCmdMgr->Process( pCmd, m_hObject, hSender );
				}
			}

			// No keys? No vehicle! 
		
			return;
		}
	}

	// Tell the player to hop aboard...

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
	if (pPlayer)
	{
		pPlayer->RideVehicle(this);
	}
}


void PlayerVehicle::SetRidden(bool bRidden)
{

	if (bRidden)
	{
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, m_dwSavedFlags);

		bool bRH = !!(m_dwSavedFlags & FLAG_RAYHIT);

		// Remove flags that cause the vehicle to go through extensive moveobjects.  It will
		// reset its own flags under the respawn.
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCEOPTIMIZEOBJECT, FLAG_GRAVITY | FLAG_SOLID | FLAG_RAYHIT | FLAG_TOUCH_NOTIFY | FLAG_FORCEOPTIMIZEOBJECT);

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);

		SetNextUpdate(UPDATE_NEVER);
		m_RespawnTimer.Stop();

		m_bBlinking = false;
		CreateSFXMsg();

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_PLAYERVEHICLE_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(kPlayerVehicleStopFade);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);


		// We have now been ridden.
		m_bVirgin = false;
	}
	else
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_ATTACH_HIDE1SHOW3);
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwSavedFlags, FLAGMASK_ALL);

		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		bool bRH = !!(dwFlags & FLAG_RAYHIT);

		// Record the last time this vehicle has been ridden.
		m_fLastRideTime = g_pLTServer->GetTime( );

		m_RespawnTimer.Start(m_fRespawnTime);

		Respawn();
	}
	m_bRidden = bRidden;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_VECTOR(m_vOriginalPos);
	SAVE_VECTOR(m_vOriginalDims);
	SAVE_ROTATION(m_rOriginalRot);
 	SAVE_FLOAT(m_fRespawnTime);
	SAVE_BYTE( m_nVehicleId );		// Was previously saving ((uint8)m_ePPhysicsModel)...
	SAVE_bool( m_bLocked );
	SAVE_bool( m_bRidden );
	SAVE_HSTRING( m_hstrLockedCommand );
	SAVE_DWORD(m_dwSavedFlags);
	SAVE_TIME( m_fLastRideTime );
	SAVE_bool( m_bVirgin );

	m_RespawnTimer.Save(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_VECTOR(m_vOriginalPos);
	LOAD_VECTOR(m_vOriginalDims);
	LOAD_ROTATION(m_rOriginalRot);
 	LOAD_FLOAT(m_fRespawnTime);

	// Must be backwards compatible and previous save versions were saving the PlayerPhysicsModel
	// so just get the VehicleId of the Snowmobile since that was the only valid PlayerVehicel...

	LOAD_BYTE( m_nVehicleId );
	
	LOAD_bool( m_bLocked );
	LOAD_bool( m_bRidden );
	LOAD_HSTRING( m_hstrLockedCommand );
	LOAD_DWORD(m_dwSavedFlags);
	LOAD_TIME( m_fLastRideTime );
	LOAD_bool( m_bVirgin );

	m_RespawnTimer.Load(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::CreateSFXMsg
//
//	PURPOSE:	Create our special fx message
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::CreateSFXMsg()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_PLAYERVEHICLE_ID);
    m_PlayerVehicleStruct.Write(cMsg);
	cMsg.Writebool(m_bBlinking);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::CapNumberOfVehicles
//
//	PURPOSE:	Limit number of vehicles allows in level by
//				removing the vehicle with the longest time since
//				it was last ridden.
//
// ----------------------------------------------------------------------- //

struct PlayerVehicleQueueEntry
{
	PlayerVehicle*	m_pVehicle;

	bool operator()( PlayerVehicleQueueEntry const& a, PlayerVehicleQueueEntry const& b) const
	{
		// a should be considered "larger" than b if its age
		// is less than b so it goes higher on the queue.
		return a.m_pVehicle->GetLastRideTime( ) > b.m_pVehicle->GetLastRideTime( );
	}
};

void PlayerVehicle::CapNumberOfVehicles( )
{
	std::priority_queue< PlayerVehicleQueueEntry, std::vector< PlayerVehicleQueueEntry >, PlayerVehicleQueueEntry > queueAge;

	// Maximum number of vehicles allowed in level.  When we
	// go above this number we try to remove old ones.
	int nVehicleMaxNumber = GetConsoleInt( "VehicleMaxNumber", 12 );
	nVehicleMaxNumber = Max( 0, nVehicleMaxNumber );

	// Vehicle must not have been riden for this time before it gets zapped.
	float fVehicleMinTimeout = GetConsoleFloat( "VehicleMinTimeout", 30.0f );
	fVehicleMinTimeout = Max( 0.0f, fVehicleMinTimeout );

	float fTime = g_pLTServer->GetTime( );

	PlayerVehicleQueueEntry queueEntry;

	// Iterate through the list of vehicles and put them into a
	// priority queue.  Sorted by oldest at top of queue.
	for( PlayerVehicleList::iterator iter = m_lstPlayerVehicles.begin( ); iter != m_lstPlayerVehicles.end( ); iter++ )
	{
		queueEntry.m_pVehicle = *iter;
		if( !queueEntry.m_pVehicle )
			continue;

		// Don't remove playervehicles that have never been ridden.
		if( queueEntry.m_pVehicle->m_bVirgin )
			continue;

		// Don't remove vehicles being ridden.
		if( queueEntry.m_pVehicle->m_bRidden )
			continue;

		// Don't take a snowmobile too quickly.
		if( fTime - queueEntry.m_pVehicle->GetLastRideTime( ) < fVehicleMinTimeout )
			continue;

		queueAge.push( queueEntry );
	}

	// Count how many we should remove from the world.
	int nRemoveCount = ( int )m_lstPlayerVehicles.size( ) - nVehicleMaxNumber;
	if( nRemoveCount <= 0 )
		return;

	// Don't remove more than we have.
	nRemoveCount = Min(( int )queueAge.size( ), nRemoveCount );

	// Check if we have any to remove.
	if( !nRemoveCount )
		return;

	// Remove all the oldest vehicles until the count is good.
	while( nRemoveCount-- )
	{
		// Get the oldest vehicle.
		PlayerVehicleQueueEntry const& queueEntry = queueAge.top( );

		// Remove vehicle.
		g_pLTServer->RemoveObject( queueEntry.m_pVehicle->m_hObject );

		// Go to the next farthest body.
		queueAge.pop( );
	}
}



LTRESULT CPlayerVehiclePlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	// Handle vehicle type...

	if (_strcmpi("VehicleType", szPropName) == 0)
	{
		
		if( !g_pVehicleButeMgr )
		{
			// This will set the g_pVehicleButeMgr...Since this could also be
			// set elsewhere, just check for the global bute mgr...

			char szFile[256];
			sprintf(szFile, "%s\\%s", szRezPath, VBMGR_DEFAULT_FILE);

			CVehicleButeMgr::Instance().SetInRezFile( LTFALSE );
			CVehicleButeMgr::Instance().Init( szFile );
		}
		
		uint8 nNumVehicles = g_pVehicleButeMgr->GetNumVehicles( );
		
		for( uint8 i = 0; i < nNumVehicles; ++i )
		{
			ASSERT( cMaxStrings > (*pcStrings) + 1 );
			if( cMaxStrings > (*pcStrings) + 1 )
			{
				VEHICLE *pVehicle = g_pVehicleButeMgr->GetVehicle( i );
				if( pVehicle && !pVehicle->sName.empty() )
				{
					LTStrCpy( aszStrings[(*pcStrings)++], pVehicle->sName.c_str(), cMaxStringLength );
				}
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerVehiclePlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CPlayerVehiclePlugin::PreHook_PropChanged( const char *szObjName,
													const char *szPropName, 
													const int  nPropType, 
													const GenericProp &gpPropValue,
													ILTPreInterface *pInterface,
													const char *szModifiers )
{
	// Check if the props are our commands and then just send it to the CommandMgr..

	if( !_stricmp( "LockedCommand", szPropName ))
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerVehiclePlugin::PreHook_Dims
//
//  PURPOSE:	Make sure the vehicle model shows up correctly in DEdit...
//
// ----------------------------------------------------------------------- //

LTRESULT CPlayerVehiclePlugin::PreHook_Dims( const char *szRezPath,
											 const char *szPropValue,
											 char *szModelFilenameBuf,
											 int nModelFilenameBufLen,
											 LTVector &vDims )
{
	if( !szModelFilenameBuf || nModelFilenameBufLen < 1 )
		return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	if( !g_pVehicleButeMgr )
	{
		// This will set the g_pVehicleButeMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, VBMGR_DEFAULT_FILE);

		CVehicleButeMgr::Instance().SetInRezFile( LTFALSE );
        CVehicleButeMgr::Instance().Init( szFile );
	}

	VEHICLE *pVehicle = g_pVehicleButeMgr->GetVehicle( szPropValue );
	if( !pVehicle || pVehicle->sModel.empty() )
		return LT_UNSUPPORTED;

	LTStrCpy( szModelFilenameBuf, pVehicle->sModel.c_str(), nModelFilenameBufLen );

	// Need to convert the .ltb filename to one that DEdit understands...
	
	ConvertLTBFilename( szModelFilenameBuf );

	return LT_OK;
}