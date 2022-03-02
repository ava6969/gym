
if(${ADD_ATARI})
    target_sources(gym PRIVATE atari_wrappers.cpp)
endif()

file(GLOB HEADERS *.h)
install (FILES ${HEADERS} DESTINATION include/gym/wrappers)


