cmake_minimum_required(VERSION 3.10)

project(srb2-nozomi C)

add_subdirectory(web)

# "SDL2" - The SDL2 backend, newer but more experimental and incomplete
# "Win32" - The Win32/DD backend, tried and true however its support for many things is not the greatest and it's from 1998
set(BACKEND "SDL2")

# Add generic files
list(APPEND SOURCES
    am_map.c
    command.c
    console.c
    d_clisrv.c
    dehacked.c
    d_items.c
    d_main.c
    d_net.c
    d_netcmd.c
    d_netfil.c
    dstrings.c
    f_finale.c
    f_wipe.c
    g_game.c
    g_input.c
    g_state.c
    hu_stuff.c
    info.c
    i_tcp.c
    m_argv.c
    m_bbox.c
    m_cheat.c
	m_easing.c
    m_fixed.c
    m_menu.c
    m_misc.c
    m_random.c
    m_swap.c
    p_ceilng.c
    p_doors.c
    p_enemy.c
    p_fab.c
    p_floor.c
    p_genlin.c
    p_inter.c
    p_lights.c
    p_map.c
    p_maputl.c
    p_mobj.c
    p_plats.c
    p_pspr.c
    p_saveg.c
    p_setup.c
    p_sight.c
    p_spec.c
    p_switch.c
    p_telept.c
    p_tick.c
    p_user.c
    qmus2mid.c
    r_bsp.c
    r_data.c
    r_draw16.c
    r_draw8.c
    r_draw.c
    r_main.c
    r_plane.c
    r_segs.c
    r_sky.c
    r_splats.c
    r_things.c
    screen.c
    sounds.c
    s_sound.c
    st_lib.c
    st_stuff.c
    tables.c
    v_video.c
    wi_stuff.c
    w_wad.c
    z_zone.c
)

# Extra games
list(APPEND SOURCES
	EXTRAS/tetris.c 
	EXTRAS/srb.c
)

# Add the backend's files
if(BACKEND STREQUAL "Win32")
	list(APPEND SOURCES
		win32/dx_error.c 
		win32/fabdxlib.c
		win32/mid2strm.c	
		win32/win_cd.c
		win32/win_dbg.c  
		win32/win_dll.c 
		win32/win_main.c	
		win32/win_net.c  
		win32/win_snd.c
		win32/win_sys.c
		win32/win_vid.c
	)
endif()

if(BACKEND STREQUAL "SDL2")
	list(APPEND SOURCES
        sdl2/dosstr.c
		sdl2/i_cdmus.c
		sdl2/i_main.c
		sdl2/i_net.c
		sdl2/i_sound.c
		sdl2/i_system.c
		sdl2/i_video.c
	)
endif()

MACRO(EXTRALIB NAME)
	FIND_LIBRARY(${NAME}_LIBRARY NAMES ${NAME})
	IF(${NAME}_LIBRARY)
		MESSAGE(STATUS "Found lib${NAME}: ${${NAME}_LIBRARY}")
		SET(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${${NAME}_LIBRARY})
	ELSE(${NAME}_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find lib${NAME}!")
	ENDIF(${NAME}_LIBRARY)
ENDMACRO(EXTRALIB)

if(${CMAKE_SYSTEM} MATCHES "Linux")
	ADD_DEFINITIONS(-DUNIXCOMMON -DLINUX)
	EXTRALIB(m)
	EXTRALIB(rt)
# Make sure on Windows it doesn't say it "may not be supported"
# I guess also here's a define incase we need it, it can't hurt to do so
elseif(${CMAKE_SYSTEM} MATCHES "Windows")
	ADD_DEFINITIONS(-DCMAKE_WINDOWS)
else()
	MESSAGE(WARNING "No specific settings for you system, it may be not supported!")
endif(${CMAKE_SYSTEM} MATCHES "Linux")

# Apply libs and CFLAGS depending on backend
if(BACKEND STREQUAL "SDL2")
	set(LIBRARIES
		SDL2
		#SDL2main
		${EXTRA_LIBRARIES}
	)
	
	list(APPEND CFLAGS
		-DNOZOMI_SDL
	)
endif()


if(BACKEND STREQUAL "Win32")
	set(LIBRARIES
		fmod
		dinput
		dxguid 
		dsound 
		ddraw 
		gdi32
		winmm 
		wsock32
	)
endif()


# Add the source files and libraries defined earlier

add_executable(${PROJECT_NAME}
	${SOURCES}
)


add_link_options("SHELL: -lidbfs.js")
add_link_options("SHELL: -s EXPORTED_RUNTIME_METHODS=ccall,cwrap,HEAP8")
add_link_options(
  "SHELL: -s EXPORTED_FUNCTIONS=_main,_main_program,_P_AddWadFile"
)
add_link_options("SHELL: -sASYNCIFY")
add_link_options("SHELL: -sUSE_SDL=2")
add_link_options("SHELL: -sUSE_ZLIB=1")
add_link_options("SHELL: -sUSE_SDL_MIXER=2")
add_link_options("SHELL: -s ALLOW_MEMORY_GROWTH=1")
add_link_options("SHELL: -s INITIAL_MEMORY=655360000")
add_link_options("SHELL: -s STACK_SIZE=65536000")
add_link_options("--preload-file=${PROJECT_SOURCE_DIR}/../assets@/")
add_link_options("SHELL: --use-preload-plugins")
add_link_options("SHELL: -s NO_EXIT_RUNTIME")
#add_link_options("SHELL: --shell-file ${CMAKE_SOURCE_DIR}/sdl/emscripten/srb2retro.html")

target_compile_definitions(${PROJECT_NAME} PRIVATE -DHAVE_MIXER -DSOUND=SOUND_MIXER -D__EMSCRIPTEN__)
target_link_options(
  ${PROJECT_NAME}
  PRIVATE
  -sUSE_SDL=2
  -sUSE_SDL_MIXER=2
  -sUSE_ZLIB
  -sUSE_LIBPNG
  -lidbfs.js
  -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,HEAP8
  -sEXPORTED_FUNCTIONS=_main,_main_program,_P_AddWadFile
  -sASYNCIFY
  -sNO_EXIT_RUNTIME 
  -sALLOW_MEMORY_GROWTH=1
  -sINITIAL_MEMORY=983040000
  -sSTACK_SIZE=655360000
  -gsource-map
  -gsource-map-base
  -fexceptions
  -sSTACK_OVERFLOW_CHECK=2
  -sMALLOC=emmalloc-memvalidate
  -fsantize=address
  --preload-file=${PROJECT_SOURCE_DIR}/../assets@/
  --use-preload-plugins 
  --use-preload-cache
  #--shell-file ${PROJECT_SOURCE_DIR}/src/sdl/emscripten/srb2retro.html
)
target_compile_options(${PROJECT_NAME} PRIVATE -sUSE_SDL=2 -sUSE_SDL_MIXER=2
                                        -sUSE_ZLIB -sUSE_LIBPNG)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "lsdl2srb2" SUFFIX ".js")

target_link_libraries(${PROJECT_NAME} PRIVATE 
	${LIBRARIES}
)

list(APPEND CFLAGS
	-DNEWBUILD
    -fPIE
    -Wno-int-conversion
    #-fpermissive
    -g
    #-m486
    -O0
    -ffast-math
    -fomit-frame-pointer
)

target_compile_options(${PROJECT_NAME} PRIVATE
	${CFLAGS}
)