
target_sources(gym PRIVATE base_lab.cpp dm_lab.cpp dm_loader.cpp )


FILE(GLOB HEADERS *h)
install (FILES ${HEADERS} DESTINATION include/gym/custom/dm_lab)
install (FILES dm_env/specs.h dm_env/environment.h DESTINATION include/gym/custom/dm_lab/dm_env)

FILE(GLOB HEADERS lab/public/*)
install (FILES ${HEADERS} DESTINATION include/gym/custom/dm_lab/lab/public)

FILE(GLOB HEADERS lab/org_deepmind_lab/*.so)
install (FILES ${HEADERS} DESTINATION include/gym/custom/dm_lab/lab/org_deepmind_lab)

FILE(GLOB HEADERS lab/org_deepmind_lab/baselab/game_scripts/levels/*.lua)
install (FILES ${HEADERS} DESTINATION include/gym/custom/dm_lab/lab/org_deepmind_lab/levels)