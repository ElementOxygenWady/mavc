#* Author: Wady <iswady@163.com>
#* Date:   Thu Feb 24 12:00:00 2022

cmake_minimum_required(VERSION 3.16.3)
project(MAvc VERSION 0.0.1 LANGUAGES C CXX)

if(EXTRA_CMAKE)
    include("${EXTRA_CMAKE}")
    message(STATUS "Including extra cmake from ${EXTRA_CMAKE}")
endif()

# Used to configure generated files.
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

# Basic Information.
MESSAGE(STATUS "CMake version: " ${CMAKE_VERSION})
MESSAGE(STATUS "CMake system name: " ${CMAKE_SYSTEM_NAME})
MESSAGE(STATUS "CMake system processor: " ${CMAKE_SYSTEM_PROCESSOR})

# Specifying a search path for CMake modules to be loaded by the the include() or find_package() commands before checking the default modules that come with CMake.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Macro to provide an option dependent on other options.
include(CMakeDependentOption)

# Provides a macro to check if a list of one or more header files can be included together in C.
include(CheckIncludeFiles)

# Check if a C function can be linked.
include(CheckFunctionExists)

# Check if a symbol exists as a function, variable, or macro.
include(CheckSymbolExists)

# Provides install directory variables as defined by the GNU Coding Standards.
# GNU Coding Standards Reference 'https://www.gnu.org/prep/standards/html_node/Directory-Variables.html'.
include(GNUInstallDirs)

# Check whether the CXX compiler supports a given flag.
include(CheckCXXCompilerFlag)

# Check whether the C compiler supports a given flag.
include(CheckCCompilerFlag)

if((NOT CMAKE_INSTALL_RPATH) AND CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR})
    message(STATUS "Setting install rpath to ${CMAKE_INSTALL_RPATH}")
endif()

include(cmake/CMakeUtils.cmake)
include(cmake/Version.cmake)

# Provides an option that the user can optionally select.
option(ENABLE_SHARED "Build shared library." OFF)
option(ENABLE_STATIC "Build static library." ON)
option(ENABLE_STRICT "Build with strict compile options." ON)
option(ENABLE_TESTS "Enable compilation of test programs." OFF)
option(ENABLE_DEBUG "Enable any functionality related to debugging." OFF)
option(ENABLE_DEBUG_LOG "Turn on or off debug level logs." ON)
option(ENABLE_CPPCHECK "Turn on or off static analysis." OFF)
option(ENABLE_DOC "Enable documentation generation with Doxygen." OFF)
option(ENABLE_PIC "Enable position-independent code." ON)

if(ENABLE_DEBUG)
    set(ENABLE_DEBUG_LOG ON)
endif()

if(ENABLE_CPPCHECK)
    include(cmake/Cppcheck.cmake)
endif()

if(ENABLE_DOC)
    include(cmake/Doxygen.cmake)
endif()

if(ENABLE_PIC)
    check_c_compiler_flag("-fpic" SUPPORT_fpic)
    if(SUPPORT_fpic)
        add_compile_options("-fpic")
    endif()
    check_c_compiler_flag("-fPIC" SUPPORT_fPIC)
    if(SUPPORT_fPIC)
        add_compile_options("-fPIC")
    endif()
endif()

if((NOT ENABLE_SHARED) AND (NOT ENABLE_STATIC))
    message(FATAL_ERROR "Please enable shared or static library.")
endif()

# Strict options, reference from Belledonne Communications, Grenoble France.
set(STRICT_OPTIONS_CPP "")
set(STRICT_OPTIONS_C "")
set(STRICT_OPTIONS_CXX "")
list(APPEND STRICT_OPTIONS_CPP
    "-Wall"
    "-Wcast-align"
    # "-Wconversion"
    "-Werror=return-type"
    "-Wfloat-equal"
    "-Winit-self"
    "-Wno-error=deprecated-declarations"
    "-Wpointer-arith"
    "-Wuninitialized"
    "-Wunused"
)
list(APPEND STRICT_OPTIONS_CXX
    "-Wnon-virtual-dtor"
    "-Woverloaded-virtual"
)
CHECK_CXX_COMPILER_FLAG("-Wsuggest-override" SUGGEST_OVERRIDE)
if(SUGGEST_OVERRIDE)
    list(APPEND STRICT_OPTIONS_CXX "-Wsuggest-override" "-Wno-error=suggest-override")
endif()
list(APPEND STRICT_OPTIONS_C "-Wstrict-prototypes" "-Werror=strict-prototypes")
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(APPEND STRICT_OPTIONS_C "-fno-inline-small-functions")
endif()
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    list(APPEND STRICT_OPTIONS_CPP "-Qunused-arguments" "-Wno-array-bounds")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND STRICT_OPTIONS_CXX "-x c++")
endif()
if(ENABLE_STRICT)
    list(APPEND STRICT_OPTIONS_C "-Werror" "-Wextra" "-Wno-unused-parameter" "-Wno-missing-field-initializers" "-fno-strict-aliasing")
    list(APPEND STRICT_OPTIONS_CPP "-Werror" "-Wextra" "-Wno-unused-parameter" "-Wno-missing-field-initializers" "-fno-strict-aliasing")
