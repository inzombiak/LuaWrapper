#include "LuaEnvironment.h"
LuaEnvironment::~LuaEnvironment()
{
	Clear();
}


template<>
std::vector<std::shared_ptr<LuaObject>> LuaEnvironment::TableToVector(lua_State* L, const std::string& tableName, const std::string& globalName)
{
	lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());
	if (!globalName.empty())
		lua_getfield(L, -1, globalName.c_str());

	std::vector<std::shared_ptr<LuaObject>> result;
	lua_getfield(L, -1, tableName.c_str());

	int size = (int)luaL_len(L, -1);

	lua_settop(L, -1);

	for (int i = 1; i <= size; ++i)
		result.push_back(std::shared_ptr<LuaObject>{GetLuaObject(L, tableName, i, globalName)});

	lua_pop(L, -1);

	return result;
}