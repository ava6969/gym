
add_library(glfont STATIC src/FontAtlas.cpp src/FTLabel.cpp src/GLFont.cpp GLUtils/GLUtils.cpp)
target_include_directories(glfont PRIVATE src GLUtils ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} )
target_link_libraries(glfont PRIVATE OpenGL::GL OpenGL::GLU glfw freetype ${GLEW_LIBRARIES})

add_executable(test_glfont test/src/GLWindow.cpp test/src/TestWindow.cpp test/src/main.cpp)
target_include_directories(test_glfont PRIVATE  ${OPENGL_INCLUDE_DIR}  src GLUtils test test/include ${GLEW_INCLUDE_DIRS})
target_link_libraries(test_glfont PRIVATE glfont  OpenGL::GL OpenGL::GLU glfw freetype ${GLEW_LIBRARIES})