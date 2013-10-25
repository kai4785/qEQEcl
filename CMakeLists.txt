project(qeqecl)

cmake_minimum_required(VERSION 2.8)

find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(HEADERS
        qEQEcl.h
)

set(SOURCES
        main.cpp
        qEQEcl.cpp
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
	${HEADERS}
)

set_source_files_properties(${HEADERS_MOC} ${FORM_HEADERS} ${QRESOURCES_RCC}
	PROPERTIES GENERATED True)

target_link_libraries(${PROJECT_NAME}
        ${QT_LIBRARIES}
)

set(APPS ${PROJECT_NAME})  # paths to executables
set(DIRS ...)   # directories to search for prerequisites
INSTALL(CODE "
   include(BundleUtilities)
    fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
    " COMPONENT Runtime
)
include(CPack)
