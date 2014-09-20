#http://www.cmake.org/Wiki/BundleUtilitiesExample
#http://mikemcquaid.com/2012/01/04/deploying-qt-applications-with-deployqt4/
set(plugin_dest_dir share/qt4)
set(qtconf_dest_dir /etc)
set(APPS "$ENV{DESTDIR}/\${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}")
if(APPLE)
	set(plugin_dest_dir ${PROJECT_NAME}.app/Contents/MacOS)
	set(qtconf_dest_dir ${PROJECT_NAME}.app/Contents/Resources)
	set(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")
endif(APPLE)
if(WIN32)
	set(qtconf_dest_dir bin)
	set(plugin_dest_dir bin)
	set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}.exe")
endif(WIN32)

install(TARGETS ${PROJECT_NAME} 
	BUNDLE DESTINATION . #COMPONENT Runtime
	RUNTIME DESTINATION bin #COMPONENT Runtime
)

#We aren't doing plugins
#install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)
if(WIN32 OR APPLE)
	# If we ever do Linux, we'll need to add \$ENV{DESTDIR}/ in there somewhere
	INSTALL(CODE "
	file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
	" 
	#COMPONENT Runtime
	)
set(DIRS ${QT_LIBRARY_DIRS})

install(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    "
    #COMPONENT Runtime
    )
endif()

if(WIN32)
	# This copies a manifest and a correct CRT into the installation directory
	# VS 2008 SP1 has a bug in the Maifest which requires manual patching
	# http://www.kitware.com/blog/home/post/4
	include(InstallRequiredSystemLibraries)
	# set(CPACK_WIX_PACKAGE_NAME ${PROJECT_NAME})
	# set(CPACK_WIX_DISPLAY_NAME ${PROJECT_NAME})
	# set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${PROJECT_NAME})
	set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
	set(CPACK_WIX_UPGRADE_GUID "7048B259-981C-49E7-A291-B89B67C7A20B")
	set(CPACK_GENERATOR WIX)
	set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} "Kai's qEQEcl")
	set(CPACK_WIX_PROGRAM_MENU_FOLDER "qEQEcl")
endif()
