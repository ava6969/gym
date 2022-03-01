#find_package(ale REQUIRED)
#
#if(${ale_FOUND})
#    target_link_libraries(${PROJECT_NAME} PRIVATE ale::ale-lib)
##    target_sources(${PROJECT_NAME} PUBLIC atari_env.tpp)
#    install (FILES atari_env.h atari_env.tpp ale_c_wrapper.h atari_ntsc_rgb_palette.h DESTINATION include/gym/atari)
#else()
#    message("ALE not INSTALLED - Cannot use ATARI ENV")
#endif()
#
find_package(SDL2 CONFIG REQUIRED )
file(GLOB ALE_LIB ${CMAKE_CURRENT_LIST_DIR}/../third_party/linux/ale/libale.a)

target_link_libraries(${PROJECT_NAME} PUBLIC ${ALE_LIB} SDL2::SDL2 )
target_include_directories(${PROJECT_NAME} PUBLIC  ${THIRD_PARTY}/linux ${THIRD_PARTY}/linux/ale)
target_sources(${PROJECT_NAME} PUBLIC atari_env.tpp)