#ifndef Lua_FUNCTION_H
#define Lua_FUNCTION_H

#include "LuaCallDispatcher.h"

#include <string>
#include <functional>
#include <tuple>
#include <iostream>

class ILuaFunction
{
public:
	virtual ~ILuaFunction() {}
	virtual int Apply(lua_State *L) = 0;

};

namespace detail
{
	inline int LuaDispatcher(lua_State* L)
	{
		ILuaFunction* function = (ILuaFunction *)lua_touserdata(L, lua_upvalueindex(1));
		return function->Apply(L);
	}

	template<typename T>
	T GetAndCheckType(lua_State* L, int i);

	template <std::size_t... Is>
	struct Indices {};

	// Recursively inherits from itself until...
	template <std::size_t N, std::size_t... Is>
	struct IndicesBuilder : IndicesBuilder<N - 1, N - 1, Is...> {};

	// The base case where we define the type tag
	template <std::size_t... Is>
	struct IndicesBuilder<0, Is...>
	{
		using type = Indices<Is...>;
	};

	template <typename... T, typename std::size_t... N>
	inline std::tuple<T...> GetArguements(lua_State* L, Indices<N...>)
	{
		return std::tuple<T...>(GetAndCheckType<T>(L, N + 1)...);
	}

	template <typename... T>
	inline std::tuple<T...> GetArguements(lua_State* L)
	{
		const std::size_t num = sizeof...(T);
		return GetArguements<T...>(L, typename IndicesBuilder<num>::type());
	}

	template <typename Ret, typename... Args, std::size_t... N>
	Ret Lift(std::function<Ret(Args...)> function, std::tuple<Args...> args, Indices<N...>)
	{
		return function(std::get<N>(args)...);
	}

	template <typename Ret, typename... Args>
	Ret Lift(std::function<Ret(Args...)> function, std::tuple<Args...> args)
	{
		return Lift(function, args, typename IndicesBuilder<sizeof...(Args)>::type());
	}
}

template <int N, typename Ret, typename... Args>
class LuaFunction : public ILuaFunction
{
private:
	using FunctionType = std::function<Ret(Args...)>;
	FunctionType m_function;
	std::string m_name = "";
	lua_State* m_luaState; // Used for destruction

public:
    LuaFunction(lua_State* L, const std::string& name, Ret(*function)(Args...))
    {
        fLuaFunction(L, name, FunctionType{ function });
    };

	LuaFunction(lua_State* L, const std::string& name, FunctionType(function)) : m_function(function)
	{
        m_name = name;
		m_luaState = L;
		// Add pointer to this object to the closure
		lua_pushlightuserdata(m_luaState, (void*) static_cast<ILuaFunction*>(this));

		// Push our dispatcher with the above upvalue
		lua_pushcclosure(m_luaState, &detail::LuaDispatcher, 1);

		// Bind it to the specified name
		lua_setglobal(m_luaState, name.c_str());
	}

	LuaFunction(const LuaFunction& other) = delete;

	LuaFunction(LuaFunction&& other)
		: m_function(other.m_function),
		m_luaState(other.m_luaState)
	{
        m_name = other.m_name;
		*other.m_luaState = nullptr;
	}
	~LuaFunction()
	{
		if (m_luaState != nullptr)
		{
			lua_pushnil(m_luaState);
			lua_setglobal(m_luaState, m_name.c_str());
		}
	}

	int Apply(lua_State* L) 
	{
	
		std::tuple<Args...> args = detail::GetArguements<Args...>(L);
		
		Ret value = detail::Lift(m_function, args);
		LuaCallDispatcher::GetInstance().Push(L, std::forward<Ret>(value));

		return N;
	}
};

template <int N, class C, typename Ret, typename... Args>
class ClassLuaFunction : public ILuaFunction
{
private:
	using FunctionType = std::function<Ret(C*, Args...)>;
	FunctionType m_function;
	std::string m_funcName;
	std::string m_objName;
	lua_State* m_luaState; // Used for destruction

