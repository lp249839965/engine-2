/**
 * @file
 */

#pragma once

#include "BoneId.h"
#include "commonlua/LUAFunctions.h"
#include "AnimationCache.h"

template<> struct clua_meta<animation::BoneIds> { static char const *name() {return "__meta_boneids";} };

namespace animation {

static int luaanim_boneidstostring(lua_State* s) {
	BoneIds** a = clua_get<BoneIds*>(s, 1);
	BoneIds& boneIds = **a;
	if (boneIds.num == 0) {
		lua_pushstring(s, "empty");
		return 1;
	}
	if (boneIds.num == 1) {
		lua_pushfstring(s, "num bones: 1, bone[0]: %i", (int)boneIds.bones[0]);
		return 1;
	}
	if (boneIds.num == 2) {
		lua_pushfstring(s, "num bones: 2, bone[0]: %i, bone[1]: %i", (int)boneIds.bones[0], (int)boneIds.bones[1]);
		return 1;
	}
	lua_pushfstring(s, "error: num bones: %i", (*a)->num);
	return 1;
}

static int luaanim_boneidsadd(lua_State* s) {
	BoneIds** boneIdsPtrPtr = clua_get<BoneIds*>(s, 1);
	BoneIds* boneIds = *boneIdsPtrPtr;
	const char* boneName = luaL_checkstring(s, 2);
	BoneId id = toBoneId(boneName);
	if (id == BoneId::Max) {
		return luaL_error(s, "Failed to resolve bone: '%s'", boneName);
	}
	if (boneIds->num > lengthof(boneIds->bones)) {
		lua_pushboolean(s, 0);
		return 1;
	}
	boneIds->bones[boneIds->num] = (uint8_t)id;
	boneIds->mirrored[boneIds->num] = clua_optboolean(s, 3, false);
	boneIds->num++;
	lua_pushboolean(s, 1);
	return 1;
}

void luaanim_boneidsregister(lua_State* s) {
	std::vector<luaL_Reg> funcs = {
		{"__tostring", luaanim_boneidstostring},
		{"add", luaanim_boneidsadd},
		{nullptr, nullptr}
	};
	clua_registerfuncs(s, &funcs.front(), clua_meta<BoneIds>::name());
}

int luaanim_pushboneids(lua_State* s, BoneIds* b) {
	return clua_push(s, b);
}

template<class T>
int luaanim_bonesetup(lua_State* l) {
	T* settings = lua::LUA::globalData<T>(l, "Settings");
	const char* meshType = luaL_checkstring(l, 1);
	BoneIds* b = settings->boneIds(meshType);
	if (b == nullptr) {
		return luaL_error(l, "Failed to resolve mesh type: '%s'", meshType);
	}
	*b = BoneIds {};
	if (luaanim_pushboneids(l, b) != 1) {
		return luaL_error(l, "Failed to push the bonesids");
	}
	return 1;
}

}