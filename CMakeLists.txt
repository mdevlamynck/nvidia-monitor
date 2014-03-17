
#	NVidia Monitor - plasmoid that displays nvidia gpu's informations
#	Copyright (C) 2013 Matthias Devlamynck
#	
#	This file is part of NVidia Monitor.
#	
#	NVidia Monitor is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#	
#	NVidia Monitor is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#	GNU General Public License for more details.
#	
#	You should have received a copy of the GNU General Public License
#	along with NVidia Monitor. If not, see <http://www.gnu.org/licenses/>.
#	
#	To contact me, please send me an email at :
#		matthias.devlamynck@mailoo.org
#	
#	The source code is aviable at :
#		https://github.com/mdevlamynck/nvidia-monitor/	
#	
#	If you wish to make a fork or maintain this project, please contact me.

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

# ------------------ Applet ----------------------
project(plasma_applet_nvidia-monitor)
 
	find_package		( KDE4 REQUIRED							)
	include				( KDE4Defaults							)
	add_definitions		( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS}	)
	include_directories	( ${KDE4_INCLUDES}						)
 
	file				( GLOB_RECURSE ${PROJECT_NAME}_src applet/*		)
	kde4_add_ui_files	( ${PROJECT_NAME}_src NVidiaMonitorConfig.ui	)
	 
	kde4_add_plugin			( ${PROJECT_NAME} ${${PROJECT_NAME}_src}					)
	target_link_libraries	( ${PROJECT_NAME} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS}	)
	 
	install	( TARGETS ${PROJECT_NAME} DESTINATION ${PLUGIN_INSTALL_DIR}			)
	install	( FILES ${PROJECT_NAME}.desktop DESTINATION ${SERVICES_INSTALL_DIR}	) 

	# Localization 
	find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

	if		( NOT GETTEXT_MSGFMT_EXECUTABLE	)
		message("------ NOTE: msgfmt not found. Translations will *not* be installed ------")
	else	( NOT GETTEXT_MSGFMT_EXECUTABLE	)

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
 
	find_package		( KDE4 REQUIRED							)
	find_package		( X11 REQUIRED							)
	find_package		( Xext REQUIRED							)
	find_package		( XNVCtrl REQUIRED						)
	include				( KDE4Defaults							)
	add_definitions 	( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS}	)
	include_directories	( ${KDE4_INCLUDES} ${X11_INCLUDE_DIR} ${Xext_INCLUDE_DIR} ${XNVCtrl_INCLUDE_DIR}	)
	 
	file	( GLOB_RECURSE ${PROJECT_NAME}_src data-engine/*	)
	 
	kde4_add_plugin			( ${PROJECT_NAME} ${${PROJECT_NAME}_src}					)
	target_link_libraries	( ${PROJECT_NAME} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES} ${Xext_LIBRARY} ${XNVCtrl_LIBRARY}	)
	 
	install	( TARGETS ${PROJECT_NAME} DESTINATION ${PLUGIN_INSTALL_DIR}			)
	install	( FILES ${PROJECT_NAME}.desktop DESTINATION ${SERVICES_INSTALL_DIR}	) 

# ------------------------------------------------

