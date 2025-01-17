// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  
#pragma warning( disable : 4503 )  

// This removes warnings in Vis C about the CodeWarrior pragma 'force_active'
//
#pragma warning( disable : 4068 )

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <limits.h>

#include "mfcstub.h"

#include "DebugNew.h"

#include "BuildDefines.h"
#include "clientheaders.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "iltsoundmgr.h"
#include "globals.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "iltmath.h"
#include "iltsoundmgr.h"
#include "ltobjectcreate.h"
#include "ltbasetypes.h"
#include "ltobjref.h"
#include "Factory.h"
#include "ClientUtilities.h"
#include "CommonUtilities.h"
#include "AutoMessage.h"
#include "GameClientShell.h"



#endif // __STDAFX_H__