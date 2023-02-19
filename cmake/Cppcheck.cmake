#* Author: Wady <iswady@163.com>
#* Date:   Mon Feb 28 12:00:00 2022

# Provides install directory variables as defined by the GNU Coding Standards.
# GNU Coding Standards Reference 'https://www.gnu.org/prep/standards/html_node/Directory-Variables.html'.
include(GNUInstallDirs)

# Check sizeof a type.
include(CheckTypeSize)

find_program(CPPCHECK_PATH cppcheck)
if(CPPCHECK_PATH)
    message(STATUS "Found cppcheck: ${CPPCHECK_PATH}")
else()
    message(FATAL_ERROR "Missing cppcheck, please add it to environment variable.")
endif()

set(HEADER_FILE_LIST
    stdio.h
)

set(HEADER_FILE_PATHS "")
foreach(HEADER_FILE ${HEADER_FILE_LIST})
    unset(HEADER_FILE_PATH CACHE)
    find_path(HEADER_FILE_PATH ${HEADER_FILE})
    if(HEADER_FILE_PATH)
        list(APPEND HEADER_FILE_PATHS "${HEADER_FILE_PATH}")
    else()
        message(FATAL_ERROR "Could not find header file: ${HEADER_FILE}")
    endif()
endforeach()

if(EXISTS "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
    list(APPEND HEADER_FILE_PATHS "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
endif()

set(CPPCHECK_INCLUDE_OPTIONS "")
set(CPPCHECK_SUPPRESS_OPTIONS "")
foreach(HEADER_FILE_PATH ${HEADER_FILE_PATHS})
    list(APPEND CPPCHECK_INCLUDE_OPTIONS "-I${HEADER_FILE_PATH}")
    list(APPEND CPPCHECK_SUPPRESS_OPTIONS "--suppress=*:${HEADER_FILE_PATH}/*")
endforeach()

list(APPEND CPPCHECK_INCLUDE_OPTIONS "-I${PROJECT_SOURCE_DIR}/include")
list(APPEND CPPCHECK_INCLUDE_OPTIONS "-I${CMAKE_CURRENT_BINARY_DIR}")

set(CPPCHECK_OPTIONS "")
list(APPEND CPPCHECK_OPTIONS "--enable=all")
list(APPEND CPPCHECK_OPTIONS "--report-progress")
list(APPEND CPPCHECK_OPTIONS "--inline-suppr")
list(APPEND CPPCHECK_OPTIONS "--inconclusive")
list(APPEND CPPCHECK_OPTIONS "--bug-hunting")
list(APPEND CPPCHECK_OPTIONS "--max-ctu-depth=8")
#list(APPEND CPPCHECK_OPTIONS "--force")
list(APPEND CPPCHECK_OPTIONS "${CPPCHECK_INCLUDE_OPTIONS}")
list(APPEND CPPCHECK_OPTIONS "${CPPCHECK_SUPPRESS_OPTIONS}")

check_type_size("char *" CPPCHECK_TYPE_SIZE)

string(TOLOWER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME_VAR)
if(("${CMAKE_SYSTEM_NAME_VAR}" STREQUAL "linux") OR ("${CMAKE_SYSTEM_NAME_VAR}" STREQUAL "android"))
    if("${CPPCHECK_TYPE_SIZE}" STREQUAL "8")
        list(APPEND CPPCHECK_OPTIONS "--platform=unix64")
    elseif("${CPPCHECK_TYPE_SIZE}" STREQUAL "4")
        list(APPEND CPPCHECK_OPTIONS "--platform=unix32")
    else()
        message(FATAL_ERROR "Invalid type size: ${CPPCHECK_TYPE_SIZE}")
    endif()
else()
    message(FATAL_ERROR "Unknown system name: ${CMAKE_SYSTEM_NAME}")
endif()

set(CPPCHECK_C_OPTIONS "${CPPCHECK_OPTIONS}")
list(APPEND CPPCHECK_C_OPTIONS "--language=c")
list(APPEND CPPCHECK_C_OPTIONS "--std=c99")

set(CPPCHECK_CPP_OPTIONS "${CPPCHECK_OPTIONS}")
list(APPEND CPPCHECK_CPP_OPTIONS "--language=c++")
list(APPEND CPPCHECK_CPP_OPTIONS "--std=c++11")

set(CMAKE_C_CPPCHECK "${CPPCHECK_PATH}" "${CPPCHECK_C_OPTIONS}")
set(CMAKE_CPP_CPPCHECK "${CPPCHECK_PATH}" "${CPPCHECK_CPP_OPTIONS}")
