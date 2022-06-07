add_subdirectory(vec_env)
target_sources(gym PRIVATE atari_wrappers.cpp)


file(GLOB HEADERS *.h)
install (FILES ${HEADERS} DESTINATION include/gym/wrappers)


