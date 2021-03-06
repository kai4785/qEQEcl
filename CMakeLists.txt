cmake_minimum_required(VERSION 3.0.2)

project(qeqecl VERSION 0.1.2)

include(qeqecl_vars.cmake)

find_package(Qt4 REQUIRED)

set(HEADERS
    fops.h
    miniz.h
    pidops.h
    qEQEcl.h
    qtfiledownload.h
)

set(SOURCES
    fops.cpp
    main.cpp
    miniz.c
    pidops.cpp
    qEQEcl.cpp
    qtfiledownload.cpp
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

add_executable(eqgame WIN32 MACOSX_BUNDLE
    eqgame_debug.cpp
)

target_link_libraries(eqgame
    Qt4::QtCore
    Qt4::QtGui
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION ${PLAT_BINDIR}
    )

include(CPack)

if(PLAT_WIN32)
    # This copies a manifest and a correct CRT into the installation directory
    # VS 2008 SP1 has a bug in the Manifest which requires manual patching
    # http://www.kitware.com/blog/home/post/4
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${PLAT_BINDIR})
    include(InstallRequiredSystemLibraries)
    include(DeployQt4)
    install_qt4_executable(${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX})
endif()
