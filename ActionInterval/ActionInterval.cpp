#include "ActionInterval.h"

// Implement of ActionIntervalSource
ActionIntervalSource::ActionIntervalSource()
{

}

ActionIntervalSource::~ActionIntervalSource()
{

}


// Methods of ActionIntervalSourece
bool ActionIntervalSource::Init(ActionIntervalMap& _refIntervalMap)
{
	// Deep copy the interval map
	m_xActionIntervalMap = _refIntervalMap;

	return true;
}

int ActionIntervalSource::GetInterval(int _nActionID) const
{
	ActionIntervalMap::const_iterator fndIter = m_xActionIntervalMap.find(_nActionID);
	if (fndIter == m_xActionIntervalMap.end())
	{
		// Not found
		return 0;
	}

	return fndIter->second;
}


// Implement of ActionIntervalData
ActionIntervalData::ActionIntervalData(const ActionIntervalSource* _pActionSource)
{
	m_pDataSource = _pActionSource;
}

ActionIntervalData::~ActionIntervalData()
{
	m_pDataSource = NULL;
}

void ActionIntervalData::Reset()
{
	m_xUserActionRecord.clear();
}

bool ActionIntervalData::DoAction(int _nActionID)
{
	int nInterval = m_pDataSource->GetInterval(_nActionID);
	if (0 == nInterval)
	{
		return true;
	}

	// Find last action time
	DWORD dwTick = GetTickCount();
	std::map<int, DWORD>::iterator fndIter = m_xUserActionRecord.find(_nActionID);
	if (fndIter == m_xUserActionRecord.end())
	{
		// First test the action, add new record
		m_xUserActionRecord.insert(std::make_pair(_nActionID, dwTick));
		return true;
	}

	DWORD dwLastActionTime = fndIter->second;

	printf("%d %d\r\n", dwLastActionTime, dwTick);

	if (dwTick - dwLastActionTime >= DWORD(nInterval))
	{
		// OK, update the action time
		fndIter->second = dwTick;
		return true;
	}

	return false;
}