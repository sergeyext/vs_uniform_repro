cmake_minimum_required(VERSION 3.8)
project(render)

find_library(LIBGLFW NAMES libglfw libglfw3 glfw glfw3)
find_library(LIBOPENGL NAMES GL opengl32)
find_library(LIBGLEW NAMES GLEW glew32 glew32s glew32sd)
set(LINK_LIBS ${LIBGLFW} ${LIBOPENGL} ${LIBGLEW})
set(CMAKE_CXX_STANDARD 17)

add_executable(uniform_alloc Loader.cpp)
target_link_libraries(uniform_alloc ${LINK_LIBS})

add_custom_target(shaders
	ALL
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Shader.vert ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Shader.frag ${CMAKE_CURRENT_BINARY_DIR}
)
add_dependencies(uniform_alloc shaders)
