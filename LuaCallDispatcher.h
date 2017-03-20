#ifndef LUA_CALL_DISPATCHER_H
#define LUA_CALL_DISPATCHER_H

#define MIN_RETURN = 0
#include <tuple>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>


#include <type_traits>

class LuaCallDispatcher
{
public:

	struct LuaUserData
	{
		LuaUserData(const void* value = 0)
        {
            m_value = value;
		}
		const void* m_value;
	};

	static LuaCallDispatcher& GetInstance()
	{
		static LuaCallDispatcher dispatch;
		return dispatch;
	}

	inline void Push() {};
	inline void Push(lua_State* L, bool value)				{ lua_pushboolean(L, value); }
	inline void Push(lua_State* L, char value)				{ lua_pushnumber(L, value); }
	inline void Push(lua_State* L, unsigned char value)		{ lua_pushnumber(L, value); }
	inline void Push(lua_State* L, short value)				{ lua_pushnumber(L, value); }
	inline void Push(lua_State* L, unsigned short value)	{ lua_pushnumber(L, value); }
	inline void Push(lua_State* L, int value)				{ lua_pushnumber(L, value); }
	inline void Push(lua_State* L, unsigned int value)		{ lua_pushnumber(L, value); }
	inline void Push(lua_State* L, long value)				{ lua_pushnumber(L, value); }
	inline void Push(lua_State* L, unsigned long value)		{ lua_pushnumber(L, value); }
	inline void Push(lua_State* L, double value)			{ lua_pushnumber(L, (lua_Number)value); }
	inline void Push(lua_State* L, float value)				{ lua_pushnumber(L, (lua_Number)value); }
	inline void Push(lua_State* L, const char* value)		{ lua_pushstring(L, value); }
	inline void Push(lua_State* L, std::string value)		{ lua_pushstring(L, value.c_str()); }
	inline void Push(lua_State* L, const char* value, int len)		{ lua_pushlstring(L, value, len); }
	inline void Push(lua_State* L)				{ }
	inline void Push(lua_State* L, lua_CFunction value)		{ lua_pushcclosure(L, value, 0); }
	inline void Push(lua_State* L, const void* value)		{ lua_pushlightuserdata(L, (void*)value); }
	inline void Push(lua_State* L, const LuaUserData& value){ *(void **)(lua_newuserdata(L, sizeof(void *))) = (void*)value.m_value; }
	template <typename T, typename... Ts>
	inline void Push(lua_State* L, const T value, const Ts... values)
	{
		Push(L, value);
		Push(L, values...);
	}

	template<typename RT>
	inline RT Read(lua_State* L, int index) const;

	template <size_t,  typename... Ts>
	struct _pop
	{
	public:
		typedef std::tuple<Ts...> type;
		static type apply(lua_State* L)
		{
			auto result = worker<Ts...>(L, 1);

			lua_pop(L, sizeof...(Ts));

			return result;
		}

	private:
		template <typename T>
		static std::tuple<T> worker(lua_State* L, const int index)
		{
			return std::make_tuple(GetInstance().Read<T>(L, index));
		}

		template <typename T1, typename T2, typename... Rest>
		static std::tuple<T1, T2, Rest...> worker(lua_State* L, const int index)
		{
			std::tuple<T1> head = std::make_tuple(GetInstance().Read<T1>(index));
			return std::tuple_cat(head, worker<T2, Rest...>(L, index + 1));
		}
	};

	template <typename T>
	struct _pop<1, T>
	{
		typedef T type;
		static type apply(lua_State* L)
		{
			T result = GetInstance().Read<T>(L, -1);

			lua_pop(L, 1);

			return result;
		}
	};

	

	

	template <typename... T>
	typename _pop<sizeof...(T), T...>::type Pop(lua_State* L)
	{
		return _pop<sizeof...(T), T...>::apply(L);
	}

