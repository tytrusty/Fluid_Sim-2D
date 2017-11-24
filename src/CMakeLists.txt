SET(pwd ${CMAKE_CURRENT_LIST_DIR})

AUX_SOURCE_DIRECTORY(${pwd} src)
add_executable(fluid ${src})
message(STATUS "fluid added")

target_link_libraries(fluid ${stdgl_libraries})
