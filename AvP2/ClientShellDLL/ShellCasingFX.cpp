//----------------------------------------------------------
//
// MODULE  : SHELLCASINGFX.CPP
//
// PURPOSE : defines classes for ejected shells
//
// CREATED : 5/1/98
//
//----------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "ShellCasingFX.h"
#include "iltclient.h"
#include "ltlink.h"
#include "ClientUtilities.h"
#include "WeaponMgr.h"
#include "GameClientShell.h"
#include "ClientWeaponUtils.h"
#include "SurfaceFunctions.h"
#include "FXButeMgr.h"

#define MAX_BOUNCE_COUNT 3

extern PhysicsState g_normalPhysicsState;
extern PhysicsState g_waterPhysicsState;
extern CGameClientShell* g_pGameClientShell;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::CShellCasingFX
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CShellCasingFX::CShellCasingFX()
{
    m_rRot.Init();
	m_vStartPos.Init();
	m_vStartVel.Init();
	m_vOffset.Init();
	m_nWeaponId = WMGR_INVALID_ID;
	m_nAmmoId	= WMGR_INVALID_ID;
    m_b3rdPerson = LTFALSE;

	m_dwFlags		= FLAG_VISIBLE; // | FLAG_NOLIGHT;
	m_fExpireTime	= 0.0f;
    m_bInVisible    = LTTRUE;
    m_bResting      = LTFALSE;
	m_nBounceCount	= MAX_BOUNCE_COUNT;	// Set maximum bounces
	m_fStartTime	= 0.0f;
	m_fStartDelay	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Init
//
//	PURPOSE:	Create the shell casing
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!GetConsoleInt("ShellCasings", 1)) return LTFALSE;
    if (!psfxCreateStruct) return LTFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	SHELLCREATESTRUCT* pShell = (SHELLCREATESTRUCT*)psfxCreateStruct;

    m_rRot = pShell->rRot;
	VEC_COPY(m_vStartPos, pShell->vStartPos);
	m_nWeaponId = pShell->nWeaponId;
	m_nAmmoId	= pShell->nAmmoId;
	m_dwFlags  |= pShell->dwFlags;
	m_vStartVel = pShell->vStartVel;
	m_b3rdPerson = pShell->b3rdPerson;
	m_vOffset = pShell->vOffset;


	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::CreateObject
