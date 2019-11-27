#ifndef _INC_MAGICRECORD_
#define _INC_MAGICRECORD_

#include <vector>
#include <string>
#include <map>

struct lua_State;

struct MagicLvRecord {
    int nIndex;
    int nLvReq;
    int nCost;
    int nIncrease;
    int nMultiple;

    MagicLvRecord() {
        nIndex = 0;
        nLvReq = nCost = nIncrease = nMultiple = 0;
    }
};

struct MagicRecord {
    int nID;
    int nJob;
    int nColldown;
    int nIncrease;
    int nMultiple;
    bool bPassive;

    std::string xName;
    std::string xDesc;
    std::vector<MagicLvRecord> xLvs;

    MagicRecord() {
        bPassive = false;
        nID = nJob = nColldown = nIncrease = nMultiple = 0;
    }
};

typedef std::map<int, MagicRecord> MagicRecordMap;

extern MagicRecordMap g_xMagicRecords;

bool LoadMagicRecordFromLua(lua_State *L, const char *pszKeyname);

#endif