	/*template <typename Ret, typename ...OtherRet, typename... Args, class = typename std::enable_if<(sizeof...(OtherRet) != 0 && std::is_void<Ret>::value), int>::type>
	typename _pop<1 + sizeof... (OtherRet), Ret, OtherRet...>::type CallGlobal(lua_State* L, const std::string &fun, const Args&... args)
	{
	lua_getfield(L, -1,fun.c_str());

	const int num_args = sizeof...(Args);
	const int num_ret = sizeof...(OtherRet)+1;

	GetInstance().Push(L, args...);

	lua_call(L, num_args, num_ret);

	return GetInstance().Pop<Ret, OtherRet...>(L);
	}

	template <typename Ret, typename ...OtherRet, typename... Args, class = typename std::enable_if<(sizeof...(OtherRet) == 0 && std::is_void<Ret>::value), int>::type>
	void CallGlobal(lua_State* L, const std::string &fun, const Args&... args)
	{
	lua_getfield(L, -1,fun.c_str());

	const int num_args = sizeof...(Args);

	GetInstance().Push(L, args...);

	lua_call(L, num_args, 0);
	}*/

	template <typename Ret, typename... Args, class = typename std::enable_if<(!std::is_void<Ret>::value), int>::type>
	Ret CallGlobal(lua_State* L, const std::string &fun, const Args&... args)
	{
		lua_getfield(L, -1, fun.c_str());

		const int num_args = sizeof...(Args);

		GetInstance().Push(L, args...);

		lua_call(L, num_args, 1);

		return GetInstance().Pop<Ret>(L);
	}

	template <typename Ret, typename... Args, class = typename std::enable_if<(std::is_void<Ret>::value), int>::type>
	void CallGlobal(lua_State* L, const std::string &fun, const Args&... args)
	{
		lua_getfield(L, -1, fun.c_str());

		const int num_args = sizeof...(Args);

		GetInstance().Push(L, args...);

		lua_call(L, num_args, 0);
	}

	template <typename Ret,typename... Args, class = typename std::enable_if<(!std::is_void<Ret>::value), int>::type>
	Ret CallMemberFunction(lua_State* L, const std::string &objName, const std::string &funcName, const Args&... args)
	{
		lua_getfield(L, -1, objName.c_str());
		lua_getfield(L, -1, funcName.c_str());
		
		const int num_args = sizeof...(Args) + 1;
		lua_pushvalue(L, -2);
		GetInstance().Push(L, args...);
		
		lua_call(L, num_args, 1);

		return GetInstance().Pop<Ret>(L);
	}

	template <typename Ret, typename ...OtherRet, typename... Args, class = typename std::enable_if<(std::is_void<Ret>::value), int>::type>
	void CallMemberFunction(lua_State* L, const std::string &objName, const std::string &funcName, const Args&... args)
	{
		lua_getfield(L, -1, objName.c_str());
		lua_getfield(L, -1, funcName.c_str());
		
		const int num_args = sizeof...(Args) + 1;
		lua_pushvalue(L, -2);
		GetInstance().Push(L, args...);
		stackDump(L);
		lua_call(L, num_args, 0);
	}

	void CreateGlobalObject(lua_State* L, const std::string& objName, const std::string& metatableName = "", void* userData = 0)
	{
		if (!metatableName.empty())
		{
			lua_getfield(L, -1, metatableName.c_str());
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
		}
		else
		{
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
		}
		if (userData != 0)
		{
			lua_pushlightuserdata(L, userData);
			lua_setfield(L, -2, "__object");
		}
		
		lua_setfield(L, -1,objName.c_str());
	}

	template <typename Ret>
	Ret GetGlobal(lua_State* L, std::string name)
	{
		lua_getfield(L, -1, name.c_str());
		return GetInstance().Pop<Ret>(L);
	}

	template <typename Ret>
	std::vector<Ret> TableToVector(lua_State* L, const std::string& tableName)
	{
		std::vector<Ret> result;
		lua_getfield(L, -1, tableName.c_str());
		int size = luaL_len(L, -1);
		lua_pushnil(L);

		for (int i = 1; i <= size; ++i)
		{
			lua_next(L, -2);
			result.push_back(GetInstance().Pop<Ret>(L));
		}

		lua_pop(L, -1);

		return result;
	}

