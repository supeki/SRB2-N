cmake_minimum_required(VERSION 3.10)

project(srb2-nozomi C)

set(BACKEND "Win32")
 
add_executable(${PROJECT_NAME}
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

target_link_libraries(${PROJECT_NAME} PRIVATE 
	fmod
	dinput
	dxguid 
	dsound 
	ddraw 
	gdi32
	winmm 
	wsock32
)

target_compile_options(${PROJECT_NAME} PRIVATE
    #-DLINUX
    #-DHAVE_SDL
    #-DHAVE_SDL_MIXER
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