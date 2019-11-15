#ifndef _INC_ITEM_AFFIX_
#define _INC_ITEM_AFFIX_

#include <vector>
#include <set>
#include <string>
#include <map>
#include <stdint.h>

#define MAX_AFFIX_SLOT  6
#define BAD_SLOT_VALUE  1 // Without group id

enum ItemAffixEffType {
    ItemAffixEffNone,
    // Slot count, at max 6 slots
    ItemAffixEffSlotCount,
    ItemAffixEffSlot1Used,
    ItemAffixEffSlot2Used,
    ItemAffixEffSlot3Used,
    ItemAffixEffSlot4Used,
    ItemAffixEffSlot5Used,
    ItemAffixEffSlot6Used,
    // Special effect
};

struct ItemAffixRecordGroup;

struct ItemAffixRecord {
    std::string xName;
    // Affix index in group, auto generated
    int nIndex;
    const ItemAffixRecordGroup *group;
    int nGradeReq;
    int nWeight;
    // Specify the value is fixed value or range value
    // If is range value, then the high 16bits are low range value,
    // the low 16bits are high range value [low, high]
    bool bRange;
    unsigned short nValLow;
    unsigned short nValHigh;

    ItemAffixRecord() {
        group = nullptr;
        bRange = false;
        nIndex = 0;
        nWeight = 0;
        nGradeReq = 0;
        nValLow = nValHigh = 0;
    }
};

struct ItemAffixRecordGroup {
    int nGroupID;
    int nWeight;
    bool bPrefix;
    unsigned short nEffType;
    std::vector<ItemAffixRecord> affixes;
    std::set<int> xItemPoses;

    ItemAffixRecordGroup() {
        bPrefix = true;
        nWeight = 0;
        nGroupID = 0;
        nEffType = 0;
    }
};

typedef std::map<int, ItemAffixRecordGroup> ItemAffixRecordGroupMap;

#define pszDefaultItemAffixTableName "config_constItemAffix"

struct lua_State;
class ByteBuffer;

bool LoadItemAffixRecordGroupFromLua(const char *pszKeyName, lua_State *L);
const ItemAffixRecord *TryGetItemAffixRecord(int nSlotCount, int nPrefixCount,
                                             int nSuffixCount, int nItemType,
                                             int nGrade);
const ItemAffixRecord *GetItemAffixRecord(int nGroupID, int nIndex);

// Used by ItemAttrib and Hero attrib
class ItemAffix
{
  public:
    struct EffValue {
        uint32_t val;
        bool range;

        EffValue() {
            val = 0;
            range = false;
        }
        EffValue(int nVal, bool bRange) {
            val = nVal;
            range = bRange;
        }
    };

  public:
    friend ByteBuffer &operator<<(ByteBuffer &buffer, const ItemAffix &ia);
    friend ByteBuffer &operator>>(ByteBuffer &buffer, ItemAffix &ia);

  public:
    ItemAffix();
    ~ItemAffix();

public:
    inline int GetAffixID() {
        return m_nAffixID;
    }
    inline void SetAffixID(int id) {
        m_nAffixID = id;
    }
    inline const std::string &GetName() {
        return m_xName;
    }

    bool InitSlotCount(int nSlot);
    int GetSlotCount();
    int GetFreeSlotCount();

    void UpdateName();
    void AddEff(int nEff, uint32_t nVal, bool bRange);

    bool SlotAttach(const ItemAffixRecord *record);

    void Reset();
    // The value already processed with range and fix mode
    uint32_t GetValue(int nEff, bool raw = false);

    void MergeTo(ItemAffix &affix);

private:
    bool SetValue(int nEff, uint32_t nVal);

private:
    // Bind to hero, and is unique in hero
    std::string m_xName;
    int m_nAffixID;
    std::map<int, EffValue> m_xAttribs;
};

ByteBuffer &operator<<(ByteBuffer &buffer, const ItemAffix::EffValue &value);
ByteBuffer &operator>>(ByteBuffer &buffer, ItemAffix::EffValue &value);

#endif
