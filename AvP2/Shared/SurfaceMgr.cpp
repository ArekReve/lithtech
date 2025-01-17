// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceMgr.cpp
//
// PURPOSE : SurfaceMgr - Implementation
//
// CREATED : 07/07/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SurfaceMgr.h"
#include "CommonUtilities.h"
#include "FXButeMgr.h"
#include "CharacterButeMgr.h"

#define SRFMGR_GLOBAL_TAG						"Global"

#define SRFMGR_SURFACE_TAG						"Surface"

#define SRFMGR_SURFACE_NAME						"Name"
#define SRFMGR_SURFACE_ID						"Id"
#define SRFMGR_SURFACE_SHOWSMARK				"ShowsMark"
#define SRFMGR_SURFACE_CANSEETHROUGH			"CanSeeThrough"
#define SRFMGR_SURFACE_CANSHOOTTHROUGH			"CanShootThrough"
#define SRFMGR_SURFACE_SHOWBREATH				"ShowBreath"
#define SRFMGR_SURFACE_MAXSHOOTTHROUGHPERTURB	"MaxShootThroughPerturb"
#define SRFMGR_SURFACE_MAXSHOOTTHROUGHTHICKNESS	"MaxShootThroughThickness"
#define SRFMGR_SURFACE_BULLETHOLESPR			"BulletHoleSpr"
#define SRFMGR_SURFACE_BULLETHOLEMINSCALE		"BulletHoleMinScale"
#define SRFMGR_SURFACE_BULLETHOLEMAXSCALE		"BulletHoleMaxScale"
#define SRFMGR_SURFACE_BULLETRANGEDAMPEN		"BulletRangeDampen"
#define SRFMGR_SURFACE_BULLETDAMAGEDAMPEN		"BulletDamageDampen"
#define SRFMGR_SURFACE_BULLETIMPACTSND			"BulletImpactSnd"
#define SRFMGR_SURFACE_PROJIMPACTSND			"ProjectileImpactSnd"
#define SRFMGR_SURFACE_MELEEIMPACTSND			"MeleeImpactSnd"
#define SRFMGR_SURFACE_SHELLSNDRADIUS			"ShellSndRadius"
#define SRFMGR_SURFACE_SHELLIMPACTSND			"ShellImpactSnd"
#define SRFMGR_SURFACE_GRENADESNDRADIUS			"GrenadeSndRadius"
#define SRFMGR_SURFACE_GRENADEIMPACTSND			"GrenadeImpactSnd"
#define SRFMGR_SURFACE_RTFOOTPRINTSPR			"RtFootPrintSpr"
#define SRFMGR_SURFACE_LTFOOTPRINTSPR			"LtFootPrintSpr"
#define SRFMGR_SURFACE_FOOTPRINTSCALE			"FootPrintScale"
#define SRFMGR_SURFACE_FOOTPRINTLIFETIME		"FootPrintLifetime"
#define SRFMGR_SURFACE_RTFOOTSND				"RtFootSnd"
#define SRFMGR_SURFACE_LTFOOTSND				"LtFootSnd"
#define SRFMGR_SURFACE_BODYFALLSND				"BodyFallSnd"
#define SRFMGR_SURFACE_BODYFALLSNDRADIUS		"BodyFallSndRadius"
#define SRFMGR_SURFACE_ACTIVATIONSND			"ActivationSnd"
#define SRFMGR_SURFACE_ACTIVATIONSNDRADIUS		"ActivationSndRadius"
#define SRFMGR_SURFACE_DEATHNOISEMOD			"DeathNoiseMod"
#define SRFMGR_SURFACE_MOVENOISEMOD				"MoveNoiseMod"
#define SRFMGR_SURFACE_IMPACTNOISEMOD			"ImpactNoiseMod"
#define SRFMGR_SURFACE_HARDNESS					"Hardness"
#define SRFMGR_SURFACE_MAGNETIC					"Magnetic"
#define SRFMGR_SURFACE_IMPACTSCALENAME			"ImpactScaleName"
#define SRFMGR_SURFACE_IMPACTPSHOWERNAME		"ImpactPShowerName"
#define SRFMGR_SURFACE_IMPACTPOLYDEBRISNAME		"ImpactPolyDebrisName"
#define SRFMGR_SURFACE_UWIMPACTPSHOWERNAME		"UWImpactPShowerName"
#define SRFMGR_SURFACE_EXITSCALENAME			"ExitScaleName"
#define SRFMGR_SURFACE_EXITPSHOWERNAME			"ExitPShowerName"
#define SRFMGR_SURFACE_EXITPOLYDEBRISNAME		"ExitPolyDebrisName"
#define SRFMGR_SURFACE_UWEXITPSHOWERNAME		"UWExitPShowerName"

