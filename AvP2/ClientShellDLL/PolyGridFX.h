 // ----------------------------------------------------------------------- //
//
// MODULE  : PolyGridFX.h
//
// PURPOSE : Polygrid special fx class - Definition
//
// CREATED : 10/13/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLY_GRID_FX_H__
#define __POLY_GRID_FX_H__

#include "SpecialFX.h"

#define PLASMA_NORMAL		0
#define PLASMA_FOUR_RING	1

// These are used for SetPolyGridPalette.
typedef struct ColorRamp_t
{
	ColorRamp_t(DVector color, int index)
	{
		m_Color = color;
		m_Index = index;
	}

	DVector m_Color;
	int m_Index;
} ColorRamp;


struct PGCREATESTRUCT : public SFXCREATESTRUCT
{
	PGCREATESTRUCT::PGCREATESTRUCT();

	DVector vDims;
	DVector vColor1;
	DVector vColor2;
	DFLOAT fXScaleMin; 
	DFLOAT fXScaleMax; 
	DFLOAT fYScaleMin; 
	DFLOAT fYScaleMax; 
	DFLOAT fXScaleDuration;
	DFLOAT fYScaleDuration;
	DFLOAT fXPan;
	DFLOAT fYPan;
	DFLOAT fFrequency;
	HSTRING hstrSurfaceSprite;
	DFLOAT fAlpha;
	DDWORD dwNumPolies;
	DBYTE nPlasmaType;
	DBYTE nRingRate[4];
	DBOOL bAdditive;
	DBOOL bMultiply; 	
};

inline PGCREATESTRUCT::PGCREATESTRUCT()
{
	vDims.Init();
	vColor1.Init();
	vColor2.Init();
	fXScaleMin			= 0.0f; 
	fXScaleMax			= 0.0f; 
	fYScaleMin			= 0.0f;	 
	fYScaleMax			= 0.0f; 
	fXScaleDuration		= 0.0f;
	fYScaleDuration		= 0.0f;
	fXPan				= 0.0f;
	fYPan				= 0.0f;
	fFrequency			= 0.0f;
	fAlpha				= 0.0f;
	dwNumPolies			= 0;
	nPlasmaType			= 0;
	hstrSurfaceSprite	= DNULL;
	bAdditive			= DFALSE;
	bMultiply			= DFALSE;
	
	for (int i=0; i < 4; i++)
	{
		nRingRate[i] = 0;
	}
}


class CPolyGridFX : public CSpecialFX
{
	public :

		CPolyGridFX() : CSpecialFX() 
		{
			VEC_INIT(m_vDims);
			VEC_SET(m_vColor1, 255.0f, 255.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 255.0f);
			m_fXScaleMin		= 15.0f; 
			m_fXScaleMax		= 25.0f; 
			m_fYScaleMin		= 15.0f; 
			m_fYScaleMax		= 25.0f; 
			m_fXScaleDuration	= 10.0f;
			m_fYScaleDuration	= 10.0f;
			m_fXPan				= 0.0f;
			m_fYPan				= 0.0f;
			m_fFrequency		= 50.0f;
			m_hstrSurfaceSprite	= DNULL;
			m_fAlpha			= 0.7f;

			m_bAlwaysUpdate		= DFALSE;
			m_DistanceGrid		= DNULL;
			m_fCount			= 0.0f;

			m_bScalingXUp		= DTRUE;
			m_bScalingYUp		= DTRUE;

			m_dwNumPolies		= 160;

			m_nPlasmaType		= PLASMA_FOUR_RING;
			m_nRingRate[0]		= 50;
			m_nRingRate[1]		= 10;
			m_nRingRate[2]		= 30;
			m_nRingRate[3]		= 20;

			m_bUseGlobalSettings = DTRUE;

			m_bAdditive			= DFALSE;
			m_bMultiply			= DFALSE;
		}

		~CPolyGridFX()
		{
			if (m_DistanceGrid)
			{
				free(m_DistanceGrid);
				m_DistanceGrid = DNULL;
			}

			if (m_hstrSurfaceSprite && m_pClientDE)
			{
				m_pClientDE->FreeString(m_hstrSurfaceSprite);
			}

		}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

		void SetAlwaysUpdate(DBOOL b=DTRUE)			{ m_bAlwaysUpdate = b; }
		void SetUseGlobalSettings(DBOOL b=DTRUE)	{ m_bUseGlobalSettings = b; }

	protected:

		void PrecalculatePlasma();
		void SetPolyGridPalette(ColorRamp *pRamps, int nRamps);
		void UpdatePlasma();
		void UpdateFourRingPlasma();
		void UpdateSurface();


	private :

		// Plasma stuff.
		DBYTE		*m_DistanceGrid;
		float		m_fCount;		
		DBOOL		m_bScalingXUp;
		DBOOL		m_bScalingYUp;

		DVector		m_vDims;
		DVector		m_vColor1;
		DVector		m_vColor2;
		DFLOAT		m_fXScaleMin; 
		DFLOAT		m_fXScaleMax; 
		DFLOAT		m_fYScaleMin; 
		DFLOAT		m_fYScaleMax; 
		DFLOAT		m_fXScaleDuration;
		DFLOAT		m_fYScaleDuration;
		DFLOAT		m_fXPan;
		DFLOAT		m_fYPan;
		DFLOAT		m_fFrequency;
		HSTRING		m_hstrSurfaceSprite;
		DFLOAT		m_fAlpha;
		DDWORD		m_dwNumPolies;

		DBYTE		m_nPlasmaType;
		DBYTE		m_nRingRate[4];

		DBOOL		m_bAlwaysUpdate;
		DBOOL		m_bUseGlobalSettings;

		DBOOL		m_bAdditive;
		DBOOL		m_bMultiply;
};

//-------------------------------------------------------------------------------------------------
// SFX_PolyGridFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_PolyGridFactory : public CSpecialFXFactory
{
	SFX_PolyGridFactory() : CSpecialFXFactory(SFX_POLYGRID_ID) {;}
	static const SFX_PolyGridFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __POLY_GRID_FX_H__