
file(GLOB ALE_LIB ../third_party/lib/libale.a)
target_link_libraries(${PROJECT_NAME} PUBLIC ${ALE_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC ../third_party/include ../third_party/include/ale)
target_sources(${PROJECT_NAME} PUBLIC atari_env.tpp)