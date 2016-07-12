#include "LuaFunction.h"

template<>
lua_Number detail::GetAndCheckType(lua_State* L, int i)
{
	int isNum = 0;
	auto result = lua_tonumberx(L, i, &isNum);
	if (!isNum)
	{
		std::cout << "NOT A NUMBER /n" << std::endl;
	}

	return result;
};

template<>
bool detail::GetAndCheckType(lua_State* L, int i)
{
	return lua_toboolean(L, i) != 0;
};

template<>
std::string detail::GetAndCheckType(lua_State* L, int i)
{
	size_t length = 0;
	const char* buff = lua_tolstring(L, i, &length);

	if (buff == nullptr)
		std::cout << "NOT A String/n" << std::endl;

	return buff;
};

template<>
int detail::GetAndCheckType(lua_State* L, int i)
{
	int isInt = 0;
	int result = (int)lua_tointegerx(L, i, &isInt);

	if (!isInt)
	{
		std::cout << "NOT AN INT/n" << std::endl;
	}

	return result;
};
