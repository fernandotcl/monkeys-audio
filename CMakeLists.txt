cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Werror -pedantic -Wno-long-long")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE} -O0")
endif(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(TestBigEndian)

check_function_exists(memcmp HAVE_MEMCMP)
check_function_exists(stat HAVE_STAT)
check_function_exists(wcscasecmp HAVE_WCSCASECMP)

check_include_files(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(float.h HAVE_FLOAT_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(stdarg.h HAVE_STDARG_H)
check_include_files(unistd.h HAVE_UNISTD_H)

if(HAVE_STDLIB_H AND HAVE_STDARG_H AND HAVE_STRING_H AND HAVE_FLOAT_H)
    set(STDC_HEADERS TRUE)
endif(HAVE_STDLIB_H AND HAVE_STDARG_H AND HAVE_STRING_H AND HAVE_FLOAT_H)

check_library_exists(rt nanosleep "" NANOSLEEP_IN_RT)
if(NANOSLEEP_IN_RT)
    set(NANOSLEEP_LIBRARY rt)
else(NANOSLEEP_IN_RT)
    check_library_exists(posix4 nanosleep "" NANOSLEEP_IN_POSIX4)
    if(NANOSLEEP_IN_POSIX4)
        set(NANOSLEEP_LIBRARY posix4)
    endif(NANOSLEEP_IN_POSIX4)
endif(NANOSLEEP_IN_RT)

if(NOT WIN32)
    set(BUILD_CROSS_PLATFORM TRUE)
endif(NOT WIN32)

option(ENABLE_ASSEMBLY "Whether or not to use assembly if possible" ON)
if(ENABLE_ASSEMBLY)
    find_program(YASM yasm)
    if(YASM)
        if(WIN32)
            set(CMAKE_ASM_NASM_OBJECT_FORMAT "win32")
        else(WIN32)
            if(APPLE)
                set(CMAKE_ASM_NASM_OBJECT_FORMAT "macho")
            else(APPLE)
                set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf")
            endif(APPLE)
        endif(WIN32)
        message(STATUS "Using Yasm output format: " ${CMAKE_ASM_NASM_OBJECT_FORMAT})

        if(WIN32)
            if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
                set(YASM_ARCH "x86")
                set(ARCH_X86 TRUE)
            else(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
                if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
                    set(YASM_ARCH "amd64")
                    set(ARCH_X86_64 TRUE)
                endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
            endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
        else(WIN32)
            execute_process(COMMAND uname -m
                OUTPUT_VARIABLE HARDWARE_NAME
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(HARDWARE_NAME MATCHES "i.*86")
                set(YASM_ARCH "x86")
                set(ARCH_X86 TRUE)
            else(HARDWARE_NAME MATCHES "i.*86")
                if(HARDWARE_NAME STREQUAL "x86_64")
                    set(YASM_ARCH "amd64")
                    set(ARCH_X86_64 TRUE)
                endif(HARDWARE_NAME STREQUAL "x86_64")
            endif(HARDWARE_NAME MATCHES "i.*86")
        endif(WIN32)

        if(YASM_ARCH)
            message(STATUS "Using Yasm architecture: " ${YASM_ARCH})
            set(CMAKE_ASM_NASM_COMPILER ${YASM})
            set(CMAKE_ASM_NASM_COMPILER_ARG1 "-m ${YASM_ARCH}")
            enable_language(ASM_NASM)
        else(YASM_ARCH)
            message(STATUS "Unsupported architecture, disabling assembly")
            set(ENABLE_ASSEMBLY OFF)
        endif(YASM_ARCH)

    else(YASM)
        message(STATUS "Unable to locate Yasm, assembly disabled")
    endif(YASM)
endif(ENABLE_ASSEMBLY)

test_big_endian(WORDS_BIGENDIAN)

configure_file(
    ${PROJECT_SOURCE_DIR}/src/Shared/config.h.in
    ${PROJECT_BINARY_DIR}/config.h)

set(SHARED_SOURCES
    src/Shared/CharacterHelper.cpp
    src/Shared/CircleBuffer.cpp
    src/Shared/GlobalFunctions.cpp
    src/Shared/MACUtils.cpp
    src/Shared/StdLibFileIO.cpp
    src/Shared/WinFileIO.cpp)

set(SHARED_HEADERS
    src/MACLib/Assembly/Assembly.h
    src/Shared/All.h
    src/Shared/CharacterHelper.h
    src/Shared/CircleBuffer.h
    src/Shared/GlobalFunctions.h
    src/Shared/ID3Genres.h
    src/Shared/IO.h
    src/Shared/MACUtils.h
    src/Shared/NoWindows.h
    src/Shared/RollBuffer.h
    src/Shared/SmartPtr.h
    src/Shared/StdLibFileIO.h
    src/Shared/WinFileIO.h)

set(LIBMAC_SOURCES
    src/MACLib/Assembly/common.cpp
    src/MACLib/APECompress.cpp
    src/MACLib/APECompressCore.cpp
    src/MACLib/APECompressCreate.cpp
    src/MACLib/APEDecompress.cpp
    src/MACLib/APEHeader.cpp
    src/MACLib/APEInfo.cpp
    src/MACLib/APELink.cpp
    src/MACLib/APESimple.cpp
    src/MACLib/APETag.cpp
    src/MACLib/BitArray.cpp
    src/MACLib/MACLib.cpp
    src/MACLib/MACProgressHelper.cpp
    src/MACLib/MD5.cpp
    src/MACLib/NNFilter.cpp
    src/MACLib/NewPredictor.cpp
    src/MACLib/Prepare.cpp
    src/MACLib/UnBitArray.cpp
    src/MACLib/UnBitArrayBase.cpp
    src/MACLib/WAVInputSource.cpp)

if(ENABLE_ASSEMBLY)
    if(ARCH_X86)
        set (LIBMAC_SOURCES ${LIBMAC_SOURCES} src/MACLib/Assembly/x86_mmx.asm)
    else(ARCH_X86)
        set (LIBMAC_SOURCES ${LIBMAC_SOURCES} src/MACLib/Assembly/x86_64_mmx.asm)
    endif(ARCH_X86)
endif(ENABLE_ASSEMBLY)

set(LIBMAC_HEADERS
    ${PROJECT_BINARY_DIR}/config.h
    src/MACLib/APECompress.h
    src/MACLib/APECompressCore.h
    src/MACLib/APECompressCreate.h
    src/MACLib/APEDecompress.h
    src/MACLib/APEHeader.h
    src/MACLib/APEInfo.h
    src/MACLib/APELink.h
    src/MACLib/APETag.h
    src/MACLib/BitArray.h
    src/MACLib/MACLib.h
    src/MACLib/MACProgressHelper.h
    src/MACLib/MD5.h
    src/MACLib/MultichannelNNFilter.h
    src/MACLib/NNFilter.h
    src/MACLib/NewPredictor.h
    src/MACLib/Predictor.h
    src/MACLib/Prepare.h
    src/MACLib/ScaledFirstOrderFilter.h
    src/MACLib/StartFilter.h
    src/MACLib/UnBitArray.h
    src/MACLib/UnBitArrayBase.h
    src/MACLib/WAVInputSource.h)

set(MAC_SOURCES
    src/Console/Console.cpp)

set(SAMPLE1_SOURCES
    src/Examples/Analyze/Sample1/Sample1.cpp)

include_directories(
    ${PROJECT_BINARY_DIR}
    src/Shared
    src/MACLib)

add_library(libmac SHARED
    ${SHARED_SOURCES}
    ${SHARED_HEADERS}
    ${LIBMAC_SOURCES}
    ${LIBMAC_HEADERS})

target_link_libraries(libmac ${NANOSLEEP_LIBRARY})

set_target_properties(libmac PROPERTIES
    OUTPUT_NAME mac
    VERSION 3.99.5
    SOVERSION 3
    LINK_FLAGS -export-dynamic)

add_executable(mac ${MAC_SOURCES})
target_link_libraries(mac libmac)

add_executable(Sample1 ${SAMPLE1_SOURCES})
target_link_libraries(Sample1 libmac)

install(TARGETS libmac DESTINATION lib)
install(TARGETS mac DESTINATION bin)
