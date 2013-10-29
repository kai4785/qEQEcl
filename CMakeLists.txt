project(qeqecl)

cmake_minimum_required(VERSION 2.8)

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(HEADERS
        qEQEcl.h
        miniz.h
)

set(SOURCES
        main.cpp
        qEQEcl.cpp
        miniz.c
)

set(FORMS
        qEQEcl.ui
)

set(QRESOURCES
)
QT4_WRAP_CPP(HEADERS_MOC ${HEADERS})
QT4_WRAP_UI(FORM_HEADERS ${FORMS})
qt4_add_resources(QRESOURCES_RCC ${QRESOURCES})

add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS_MOC}
        ${FORM_HEADERS}
        ${QRESOURCES_RCC}
)

target_link_libraries(${PROJECT_NAME}
        ${QT_LIBRARIES}
)

if(WIN32)
    # This copies a manifest and a correct CRT into the installation directory
    include(InstallRequiredSystemLibraries)
    #http://www.cmake.org/Wiki/BundleUtilitiesExample
    #http://mikemcquaid.com/2012/01/04/deploying-qt-applications-with-deployqt4/
endif()
