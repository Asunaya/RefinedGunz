#include "MActParam.h"
#include "MZoneServer.h"


MObject* MActParam::GetTarget()
{
	MZoneServer* pZoneServer = MZoneServer::GetInstance();
	return pZoneServer->GetObjectCharacter(m_TargetUID);
}

void MActParam::SetTarget(MObject* pTarget)
{ 
	m_TargetUID = pTarget->GetUID(); 
}