	template <typename Ret1, typename Ret2 >
	std::vector<std::pair<Ret1, Ret2>> TableToVector(lua_State* L, const std::string& tableName)
	{
		std::vector<std::pair<Ret1, Ret2>> result;
		lua_getfield(L, -1, tableName.c_str());
		int size = (int)luaL_len(L, -1);
		lua_pushnil(L);
		std::pair<Ret1, Ret2> p;
		for (int i = 1; i <= size; ++i)
		{
			lua_next(L, -2);
			lua_pushnil(L);
			lua_next(L, -2);
			p.first = GetInstance().Pop<Ret1>(L);
			lua_next(L, -2);
			p.second = GetInstance().Pop<Ret2>(L);

			result.push_back(p);
			lua_pop(L, -4);
		}

		lua_pop(L, -1);
		return result;
	}


	template <typename Ret1, typename Ret2 >
	std::vector<std::pair<Ret1, Ret2>> TableToVectorFromTop(lua_State* L)
	{
		std::vector<std::pair<Ret1, Ret2>> result;
		int size = (int)luaL_len(L, -1);
		lua_pushnil(L);
		std::pair<Ret1, Ret2> p;

		for (int i = 1; i <= size; ++i)
		{
			lua_next(L, -2);
			lua_pushnil(L);
			lua_next(L, -2);
			p.first = GetInstance().Pop<Ret1>(L);
			lua_next(L, -2);
			p.second = GetInstance().Pop<Ret2>(L);
			result.push_back(p);
			lua_pop(L, 2);
		}

		lua_pop(L, -1);
		return result;
	}

	static void stackDump(lua_State *L)
	{
		int i;
		int top = lua_gettop(L);
		for (i = 1; i <= top; i++) {  /* repeat for each level */
			int t = lua_type(L, i);
			switch (t) {

			case LUA_TSTRING:  /* strings */
				printf("`%s'", lua_tostring(L, i));
				break;

			case LUA_TBOOLEAN:  /* booleans */
				printf(lua_toboolean(L, i) ? "true" : "false");
				break;

			case LUA_TNUMBER:  /* numbers */
				printf("%g", lua_tonumber(L, i));
				break;

			default:  /* other values */
				printf("%s", lua_typename(L, t));
				break;

			}
			printf("  ");  /* put a separator */
		}
		printf("\n");  /* end the listing */
	}

private:
    LuaCallDispatcher() {};
};

template <>
struct LuaCallDispatcher::_pop<1, void>
{
	typedef void type;
	static void apply(lua_State* L)
	{
		return void();
	}
};

template<>
inline int LuaCallDispatcher::Read(lua_State* L, int index) const
{
    return (int)lua_tointeger(L, index);
}

template<>
inline float LuaCallDispatcher::Read(lua_State* L, int index) const
{
    return (float)lua_tonumber(L, index);
}

template<>
inline double LuaCallDispatcher::Read(lua_State* L, int index) const
{
    return lua_tonumber(L, index);
}

template<>
inline bool LuaCallDispatcher::Read(lua_State* L, int index) const
{
    return lua_toboolean(L, index) != 0;
}

template<>
inline const char* LuaCallDispatcher::Read(lua_State* L, int index) const
{
    return lua_tostring(L, index);
}

template<>
inline std::string LuaCallDispatcher::Read(lua_State* L, int index) const
{
    return lua_tostring(L, index);
}

template<>
inline lua_CFunction LuaCallDispatcher::Read(lua_State* L, int index) const
{
    return lua_tocfunction(L, index);
}

template<>
inline LuaCallDispatcher::LuaUserData LuaCallDispatcher::Read(lua_State* L, int index) const
{
    return lua_touserdata(L, index);
}



#endif