// ----------------------------------------------------------------------- //
//
// MODULE  : SnowVolume.cpp
//
// PURPOSE : Large-scale procedural snow volume implementation:
//           - snow specific parameters
//
// CREATED : 1/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MsgIDs.h"
#include "SnowVolume.h"
#include "DEditColors.h"
#include "ParsedMsg.h"

extern HSTRING ReadHStringProp(const char* pszPropName, ILTServer* pServer);


LINKFROM_MODULE( SnowVolume );

BEGIN_CLASS(SnowVolume)
	ADD_DEDIT_COLOR( SnowVolume )
	ADD_REALPROP(Density, 512.0f)
	ADD_REALPROP(ParticleRadius, 1.0f)
	ADD_REALPROP(FallRate, 80.0f)
	ADD_REALPROP(TumbleRate, 180.0f)
	ADD_REALPROP(TumbleRadius, 5.0f)
	ADD_REALPROP(MaxDrawDist, 1024.0f)
	ADD_COLORPROP(AmbientColor, 255.0f, 255.0f, 255.0f)
	ADD_BOOLPROP(UseLighting, FALSE)
	ADD_BOOLPROP(UseSaturate, TRUE)
	ADD_STRINGPROP_FLAG(TextureName, "", PF_FILENAME)
	ADD_BOOLPROP( StartOn, LTTRUE )
END_CLASS_DEFAULT(SnowVolume, VolumeEffect, NULL, NULL)


SnowVolume::SnowVolume()
{
	m_fDensity = 512.0f;
	m_fParticleRadius = 1.0f;
	m_fFallRate = 80.0f;
	m_fTumbleRate = 180.0f;
	m_fTumbleRadius = 5.0f;
	m_fMaxDrawDist = 1024.0f;
	m_vAmbientColor.Init( 255.0f, 255.0f, 255.0f );
	m_bUseLighting = false;
	m_bUseSaturate = true;
	m_hstrTextureName = LTNULL;
	m_bOn = true;
}


SnowVolume::~SnowVolume()
{
	if( g_pLTServer )
	{
		if( m_hstrTextureName )
		{
			g_pLTServer->FreeString( m_hstrTextureName );
		}
	}
}


uint32 SnowVolume::EngineMessageFn( uint32 messageID, void* pData, LTFLOAT fData )
{
	switch( messageID )
	{
	case MID_PRECREATE:
		{
			ObjectCreateStruct* pOCS = (ObjectCreateStruct*)pData;
			if( !pOCS )
				break;

			if( fData == PRECREATE_WORLDFILE )
			{
				ReadProp( pOCS );
				// make sure this object is always in the visible object list
				pOCS->m_Flags |= FLAG_FORCECLIENTUPDATE;
			}
		}
		break;

	case MID_INITIALUPDATE:
		{
			if( fData == INITIALUPDATE_WORLDFILE )			
			{
				InitialUpdate();
			}
		}
		break;
	}

	return VolumeEffect::EngineMessageFn( messageID, pData, fData );
}


bool SnowVolume::ReadProp( ObjectCreateStruct* pOCS )
{
	if( !pOCS )
		return false;

	g_pLTServer->GetPropReal( "Density", &m_fDensity );
	g_pLTServer->GetPropReal( "ParticleRadius", &m_fParticleRadius );
	g_pLTServer->GetPropReal( "FallRate", &m_fFallRate );
	g_pLTServer->GetPropReal( "TumbleRate", &m_fTumbleRate );
	g_pLTServer->GetPropReal( "TumbleRadius", &m_fTumbleRadius );
	g_pLTServer->GetPropReal( "MaxDrawDist", &m_fMaxDrawDist );
	g_pLTServer->GetPropVector( "AmbientColor", &m_vAmbientColor );
	g_pLTServer->GetPropBool( "UseLighting", &m_bUseLighting );
	g_pLTServer->GetPropBool( "UseSaturate", &m_bUseSaturate );
	m_hstrTextureName = ReadHStringProp( "TextureName", g_pLTServer );

	g_pLTServer->GetPropBool( "StartOn", &m_bOn );
	
	return true;
}


void SnowVolume::InitialUpdate( void )
{
	CreateSpecialFX();
}


void SnowVolume::WriteSnowInfo( ILTMessage_Write& cMsg )
{
	cMsg.WriteLTVector( m_vDims );
	cMsg.Writefloat( m_fDensity );
	cMsg.Writefloat( m_fParticleRadius );
	cMsg.Writefloat( m_fFallRate );
	cMsg.Writefloat( m_fTumbleRate );
	cMsg.Writefloat( m_fTumbleRadius );
	cMsg.Writefloat( m_fMaxDrawDist );
	cMsg.WriteLTVector( m_vAmbientColor );
	cMsg.Writeuint8( m_bUseLighting );
	cMsg.Writeuint8( m_bUseSaturate );
	cMsg.WriteHString( m_hstrTextureName );
	cMsg.Writebool( m_bOn );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::CreateSpecialFX
//
//  PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void SnowVolume::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	CAutoMessage cMsg;

	cMsg.Writeuint8( SFX_SNOW_ID );
	WriteSnowInfo( cMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_SNOW_ID );
		cMsg.WriteObject( m_hObject );
		WriteSnowInfo( cMsg );
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::OnTrigger
//
//  PURPOSE:	Handle trigger messages 
//
// ----------------------------------------------------------------------- //

bool SnowVolume::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_On( "ON" );
	static CParsedMsg::CToken s_cTok_Off( "OFF" );

	if( cMsg.GetArg(0) == s_cTok_On )
	{
		TurnOn( true );
	}
	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		TurnOn( false );
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::TurnOn
//
//  PURPOSE:	Turn the snow volume on and off... 
//
// ----------------------------------------------------------------------- //

void SnowVolume::TurnOn( bool bOn )
{
	if( m_bOn == bOn )
		return;

	m_bOn = bOn;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_SNOW_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( SVFX_TURNON );
	cMsg.Writebool( m_bOn );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::Save
//
//  PURPOSE:	Save the object... 
//
// ----------------------------------------------------------------------- //

void SnowVolume::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	VolumeEffect::Save( pMsg );

	SAVE_FLOAT( m_fDensity );
	SAVE_FLOAT( m_fParticleRadius );
	SAVE_FLOAT( m_fFallRate );
	SAVE_FLOAT( m_fTumbleRate );
	SAVE_FLOAT( m_fTumbleRadius );
	SAVE_FLOAT( m_fMaxDrawDist );
	SAVE_VECTOR( m_vAmbientColor );
	SAVE_bool( m_bUseLighting );
	SAVE_bool( m_bUseSaturate );
	SAVE_HSTRING( m_hstrTextureName );
	SAVE_bool( m_bOn );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::Load
//
//  PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void SnowVolume::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	VolumeEffect::Load( pMsg );

	LOAD_FLOAT( m_fDensity );
	LOAD_FLOAT( m_fParticleRadius );
	LOAD_FLOAT( m_fFallRate );
	LOAD_FLOAT( m_fTumbleRate );
	LOAD_FLOAT( m_fTumbleRadius );
	LOAD_FLOAT( m_fMaxDrawDist );
	LOAD_VECTOR( m_vAmbientColor );
	LOAD_bool( m_bUseLighting );
	LOAD_bool( m_bUseSaturate );
	LOAD_HSTRING( m_hstrTextureName );
	LOAD_bool( m_bOn );

	CreateSpecialFX( true );
}
