target_sources(gym PRIVATE dict.cpp discrete.cpp multi_discrete.cpp space.cpp util.cpp )

install(FILES box.h dict.h discrete.h multi_discrete.h space.h util.h box.tpp util.tpp DESTINATION include/gym/spaces)