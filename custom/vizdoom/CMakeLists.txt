target_sources(gym PUBLIC viz_doom.cpp)

target_link_libraries(gym PRIVATE ${THIRD_PARTY}/viz_doom/lib/libvizdoom.so)

install (FILES viz_doom.h DESTINATION include/gym/custom/vizdoom)

FILE(GLOB HEADERS include/*.h)
install (FILES ${HEADERS} DESTINATION include/gym/custom/vizdoom/include)