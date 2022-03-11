## #################################################################
## Root CMakeLists.txt
## #################################################################

cmake_minimum_required(VERSION 3.0)


project(CrosswordDesigner)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

file( GLOB CROSSWORD_SOURCES CrosswordDesigner/*.c )
file( GLOB CROSSWORD_HEADERS CrosswordDesigner/*.h )

add_executable( "CrosswordDesigner" ${CROSSWORD_SOURCES} ${CROSSWORD_HEADERS} )
target_link_libraries("CrosswordDesigner" Comctl32.lib)
set_target_properties("CrosswordDesigner" PROPERTIES WIN32_EXECUTABLE TRUE)



