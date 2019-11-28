#ifndef _INC_MAGICRECORD_
#define _INC_MAGICRECORD_

#include <vector>
#include <string>
#include <map>

struct lua_State;

// Bright   restrict    Dark
// Dark     restrict    Thunder
// Thunder  restrict    Water
// Water    restrict    Fire
// Fire     restrict    Bright
enum MagicElementType {
    MagicElementNone,
    MagicElementThunder,
    MagicElementBright,
    MagicElementDark,
    MagicElementWater,
    MagicElementFire,
    MagicElementTotal,
};

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
    int nElement;
    bool bPassive;

    std::string xName;
    std::string xDesc;
    std::vector<MagicLvRecord> xLvs;

    MagicRecord() {
        nElement = 0;
        bPassive = false;
        nID = nJob = nColldown = nIncrease = nMultiple = 0;
    }
};

typedef std::map<int, MagicRecord> MagicRecordMap;

extern MagicRecordMap g_xMagicRecords;

bool LoadMagicRecordFromLua(lua_State *L, const char *pszKeyname);

#endif
