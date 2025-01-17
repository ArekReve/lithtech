// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsDayDevice.cpp
//
// PURPOSE : The object used to represent the doomsday device.
//
// CREATED : 12/19/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "ParsedMsg.h"
#include "DoomsDayDevice.h"
#include "TeamMgr.h"
#include "PlayerObj.h"
#include "DoomsDayMissionMgr.h"
#include "DoomsDayPiece.h"
#include "Spawner.h"
#include "ChassisButeMgr.h"

LINKFROM_MODULE( DoomsDayDevice );

BEGIN_CLASS( DoomsDayDevice )
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(CanTransition, LTFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CommandOn, "", PF_NOTIFYCHANGE | PF_HIDDEN )
	ADD_STRINGPROP_FLAG(CommandOff, "", PF_NOTIFYCHANGE | PF_HIDDEN )
	ADD_STRINGPROP_FLAG(Type, "Doomsday_base", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
	PROP_DEFINEGROUP(GameType, PF_GROUP(16) | PF_HIDDEN)
		ADD_BOOLPROP_FLAG(SinglePlayer, 0, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(Cooperative, 0, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(Deathmatch, 0, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(TeamDeathmatch, 0, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(DoomsDay, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(Demolition, 0, PF_GROUP(16))
END_CLASS_DEFAULT_FLAGS_PLUGIN( DoomsDayDevice, Chassis, NULL, NULL, 0, CDoomsDayDevicePlugin)


// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( DoomsDayDevice )
	CMDMGR_ADD_MSG( FIRE, 1, NULL, "FIRE" )
CMDMGR_END_REGISTER_CLASS( DoomsDayDevice, Chassis )

DoomsDayDevice::DoomsDayDeviceList DoomsDayDevice::m_lstDoomsDayDevices;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DoomsDayDevice::DoomsDayDevice
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

DoomsDayDevice::DoomsDayDevice( )
:	m_eDoomsDayDeviceState	( kDoomsDayDeviceState_Idle )
{
	m_lstDoomsDayDevices.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DoomsDayDevice::DoomsDayDevice
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

DoomsDayDevice::~DoomsDayDevice( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 DoomsDayDevice::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = Chassis::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				if( !ReadProp(( ObjectCreateStruct* )pData ))
					return 0;

				// Always start out with the base proptype.
				m_sPropType = "Doomsday_base";

				// Add the doomsday pieces to the slots.
				ChassisPieceSlot chassisPieceSlot;
				chassisPieceSlot.m_pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceBute( "Doomsday_transmitter" );
				m_lstChassisPieceSlots.push_back( chassisPieceSlot );
				chassisPieceSlot.m_pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceBute( "Doomsday_batteries" );
				m_lstChassisPieceSlots.push_back( chassisPieceSlot );
				chassisPieceSlot.m_pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceBute( "Doomsday_core" );
				m_lstChassisPieceSlots.push_back( chassisPieceSlot );
			}
			
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = Chassis::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;
		
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return Chassis::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::OnTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken	s_cTok_Fire( "FIRE" );

	if( cMsg.GetArg( 0 ) == s_cTok_Fire )
	{
		// The fire trigger should come from a player.
		CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hSender ));
		if( !pPlayerObj )
			return false;
		
		Fire( *pPlayerObj );
		return true;
	}

	return Chassis::OnTrigger( hSender, cMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::ReadProp
//
//	PURPOSE:	Reads properties.
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::ReadProp( ObjectCreateStruct* pStruct )
{
	GenericProp genProp;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::InitialUpdate
//
//	PURPOSE:	MID_INITIALUPDATE handler.
//
// ----------------------------------------------------------------------- //

void DoomsDayDevice::InitialUpdate( )
{
	BeginIdle( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::Update
//
//	PURPOSE:	Frame Update.
//
// ----------------------------------------------------------------------- //

void DoomsDayDevice::Update( )
{
	switch( m_eDoomsDayDeviceState )
	{
		case kDoomsDayDeviceState_Idle:
		{
			UpdateIdle( );
			return;
		}
		break;

		case kDoomsDayDeviceState_Fire:
		{
			UpdateFire( );
			return;
		}
		break;

		case kDoomsDayDeviceState_End:
		{
			UpdateEnd( );
			return;
		}
		break;

		default:
		{
			ASSERT( !"DoomsDayDevice::Update:  Invalid state." );
			return;
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::BeginIdle
//
//	PURPOSE:	Start idling.
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::BeginIdle( )
{
	TRACE( "DoomsDayDevice::BeginIdle\n" );

	// Return to just the base model for now.
	if( !SetPropType( "Doomsday_base" ))
	{
		return false;
	}

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex( m_hObject, "base" );
	if( hAnim == INVALID_ANI )
	{
		ASSERT( !"DoomsDayDevice::BeginIdle:  Invalid animation." );
		return false;
	}

	g_pLTServer->SetModelAnimation( m_hObject, hAnim );
	g_pLTServer->SetModelLooping( m_hObject, LTFALSE );
	g_pLTServer->ResetModelAnimation( m_hObject );

	// Do one update.
	SetNextUpdate( UPDATE_NEXT_FRAME );
	m_eDoomsDayDeviceState = kDoomsDayDeviceState_Idle;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::UpdateIdle
//
//	PURPOSE:	Device is idling.
//
// ----------------------------------------------------------------------- //

void DoomsDayDevice::UpdateIdle( )
{
	SetNextUpdate( UPDATE_NEVER );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::BeginFire
//
//	PURPOSE:	Start the fire state.
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::BeginFire( )
{
	TRACE( "DoomsDayDevice::BeginFire\n" );

	// Change to the doomsday proptype.
	if( !SetPropType( "Doomsday" ))
		return false;

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex( m_hObject, "Idle1" );
	if( hAnim == INVALID_ANI )
	{
		ASSERT( !"DoomsDayDevice::BeginFire:  Invalid animation." );
		return false;
	}

	CDoomsDayMissionMgr* pDoomsDayMissionMgr = dynamic_cast< CDoomsDayMissionMgr* >( g_pServerMissionMgr );
	if( !pDoomsDayMissionMgr )
	{
		ASSERT( !"DoomsDayDevice::BeginEffect:  Invalid missionmgr." );
		return false;
	}

	// Tell the missionmgr we started firing.
	pDoomsDayMissionMgr->SetDeviceFiring( m_nOwningTeamID );

	// Get the Fx to play.
	char szDoomsDayDeviceFxName[256] = "";
	if( !g_pServerButeMgr->GetDoomsDayDeviceFxName( szDoomsDayDeviceFxName, ARRAY_LEN( szDoomsDayDeviceFxName )))
		return false;

	// Play the fx.
	PlayClientFX( szDoomsDayDeviceFxName, m_hObject );


	// Hide the target models and the piece models...

	ChassisPieceSlotList::const_iterator iterSlot = m_lstChassisPieceSlots.begin( );
	for( ; iterSlot != m_lstChassisPieceSlots.end(); ++iterSlot )
	{
		ChassisPieceSlot const& slot = *iterSlot;
		g_pCommonLT->SetObjectFlags( slot.m_hPlaceHolderObject, OFT_Flags, 0, FLAG_VISIBLE );
		g_pCommonLT->SetObjectFlags( slot.m_hChassisPiece, OFT_Flags, 0, FLAG_VISIBLE );
	}

	g_pLTServer->SetModelAnimation( m_hObject, hAnim );
	g_pLTServer->SetModelLooping( m_hObject, LTFALSE );
	g_pLTServer->ResetModelAnimation( m_hObject );

	SetNextUpdate( UPDATE_NEXT_FRAME );
	m_eDoomsDayDeviceState = kDoomsDayDeviceState_Fire;

	// Do the effect for a little bit before damaging players.
	float fDamageDelay = g_pServerButeMgr->GetDoomsDayDeviceDamageDelay( );
	m_Timer.Start( fDamageDelay );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::UpdateFire
//
//	PURPOSE:	Update for fire state.
//
// ----------------------------------------------------------------------- //

void DoomsDayDevice::UpdateFire( )
{
	SetNextUpdate( UPDATE_NEXT_FRAME );

	HMODELANIM hIdle2Anim = g_pLTServer->GetAnimIndex( m_hObject, "Idle2" );
	if( hIdle2Anim == INVALID_ANI )
	{
		ASSERT( !"DoomsDayDevice::UpdateFire:  Invalid animation." );
		BeginEnd( );
		return;
	}

	// Check if we need to switch to the next anim.
	if( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState( m_hObject ))
	{
		g_pLTServer->SetModelAnimation( m_hObject, hIdle2Anim );
		g_pLTServer->SetModelLooping( m_hObject, LTTRUE );
		g_pLTServer->ResetModelAnimation( m_hObject );
	}

	// Check if it's time to damage the victims.
	bool bAllDead = false;
	if( m_Timer.Stopped( ))
	{
		// Give the players some damage.
		if( !DamageVictims( bAllDead ))
		{
			ASSERT( !"DoomsDayDevice::UpdateFire: Could not damage players." );
			BeginEnd( );
			return;
		}


		// Damage the victims at a periodic rate to make sure they go down.
		float fDamageInterval = g_pServerButeMgr->GetDoomsDayDeviceDamageInterval( );
		m_Timer.Start( fDamageInterval );
	}

	// Check if all the other teams are dead.  Don't End before
	// we go into Idle2 anim though.
	if( bAllDead )
	{
		HMODELANIM hCurAnim = g_pLTServer->GetModelAnimation( m_hObject );
		if( hIdle2Anim == hCurAnim )
		{
			// Go straight to the end.
			BeginEnd( );
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::BeginEnd
//
//	PURPOSE:	Start the End state.
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::BeginEnd( )
{
	TRACE( "DoomsDayDevice::BeginEnd\n" );

	SetNextUpdate( UPDATE_NEXT_FRAME );
	m_eDoomsDayDeviceState = kDoomsDayDeviceState_End;

	// Wait a little before considering the game to be over.
	float fEndTime = g_pServerButeMgr->GetDoomsDayDeviceEndTime( );
	m_Timer.Start( fEndTime );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::UpdateEnd
//
//	PURPOSE:	Update for End state.
//
// ----------------------------------------------------------------------- //

void DoomsDayDevice::UpdateEnd( )
{
	SetNextUpdate( UPDATE_NEXT_FRAME );

	// Check if not done with end time.
	if( !m_Timer.Stopped( ))
		return;

	CDoomsDayMissionMgr* pDoomsDayMissionMgr = dynamic_cast< CDoomsDayMissionMgr* >( g_pServerMissionMgr );
	if( !pDoomsDayMissionMgr )
	{
		ASSERT( !"DoomsDayDevice::UpdateEnd:  Invalid ServerMissionMgr." );
		return;
	}

	// Tell the missionmgr we finished the effect.
	pDoomsDayMissionMgr->SetDeviceEffectComplete( m_nOwningTeamID );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::DamageVictims
//
//	PURPOSE:	Damages victim teams.
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::DamageVictims( bool& bAllDead )
{
	TRACE( "DoomsDayDevice::DamageVictims\n" );

	char szDamageType[256];
	if( !g_pServerButeMgr->GetDoomsDayDeviceDamageType( szDamageType, ARRAY_LEN( szDamageType )))
	{
		return false;
	}

	DamageStruct damage;
	damage.eType	= StringToDamageType( szDamageType );
	damage.fDamage	= g_pServerButeMgr->GetDoomsDayDeviceDamageAmount( );
	damage.hDamager = m_hObject;
	damage.fDuration = 30.0f;

	// Initialize the status to all the players being dead.
	bAllDead = true;

	// Find all the victim teams and kill them.
	uint8 nNumTeams = CTeamMgr::Instance( ).GetNumTeams( );
	for( uint8 nTeam = 0; nTeam < nNumTeams; nTeam++ )
	{
		// Skip the activating team.
		if( nTeam == m_nOwningTeamID )
			continue;

		CTeam* pTeam = CTeamMgr::Instance( ).GetTeam( nTeam );
		if( !pTeam )
		{
			ASSERT( !"DoomsDayDevice::DamageVictims:  Invalid team." );
			continue;
		}

		// Iterate over each player on the team and kill him.
		PlayerIDSet const& players = pTeam->GetPlayerIDSet( );
		PlayerIDSet::const_iterator iter = players.begin( );
		while( iter != players.end( ))
		{
			uint32 nClientId = *iter;
			iter++;

			HCLIENT hClient = g_pLTServer->GetClientHandle( nClientId );
			if( !hClient )
				continue;

			CPlayerObj* pTeamPlayer = ( CPlayerObj* )g_pLTServer->GetClientUserData( hClient );
			if( !pTeamPlayer )
				continue;

			// Skip if already dead.
			if( pTeamPlayer->IsDead( ))
				continue;

			// This player is still alive, so update the status variable.
			bAllDead = false;

			// Do the damage.
			damage.DoDamage( this, pTeamPlayer->m_hObject );
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::Fire
//
//	PURPOSE:	Fire the device.
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::Fire( CPlayerObj& playerObj )
{
	// Make sure it's from someone on our team.
	if( playerObj.GetTeamID( ) != m_nOwningTeamID )
		return false;

	// The Device needs updates to finish the DoomsDay process and end the level.
	// Make sure Prop does not try to deactivate the device once it begins firing...

	m_bCanDeactivate = false;

	// Send a message letting the players know the device has been activated.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_DOOMSDAY_MESSAGE );
	cMsg.Writeuint8( MID_DOOMSDAY_DEVICE_COMPLETED );
	cMsg.Writeuint8( m_nOwningTeamID );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

	// Give the activating player a bunch of points.
	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	uint32 nBonus = sgo.GetDoomsday( ).m_nDeviceCompletedScore;
	playerObj.GetPlayerScore()->AddBonus( nBonus );

	if( !BeginFire( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::AddChassisPiece
//
//	PURPOSE:	Add a piece to the base and check for completion...
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::AddChassisPiece( ChassisPiece& chassisPiece, CPlayerObj* pPlayer )
{
	if( !pPlayer )
		return false;
	
	// Don't allow dead players to add pieces to a device...

	if( pPlayer->IsDead() )
		return false;

	// Try to add the piece.
	if( !Chassis::AddChassisPiece( chassisPiece, pPlayer ))
		return false;

	// Give the player points for adding the piece.
	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	uint32 nBonus = chassisPiece.IsHeavy( ) ? sgo.GetDoomsday( ).m_nHeavyPiecePlacedScore : sgo.GetDoomsday( ).m_nLightPiecePlacedScore;
	pPlayer->GetPlayerScore()->AddBonus( nBonus );

	// Check if we completed the device.
	if( IsAllPiecesPlaced( ))
	{
		// All the pieces are now on the base...
		
		Fire( *pPlayer );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::RemoveDoomsDayPiece
//
//	PURPOSE:	Remove a piece from the base...
//
// ----------------------------------------------------------------------- //

bool DoomsDayDevice::RemoveChassisPiece( ChassisPiece& chassisPiece, CPlayerObj* pPlayer )
{
	// Don't allow removal if we've already started.
	if( m_eDoomsDayDeviceState != kDoomsDayDeviceState_Idle )
		return false;

	// Try to remove the piece.
	if( !Chassis::RemoveChassisPiece( chassisPiece, pPlayer ))
		return false;

	// Give the player some points for removing the piece.
	if( pPlayer )
	{
		ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
		uint32 nBonus = sgo.GetDoomsday( ).m_nPieceRemovedScore;
		pPlayer->GetPlayerScore()->AddBonus( nBonus );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DoomsDayDevice::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if( !pMsg ) 
		return;

	SAVE_BYTE( m_eDoomsDayDeviceState );
	m_Timer.Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayDevice::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DoomsDayDevice::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if( !pMsg )
		return;

	LOAD_BYTE_CAST( m_eDoomsDayDeviceState, DoomsDayDeviceState );
	m_Timer.Load( pMsg );
}