// Global pointer to surface mgr...

CSurfaceMgr*    g_pSurfaceMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];

#ifndef _CLIENTBUILD

// Plugin statics

static CSurfaceMgr		sm_SurfaceMgr;
static CFXButeMgr		sm_FXMgr;

#endif // _CLIENTBUILD

static CAVector ToCAVector(const LTVector & vec)
{
	return CAVector(vec.x,vec.y,vec.z);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::CSurfaceMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSurfaceMgr::CSurfaceMgr()
{
    m_SurfaceList.Init(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::~CSurfaceMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSurfaceMgr::~CSurfaceMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSurfaceMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pSurfaceMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	g_pSurfaceMgr = this;


	// Save all the global info...



	// Read in the properties for each surface...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", SRFMGR_SURFACE_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		SURFACE* pSurf = new SURFACE;

		if (pSurf && pSurf->Init(m_buteMgr, s_aTagName))
		{
			m_SurfaceList.AddTail(pSurf);
		}
		else
		{
			delete pSurf;
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", SRFMGR_SURFACE_TAG, nNum);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::GetSurface
//
//	PURPOSE:	Get the specified surface
//
// ----------------------------------------------------------------------- //

SURFACE* CSurfaceMgr::GetSurface(SurfaceType eType)
{
    SURFACE** pCur  = LTNULL;

	pCur = m_SurfaceList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->eType == eType)
		{
			return *pCur;
		}

		pCur = m_SurfaceList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::GetSurface
//
//	PURPOSE:	Get the specified surface
//
// ----------------------------------------------------------------------- //

SURFACE* CSurfaceMgr::GetSurface(char* pName)
{
    if (!pName) return LTNULL;

    SURFACE** pCur  = LTNULL;

	pCur = m_SurfaceList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_SurfaceList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CSurfaceMgr::Term()
{
    g_pSurfaceMgr = LTNULL;

	m_SurfaceList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::CacheAll
//
//	PURPOSE:	Cache all the surface related resources
//
// ----------------------------------------------------------------------- //

void CSurfaceMgr::CacheAll()
{
#ifndef _CLIENTBUILD  // Server-side only

	// Cache all the surfaces...

    SURFACE** pCurSurf  = LTNULL;
	pCurSurf = m_SurfaceList.GetItem(TLIT_FIRST);

	while (pCurSurf)
	{
		if (*pCurSurf)
		{
			(*pCurSurf)->Cache(this);
		}

		pCurSurf = m_SurfaceList.GetItem(TLIT_NEXT);
	}

#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::CacheAll
//
//	PURPOSE:	Cache all the surface related resources
//
// ----------------------------------------------------------------------- //

void CSurfaceMgr::CacheFootsteps(int nCharacterID)
{
#ifndef _CLIENTBUILD  // Server-side only

	// Cache all the surfaces...

    SURFACE** pCurSurf  = LTNULL;
	pCurSurf = m_SurfaceList.GetItem(TLIT_FIRST);

	while (pCurSurf)
	{
		if (*pCurSurf)
		{
			(*pCurSurf)->CacheFootsteps(nCharacterID);
		}

		pCurSurf = m_SurfaceList.GetItem(TLIT_NEXT);
	}

#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SURFACE::SURFACE
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SURFACE::SURFACE()
{
	eType						= ST_UNKNOWN;
    bShowsMark                  = LTFALSE;
    bCanSeeThrough              = LTFALSE;
    bCanShootThrough            = LTFALSE;
	bShowBreath					= LTFALSE;
	nMaxShootThroughPerturb		= 0;
	nMaxShootThroughThickness	= 0;
	szBulletHoleSpr[0]			= '\0';
	fBulletHoleMinScale			= 1.0f;
	fBulletHoleMaxScale			= 1.0f;
	fBulletRangeDampen			= 1.0f;
	fBulletDamageDampen			= 1.0f;
	fShellSndRadius				= 0.0f;

	vFootPrintScale.Init(1, 1, 1);

    int i;
    for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		szBulletImpactSnds[i][0]	= '\0';
	}

	for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		szProjectileImpactSnds[i][0]	= '\0';
	}

	for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		szMeleeImpactSnds[i][0]	= '\0';
	}

	for (i=0; i < SRF_MAX_SHELL_SNDS; i++)
	{
		szShellImpactSnds[i][0]	= '\0';
	}

	szRtFootPrintSpr[0]	= '\0';
	szLtFootPrintSpr[0]	= '\0';

	for (i=0; i < SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		szRtFootStepSnds[i][0]	= '\0';
		szLtFootStepSnds[i][0]	= '\0';
	}

	szName[0]				= '\0';
	szBodyFallSnd[0]		= '\0';
	fBodyFallSndRadius		= 400.0f;
	fDeathNoiseModifier		= 1.0f;
	fMovementNoiseModifier	= 1.0f;
	fImpactNoiseModifier	= 1.0f;

	szActivationSnd[0]		= '\0';
	fActivationSndRadius	= 0;

	szGrenadeImpactSnd[0]	= '\0';
	fGrenadeSndRadius		= 0;

	fHardness				= 0.0f;
	fFootPrintLifetime		= 0.0f;
    bMagnetic               = LTFALSE;

	nNumImpactScaleFX			= 0;
	aImpactScaleFXIds[0]		= -1;

	nNumImpactPShowerFX			= 0;
	aImpactPShowerFXIds[0]		= -1;

	nNumImpactPolyDebrisFX		= 0;
	aImpactPolyDebrisFXIds[0]	= -1;

	nNumUWImpactPShowerFX		= 0;
	aUWImpactPShowerFXIds[0]	= -1;


	nNumExitScaleFX			= 0;
	aExitScaleFXIds[0]		= -1;

	nNumExitPShowerFX		= 0;
	aExitPShowerFXIds[0]	= -1;

	nNumExitPolyDebrisFX	= 0;
	aExitPolyDebrisFXIds[0]	= -1;

	nNumUWExitPShowerFX		= 0;
	aUWExitPShowerFXIds[0]	= -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SURFACE::Init
//
//	PURPOSE:	Build the surface struct
//
// ----------------------------------------------------------------------- //

LTBOOL SURFACE::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	eType					= (SurfaceType) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_ID);
    bShowsMark              = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_SHOWSMARK,bShowsMark);
    bCanSeeThrough          = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_CANSEETHROUGH),bCanSeeThrough;
    bCanShootThrough        = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_CANSHOOTTHROUGH,bCanShootThrough);
    bShowBreath             = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_SHOWBREATH,bShowBreath);
	nMaxShootThroughPerturb	= buteMgr.GetInt(aTagName, SRFMGR_SURFACE_MAXSHOOTTHROUGHPERTURB,nMaxShootThroughPerturb);
	nMaxShootThroughThickness= buteMgr.GetInt(aTagName, SRFMGR_SURFACE_MAXSHOOTTHROUGHTHICKNESS,nMaxShootThroughThickness);
    fDeathNoiseModifier     = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_DEATHNOISEMOD,fDeathNoiseModifier);
    fMovementNoiseModifier  = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_MOVENOISEMOD,fMovementNoiseModifier);
    fImpactNoiseModifier    = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_IMPACTNOISEMOD,fImpactNoiseModifier);
    fBodyFallSndRadius      = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BODYFALLSNDRADIUS,fBodyFallSndRadius);
    fBulletHoleMinScale     = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BULLETHOLEMINSCALE,fBulletHoleMinScale);
    fBulletHoleMaxScale     = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BULLETHOLEMAXSCALE,fBulletHoleMaxScale);
    fBulletRangeDampen      = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BULLETRANGEDAMPEN,fBulletRangeDampen);
    fBulletDamageDampen     = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BULLETDAMAGEDAMPEN,fBulletDamageDampen);
    fActivationSndRadius    = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_ACTIVATIONSNDRADIUS,fActivationSndRadius);
    fShellSndRadius         = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_SHELLSNDRADIUS,fShellSndRadius);
    fGrenadeSndRadius       = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_GRENADESNDRADIUS,fGrenadeSndRadius);
    fHardness               = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_HARDNESS,fHardness);
    fFootPrintLifetime      = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_FOOTPRINTLIFETIME,fFootPrintLifetime);
    bMagnetic               = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_MAGNETIC,bMagnetic);

	CAVector vTemp;

	vTemp = ToCAVector(vFootPrintScale);
	vFootPrintScale			= buteMgr.GetVector(aTagName, SRFMGR_SURFACE_FOOTPRINTSCALE,vTemp);

	CString str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_BULLETHOLESPR,CString(""));
	if (!str.IsEmpty())
	{
		strncpy(szBulletHoleSpr, (char*)(LPCSTR)str, ARRAY_LEN(szBulletHoleSpr));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_RTFOOTPRINTSPR,CString(""));
	if (!str.IsEmpty())
	{
		strncpy(szRtFootPrintSpr, (char*)(LPCSTR)str, ARRAY_LEN(szRtFootPrintSpr));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_LTFOOTPRINTSPR,CString(""));
	if (!str.IsEmpty())
	{
		strncpy(szLtFootPrintSpr, (char*)(LPCSTR)str, ARRAY_LEN(szLtFootPrintSpr));
	}

    int i;
    for (i=1; i <= SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_RTFOOTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

		if (!str.IsEmpty())
		{
			strncpy(szRtFootStepSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szRtFootStepSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_LTFOOTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

		if (!str.IsEmpty())
		{
			strncpy(szLtFootStepSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szLtFootStepSnds[i-1]));
		}
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_BODYFALLSND,CString(""));
	if (!str.IsEmpty())
	{
		strncpy(szBodyFallSnd, (char*)(LPCSTR)str, ARRAY_LEN(szBodyFallSnd));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_ACTIVATIONSND,CString(""));
	if (!str.IsEmpty())
	{
		strncpy(szActivationSnd, (char*)(LPCSTR)str, ARRAY_LEN(szActivationSnd));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_GRENADEIMPACTSND,CString(""));
	if (!str.IsEmpty())
	{
		strncpy(szGrenadeImpactSnd, (char*)(LPCSTR)str, ARRAY_LEN(szGrenadeImpactSnd));
	}

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_BULLETIMPACTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

		if (!str.IsEmpty())
		{
			strncpy(szBulletImpactSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szBulletImpactSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_PROJIMPACTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

		if (!str.IsEmpty())
		{
			strncpy(szProjectileImpactSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szProjectileImpactSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_MELEEIMPACTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

		if (!str.IsEmpty())
		{
			strncpy(szMeleeImpactSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szMeleeImpactSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_SHELL_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_SHELLIMPACTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));

		if (!str.IsEmpty())
		{
			strncpy(szShellImpactSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szShellImpactSnds[i-1]));
		}
	}

	// Impact specific scale fx...

	// Build our impact scale fx id list...

	nNumImpactScaleFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTSCALENAME, nNumImpactScaleFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumImpactScaleFX < SRF_MAX_IMPACT_SCALEFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));
		if (!str.IsEmpty())
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
			if (pScaleFX)
			{
				aImpactScaleFXIds[nNumImpactScaleFX] = pScaleFX->nId;
			}
		}

		nNumImpactScaleFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTSCALENAME, nNumImpactScaleFX);
	}

	// Build our impact particle shower fx id list...

	nNumImpactPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTPSHOWERNAME, nNumImpactPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumImpactPShowerFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aImpactPShowerFXIds[nNumImpactPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumImpactPShowerFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTPSHOWERNAME, nNumImpactPShowerFX);
	}

	// Build our impact poly debris fx id list...

	nNumImpactPolyDebrisFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTPOLYDEBRISNAME, nNumImpactPolyDebrisFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumImpactPolyDebrisFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));
		if (!str.IsEmpty())
		{
			CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX((char*)(LPCSTR)str);
			if (pPolyDebrisFX)
			{
				aImpactPolyDebrisFXIds[nNumImpactPolyDebrisFX] = pPolyDebrisFX->nId;
			}
		}

		nNumImpactPolyDebrisFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTPOLYDEBRISNAME, nNumImpactPolyDebrisFX);
	}

	// Build our under water impact particle shower fx id list...


	nNumUWImpactPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_UWIMPACTPSHOWERNAME, nNumUWImpactPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumUWImpactPShowerFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aUWImpactPShowerFXIds[nNumUWImpactPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumUWImpactPShowerFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_UWIMPACTPSHOWERNAME, nNumUWImpactPShowerFX);
	}



	// Exit specific scale fx...

	// Build our exit scale fx id list...

	nNumExitScaleFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITSCALENAME, nNumExitScaleFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumExitScaleFX < SRF_MAX_IMPACT_SCALEFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));
		if (!str.IsEmpty())
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
			if (pScaleFX)
			{
				aExitScaleFXIds[nNumExitScaleFX]  = pScaleFX->nId;
			}
		}

		nNumExitScaleFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITSCALENAME, nNumExitScaleFX);
	}

	// Build our exit particle shower fx id list...

	nNumExitPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITPSHOWERNAME, nNumExitPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumExitPShowerFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aExitPShowerFXIds[nNumExitPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumExitPShowerFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITPSHOWERNAME, nNumExitPShowerFX);
	}

	// Build our exit poly debris fx id list...

	nNumExitPolyDebrisFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITPOLYDEBRISNAME, nNumExitPolyDebrisFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumExitPolyDebrisFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));
		if (!str.IsEmpty())
		{
			CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX((char*)(LPCSTR)str);
			if (pPolyDebrisFX)
			{
				aExitPolyDebrisFXIds[nNumExitPolyDebrisFX] = pPolyDebrisFX->nId;
			}
		}

		nNumExitPolyDebrisFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITPOLYDEBRISNAME, nNumExitPolyDebrisFX);
	}

	// Build our under water exit particle shower fx id list...

	nNumUWExitPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_UWIMPACTPSHOWERNAME, nNumUWExitPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumUWExitPShowerFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName,CString(""));
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aUWExitPShowerFXIds[nNumUWExitPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumUWExitPShowerFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_UWIMPACTPSHOWERNAME, nNumUWExitPShowerFX);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SURFACE::Cache
//
//	PURPOSE:	Cache all the resources associated with the surface
//
// ----------------------------------------------------------------------- //

