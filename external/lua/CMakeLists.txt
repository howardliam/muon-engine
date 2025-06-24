cmake_minimum_required(VERSION 3.16)
project(lua LANGUAGES C)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LUA_HEADERS
    ${SRC_DIR}/lapi.h
    ${SRC_DIR}/lauxlib.h
    ${SRC_DIR}/lcode.h
    ${SRC_DIR}/lctype.h
    ${SRC_DIR}/ldebug.h
    ${SRC_DIR}/ldo.h
    ${SRC_DIR}/lfunc.h
    ${SRC_DIR}/lgc.h
    ${SRC_DIR}/ljumptab.h
    ${SRC_DIR}/llex.h
    ${SRC_DIR}/llimits.h
    ${SRC_DIR}/lmem.h
    ${SRC_DIR}/lobject.h
    ${SRC_DIR}/lopcodes.h
    ${SRC_DIR}/lopnames.h
    ${SRC_DIR}/lparser.h
    ${SRC_DIR}/lprefix.h
    ${SRC_DIR}/lstate.h
    ${SRC_DIR}/lstring.h
    ${SRC_DIR}/ltable.h
    ${SRC_DIR}/ltests.h
    ${SRC_DIR}/ltm.h
    ${SRC_DIR}/lua.h
    ${SRC_DIR}/luaconf.h
    ${SRC_DIR}/lualib.h
    ${SRC_DIR}/lundump.h
    ${SRC_DIR}/lvm.h
    ${SRC_DIR}/lzio.h
)

set(LUA_SOURCES
    ${SRC_DIR}/lapi.c
    ${SRC_DIR}/lauxlib.c
    ${SRC_DIR}/lbaselib.c
    ${SRC_DIR}/lcode.c
    ${SRC_DIR}/lcorolib.c
    ${SRC_DIR}/lctype.c
    ${SRC_DIR}/ldblib.c
    ${SRC_DIR}/ldebug.c
    ${SRC_DIR}/ldo.c
    ${SRC_DIR}/ldump.c
    ${SRC_DIR}/lfunc.c
    ${SRC_DIR}/lgc.c
    ${SRC_DIR}/linit.c
    ${SRC_DIR}/liolib.c
    ${SRC_DIR}/llex.c
    ${SRC_DIR}/lmathlib.c
    ${SRC_DIR}/lmem.c
    ${SRC_DIR}/loadlib.c
    ${SRC_DIR}/lobject.c
    ${SRC_DIR}/lopcodes.c
    ${SRC_DIR}/loslib.c
    ${SRC_DIR}/lparser.c
    ${SRC_DIR}/lstate.c
    ${SRC_DIR}/lstring.c
    ${SRC_DIR}/lstrlib.c
    ${SRC_DIR}/ltable.c
    ${SRC_DIR}/ltablib.c
    ${SRC_DIR}/ltests.c
    ${SRC_DIR}/ltm.c
    ${SRC_DIR}/lundump.c
    ${SRC_DIR}/lutf8lib.c
    ${SRC_DIR}/lvm.c
    ${SRC_DIR}/lzio.c
)

add_library(lua_static STATIC ${LUA_SOURCES} ${LUA_HEADERS})
add_library(lua_shared SHARED ${LUA_SOURCES} ${LUA_HEADERS})
target_compile_features(lua_static PRIVATE c_std_99)
target_compile_features(lua_shared PRIVATE c_std_99)
