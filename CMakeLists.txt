cmake_minimum_required(VERSION 2.8)

project(qeqecl)

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
if(WIN32)
	set(QT_USE_QTMAIN TRUE)
endif()
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(QT_HEADERS
        qEQEcl.h
        qtfiledownload.h
)

set(HEADERS
        fops.h
        miniz.h
)

set(SOURCES
        main.cpp
        qEQEcl.cpp
        fops.cpp
        qtfiledownload.cpp
        miniz.c
)

set(FORMS
        qEQEcl.ui
)

set(QRESOURCES
)
qt4_wrap_cpp(HEADERS_MOC ${QT_HEADERS})
qt4_wrap_ui(FORM_HEADERS ${FORMS})
qt4_add_resources(QRESOURCES_RCC ${QRESOURCES})

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${SOURCES}
        ${HEADERS_MOC}
        ${FORM_HEADERS}
        ${QRESOURCES_RCC}
        ${QT_HEADERS}
        ${HEADERS}
)

set_source_files_properties(${HEADERS_MOC} ${FORM_HEADERS} ${QRESOURCES_RCC}
	PROPERTIES GENERATED True)

target_link_libraries(${PROJECT_NAME}
        ${QT_LIBRARIES}
)

set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 3)
set(CPACK_PACKAGE_CONTACT "Kai Meyer <kai@gnukai.com>")

include(qeqecl_install.cmake)

include(CPack)