void SURFACE::Cache(CSurfaceMgr* pSurfaceMgr)
{
#ifndef _CLIENTBUILD

	if (!pSurfaceMgr || !g_pFXButeMgr) return;
	
	// Cache sprites...

	if (szBulletHoleSpr[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szBulletHoleSpr);
	}

	if (szRtFootPrintSpr[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szRtFootPrintSpr);
	}

	if (szLtFootPrintSpr[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szLtFootPrintSpr);
	}


	// Cache sounds...

    int i;
	char buf[128];
	IMPACTFX* pFX;
    for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		if (szBulletImpactSnds[i][0])
		{
			for (int j=0; j<g_pFXButeMgr->GetNumImpactFX(); j++)
			{
				pFX = g_pFXButeMgr->GetImpactFX(j);

				if(pFX && pFX->szSoundDir[0])
				{
					sprintf(buf, szBulletImpactSnds[i], pFX->szSoundDir);
					g_pLTServer->CacheFile(FT_SOUND, buf);
				}
			}
		}
	}

	for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		if (szMeleeImpactSnds[i][0])
		{
			for (int j=0; j<g_pFXButeMgr->GetNumImpactFX(); j++)
			{
				pFX = g_pFXButeMgr->GetImpactFX(j);

				if(pFX && pFX->szSoundDir[0])
				{
					sprintf(buf, szMeleeImpactSnds[i], pFX->szSoundDir);
					g_pLTServer->CacheFile(FT_SOUND, buf);
				}
			}
		}
	}

	for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		if (szProjectileImpactSnds[i][0])
		{
			for (int j=0; j<g_pFXButeMgr->GetNumImpactFX(); j++)
			{
				pFX = g_pFXButeMgr->GetImpactFX(j);

				if(pFX && pFX->szSoundDir[0])
				{
					sprintf(buf, szProjectileImpactSnds[i], pFX->szSoundDir);
					g_pLTServer->CacheFile(FT_SOUND, buf);
				}
			}
		}
	}

	for (i=0; i < SRF_MAX_SHELL_SNDS; i++)
	{
		if (szShellImpactSnds[i][0])
		{
			for (int j=0; j<g_pFXButeMgr->GetNumImpactFX(); j++)
			{
				pFX = g_pFXButeMgr->GetImpactFX(j);

				if(pFX && pFX->szSoundDir[0])
				{
					sprintf(buf, szShellImpactSnds[i], pFX->szSoundDir);
					g_pLTServer->CacheFile(FT_SOUND, buf);
				}
			}
		}
	}

	if (szBodyFallSnd[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szBodyFallSnd);
	}

	if (szActivationSnd[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szActivationSnd);
	}

	if (szGrenadeImpactSnd[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szGrenadeImpactSnd);
	}

	for (i=0; i < nNumImpactScaleFX; i++)
	{
		CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(aImpactScaleFXIds[i]);
		if (pScaleFX)
		{
			pScaleFX->Cache();
		}
	}

	for (i=0; i < nNumImpactPShowerFX; i++)
	{
		CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX(aImpactPShowerFXIds[i]);
		if (pPShowerFX)
		{
			pPShowerFX->Cache();
		}
	}

	for (i=0; i < nNumImpactPolyDebrisFX; i++)
	{
		CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX(aImpactPolyDebrisFXIds[i]);
		if (pPolyDebrisFX)
		{
			pPolyDebrisFX->Cache();
		}
	}

	for (i=0; i < nNumUWImpactPShowerFX; i++)
	{
		CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX(aUWImpactPShowerFXIds[i]);
		if (pPShowerFX)
		{
			pPShowerFX->Cache();
		}
	}
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SURFACE::CacheFootsteps
//
//	PURPOSE:	Cache all the footstep resources associated with the surface
//
// ----------------------------------------------------------------------- //

