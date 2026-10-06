add_library(sound MODULE
            sound.cpp
            SoundManager.cpp
            SoundPlayer.cpp
            SoundShader.cpp)
target_link_libraries(sound PUBLIC wxutil ${AL_LINK_LIBRARIES} ${VORBIS_LINK_LIBRARIES})
set_target_properties(
    sound PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${DR_STAGING_MODULESDIR}"
)
