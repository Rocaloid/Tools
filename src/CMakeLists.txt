cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -fPIC")

aux_source_directory(./RSegment RSEG_SRC_LIST)
add_executable(rsegment ${RSEG_SRC_LIST})
target_link_libraries(rsegment RUCE)
target_link_libraries(rsegment CVESVP)
target_link_libraries(rsegment CVEDSP2)
target_link_libraries(rsegment RFNL)
target_link_libraries(rsegment m)

aux_source_directory(./GenRoto GROTO_SRC_LIST)
add_executable(genroto ${GROTO_SRC_LIST})
target_link_libraries(genroto RUCE)
target_link_libraries(genroto RFNL)
target_link_libraries(genroto m)

aux_source_directory(./GenRUDB GRUDB_SRC_LIST)
add_executable(genrudb ${GRUDB_SRC_LIST})
target_link_libraries(genrudb RUCE)
target_link_libraries(genrudb CVESVP)
target_link_libraries(genrudb CVEDSP2)
target_link_libraries(genrudb RFNL)
target_link_libraries(genrudb m)

ADD_CUSTOM_TARGET(ManPages ALL)

ADD_CUSTOM_COMMAND(
    TARGET ManPages
    SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/GenRUDB/man.pod 
    COMMAND pod2man ARGS -s 1  -n GenRUDB --center='Rocaloid Toolchain' -s 1
        -r GenRUDB-1.0.0.1 ${CMAKE_CURRENT_SOURCE_DIR}/GenRUDB/man.pod
        ${CMAKE_CURRENT_BINARY_DIR}/genrudb.1 
    OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/genrudb.1
)

ADD_CUSTOM_COMMAND(
    TARGET ManPages
    SOURCE ManPages
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/genrudb.1
)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/genrudb.1
    DESTINATION share/man/man1
)

INSTALL(
    TARGETS genrudb
    DESTINATION bin/
)

