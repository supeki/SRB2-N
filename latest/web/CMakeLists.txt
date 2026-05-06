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
  --shell-file=${PROJECT_SOURCE_DIR}/web/shell.html
)
target_compile_options(${PROJECT_NAME} PRIVATE -sUSE_SDL=2 -sUSE_SDL_MIXER=2
                                        -sUSE_ZLIB -sUSE_LIBPNG)