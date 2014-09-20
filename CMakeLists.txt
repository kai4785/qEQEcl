cmake_minimum_required(VERSION 3.0.2)

project(qeqecl)
set(CMAKE_INCLUDE_CURRENT_DIR On)
set(CMAKE_AUTOMOC On)
set(CMAKE_AUTOUIC On)
set(CMAKE_AUTORCC On)

find_package(Qt4 REQUIRED)

set(HEADERS
        fops.h
        miniz.h
        qEQEcl.h
        qtfiledownload.h
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

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${SOURCES}
        ${HEADERS}
)


target_link_libraries(${PROJECT_NAME}
        Qt4::QtCore
        Qt4::QtGui
        Qt4::QtNetwork
)

set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 1)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS On)
set(CPACK_PACKAGE_CONTACT "Kai Meyer <kai@gnukai.com>")

include(qeqecl_install.cmake)

include(CPack)