	C*_get(lua_State *state) 
	{
		C *ret = (C *)lua_touserdata(state, -2);
		lua_remove(state, 1);
		return ret;
	}

public:
	ClassLuaFunction(lua_State* L, const std::string& objName, const std::string& name, Ret(*function)(Args...), C* c) : ClassLuaFunction(L, objName, name, FunctionType(function)) {};

	ClassLuaFunction(lua_State* L, const std::string& objName, const std::string& funcName, FunctionType function) : m_function(function), m_funcName(funcName), m_objName(objName)
	{
		m_luaState = L;
		// Add pointer to this object to the closure
		lua_pushlightuserdata(m_luaState, (void*) static_cast<ILuaFunction*>(this));

		// Push our dispatcher with the above upvalue
		lua_pushcclosure(m_luaState, &detail::LuaDispatcher, 1);

		// Bind it to the specified name
		lua_setfield(m_luaState, -2, funcName.c_str());

	}

	ClassLuaFunction(const ClassLuaFunction& other) = delete;

	ClassLuaFunction(ClassLuaFunction&& other)
		: m_function(other.m_function),
		m_funcName(other.m_funcName),
		m_luaState(other.m_luaState),
		m_objName(other.m_objName)
	{
		*other.m_luaState = nullptr;
	}
	~ClassLuaFunction()
	{
		if (m_luaState != nullptr)
		{
			lua_getglobal(m_luaState, m_objName.c_str());
			lua_pushnil(m_luaState);
			lua_setfield(m_luaState, -2, m_funcName.c_str());
		}
	}

	int Apply(lua_State* L)
	{
		std::tuple<C*> t = std::make_tuple(_get(L));
		std::tuple<Args...> args = detail::GetArguements<Args...>(L);
		std::tuple<C*, Args...> pack = std::tuple_cat(t, args);
		Ret value = detail::Lift(m_function, pack);
		LuaCallDispatcher::GetInstance().Push(L, std::forward<Ret>(value));
		return N;
	}
};

template <class C, typename... Args>
class ClassLuaFunction<1, C, void, Args...> : public ILuaFunction
{
private:
	using FunctionType = std::function<void(C*, Args...)>;
	FunctionType m_function;
	std::string m_funcName;
	std::string m_objName;
	lua_State* m_luaState; // Used for destruction

	C*_get(lua_State *state)
	{
		C *ret = (C *)lua_touserdata(state, -2);
		lua_remove(state, 1);
		return ret;
	}

public:
	ClassLuaFunction(lua_State* L, const std::string& objName, const std::string& name, void(*function)(Args...), C* c) : ClassLuaFunction(L, objName, name, FunctionType(function)) {};

	ClassLuaFunction(lua_State* L, const std::string& objName, const std::string& funcName, FunctionType function) : m_function(function), m_funcName(funcName), m_objName(objName)
	{
		m_luaState = L;
		// Add pointer to this object to the closure
		lua_pushlightuserdata(m_luaState, (void*) static_cast<ILuaFunction*>(this));

		// Push our dispatcher with the above upvalue
		lua_pushcclosure(m_luaState, &detail::LuaDispatcher, 1);

		// Bind it to the specified name
		lua_setfield(m_luaState, -2, funcName.c_str());

	}

	ClassLuaFunction(const ClassLuaFunction& other) = delete;

	ClassLuaFunction(ClassLuaFunction&& other)
		: m_function(other.m_function),
		m_funcName(other.m_funcName),
		m_luaState(other.m_luaState),
		m_objName(other.m_objName)
	{
		*other.m_luaState = nullptr;
	}
	~ClassLuaFunction()
	{
		if (m_luaState != nullptr)
		{
			lua_getglobal(m_luaState, m_objName.c_str());
			lua_pushnil(m_luaState);
			lua_setfield(m_luaState,-2, m_funcName.c_str());
		}
	}

	int Apply(lua_State* L)
	{

		std::tuple<C*> t = std::make_tuple(_get(L));
		std::tuple<Args...> args = detail::GetArguements<Args...>(L);
		std::tuple<C*, Args...> pack = std::tuple_cat(t, args);
		detail::Lift(m_function, pack);

		return 0;
	}
};

#endif