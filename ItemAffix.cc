#include "ItemAffix.h"
#include "LuaStackGuard.h"
#include "bytebuffer.h"

#include <lua.hpp>

ItemAffixRecordGroupMap g_itemAffixGroupMap;

static const int slotposKey[] = {
    ItemAffixEffSlot1Used, ItemAffixEffSlot2Used, ItemAffixEffSlot3Used,
    ItemAffixEffSlot4Used, ItemAffixEffSlot5Used, ItemAffixEffSlot6Used,
};

ItemAffix::ItemAffix() {
    m_nAffixID = 0;
}

ItemAffix::~ItemAffix() {

}

void ItemAffix::Reset() {
    m_nAffixID = 0;
    m_xAttribs.clear();
}

bool ItemAffix::InitSlotCount(int nCount) {
    if (nCount <= 0 || nCount > MAX_AFFIX_SLOT) {
        return false;
    }
    // Check already init slot
    int nCurSlot = GetValue(ItemAffixEffSlotCount);
    if (nCurSlot != 0) {
        return false;
    }
    SetValue(ItemAffixEffSlotCount, nCount);
    return true;
}

int ItemAffix::GetSlotCount() {
    return GetValue(ItemAffixEffSlotCount);
}

int ItemAffix::GetFreeSlotCount() {
    int nCount = GetValue(ItemAffixEffSlotCount);
    int nUsed = 0;

    for (int i = 0; i < nCount; i++) {
        if (GetValue(slotposKey[i]) != 0) {
            ++nUsed;
        }
    }
    return nCount - nUsed;
}

void ItemAffix::AddEff(int nEff, uint32_t nVal, bool bRange) {
    auto it = m_xAttribs.find(nEff);
    if (it == m_xAttribs.end()) {
        m_xAttribs.insert(std::make_pair(nEff, EffValue(nVal, bRange)));
        return;
    }

    assert(it->second.range == bRange);
    if (it->second.range) {
        int vallow = nVal >> 16;
        int valhigh = nVal & 0x0000ffff;
        int cvallow = it->second.val >> 16;
        int cvalhigh = it->second.val & 0x0000ffff;
        it->second.val = ((vallow + cvallow) << 16) | ((valhigh + cvalhigh));
    } else {
        it->second.val += nVal;
    }
}

bool ItemAffix::SlotAttach(const ItemAffixRecord *record) {
    int nSlot = GetValue(ItemAffixEffSlotCount);
    if (nSlot <= 0 || nSlot > MAX_AFFIX_SLOT)
    {
        return false;
    }

    for (int i = 0; i < nSlot; i++) {
        int nUsed = GetValue(slotposKey[i]);
        if (0 != nUsed) {
            continue;
        }
        if (nullptr == record) {
            // The slot is used and has no value
            SetValue(slotposKey[i], BAD_SLOT_VALUE);
            return true;
        }
        // Use the slot
        uint32_t slot_val = ((record->group->nGroupID << 16) | record->nIndex);
        SetValue(slotposKey[i], slot_val);
        // Add the value
        if (0 != record->bRange) {
            // Range value
            if ((0 != record->nValHigh || 0 != record->nValLow) &&
                record->nValHigh >= record->nValLow) {
                int middle = (record->nValHigh + record->nValLow + 1) / 2;
                int vallow = record->nValLow + rand() % (middle - record->nValLow);
                int valhigh = middle + rand() % (record->nValHigh - middle);
                if (valhigh >= vallow) {
                    AddEff(record->group->nEffType, (vallow << 16) | (valhigh & 0x0000ffff), true);
                    return true;
                }
            }
        } else {
            if (0 != record->nValHigh || 0 != record->nValLow) {
                int nVal = 0;
                if (record->nValLow != 0 && record->nValHigh == 0) {
                    // Fix value
                    nVal = record->nValLow;
                } else if (record->nValHigh != 0 && record->nValLow != 0 &&
                           record->nValHigh >= record->nValLow) {
                    nVal = record->nValLow +
                           rand() % (record->nValHigh - record->nValLow + 1);
                }
                if (nVal != 0) {
                    AddEff(record->group->nEffType, nVal, false);
                    return true;
                }
            }
        }

        break;
    }
    return false;
}

