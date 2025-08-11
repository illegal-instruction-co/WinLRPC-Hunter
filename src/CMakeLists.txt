add_subdirectory(core)
add_subdirectory(util)
add_subdirectory(io)

add_executable(${PROJECT_NAME} main.cpp
    $<TARGET_OBJECTS:core-end_point_collector>
    $<TARGET_OBJECTS:core-object_directory>
    $<TARGET_OBJECTS:core-session_enumerator>
    $<TARGET_OBJECTS:core-if_version_predicate>
    $<TARGET_OBJECTS:core-rpc_binding>
    $<TARGET_OBJECTS:core-rpc_prober>

    $<TARGET_OBJECTS:util-strings>

    $<TARGET_OBJECTS:io-cli>
    $<TARGET_OBJECTS:io-table_writer>
    $<TARGET_OBJECTS:io-json_writer>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Rpcrt4.lib
        ntdll.lib
        Wtsapi32.lib
)
