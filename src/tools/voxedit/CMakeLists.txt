include_directories(modules)
add_subdirectory(modules)

project(voxedit)
set(SRCS
	VoxEdit.h VoxEdit.cpp
)

set(LUA_SRCS
	scripts/thicken.lua
)

set(FILES
	voxedit/chr_knight.qb
	voxedit/voxedit-keybindings.cfg
	voxedit/scripts/test-import-plane.cfg
	voxedit/scripts/test-layers.cfg
	voxedit/scripts/test-animate.cfg
)

engine_add_executable(TARGET ${PROJECT_NAME} SRCS ${SRCS} FILES ${FILES} LUA_SRCS ${LUA_SRCS} WINDOWED)
engine_target_link_libraries(TARGET ${PROJECT_NAME} DEPENDENCIES voxedit-ui voxedit-util)
if (HOT_RELOAD_ANIM)
	add_dependencies(${PROJECT_NAME} anim)
endif()
