add_library(communication STATIC communication.c)
target_include_directories(communication PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(communication PUBLIC squirrel hardware_i2c pico_i2c_slave
                                           pico_stdlib display)
set_target_properties(communication PROPERTIES PUBLIC_HEADER communication.h)
