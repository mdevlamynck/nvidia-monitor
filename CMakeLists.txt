# ------------------ Applet ----------------------
project(plasma_applet_nvidia-monitor)
 
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	include_directories(${KDE4_INCLUDES})
 
	file(GLOB_RECURSE nvidia-monitor-applet-src applet/*)
	kde4_add_ui_files(nvidia-monitor-applet-src NVidiaMonitorConfig.ui)
	 
	kde4_add_plugin(${PROJECT_NAME} ${nvidia-monitor-applet-src})
	target_link_libraries(${PROJECT_NAME} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} plasma_applet-system-monitor)
	 
	install(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGIN_INSTALL_DIR})
	 
	install(FILES ${PROJECT_NAME}.desktop DESTINATION ${SERVICES_INSTALL_DIR}) 

	# Localization 
	find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

	if(NOT GETTEXT_MSGFMT_EXECUTABLE)
		message( "------ NOTE: msgfmt not found. Translations will *not* be installed ------")
	else(NOT GETTEXT_MSGFMT_EXECUTABLE)

		set(catalogname ${PROJECT_NAME})

		add_custom_target(translations ALL)

		file(GLOB PO_FILES *.po)

		foreach(_poFile ${PO_FILES})
			get_filename_component(_poFileName ${_poFile} NAME)
			string(REGEX REPLACE "^${catalogname}_?" "" _langCode ${_poFileName} )
			string(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

			if( _langCode )
				get_filename_component(_lang ${_poFile} NAME_WE)
				set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

				add_custom_command(TARGET translations
					COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
					DEPENDS ${_poFile})
				install(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${catalogname}.mo)
			endif( _langCode )

		endforeach(_poFile ${PO_FILES})

	endif(NOT GETTEXT_MSGFMT_EXECUTABLE)

# ------------------ Engine ----------------------
project(plasma_engine_nvidia-monitor)
 
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	include_directories(${KDE4_INCLUDES})
	 
	file(GLOB_RECURSE nvidia-monitor-engine-src data-engine/*)
	 
	kde4_add_plugin(${PROJECT_NAME} ${nvidia-monitor-engine-src})
	target_link_libraries(${PROJECT_NAME} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
	 
	install(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGIN_INSTALL_DIR})
	 
	install(FILES ${PROJECT_NAME}.desktop DESTINATION ${SERVICES_INSTALL_DIR}) 

