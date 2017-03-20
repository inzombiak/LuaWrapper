#ifndef Lua_ENVIRONMENT_H
#define Lua_ENVIRONMENT_H

#include <string>
#include <map>
#include <memory>


#include "LuaFunction.h"
#include "LuaObject.h"

struct FuncNameInfo
{
    FuncNameInfo(const std::string& newFuncName, const std::string& newGlobaleName = "")
    {
        funcName = newFuncName;
        globalName = newGlobaleName;
    }
    
    std::string funcName;
    std::string globalName = "";
};

class LuaEnvironment
{
public:
	LuaEnvironment(std::string name)
	{
		m_environmentName = name;
	}
	~LuaEnvironment();

	template <typename Ret, typename... Args>
	void RegisterGlobalFunction(lua_State* L, const std::string &name, std::function<Ret(Args...)> function)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());

		auto temp = std::unique_ptr<ILuaFunction>
		{
			new LuaFunction<1, Ret, Args...>{ L, name, function }
		};

		m_functionMap[name] = std::move(temp);
	}

	template <typename Class, typename Ret, typename... Args, class = typename std::enable_if<!(std::is_void<Ret>::value), int>::type>
	void RegisterMemberFunction(lua_State* L, Class *pClass, std::string metatableName, std::string name, Ret(Class::*function)(Args...), std::string funcName)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());

		auto it = m_objectMap.find(name);
		if (it == m_objectMap.end())
		{
			LuaCallDispatcher::GetInstance().CreateGlobalObject(L, name, metatableName, pClass);
			lua_getglobal(L, name.c_str());
			std::function<Ret(Class*, Args...)> lambda =
				[function](Class *t, Args... args) {
				return (t->*function)(args...);
			};
            ClassLuaFunction<1, Class, Ret, Args...>*classFunc = new ClassLuaFunction<1, Class, Ret, Args...>(L, name, funcName, lambda);

			auto temp = std::shared_ptr<LuaObject>(new LuaObject{ L,  metatableName, name, classFunc, funcName });
			m_objectMap[name] = std::move(temp);
		}
		else
		{
			lua_getglobal(L, name.c_str());

			std::function<Ret(Class*, Args...)> lambda =
				[function](Class *t, Args... args) {
				return (t->*function)(args...);
			};
			ClassLuaFunction<1, Class, Ret, Args...>*classFunc = new ClassLuaFunction<1, Class, Ret, Args...>(L, name, funcName, lambda);

			//it->second->AddFunction(classFunc, std::move(funcName));
		}

	}

	template <typename Class, typename Ret, typename... Args, class = typename std::enable_if<(std::is_void<Ret>::value), int>::type>
	void RegisterMemberFunction(lua_State* L, Class *pClass, std::string metatableName, std::string name, void(Class::*function)(Args...), std::string funcName)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());

		auto it = m_objectMap.find(name);
		if (it == m_objectMap.end())
		{
			LuaCallDispatcher::GetInstance().CreateGlobalObject(L, name, metatableName, pClass);
			lua_getglobal(L, name.c_str());
			std::function<Ret(Class*, Args...)> lambda =
				[function](Class *t, Args... args) {
				return (t->*function)(args...);
			};
			ClassLuaFunction<1, Class, Ret, Args...>*classFunc = new ClassLuaFunction<1, Class, Ret, Args...>(L, name, funcName, lambda);

			auto temp = std::shared_ptr<LuaObject>(new LuaObject{ L, metatableName, name, classFunc, funcName });
			m_objectMap[name] = std::move(temp);
		}
		else
		{
			lua_getglobal(L, name.c_str());

			std::function<Ret(Class*, Args...)> lambda =
				[function](Class *t, Args... args) {
				return (t->*function)(args...);
			};
			ClassLuaFunction<1, Class, Ret, Args...>*classFunc = new ClassLuaFunction<1, Class, Ret, Args...>(L, name, funcName, lambda);

			//it->second->AddFunction(classFunc, std::move(funcName));
		}

	}

	template <typename Ret, typename... Args, class = typename std::enable_if<!(std::is_void<Ret>::value), int>::type>
	Ret RunFunction(lua_State* L, const FuncNameInfo& funcNameInfo, const Args&... args)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());

		if (funcNameInfo.globalName.empty())
			return LuaCallDispatcher::GetInstance().CallGlobal<Ret, Args...>(L, funcNameInfo.funcName, args...);
		else
			return LuaCallDispatcher::GetInstance().CallMemberFunction<Ret, Args...>(L, funcNameInfo.globalName, funcNameInfo.funcName, args...);
	}

	template <typename Ret, typename... Args, class = typename std::enable_if<(std::is_void<Ret>::value), int>::type>
	void RunFunction(lua_State* L, const FuncNameInfo& funcNameInfo, Args... args)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());

        if (funcNameInfo.globalName.empty())
            return LuaCallDispatcher::GetInstance().CallGlobal<Ret, Args...>(L, funcNameInfo.funcName, args...);
        else
            return LuaCallDispatcher::GetInstance().CallMemberFunction<Ret, Args...>(L, funcNameInfo.globalName, funcNameInfo.funcName, args...);
	}

	template <typename Ret>
	Ret GetGlobalValue(lua_State* L, std::string name)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());
		return LuaCallDispatcher::GetInstance().GetGlobal<Ret>(L, name);
	}

	std::shared_ptr<LuaObject> GetLuaObject(lua_State* L, const std::string& name, int index = -1, const std::string& globalName = "")
	{
		std::string objName(name);
		if (index != -1)
			objName = name + std::to_string(index);

		auto it = m_objectMap.find(objName);
		if (it != m_objectMap.end())
			return it->second;

		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());

		if (!globalName.empty())
		{
			lua_getfield(L, -1,globalName.c_str());
			lua_getfield(L, -1, name.c_str());
		}
		else
			lua_getglobal(L, name.c_str());
	
		if (index != -1)
		{
			 lua_pushnumber(L, index); 
			 lua_gettable(L, -2); 
		}
			

		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		LuaObject* obj = new LuaObject(L, objName, ref);
		auto temp = std::shared_ptr<LuaObject>(obj);
		m_objectMap[objName] = temp;

		return temp;
	}

	void DeleteLuaObject(lua_State* L, const std::string& name)
	{
		m_objectMap.erase(name);
	}

	template <typename Ret>
	std::vector<Ret> TableToVector(lua_State* L, const std::string& tableName, const std::string& globalName = "")
	{
		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());
		if (!globalName.empty())
			lua_getfield(L, -1, globalName.c_str());

		return LuaCallDispatcher::GetInstance().TableToVector<Ret>(L, tableName);
	}

	template <typename Ret1, typename Ret2>
	std::vector<std::pair<Ret1, Ret2>> TableToVector(lua_State* L, const std::string& tableName, const std::string& globalName = "")
	{
		lua_getfield(L, LUA_REGISTRYINDEX, m_environmentName.c_str());
		if (!globalName.empty())
			lua_getfield(L, -1, globalName.c_str());

		return LuaCallDispatcher::GetInstance().TableToVector<Ret1, Ret2>(L, tableName);
	}
private:

	inline void Clear()
	{
		m_functionMap.clear();
		m_objectMap.clear();

	}

	std::string m_environmentName;
	std::map<std::string, std::unique_ptr<ILuaFunction>> m_functionMap;
	std::map<std::string, std::shared_ptr<LuaObject>> m_objectMap;
};

template<>
std::vector<std::shared_ptr<LuaObject>> LuaEnvironment::TableToVector(lua_State* L, const std::string& tableName, const std::string& globalName);

#endif