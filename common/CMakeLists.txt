target_sources(gym PRIVATE rendering.cpp tensor_adapter.cpp)

file(GLOB HEADERS rendering.h tensor_adapter.h utils.h wrapper.h)
install (FILES ${HEADERS} DESTINATION include/gym/common)