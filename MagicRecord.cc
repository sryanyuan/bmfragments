#include "MagicRecord.h"
#include "LuaStackGuard.h"

MagicRecordMap g_xMagicRecords;

bool LoadMagicRecordFromLua(lua_State *L, const char *pszKeyname) {
    g_xMagicRecords.clear();

    LuaStackGuard guard(L);

    lua_getglobal(L, pszKeyname);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    // Stack: top_table
    LuaIterator it(L);
    if (!it.Begin()) {
        return false;
    }

    char szBuf[256] = {0};
    bool bRet = true;

    while (it.Next()) {
        if (LUA_TNUMBER != it.GetKeyType()) {
            continue;
        }
        if (LUA_TTABLE != it.GetValueType()) {
            continue;
        }

        MagicRecord mr;
        // Stack: root_table key value
        mr.nID = it.GetKeyInt();

        // Load name
        if (!loadKeyString(L, "Name", szBuf)) {
            return false;
        }
        mr.xName = szBuf;

        // Load desc
        if (!loadKeyString(L, "Desc", szBuf)) {
            return false;
        }
        mr.xDesc = szBuf;

        // Load job
        if (!loadKeyInt(L, "Job", mr.nJob)) {
            return false;
        }

        // Load passive
        int nPassive = 0;
        if (!loadKeyInt(L, "Passive", nPassive)) {
            return false;
        }
        mr.bPassive = (nPassive != 0);

        // Load colldown
        if (!loadKeyInt(L, "Colldown", mr.nColldown)) {
            return false;
        }

        // Load increase
        if (!loadKeyInt(L, "Increase", mr.nIncrease)) {
            return false;
        }

        // Load multiple
        if (!loadKeyInt(L, "Multiple", mr.nMultiple)) {
            return false;
        }

        // Load levels
        lua_getfield(L, -1, "Lvs");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return false;
        }

        int nBaseCost = 0;
        for (int i = 1; ;i++) {
            lua_pushnumber(L, i);
            lua_rawget(L, -2);
            if (!lua_istable(L, -1)) {
                lua_pop(L, 1);
                break;
            }
            // Stack: root_table key value lv_root_table lv_table

            MagicLvRecord lvr;
            lvr.nIndex = i - 1;

            if (!loadKeyInt(L, "LvReq", lvr.nLvReq)) {
                lua_pop(L, 1);
                bRet = false;
                break;
            }
            if (lvr.nLvReq <= 0) {
                lua_pop(L, 1);
                bRet = false;
                break;
            }
            // Load cost
            if (!loadKeyInt(L, "Cost", lvr.nCost)) {
                lua_pop(L, 1);
                bRet = false;
                break;
            }
            if (i == 1) {
                nBaseCost = lvr.nCost;
            }
            if (0 == lvr.nCost) {
                lvr.nCost = nBaseCost + (nBaseCost / 2 * lvr.nIndex);
            }
            // Load increase
            if (!loadKeyInt(L, "Increase", lvr.nIncrease)) {
                lvr.nIncrease = mr.nIncrease;
            }
            if (!loadKeyInt(L, "Multiple", lvr.nMultiple)) {
                lvr.nMultiple = mr.nMultiple;
            }

            lua_pop(L, 1);
            mr.xLvs.push_back(lvr);
        }

        g_xMagicRecords.insert(std::make_pair(mr.nID, mr));
        // Pop lv table
        lua_pop(L, 1);
    }

    // Pop the root table
    lua_pop(L, 1);
    return bRet;
}
