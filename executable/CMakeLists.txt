#* Author: Wady <iswady@163.com>
#* Date:   Thu Feb 24 12:00:00 2022

set(EXECUTABLE_NAME "${PROJECT_NAME_LOWER}_exe")

set(SOURCE_FILES_C
    main.c
)

if(ENABLE_SHARED)
    set(LIBS
        ${LIBRARY_NAME}
    )
else()
    set(LIBS
        ${LIBRARY_NAME}-static
    )
endif()

list(APPEND LIBS
    ${COMMON_LIBS}
)

set(INCLUDE_DIRS
    ${COMMON_INCLUDE_DIRS}
)

find_package(MHub)
if(NOT MHUB_FOUND)
    message(FATAL_ERROR "Missing MHub, please compile it first.")
endif()
list(APPEND LIBS "${MHUB_LIBRARIES}")
list(APPEND INCLUDE_DIRS "${MHUB_INCLUDE_DIRS}")

utils_apply_compile_flags(SOURCE_FILES_C STRICT_OPTIONS_CPP STRICT_OPTIONS_C)

add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES_C})
target_link_libraries(${EXECUTABLE_NAME} ${LIBS})
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDE_DIRS})

install(TARGETS ${EXECUTABLE_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
