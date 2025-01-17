// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSniper.h
//
// PURPOSE : AIGoalSniper class definition
//
// CREATED : 4/10/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SNIPER_H__
#define __AIGOAL_SNIPER_H__

#include "AIGoalAbstractUseObject.h"


class CAIGoalSniper : public CAIGoalAbstractUseObject
{
	typedef CAIGoalAbstractUseObject super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSniper, kGoal_Sniper);

		CAIGoalSniper( );

		virtual void InitGoal(CAI* pAI);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		virtual void UpdateGoal();

		// Attractor Handling.

		virtual AINode* FindNearestAttractorNode();

		// Command Handling.

		virtual LTBOOL	HandleNameValuePair(const char *szName, const char *szValue);

		// ILTObjRefReceiver function.

		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	protected:

		// State Handling.

		void HandleStateSniper();
		virtual EnumAIStateType GetUseObjectState();

		// Target Handling.

		AINode* HandleTargetPos();

		// Node Handling.

		void SetSniperNode(AINodeUseObject* pNewNode);

	protected:

		AIVolume*			m_pLastTargetInfoVol;
		LTFLOAT				m_fNextViewNodeUpdate;
		LTObjRefNotifier	m_hThreat;
		LTBOOL				m_bSeekPlayer;
		LTBOOL				m_bNoRadius;
};


#endif
