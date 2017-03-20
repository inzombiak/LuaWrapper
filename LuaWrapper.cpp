#include "LuaWrapper.h"
#include "LuaCallDispatcher.h"

using std::cout;
using std::endl;

LuaWrapper::LuaWrapper()
{
	m_luaState = luaL_newstate();
	luaL_openlibs(m_luaState);
}


LuaWrapper::~LuaWrapper()
{
	m_environmentMap.clear();
	if (m_luaState)
		lua_close(m_luaState);
}

void LuaWrapper::LoadScript(const std::string& environmentName, const std::string& filename)
{

	cout << "LOADING: " << filename << endl;
	if (!m_luaState)
		return;
	if (luaL_loadfile(m_luaState, filename.c_str()))
	{
		std::string msg = lua_tostring(m_luaState, -1);
		LuaCallDispatcher::GetInstance().stackDump(m_luaState);
		std::cout << "Error: failed to load (" << filename << ")" << "Error: " << msg << std::endl;
	}

	lua_newtable(m_luaState);
	lua_newtable(m_luaState);
	lua_getglobal(m_luaState, "_G");
	lua_setfield(m_luaState, -2, "__index");
	lua_setmetatable(m_luaState, -2);
	lua_setfield(m_luaState, LUA_REGISTRYINDEX, environmentName.c_str());
	lua_getfield(m_luaState, LUA_REGISTRYINDEX, environmentName.c_str());
	lua_setupvalue(m_luaState, 1, 1);

	if (lua_pcall(m_luaState, 0, LUA_MULTRET, 0))
	{
		std::string msg = lua_tostring(m_luaState, -1);
		LuaCallDispatcher::GetInstance().stackDump(m_luaState);
		std::cout << "Error: failed to load (" << filename << ")" << "Error: " << msg << std::endl;
	}

	m_environmentMap[environmentName] = std::unique_ptr<LuaEnvironment>(new LuaEnvironment(environmentName));
}


void LuaWrapper::PrintError(const std::string& variableName, const std::string& reason)
{
	std::cout << "Error: can't get [" << variableName << "]. " << reason << std::endl;
}

void LuaWrapper::CreateLuaEnvironment(const std::string& name)
{
	if (FindLuaEnvironment(name))
		return;

	lua_getupvalue(m_luaState, -1, 1);
	LuaCallDispatcher::GetInstance().stackDump(m_luaState);

	m_environmentMap[name] = std::unique_ptr<LuaEnvironment>(new LuaEnvironment(name));
}

bool LuaWrapper::FindLuaEnvironment(const std::string& name) const
{
	auto it = m_environmentMap.find(name);
	if (it == m_environmentMap.end())
		return false;

	return true;
}

lua_State* LuaWrapper::m_luaState = luaL_newstate();
