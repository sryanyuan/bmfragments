local ITEM_NO		=	0
local ITEM_CLOTH	=	2
local ITEM_NECKLACE=	3
local ITEM_BRACELAT=	4
local ITEM_RING	=	5
local ITEM_MEDAL	=	6
local ITEM_HELMET	=	7
local ITEM_WEAPON	=	9
local ITEM_SHOE	=	14
local ITEM_BELT	=	15
local ITEM_GEM		=	16
local ITEM_CHARM	=	17

-- Item affix
config_constItemAffix = {
	{
		ID = 1,
		Format = "一定几率产生{vallow}-{valhigh}点毒素伤害，持续{dura}秒";
		Weight = 200,
		Prefix = true,
		Range = true,
		Type = ItemAffixEffAttackPoison,
		Poses = {ITEM_NECKLACE, ITEM_RING, ITEM_GEM},
		Affixes = {
			{
				Name = "腐败",
				Grade = 0,
				Weight = 100,
				LowValue = 1,
				HighValue = 2,
				Dura = 4,
			},
			{
				Name = "腐烂",
				Grade = 1,
				Weight = 80,
				LowValue = 1,
				HighValue = 3,
				Dura = 4,
			},
			{
				Name = "腐蚀",
				Grade = 3,
				Weight = 60,
				LowValue = 2,
				HighValue = 5,
				Dura = 4,
			},
			{
				Name = "剧毒",
				Grade = 4,
				Weight = 10,
				LowValue = 3,
				HighValue = 6,
				Dura = 4,
			},
			{
				Name = "瘟疫",
				Grade = 6,
				Weight = 5,
				LowValue = 4,
				HighValue = 8,
				Dura = 4,
			}
		}
	}
}
