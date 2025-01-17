// FolderMain.h: interface for the CFolderMain class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDERMAIN_H_
#define _FOLDERMAIN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "MessageBox.h"
#include "BaseScaleFX.h"

class CFolderMain : public CBaseFolder
{
public:
	CFolderMain();
	virtual ~CFolderMain();

	// Build the folder
    LTBOOL   Build();

	// This is called when the folder gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);

	void	Escape();
    LTBOOL   DoMultiplayer(LTBOOL bMinimize);

	virtual char*	GetSelectSound();



protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    LTBOOL   Render(HSURFACE hDestSurf);


	void	ContinueLastSaved();

	HSTRING	m_BuildVersion;
    LTIntPt  m_BuildPos;
	CLTGUITextItemCtrl* m_pResume;
	CLTGUITextItemCtrl* m_pQuit;

};

#endif // _FOLDERMAIN_H_