// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFuncs.h
//
// PURPOSE : Misc functions for creating debris
//
// CREATED : 6/29/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_FUNCS_H__
#define __DEBRIS_FUNCS_H__

// Includes...

#include "ltbasetypes.h"
#include "DebrisMgr.h"
#include "SurfaceFunctions.h"
#include "iobjectplugin.h"
#include <memory.h>  // for memset

// Use ADD_DEBRISTYPE_PROPERTY() in your class definition to enable
// the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DEBRISTYPE_PROPERTY()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DEBRISTYPE_PROPERTY(flags) \
	ADD_STRINGPROP_FLAG(DebrisType, "", PF_STATICLIST | (flags))

class CDebrisPlugin : public IObjectPlugin
{

  public:

    virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

struct CLIENTDEBRIS
{
    CLIENTDEBRIS();

    LTRotation   rRot;
    LTVector     vPos;
    uint8       nDebrisId;
	LTVector	vVelOffset;
};

inline CLIENTDEBRIS::CLIENTDEBRIS()
{
	vPos.Init();
	rRot.Init();
	nDebrisId = DEBRISMGR_INVALID_ID;
	vVelOffset.Init();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateDebris()
//
//	PURPOSE:	Create client-side debris...
//
// ----------------------------------------------------------------------- //

void CreateDebris(CLIENTDEBRIS & cd);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateMultiDebris()
//
//	PURPOSE:	Create client-side debris...
//
// ----------------------------------------------------------------------- //

void CreateMultiDebris(CLIENTDEBRIS & cd);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisProperties()
//
//	PURPOSE:	Determine the debris properties (This should only be
//				called during an object's ReadProp function if the object
//				added the ADD_DEBRISTYPE_PROPERTY macro).
//
// ----------------------------------------------------------------------- //

void GetDebrisProperties(uint8 & nDebrisId);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreatePropDebris()
//
//	PURPOSE:	Create client-side debris for props...
//
// ----------------------------------------------------------------------- //

void CreatePropDebris(LTVector & vPos, LTVector & vDir, uint8 nDebrisId, LTVector vVelOffset=LTVector(0.0f, 0.0f, 0.0f));


#endif // __DEBRIS_FUNCS_H__