endif()
if(STRICT_OPTIONS_CPP)
    list(REMOVE_DUPLICATES STRICT_OPTIONS_CPP)
endif()
if(STRICT_OPTIONS_C)
    list(REMOVE_DUPLICATES STRICT_OPTIONS_C)
endif()

if(ENABLE_DEBUG)
    add_definitions(-DENABLE_DEBUG=1)
endif()

if(ENABLE_DEBUG_LOG)
    add_definitions(-DENABLE_DEBUG_LOG=1)
endif()

if(DEFAULT_LOG_FILE)
    add_definitions(-DDEFAULT_LOG_FILE="${DEFAULT_LOG_FILE}")
endif()

if(DEFAULT_LOG_FILE_MAX_SIZE)
    add_definitions(-DDEFAULT_LOG_FILE_MAX_SIZE=${DEFAULT_LOG_FILE_MAX_SIZE})
endif()

if(DEFAULT_LOG_FILE_MAX_COUNT)
    add_definitions(-DDEFAULT_LOG_FILE_MAX_COUNT=${DEFAULT_LOG_FILE_MAX_COUNT})
endif()

if(ABASE_LOG_DEFAULT_BUFFSIZE)
    add_definitions(-DABASE_LOG_DEFAULT_BUFFSIZE=${ABASE_LOG_DEFAULT_BUFFSIZE})
endif()

if(ABASE_LOG_DEFAULT_OPTION)
    add_definitions(-DABASE_LOG_DEFAULT_OPTION=${ABASE_LOG_DEFAULT_OPTION})
endif()

if(ABASE_LOG_DEFAULT_LEVEL)
    add_definitions(-DABASE_LOG_DEFAULT_LEVEL=${ABASE_LOG_DEFAULT_LEVEL})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
# Source files can have properties that affect how they are built.
#  GENERATED - Is this source file generated as part of the build process.
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/config.h PROPERTIES GENERATED ON)
add_definitions(-DHAVE_CONFIG_H=1)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

set(COMMON_LIBS "")
set(COMMON_INCLUDE_DIRS "")

find_package(ABase)
if(NOT ABASE_FOUND)
    message(FATAL_ERROR "Missing ABase, please compile it first.")
endif()
list(APPEND COMMON_LIBS "${ABASE_LIBRARIES}")
list(APPEND COMMON_INCLUDE_DIRS "${ABASE_INCLUDE_DIRS}")

find_package(MTool)
if(NOT MTOOL_FOUND)
    message(FATAL_ERROR "Missing module tool, please compile it first.")
endif()
list(APPEND COMMON_LIBS "${MTOOL_LIBRARIES}")
list(APPEND COMMON_INCLUDE_DIRS "${MTOOL_INCLUDE_DIRS}")

find_package(E2Str)
if(NOT E2STR_FOUND)
    message(FATAL_ERROR "Missing E2Str, please compile it first.")
endif()
list(APPEND COMMON_LIBS "${E2STR_LIBRARIES}")
list(APPEND COMMON_INCLUDE_DIRS "${E2STR_INCLUDE_DIRS}")

find_package(cJSON)
if(NOT CJSON_LIBRARIES)
    message(FATAL_ERROR "Missing cJSON, please compile it first.")
endif()
list(APPEND COMMON_LIBS "${CJSON_LIBRARY}")
list(APPEND COMMON_INCLUDE_DIRS "${CJSON_INCLUDE_DIRS}")

find_package(Pjutil)
if (NOT PJUTIL_FOUND)
    message(FATAL_ERROR "Missing Pjutil, please compile it first.")
endif()

find_package(Pjlib)
if (NOT PJLIB_FOUND)
    message(FATAL_ERROR "Missing Pjlib, please compile it first.")
endif()

find_package(Pjsip)
if (NOT PJSIP_FOUND)
    message(FATAL_ERROR "Missing Pjsip, please compile it first.")
endif()

find_package(PJApp)
if(NOT PJAPP_LIBRARIES)
    message(FATAL_ERROR "Missing Pjapp, please compile it first.")
endif()
list(APPEND COMMON_LIBS "${PJAPP_LIBRARIES}")
list(APPEND COMMON_INCLUDE_DIRS "${PJAPP_INCLUDE_DIRS}")

list(REMOVE_DUPLICATES COMMON_LIBS)
list(REMOVE_DUPLICATES COMMON_INCLUDE_DIRS)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(executable)

if(ENABLE_TESTS)
    add_definitions(-DENABLE_TESTS=1)
    add_subdirectory(tester)
endif()

set(CONFIG_SIGNATURE "${PROJECT_NAME_UPPER}")
configure_file(cmake/Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)

set(CONFIG_PACKAGE_LOCATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake")
install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION ${CONFIG_PACKAGE_LOCATION}
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION ${CONFIG_PACKAGE_LOCATION}
)
