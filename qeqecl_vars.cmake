# CPack version
set(CPACK_PACKAGE_VERSION_MAJOR ${${PROJECT_NAME}_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${${PROJECT_NAME}_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${${PROJECT_NAME}_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${${PROJECT_NAME}_VERSION})
# CMake vars
set(CMAKE_INCLUDE_CURRENT_DIR On)
set(CMAKE_AUTOMOC On)
set(CMAKE_AUTOUIC On)
set(CMAKE_AUTORCC On)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
# CPack vars
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS On)
set(CPACK_PACKAGE_CONTACT "Kai Meyer <kai@gnukai.com>")
# Platform Identification
if(WIN32) # This doesn't include Cygwin anymore, as of 2.8.4
    set(PLATFORM WIN32)
    set(PLAT_WIN32 On)
    set(PLAT_BINDIR ".")
    add_definitions(-DPLAT_WIN32)
    if(NOT MSVC)
        message(AUTHOR_WARNING "Nobody's tried to do this on Windows with out MSVC. Let me know how it goes.")
    endif()
    # set(CPACK_WIX_PACKAGE_NAME ${PROJECT_NAME})
    # set(CPACK_WIX_DISPLAY_NAME ${PROJECT_NAME})
    # set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${PROJECT_NAME})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
    set(CPACK_WIX_UPGRADE_GUID "7048B259-981C-49E7-A291-B89B67C7A20B")
    set(CPACK_GENERATOR WIX)
    set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} "qEQEcl")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "qEQEcl")
elseif(UNIX)
    # TODO: Expand this to be "Ubuntu", "Fedora", ect; probably lsb_release output
    #       At least differentiate between deb/rpm for CPack's sake
    set(PLATFORM LINUX)
    set(PLAT_LINUX On)
    set(PLAT_BINDIR "bin")
    add_definitions(-DPLAT_LINUX)
elseif(APPLE)
    set(PLATFORM APPLE)
    set(PLAT_APPLE On)
    set(PLAT_BINDIR ".")
    add_definitions(-DPLAT_APPLE)
    message(AUTHOR_WARNING "Nobody's tried this on Apple yet. Let me know how it goes.")
endif()
# Build configuration specific compile definitions
set_property(DIRECTORY
    APPEND
    PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:Debug>:QEQECL_DEBUG>
    $<$<CONFIG:Release>:QEQECL_RELEASE>
    )
