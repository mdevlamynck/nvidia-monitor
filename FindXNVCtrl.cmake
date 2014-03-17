# - Try to find XNVCtrl
# Once done, this will define
#
#  XNVCtrl_FOUND		- system has XNVCtrl
#  XNVCtrl_INCLUDE_DIR	- the XNVCtrl include directories
#  XNVCtrl_LIBRARY		- link these to use XNVCtrl

# define the list of search paths for headers and libraries
set(	FIND_XNVCtrl_PATH
		/usr/local
		/opt
		/opt/local
)

# find the XNVCtrl include directory
find_path(	XNVCtrl_INCLUDE_DIR NVCtrl/NVCtrlLib.h
          	PATH_SUFFIXES include
		  	PATHS ${FIND_XNVCtrl_PATHS}
)

# find the XNVCtrl lib path
find_library(	XNVCtrl_LIBRARY XNVCtrl ${FIND_XNVCtrl_PATH}	)

# Check if include dir and lib path are found
if(XNVCtrl_INCLUDE_DIR AND XNVCtrl_LIBRARY)
	set(XNVCtrl_FOUND true)
else ()
	set(XNVCtrl_FOUND false)
endif()

# Display result
if (NOT XNVCtrl_FOUND)
	if(XNVCtrl_FIND_REQUIRED)
        # fatal error
		message(FATAL_ERROR "Could not find XNVCtrl")
	elseif(NOT XNVCtrl_FIND_QUIETLY)
        # error but continue
		message("Could not find XNVCtrl")
    endif()
else()
	find_package_message(XNVCtrl "Found XNVCtrl include: ${XNVCtrl_INCLUDE_DIR}" "${XNVCtrl_INCLUDE_DIR}")
	find_package_message(XNVCtrl "Found XNVCtrl lib: ${XNVCtrl_LIBRARY}" "${XNVCtrl_LIBRARY}")
endif()

