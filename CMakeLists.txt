cmake_minimum_required(VERSION 3.10)
set(prj note_taker)
project(${prj})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
find_library(PANEL_LIBRARY panel)
find_library(MENU_LIBRARY menu)

add_executable(${prj} src/main.cpp src/comps/app.cpp src/comps/menu_loop.cpp src/comps/window_loop.cpp)
target_link_libraries(${prj} ${CURSES_LIBRARIES} ${PANEL_LIBRARY} ${MENU_LIBRARY})

install(TARGETS ${prj} RUNTIME DESTINATION bin)
