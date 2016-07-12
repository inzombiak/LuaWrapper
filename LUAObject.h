#ifndef Lua_OBJECT_H
#define Lua_OBJECT_H

#include "lua.h"
#include "LuaFunction.h"
#include <memory>

class ILuaObject
{
public:
	virtual ~ILuaObject(){};
	virtual void AddFunction(ILuaFunction* function, std::string funcName) = 0;
	//virtual int Apply(lua_State* L) = 0;
};

class LuaObject : public ILuaObject
{
public:
	LuaObject(lua_State* L, const std::string& metatableName, const std::string& name, ILuaFunction* function , const std::string& functionName)
	{
		m_luaState = L;
		m_functions.push_back(std::move(std::unique_ptr<ILuaFunction>{function}));
		m_name = name;

		lua_setglobal(m_luaState, name.c_str());
	}

	LuaObject(lua_State* L, const std::string& name, int ref)
	{
		m_luaState = L;
		m_name = name;
		m_luaRef = ref;
	}

	LuaObject(const LuaObject& other) = delete;

	LuaObject(LuaObject&& other)
		: 
		m_name(other.m_name),
		m_luaState(other.m_luaState)
	{
		other.m_luaState = nullptr;
		m_functions = std::move(other.m_functions);
	}

	~LuaObject()
	{
		m_functions.clear();
		//lua_getglobal(m_luaState, m_name.c_str());
		//lua_pushnil(m_luaState);
		//lua_setfield(m_luaState, -2, "__object");
		//lua_pushnil(m_luaState);
		//lua_setglobal(m_luaState, m_name.c_str());

		luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_luaRef);
	}

	void AddFunction(ILuaFunction* function, std::string funcName)
	{
		m_functions.push_back(std::move(std::unique_ptr<ILuaFunction>{function}));
	};

	template <typename Ret, typename ...Args>
	Ret CallFunction(std::string funcName, Args... args)
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_luaRef);
		lua_getfield(m_luaState, -1, funcName.c_str());

		const int num_args = sizeof...(Args)+1;
		const int num_ret = 1;
		lua_pushvalue(m_luaState, -2);
		LuaCallDispatcher::GetInstance().Push(m_luaState, args...);

		lua_call(m_luaState, num_args, num_ret);

		return LuaCallDispatcher::GetInstance().Pop<Ret>(m_luaState);
	}

	template <typename ...Args>
	void CallFunction(std::string funcName, Args... args)
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_luaRef);
		lua_getfield(m_luaState, -1, funcName.c_str());

		const int num_args = sizeof...(Args)+1;
		lua_pushvalue(m_luaState, -2);
		LuaCallDispatcher::GetInstance().Push(m_luaState, args...);

		lua_call(m_luaState, num_args, 0);

	}

	template <typename Ret, typename ...Args>
	std::vector<Ret> CallFunctionVec(std::string funcName, Args... args)
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_luaRef);
		lua_getfield(m_luaState, -1, funcName.c_str());

		const int num_args = sizeof...(Args)+1;
		const int num_ret = 1;
		lua_pushvalue(m_luaState, -2);
		LuaCallDispatcher::GetInstance().Push(m_luaState, args...);

		lua_call(m_luaState, num_args, num_ret);

		return LuaCallDispatcher::GetInstance().TableToVectorFromTop<Ret>(m_luaState);
	}

	template <typename Ret1, typename Ret2, typename ...Args>
	std::vector<std::pair<Ret1, Ret2>> CallFunctionVec(std::string funcName, Args... args)
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_luaRef);
		lua_getfield(m_luaState, -1, funcName.c_str());

		const int num_args = sizeof...(Args)+1;
		const int num_ret = 1;
		lua_pushvalue(m_luaState, -2);
		LuaCallDispatcher::GetInstance().Push(m_luaState, args...);

		lua_call(m_luaState, num_args, num_ret);

		return LuaCallDispatcher::GetInstance().TableToVectorFromTop<Ret1, Ret2>(m_luaState);
	}

	const std::string& GetName() const
	{
		return m_name;
	}

private:
	lua_State* m_luaState;
	std::string m_name;
	std::vector<std::unique_ptr<ILuaFunction>> m_functions;
	int m_luaRef;
};

#endif