uint32_t ItemAffix::GetValue(int nEff, bool raw) {
    auto it = m_xAttribs.find(nEff);
    if (it == m_xAttribs.end()) {
        return 0;
    }

    if (raw) {
        return it->second.val;
    }
    
    if (it->second.range) {
        int vallow = it->second.val >> 16;
        int valhigh = it->second.val & 0x0000ffff;
        return vallow + rand() % (valhigh - vallow + 1);
    }
    return it->second.val;
}

bool ItemAffix::SetValue(int nEff, uint32_t nVal) {
    auto it = m_xAttribs.find(nEff);
    if (it == m_xAttribs.end()) {
        m_xAttribs.insert(std::make_pair(nEff, EffValue(nVal, false)));
        return true;
    }
    it->second.val = nVal;
    return false;
}

void ItemAffix::UpdateName() {
    int nSlot = GetValue(ItemAffixEffSlotCount);
    if (nSlot <= 0 || nSlot > MAX_AFFIX_SLOT)
    {
        return;
    }

    const ItemAffixRecord *best_prefix = nullptr;
    const ItemAffixRecord *best_suffix = nullptr;

    m_xName.clear();
    for (int i = 0; i < nSlot; i++) {
        uint32_t nUsed = GetValue(slotposKey[i]);
        if (nUsed == BAD_SLOT_VALUE) {
            continue;
        }
        if (0 == nUsed) {
            break;
        }
        int nGroupID = nUsed >> 16;
        int nIndex = (nUsed & 0x0000ffff);

        auto affix = GetItemAffixRecord(nGroupID, nIndex);
        if (nullptr == affix) {
            continue;
        }
        if (affix->group->bPrefix) {
            if (nullptr == best_prefix ||
                affix->nGradeReq > best_prefix->nGradeReq) {
                best_prefix = affix;
            }
        } else {
            if (nullptr == best_suffix ||
                affix->nGradeReq > best_suffix->nGradeReq) {
                best_suffix = affix;
            }
        }
    }

    if (nullptr != best_prefix) {
        m_xName += best_prefix->xName + "TODO:ZHI";
    }
    if (nullptr != best_suffix) {
        m_xName += best_suffix->xName;
    }
}




bool LoadItemAffixRecordGroupFromLua(const char *pszKeyName, lua_State *L) {
    LuaStackGuard guard(L);

    lua_getglobal(L, pszKeyName);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    // Stack: top_table
    for (int ti = 1; ; ti++) {
        lua_pushnumber(L, ti);
        lua_rawget(L, -2);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            break;
        }
        // Stack: top_table group_table

        ItemAffixRecordGroup group;
        // Get group id
        if (!loadKeyInt(L, "ID", group.nGroupID)) {
            return false;
        }
        if (group.nGroupID <= 0) {
            return false;
        }
        // Get weight
        if (!loadKeyInt(L, "Weight", group.nWeight)) {
            return false;
        }
        // Get is prefix
        int prefix = 0;
        if (!loadKeyInt(L, "Prefix", prefix)) {
            return false;
        }
        group.bPrefix = prefix != 0;
        // Get type
        int nType = 0;
        if (!loadKeyInt(L, "Type", nType)) {
            return false;
        }
        group.nEffType = nType;
        // Get poses
        std::vector<int> poses;
        if (!loadKeyIntArray(L, "Poses", poses)) {
            return false;
        }
        for (auto v : poses) {
            group.xItemPoses.insert(v);
        }
        // Insert the group
        auto group_it = g_itemAffixGroupMap.insert(std::make_pair(group.nGroupID, std::move(group)));
        // Get affixes
        lua_getfield(L, -1, "Affixes");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return false;
        }
        // Stack: top_table group_table affixes_table
        for (int i = 1; ; i++) {
            lua_pushnumber(L, i);
            lua_rawget(L, -2);
            if (!lua_istable(L, -1)) {
                lua_pop(L, 1);
                break;
            }
            // Stack: top_table group_table affixes_table affix_table
            ItemAffixRecord record;
            // Grade
            if (!loadKeyInt(L, "Grade", record.nGradeReq)) {
                return false;
            }
            // Weight
            if (!loadKeyInt(L, "Weight", record.nWeight)) {
                return false;
            }
            // Val low and high
            int nVal = 0;
            if (!loadKeyInt(L, "LowValue", nVal)) {
                return false;
            }
            record.nValLow = nVal;
            if (!loadKeyInt(L, "HighValue", nVal)) {
                return false;
            }
            record.nValHigh = nVal;
            // Range
            nVal = 0;
            if (!loadKeyInt(L, "Range", nVal)) {
                return false;
            }
            if (nVal != 0) {
                record.bRange = true;
            } else {
                record.bRange = false;
            }
            // Name
            char szName[32];
            if (!loadKeyString(L, "Name", szName)) {
                return false;
            }
            record.xName = szName;

            record.nIndex = i;
            record.group = &group_it.first->second;

            group_it.first->second.affixes.push_back(record);

            // Pop the table: top_table group_table affixes_table
            lua_pop(L, 1);
        }
        // Pop the table: top_table group_table
        lua_pop(L, 1);
        // Group is done
        lua_pop(L, 1);
    }

    // Pop the value table
    lua_pop(L, 1);

    int top = lua_gettop(L);
    return true;
}

