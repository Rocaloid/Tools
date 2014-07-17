cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -fPIC")

aux_source_directory(./RSegment RSEG_SRC_LIST)
add_executable(rsegment ${RSEG_SRC_LIST})
target_link_libraries(rsegment RUCE)
target_link_libraries(rsegment CVESVP)
target_link_libraries(rsegment CVEDSP2)
target_link_libraries(rsegment RFNL)
target_link_libraries(rsegment m)

