#include "LuaCallDispatcher.h"
#include "LUAObject.h"

//
//template<>
//int LuaCallDispatcher::Read(lua_State* L, int index) const
//{
//    return (int)lua_tointeger(L, index);
//}
//
//template<>
//float LuaCallDispatcher::Read(lua_State* L, int index) const
//{
//    return (float)lua_tonumber(L, index);
//}
//
//template<>
//double LuaCallDispatcher::Read(lua_State* L, int index) const
//{
//    return lua_tonumber(L, index);
//}
//
//template<>
//bool LuaCallDispatcher::Read(lua_State* L, int index) const
//{
//    return lua_toboolean(L, index) != 0;
//}
//
//template<>
//const char* LuaCallDispatcher::Read(lua_State* L, int index) const
//{
//    return lua_tostring(L, index);
//}
//
//template<>
//std::string LuaCallDispatcher::Read(lua_State* L, int index) const
//{
//    return lua_tostring(L, index);
//}
//
//template<>
//lua_CFunction LuaCallDispatcher::Read(lua_State* L, int index) const
//{
//    return lua_tocfunction(L, index);
//}
//
//template<>
//LuaCallDispatcher::LuaUserData LuaCallDispatcher::Read(lua_State* L, int index) const
//{
//    return lua_touserdata(L, index);
//}
//
template <>
struct LuaCallDispatcher::_pop<0, void>
{
    typedef void type;
    static void apply(lua_State* L)
    {
        
        return ;
    }
};

