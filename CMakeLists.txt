
cmake_minimum_required (VERSION 3.0)

# project info
######################################################

set(PROJECT_NAME polyjam)
set(TARGET_NAME "${PROJECT_NAME}")
project( ${PROJECT_NAME})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


file(GLOB_RECURSE TARGET_SRC src/*.cpp)
file(GLOB_RECURSE TARGET_HEADERS src/*.h src/*.hpp)

include_directories(src/)

#compiler
######################################################

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_definitions(-Wall)
  add_definitions(-std=c++14)
  add_definitions(-o2)
endif()


# libs for Windows from space_ships/libs
######################################################

if(WIN32)
  set(WIN_LIB_DIR "${CMAKE_SOURCE_DIR}/libs")

  set( CMAKE_PREFIX_PATH "${WIN_LIB_DIR}/glew")
  set( GLM_ROOT_DIR "${WIN_LIB_DIR}/glm")
  set( ENV{SDL2DIR} "${WIN_LIB_DIR}/SDL2")
  set( BULLET_ROOT  "${WIN_LIB_DIR}/bullet")
  set( SOIL_ROOT_DIR "${WIN_LIB_DIR}/soil")

  # for static SDL2 linking
  set( LIBRARIES "version.lib;imm32.lib;winmm.lib;")
endif()



# Find libs
######################################################

# Find GLM
find_package(GLM REQUIRED)
if( GLM_FOUND )
  include_directories( ${GLM_INCLUDE_DIRS} )
  set( LIBRARIES ${LIBRARIES} ${GLM_LIBRARIES} )
endif( GLM_FOUND )

# Find GLEW
set(GLEW_USE_STATIC_BUILDS ON)
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories( ${GLEW_INCLUDE_DIRS} )
    set( LIBRARIES ${LIBRARIES} ${GLEW_LIBRARIES} )
endif(GLEW_FOUND)

# Find OpenGL
find_package(OpenGL 3.3 REQUIRED)
if(OPENGL_FOUND)
  set(LIBRARIES ${LIBRARIES} ${OPENGL_LIBRARIES})
  include_directories( ${OPENGL_INCLUDE_DIR})
endif()

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories( ${SDL2_INCLUDE_DIR})
set(LIBRARIES ${LIBRARIES} ${SDL2_LIBRARY})

# Find SDL_image
find_package(SDL_image REQUIRED)
include_directories( ${SDL_IMAGE_INCLUDE_DIRS})
set(LIBRARIES ${LIBRARIES} ${SDL_IMAGE_LIBRARIES})

# Find SDL_mixer
find_package(SDL_mixer REQUIRED)
include_directories( ${SDL_MIXER_INCLUDE_DIR})
set(LIBRARIES ${LIBRARIES} ${SDL_MIXER_LIBRARY})


# Find bullet
find_package(Bullet REQUIRED)
if( BULLET_FOUND )
  set( LIBRARIES ${LIBRARIES} ${BULLET_LIBRARIES} )
  include_directories( ${BULLET_INCLUDE_DIRS} )
endif( BULLET_FOUND )

# Find SOIL
find_package(SOIL REQUIRED)
if(SOIL_FOUND)
  set( LIBRARIES ${LIBRARIES} ${SOIL_LIBRARIES})
  include_directories( ${SOIL_INCLUDE_DIR})
endif()

#exe
######################################################

add_executable(${TARGET_NAME} ${TARGET_SRC} ${TARGET_HEADERS})
target_link_libraries(${TARGET_NAME} ${LIBRARIES})
