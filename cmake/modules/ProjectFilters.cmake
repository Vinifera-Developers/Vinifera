# This function sorts the project files as they appear in the root directory.
# Set up source groups (for better browsing inside IDEs) for the provided list
# of source files.

# Filenames can be absolute or relative to SOURCE_DIR.
# Group names get created based on that path.

# The important part ("Header Files", "Source Files" and "Resource Files" are literals.)

function(SetupProjectFilters REMOVE_FOLDER)

#	message(STATUS "SetupProjectFilters: \n  ARGN: ${ARGN}\n")

	foreach(arg_file IN LISTS ARGN)

#		message(STATUS "* Processing ${arg_file}")

		# Get the directory of the source file.
		get_filename_component(parent_dir "${arg_file}" DIRECTORY)
		
		# Remove project source directory prefix to make the group.
		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" group "${parent_dir}")

		# Remove slashes.
		#string(REPLACE "/" "" group "${group}")
		
		# Remove any "src" in the path.
		string(REPLACE "src" "" group "${group}")
		
		# Remove the root containing folder in the path.
		if(NOT ${REMOVE_FOLDER} STREQUAL "") 
			string(REPLACE ${REMOVE_FOLDER} "" group "${group}")
		endif()
		
#		message(STATUS "* group ${group}")
		
		# Make sure we are using windows slashes.
		string(REPLACE "/" "\\" group "${group}")
		
		# Loose files get put into the project "root".
		if(group STREQUAL "")
#			set(group "root")
			
		# Group resources into "Resource Files"
		elseif(("${arg_file}" MATCHES ".*\\res"
		    OR "${arg_file}" MATCHES ".*\\resource"
		    OR "${arg_file}" MATCHES ".*\\resources")
		  AND ("${arg_file}" MATCHES ".*\\.h"
			OR "${arg_file}" MATCHES ".*\\.ico"
			OR "${arg_file}" MATCHES ".*\\.cur"
			OR "${arg_file}" MATCHES ".*\\.rc"))
			set(group "Resource Files")
#			message(STATUS "Added Resource: ${arg_file}")
			
		# Group .c and .cpp into "Source Files"
		elseif("${arg_file}" MATCHES ".*\\.c"
			OR "${arg_file}" MATCHES ".*\\.cpp"
			OR "${arg_file}" MATCHES ".*\\.cxx")
			set(group "Source Files${group}")
#			message(STATUS "Added Source: ${arg_file}")
			
		# Group .h and .hpp into "Header Files"
		elseif("${arg_file}" MATCHES ".*\\.h"
			OR "${arg_file}" MATCHES ".*\\.hh"
			OR "${arg_file}" MATCHES ".*\\.hpp")
			set(group "Source Files${group}")
#    		set(group "Header Files${group}") # We want headers and sources to be in the same tree.
#			message(STATUS "Added Header: ${arg_file}")
		
		endif()

#		message(STATUS "  FILE = ${arg_file}\n  GROUP = ${group}\n")
		
		# Set the source file filter.
		source_group("${group}" FILES "${arg_file}")

	endforeach()

endfunction()

function(SetCustomProjectFilters FILTER_NAME)

#	message(STATUS "SetCustomProjectFilters: \n  FILTER_NAME: ${FILTER_NAME}\n\n  ARGN: ${ARGN}\n")

	foreach(arg_file IN LISTS ARGN)

		# Make sure we are using windows slashes.
#		string(REPLACE "/" "\\" group "${FILTER_NAME}")

#		message(STATUS "  GROUP = ${group}")

		# Set the custom source files filter.
		source_group("${FILTER_NAME}" FILES "${arg_file}")
		
#		message(STATUS "  FILE = ${arg_file}\n  GROUP = ${FILTER_NAME}\n")

	endforeach()
	
endfunction()
