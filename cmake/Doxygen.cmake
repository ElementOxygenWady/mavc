#* Author: Wady <iswady@163.com>
#* Date:   Tue Mar 01 12:00:00 2022

find_package(Doxygen REQUIRED dot mscgen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
    set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION_DETAIL}")
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/doc")
    set(DOXYGEN_FILE_PATTERNS "*.c" "*.h" "*.cpp" "*.hpp" "*.cc" "*.hh" "*.md")
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_JAVADOC_AUTOBRIEF NO)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_GENERATE_BUGLIST YES)
    set(DOXYGEN_GENERATE_TESTLIST YES)
    set(DOXYGEN_GENERATE_TODOLIST YES)
    set(DOXYGEN_GENERATE_DEPRECATEDLIST YES)
    set(DOXYGEN_HAVE_DOT YES)
    set(DOXYGEN_DOT_MULTI_TARGETS YES)
    set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
    if(PLANTUML_JAR_PATH)
        set(DOXYGEN_PLANTUML_JAR_PATH "${PLANTUML_JAR_PATH}")
        set(DOXYGEN_PLANTUML_INCLUDE_PATH "${PROJECT_SOURCE_DIR}")
    endif()
    doxygen_add_docs(doc
        ${PROJECT_SOURCE_DIR}
        ALL
    )
else()
    message(FATAL_ERROR "Missing doxygen, please add it to environment variable.")
endif()
