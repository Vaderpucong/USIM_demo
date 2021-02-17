project(USIM_demo C)

cmake_minimum_required(VERSION 3.11)

set(interface_files "")

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
    include_directories(/usr/include/PCSC)
    set(interface_files ${interface_files} usim_if_linux.c usim_if_linux.h)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -g)
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    set(interface_files ${interface_files} usim_if_windows.c usim_if_windows.h)
else()
    set(interface_files ${interface_files} usim_if_noPCSC.c usim_if_noPCSC.h)
endif()

file(GLOB decode_file decode_*_tlv.*)

add_executable(USIM_demo
        main.c
        usim.c
        usim.h
        history.c
        history.h
        ${interface_files}
        ${decode_file}
        )

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
    target_link_libraries(USIM_demo
            -lpcsclite)
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries(USIM_demo
            -lwinscard)
endif()

