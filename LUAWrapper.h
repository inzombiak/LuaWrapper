#ifndef LUA_WRAPPER_H
#define LUA_WRAPPER_H

#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <stdio.h>

#include "LuaEnvironment.h"

class LuaWrapper
{
public:
	~LuaWrapper();
    
   
    
	static LuaWrapper& GetInstance() 
	{
		static LuaWrapper instance;
		return instance;
	}
	void CleanStack()
	{
		int stackSize = lua_gettop(m_luaState);
		lua_pop(m_luaState, stackSize);
	}
	void LoadScript(const std::string& environmentName, const std::string& filename);
	std::vector<int> GetIntVector(const std::string& name);
	std::vector<std::string> GetTableKeys(const std::string& name);

	template <typename Ret, typename... Args>
	void RegisterGlobalFunction(const std::string& envName, const std::string &name, std::function<Ret(Args...)> function)
	{
		if (!FindLuaEnvironment(envName))
			return;

		m_environmentMap[envName]->RegisterGlobalFunction<Ret>(m_luaState, name, function);
		CleanStack();
	}

	template <typename Class, typename Ret, typename... Args, class = typename std::enable_if<!(std::is_void<Ret>::value), int>::type>
	void RegisterMemberFunction(const std::string& envName, Class *pClass, const std::string& metatableName, const std::string& name, Ret(Class::*function)(Args...), const std::string& funcName)
	{
		if (!FindLuaEnvironment(envName))
			return;

		m_environmentMap[envName]->RegisterMemberFunction(m_luaState, pClass, metatableName, name, function, funcName);
		CleanStack();
	}
	template <typename Class, typename Ret, typename... Args, class = typename std::enable_if<(std::is_void<Ret>::value), int>::type>
	void RegisterMemberFunction(const std::string& envName, Class *pClass, const std::string& metatableName, const std::string& name, void(Class::*function)(Args...), const std::string& funcName)
	{
		if (!FindLuaEnvironment(envName))
			return;

		m_environmentMap[envName]->RegisterMemberFunction<Class,Ret>(m_luaState, pClass, metatableName, name, function, funcName);
		CleanStack();
	}

	template <typename Ret, typename... Args, class = typename std::enable_if<!(std::is_void<Ret>::value), int>::type>
	Ret RunFunction(const std::string& envName,const FuncNameInfo& nameInfo, Args... args)
	{
		if (!FindLuaEnvironment(envName))
			return Ret();

		Ret result = m_environmentMap[envName]->RunFunction<Ret>(m_luaState, nameInfo, args...);
		CleanStack();
		return result;

	}
	template <typename Ret, typename... Args, class = typename std::enable_if<(std::is_void<Ret>::value), int>::type>
	void RunFunction(const std::string& envName,const FuncNameInfo& nameInfo, Args... args)
	{
		if (!GetInstance().FindLuaEnvironment(envName))
			return;

		m_environmentMap[envName]->RunFunction<void>(m_luaState, nameInfo, args...);
		CleanStack();
	}

	template <typename Ret>
	Ret GetGlobalValue(const std::string& envName, const std::string& name)
	{
		if (!FindLuaEnvironment(envName))
			return Ret();

		auto result = m_environmentMap[envName]->GetGlobalValue<Ret>(m_luaState, name);
		CleanStack();

		return result;
		
	}

	std::shared_ptr<LuaObject> GetLuaObject(const std::string& envName, const std::string& name, int index = -1, const std::string& globalName = "")
	{
		if (!FindLuaEnvironment(envName))
			return nullptr;

		auto result = m_environmentMap[envName]->GetLuaObject(m_luaState, name, index, globalName);
		CleanStack();

		return result;
	}
	void DeleteLuaObject(const std::string& envName, const std::string& name)
	{
		if (!FindLuaEnvironment(envName))
			return;

		m_environmentMap[envName]->DeleteLuaObject(m_luaState, name);

		CleanStack();
	}
	template <typename Ret>
	std::vector<Ret> TableToVector(const std::string& envName, const std::string& tableName, const std::string& globalName = "")
	{
		if (!FindLuaEnvironment(envName))
			return std::vector<Ret>();

		auto result = m_environmentMap[envName]->TableToVector<Ret>(m_luaState, tableName, globalName);

		CleanStack();
		return result;
		
	}
	template <typename Ret1, typename Ret2>
	std::vector<std::pair<Ret1, Ret2>> TableToVector(const std::string& envName, const std::string& tableName, const std::string& globalName = "")
	{

		if (!FindLuaEnvironment(envName))
			return std::vector<std::pair<Ret1, Ret2>>();

		auto result = m_environmentMap[envName]->TableToVector<Ret1, Ret2>(m_luaState, tableName, globalName);

		CleanStack();
		return result;
	}

	static lua_State* GetState()
	{
		return m_luaState;
	}

	void Clear()
	{
		m_environmentMap.clear();

		//THIS IS WRONG
		lua_close(m_luaState);
		m_luaState = luaL_newstate();
		luaL_openlibs(m_luaState);
	}
    
    static void StackDump()
    {
        int i;
        int top = lua_gettop(m_luaState);
        for (i = 1; i <= top; i++) {  /* repeat for each level */
            int t = lua_type(m_luaState, i);
            switch (t) {
                    
                case LUA_TSTRING:  /* strings */
                    printf("`%s'", lua_tostring(m_luaState, i));
                    break;
                    
                case LUA_TBOOLEAN:  /* booleans */
                    printf(lua_toboolean(m_luaState, i) ? "true" : "false");
                    break;
                    
                case LUA_TNUMBER:  /* numbers */
                    printf("%g", lua_tonumber(m_luaState, i));
                    break;
                    
                default:  /* other values */
                    printf("%s", lua_typename(m_luaState, t));
                    break;
                    
            }
            printf("  ");  /* put a separator */
        }
        printf("\n");  /* end the listing */
    }

private:
	void CreateLuaEnvironment(const std::string& name);
	bool FindLuaEnvironment(const std::string& name) const;

	LuaWrapper();

	void PrintError(const std::string& variableName, const std::string& reason);

	static lua_State* m_luaState;
	int level;

	std::map<std::string, std::unique_ptr<LuaEnvironment>> m_environmentMap;
};

#endif