const ItemAffixRecord * GetItemAffixRecord(int nGroupID, int nIndex) {
    auto it = g_itemAffixGroupMap.find(nGroupID);
    if (it == g_itemAffixGroupMap.end()) {
        return nullptr;
    }
    if (nIndex < 0 || nIndex >= it->second.affixes.size()) {
        return nullptr;
    }
    return &it->second.affixes[nIndex];
}

const ItemAffixRecord *
TryGetItemAffixRecord(int nSlotCount, int nPrefixCount,
                      int nSuffixCount, int nItemType, int nGrade) {
    if (nPrefixCount + nSuffixCount >= nSlotCount) {
        return nullptr;
    }

    bool bPrefixOnly = true;
    if (nPrefixCount < (nSlotCount + 1) / 2 &&
        nSuffixCount < (nSlotCount + 1) / 2) {
        if (rand() % 2 == 0) {
            bPrefixOnly = false;
        }
    } else if (nPrefixCount >= (nSlotCount + 1) / 2) {
        bPrefixOnly = false;
    }
    int nTotalWeight = 0;
    std::map<int, const ItemAffixRecordGroup*> xProbAffixes;

    for (auto &group : g_itemAffixGroupMap) {
        if (0 == group.second.xItemPoses.count(nItemType)) {
            continue;
        }
        if (bPrefixOnly != group.second.bPrefix) {
            continue;
        }
        if (0 == group.second.nWeight) {
            continue;
        }
        nTotalWeight += group.second.nWeight;
        xProbAffixes.insert(std::make_pair(nTotalWeight, &group.second));
    }
    if (xProbAffixes.empty()) {
        return nullptr;
    }
    // Get by weight
    int nWeight = rand() % nTotalWeight;
    nTotalWeight = 0;
    const ItemAffixRecordGroup *pTargetGroup = nullptr;
    for (auto &group : xProbAffixes) {
        nTotalWeight += group.first;
        if (nWeight < nTotalWeight) {
            pTargetGroup = group.second;
            break;
        }
    }
    if (nullptr == pTargetGroup) {
        return nullptr;
    }
    // Find in group
    nTotalWeight = 0;
    const ItemAffixRecord *pTargetRecord = nullptr;
    if (pTargetGroup->affixes.empty()) {
        return nullptr;
    }
    for (auto & record : pTargetGroup->affixes) {
        nTotalWeight += record.nWeight;
    }
    if (0 == nTotalWeight) {
        int nIndex = rand() % pTargetGroup->affixes.size();
        while (pTargetGroup->affixes[nIndex].nGradeReq > nGrade &&
               nIndex >= 0) {
            nIndex--;
        }
        if (nIndex < 0) {
            return nullptr;
        }
        return &pTargetGroup->affixes[nIndex];
    } else {
        // Find by weight
        nWeight = rand() % nTotalWeight;
        nTotalWeight = 0;
        for (auto & record : pTargetGroup->affixes) {
            nTotalWeight += record.nWeight;
            if (nWeight < nTotalWeight) {
                return &record;
            }
        }
    }

    return nullptr;
}

ByteBuffer &operator<<(ByteBuffer &buffer, const ItemAffix::EffValue &value) {
    buffer << value.range << value.val;
    return buffer;
}

ByteBuffer &operator>>(ByteBuffer &buffer, ItemAffix::EffValue &value) {
    buffer >> value.range >> value.val;
    return buffer;
}

ByteBuffer &operator<<(ByteBuffer &buffer, const ItemAffix &ia) {
    buffer << ia.m_nAffixID << ia.m_xAttribs;
    return buffer;
}

ByteBuffer &operator>>(ByteBuffer &buffer, ItemAffix &ia) {
    buffer >> ia.m_nAffixID >> ia.m_xAttribs;
    return buffer;
}
