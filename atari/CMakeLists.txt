
find_package(SDL2 CONFIG REQUIRED )
file(GLOB ALE_LIB ${THIRD_PARTY}/ale/lib/libale.a)

target_link_libraries(${PROJECT_NAME} PUBLIC ${ALE_LIB} SDL2::SDL2 SDL2::SDL2main SDL2::SDL2-static )
target_include_directories(${PROJECT_NAME} PUBLIC  ${THIRD_PARTY}/ale/include)
target_sources(${PROJECT_NAME} PUBLIC atari_env.tpp)