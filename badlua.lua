5.1.4

lua_getglobal(L, "DropDownMonsterItemsLua");
	if (!lua_isfunction(L, -1)) {
		g_xConsole.CPrint("Nil lua call function");
		lua_pop(L, 1);
		SAFE_DELETE(_pParam);
		return 0;
	}
	lua_newtable(L);
	MonsDropItemInfoVec *pVec = nullptr;
	GetRecordsInMonsDropTable(int(_pParam->dwParam[0]), &pVec);
	if (nullptr != pVec) {
		int nLuaIndex = 1;
		for (auto &di : *pVec) {
			lua_newtable(L);
			lua_pushstring(L, "item");
			lua_pushnumber(L, di.nItemId);
			lua_settable(L, -3);
			lua_pushstring(L, "prob");
			lua_pushnumber(L, di.nProb);
			lua_settable(L, -3);
			lua_rawseti(L, -2, nLuaIndex++);
		}
	}
	tolua_pushusertype(L, &xLuaDropContext, "DBDropDownContext");
	int nRet = lua_pcall(L, 2, 0, 0);
	if (0 != nRet)
	{
		ConsolePrint(lua_tostring(L, -1));
		lua_pop(L, 1);
	}
  
  
  local function getDropItemIDTableFromTable(_datas, _prob)
	local finddata = _data
	local dropDownItems = {}

	if _prob == nil or
	_prob == 0 then
		_prob = 1
	end

	--	calculate property
	local probMulti = _prob
	local loop = 0
	local length = #_datas
	--for i = 1, length do
	for i, v in pairs(_datas) do
		--local v = _datas[i]
		local prob = v.prob
		local itemId = v.item
		loop = loop + 1

		local actProb = prob
		if _prob ~= 1 then
			--actProb = div(actProb, probMulti)
			actProb = actProb / probMulti
		end

		if actProb < 1 then
			actProb = 1
		end

		local ra = math.random(1, actProb)
		if ra == 1 then
			table.insert(dropDownItems, itemId)
		end
	end
	
	ConsolePrint("Looped"..loop.."Len"..length.."Prob"..probMulti)
	
	if loop <= 1 then
		dump(_datas)
	end

	return dropDownItems
end

function DropDownMonsterItemsLua(_dropTable, _context)
	if not _context:IsDropValid() then
		ConsolePrint("Context not valid")
		return
	end

	local dropDownItems = getDropItemIDTableFromTable(_dropTable, _context:GetDropMultiple())

	local dropItemsCounter = #dropDownItems
	if dropItemsCounter == 0 then
		ConsolePrint("Drop down items counter is 0")
		return
	end

	local canDropSum = _context:InitDropPosition(dropItemsCounter)

	if DEBUG then
		--ConsolePrint("可掉落物品数:"..canDropSum)
	end

	for i = 0, canDropSum - 1 do
		local posX = _context:GetDropPosX(i)
		local posY = _context:GetDropPosY(i)
		local owner = _context:GetOwnerID()

		if 0 == posX or
		0 == posY then
			break
		end

		local dropItemID = dropDownItems[i + 1]
		local dropItem = _context:NewGroundItem(dropItemID, posX, posY, owner)
		if nil == dropItem then
			break
		end

		--	upgrade items
		updateItemWithContext(dropItem:GetItemAttrib(), _context)
		GameWorld:GetInstancePtr():InitDura(dropItem:GetItemAttrib())
		
		-- Init item affix
		GameWorld:GetInstancePtr():InitItemAffix(dropItem, 1)
	end
end