void SURFACE::CacheFootsteps(int nCharacterId)
{
#ifndef _CLIENTBUILD

	char buf[255];
	
	for (int i=0; i < SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		if (szRtFootStepSnds[i][0])
		{
			sprintf(buf, szRtFootStepSnds[i], g_pCharacterButeMgr->GetFootStepSoundDir(nCharacterId));
            if( LT_NOTFOUND == g_pLTServer->CacheFile(FT_SOUND, buf) )
			{
#ifndef _FINAL
//						g_pLTServer->CPrint( "WARNING : Sound file \"%s\" not found.", buf );
#endif
			}
		}

		if (szLtFootStepSnds[i][0])
		{
			sprintf(buf, szLtFootStepSnds[i], g_pCharacterButeMgr->GetFootStepSoundDir(nCharacterId));
            if( LT_NOTFOUND == g_pLTServer->CacheFile(FT_SOUND, buf) )
			{
#ifndef _FINAL
//						g_pLTServer->CPrint( "WARNING : Sound file \"%s\" not found.", buf );
#endif
			}
		}
	}

#endif
}


#ifndef _CLIENTBUILD

////////////////////////////////////////////////////////////////////////////
//
// CSurfaceMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CSurfaceMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CSurfaceMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char * const * aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	char szFile[256];

	// 12/21/00 - MarkS:
	// FXButeMgr needs to be valid before SurfaceMgr can Init
	if (!g_pFXButeMgr)
	{
#ifdef _WIN32
		sprintf(szFile, "%s\\%s", szRezPath, FXBMGR_DEFAULT_FILE);
#else
		sprintf(szFile, "%s/%s", szRezPath, FXBMGR_DEFAULT_FILE);
#endif
        sm_FXMgr.SetInRezFile(LTFALSE);
        sm_FXMgr.Init(g_pLTServer, szFile);
	}

	if (!g_pSurfaceMgr)
	{
		// This will set the g_pSurfaceMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...
#ifdef _WIN32
		sprintf(szFile, "%s\\%s", szRezPath, SRFMGR_DEFAULT_FILE);
#else
		sprintf(szFile, "%s/%s", szRezPath, SRFMGR_DEFAULT_FILE);
#endif
        sm_SurfaceMgr.SetInRezFile(LTFALSE);
        sm_SurfaceMgr.Init(g_pLTServer, szFile);
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CSurfaceMgrPlugin::PopulateStringList(char* const * aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	_ASSERT(aszStrings && pcStrings && g_pSurfaceMgr);
    if (!aszStrings || !pcStrings || !g_pSurfaceMgr) return LTFALSE;

	// Add an entry for each surface

	SurfaceList* pList = g_pSurfaceMgr->GetSurfaceList();
    if (!pList || pList->GetLength() < 1) return LTFALSE;

	// Cache all the surfaces...

    SURFACE** pCurSurf  = LTNULL;
	pCurSurf = pList->GetItem(TLIT_FIRST);

	while (pCurSurf)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		// Be sure the surface has a name.
		if (*pCurSurf && (*pCurSurf)->szName[0] )
		{
			// Be sure the surface can fit into the userflag mask.  
			// USRFLG_SURFACE# goes up to 4 currently.
			if( uint32((*pCurSurf)->eType) < 16 )
			{
				uint32 dwSurfaceNameLen = strlen((*pCurSurf)->szName);

				if (dwSurfaceNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
				{
					strcpy(aszStrings[(*pcStrings)++], (*pCurSurf)->szName);
				}
			}
		}

		pCurSurf = pList->GetItem(TLIT_NEXT);
	}

    return LTTRUE;
}

#endif // #ifndef _CLIENTBUILD