//
//	PURPOSE:	Create the model associated with the shell
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::CreateObject(ILTClient *pClientDE)
{
	if (!GetConsoleInt("ShellCasings", 1)) return LTFALSE;
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	// Setup the shell...

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
    if (!pAmmo || !pAmmo->pFireFX) return LTFALSE;

	//record start time and delay time
	m_fStartTime = g_pLTClient->GetTime();
	m_fStartDelay = pAmmo->fShellCasingDelay;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Update
//
//	PURPOSE:	Update the shell
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::Update()
{
    if (!m_pClientDE) return LTFALSE;

	if(!m_hObject)
	{
		if(g_pLTClient->GetTime()-m_fStartTime > m_fStartDelay)
		{
			AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
			if (!pAmmo || !pAmmo->pFireFX) return LTFALSE;

			ObjectCreateStruct createStruct;
			INIT_OBJECTCREATESTRUCT(createStruct);

			SAFE_STRCPY(createStruct.m_Filename, pAmmo->pFireFX->szShellModel);
			SAFE_STRCPY(createStruct.m_SkinName, pAmmo->pFireFX->szShellSkin);
			m_vScale = pAmmo->pFireFX->vShellScale;

			createStruct.m_ObjectType = OT_MODEL;
			createStruct.m_Flags = m_dwFlags;

			if (!m_b3rdPerson)
			{
				// Make sure our initial position is accurate at this point in time
				// since there may have been a delay.
				m_vStartPos = g_pGameClientShell->GetWeaponModel()->GetFlashPos();
			}

			// Now add in the offset
			m_vStartPos += m_vOffset;

			VEC_COPY(createStruct.m_Pos, m_vStartPos);
			createStruct.m_Rotation = m_rRot;

			m_hObject = g_pClientDE->CreateObject(&createStruct);
			if (!m_hObject) return LTFALSE;

			m_pClientDE->SetObjectScale(m_hObject, &m_vScale);

			// User camera rotation if not 3rd person ;)

			if (!m_b3rdPerson)
			{
				HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
				g_pGameClientShell->GetCameraMgr()->GetCameraRotation(hCamera, m_rRot, LTTRUE);
			}

			LTVector vU, vR, vF;
			g_pClientDE->GetRotationVectors(&m_rRot, &vU, &vR, &vF);

			VEC_MULSCALAR(vU, vU, GetRandom(50.0f, 110.0f));
			VEC_MULSCALAR(vR, vR, GetRandom(50.0f, 110.0f));

			m_vStartVel += (vU + vR);

			InitMovingObject(&m_movingObj, &m_vStartPos, &m_vStartVel);;

			m_fDieTime = g_pClientDE->GetTime() + GetRandom(10.0f, 15.0f);
		}
		else
		{
			//not time to get started
			return LTTRUE;
		}
	}

    if (m_pClientDE->GetTime() > m_fDieTime) return LTFALSE;

    if (m_bResting) return LTTRUE;

    LTRotation rRot;

	// If velocity slows enough, and we're on the ground, just stop bouncing and just wait to expire.

	if (m_movingObj.m_dwPhysicsFlags & MO_RESTING)
	{
        m_bResting = LTTRUE;

		// Stop the spinning...

		m_pClientDE->SetupEuler(&rRot, 0, m_fYaw, 0);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);

		// Shell is at rest, we can add a check here to see if we really want
		// to keep it around depending on detail settings...

		HLOCALOBJ hObjs[1];
        uint32 nNumFound, nBogus;
		m_pClientDE->FindObjectsInSphere(&m_movingObj.m_vPos, 64.0f, hObjs, 1, &nBogus, &nNumFound);

		// Remove thyself...

        if (nNumFound > 15) return LTFALSE;
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
            LTFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw   += m_fYawVel * fDeltaTime;

			m_pClientDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			m_pClientDE->SetObjectRotation(m_hObject, &rRot);
		}
	}


    LTVector vNewPos;
    if (UpdateMovingObject(LTNULL, &m_movingObj, &vNewPos))
	{
		ClientIntersectInfo info;
        if (BounceMovingObject(LTNULL, &m_movingObj, &vNewPos, &info, INTERSECT_HPOLY))
		{
			// If we hit the sky/invisible surface we're done...

			SurfaceType eType = GetSurfaceType(info);
			if (eType == ST_SKY || eType == ST_INVISIBLE)
			{
                return LTFALSE;
			}

			if (m_nBounceCount >= MAX_BOUNCE_COUNT)
			{
				if (!(m_movingObj.m_dwPhysicsFlags & MO_LIQUID))
				{
					SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eType);
					if (pSurf)
					{
						// Play appropriate sound...

						if (pSurf->szShellImpactSnds[0])
						{
							g_pClientSoundMgr->PlaySoundFromPos(vNewPos, pSurf->szShellImpactSnds[0], pSurf->fShellSndRadius,
								SOUNDPRIORITY_MISC_LOW);
						}
					}
				}
			}

			// Adjust the bouncing..

			m_fPitchVel = GetRandom(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);
			m_fYawVel	= GetRandom(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);

			m_nBounceCount--;

			if (m_nBounceCount <= 0)
			{
				//if (info.m_Plane.m_Normal.y > 0.6f)
				//{
				//	m_nBounceCount = 1;
				//}
				//else
				//{
					m_movingObj.m_dwPhysicsFlags |= MO_RESTING;
				//}
			}
		}

		m_movingObj.m_vPos = vNewPos;

        if (m_pClientDE->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
            return LTFALSE;
		}

		m_pClientDE->SetObjectPos(m_hObject, &vNewPos);
	}

    return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_ShellCasingFactory
//-------------------------------------------------------------------------------------------------

const SFX_ShellCasingFactory SFX_ShellCasingFactory::registerThis;

CSpecialFX* SFX_ShellCasingFactory::MakeShape() const
{
	return new CShellCasingFX();
}


