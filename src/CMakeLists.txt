cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -fPIC -std=gnu99")

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

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
aux_source_directory(. GRUDB_SRC_LIST)
add_executable(genrudb ${GRUDB_SRC_LIST})
target_link_libraries(genrudb RUCE)
target_link_libraries(genrudb CVESVP)
target_link_libraries(genrudb CVEDSP2)
target_link_libraries(genrudb RFNL)
target_link_libraries(genrudb m)

aux_source_directory(./WavNorm WNORM_SRC_LIST)
aux_source_directory(. WNORM_SRC_LIST)
add_executable(wavnorm ${WNORM_SRC_LIST})
target_link_libraries(wavnorm RUCE)
target_link_libraries(wavnorm CVESVP)
target_link_libraries(wavnorm CVEDSP2)
target_link_libraries(wavnorm RFNL)
target_link_libraries(wavnorm m)

aux_source_directory(./Roto2Oto R2OTO_SRC_LIST)
aux_source_directory(. R2OTO_SRC_LIST)
add_executable(roto2oto ${R2OTO_SRC_LIST})
target_link_libraries(roto2oto RUCE)
target_link_libraries(roto2oto CVEDSP2)
target_link_libraries(roto2oto RFNL)
target_link_libraries(roto2oto m)

aux_source_directory(./Utau2Rec U2REC_SRC_LIST)
aux_source_directory(. U2REC_SRC_LIST)
add_executable(utau2rec ${U2REC_SRC_LIST})
target_link_libraries(utau2rec RFNL)

aux_source_directory(./PrecompDiff PCDIFF_SRC_LIST)
aux_source_directory(. PCDIFF_SRC_LIST)
add_executable(pcompdiff ${PCDIFF_SRC_LIST})
target_link_libraries(pcompdiff CVEDSP2)
target_link_libraries(pcompdiff RFNL)
target_link_libraries(pcompdiff m)

aux_source_directory(./RUDB2Oct R2OCT_SRC_LIST)
aux_source_directory(. R2OCT_SRC_LIST)
add_executable(rudb2oct ${R2OCT_SRC_LIST})
target_link_libraries(rudb2oct RUCE)
target_link_libraries(rudb2oct CVEDSP2)
target_link_libraries(rudb2oct RFNL)
target_link_libraries(rudb2oct m)

aux_source_directory(./Context2Oct C2OCT_SRC_LIST)
aux_source_directory(. C2OCT_SRC_LIST)
add_executable(con2oct ${C2OCT_SRC_LIST})
target_link_libraries(con2oct RUCE)
target_link_libraries(con2oct CVEDSP2)
target_link_libraries(con2oct RFNL)
target_link_libraries(con2oct m)

ADD_CUSTOM_TARGET(ManPages ALL)

ADD_CUSTOM_COMMAND(
    TARGET ManPages
    COMMAND pod2man ARGS -n GenRUDB --center='Rocaloid Toolchain' -s 1
        -r GenRUDB ${CMAKE_CURRENT_SOURCE_DIR}/GenRUDB/man.pod
        ${CMAKE_CURRENT_BINARY_DIR}/genrudb.1
    COMMAND pod2man ARGS -n WavNorm --center='Rocaloid Toolchain' -s 1
        -r WavNorm ${CMAKE_CURRENT_SOURCE_DIR}/WavNorm/man.pod
        ${CMAKE_CURRENT_BINARY_DIR}/wavnorm.1
    COMMAND pod2man ARGS -n WavNorm --center='Rocaloid Toolchain' -s 1
        -r Roto2Oto ${CMAKE_CURRENT_SOURCE_DIR}/Roto2Oto/man.pod
        ${CMAKE_CURRENT_BINARY_DIR}/roto2oto.1
    OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/genrudb.1
            ${CMAKE_CURRENT_BINARY_DIR}/wavnorm.1
            ${CMAKE_CURRENT_BINARY_DIR}/roto2oto.1
)

ADD_CUSTOM_COMMAND(
    TARGET ManPages
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/genrudb.1
            ${CMAKE_CURRENT_BINARY_DIR}/wavnorm.1
            ${CMAKE_CURRENT_BINARY_DIR}/roto2oto.1
)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/genrudb.1
          ${CMAKE_CURRENT_BINARY_DIR}/wavnorm.1
          ${CMAKE_CURRENT_BINARY_DIR}/roto2oto.1
    DESTINATION share/man/man1
)

INSTALL(
    TARGETS genrudb rsegment genroto wavnorm roto2oto utau2rec pcompdiff
            rudb2oct con2oct
    DESTINATION bin/
)

