#ifndef _INC_ACTION_INTERVAL_
#define _INC_ACTION_INTERVAL_

#include <Windows.h>
#include <map>

typedef std::map<int, DWORD> ActionIntervalMap;

class ActionIntervalSource
{
public:
	ActionIntervalSource();
	~ActionIntervalSource();

public:
	bool Init(ActionIntervalMap& _refIntervalMap);
	int GetInterval(int _nActionID) const;

public:
	ActionIntervalMap m_xActionIntervalMap;
};

class ActionIntervalData
{
public:
	ActionIntervalData(const ActionIntervalSource* _pActionSource);
	~ActionIntervalData();

public:
	bool DoAction(int _nActionID);
	void Reset();

private:
	// Key: ActionID Value: Last action time
	std::map<int, DWORD> m_xUserActionRecord;
	const ActionIntervalSource* m_pDataSource;
};

#endif