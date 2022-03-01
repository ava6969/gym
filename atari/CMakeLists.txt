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
file(GLOB ALE_LIB ${CMAKE_CURRENT_LIST_DIR}/../third_party/linux/ale/libale.a)

target_link_libraries(${PROJECT_NAME} PRIVATE ${ALE_LIB})
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../third_party/linux
        ${CMAKE_CURRENT_LIST_DIR}/../third_party/linux/ale)
target_sources(${PROJECT_NAME} PUBLIC atari_env.tpp)