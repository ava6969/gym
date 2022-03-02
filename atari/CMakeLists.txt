
file(GLOB ALE_LIB ${THIRD_PARTY}/ale/lib/libale.a)
target_link_libraries(${PROJECT_NAME} PUBLIC ${ALE_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC  ${THIRD_PARTY}/ale/include ${THIRD_PARTY}/ale/include/ale)
target_sources(${PROJECT_NAME} PUBLIC atari_env.tpp)