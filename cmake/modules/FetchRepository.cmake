#
# Checkout remote repository
#
macro(Clone_Repository name url tag)
    if(NOT ${name}_REPOSITORY)
        set(${name}_REPOSITORY ${url})
    endif()
    if(NOT ${name}_TAG)
        set(${name}_TAG ${tag})
    endif()
    message(STATUS "Fetching '${name}' - URL: ${${name}_REPOSITORY} Tag: ${${name}_TAG}")

    # Check for FetchContent cmake support
    if(${CMAKE_VERSION} VERSION_LESS "3.14")
        message(FATAL_ERROR "CMake 3.14 required to fetch ${name}!")
		
    else()
        include(FetchContent)

        string(TOLOWER ${name} name_lower)
        string(TOUPPER ${name} name_upper)

        FetchContent_Declare(${name}
            GIT_REPOSITORY ${${name}_REPOSITORY}
            GIT_TAG ${${name}_TAG}
            #SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${name_lower}
			)

		# Check if population has already been performed
        FetchContent_GetProperties(${name} POPULATED ${name_lower}_POPULATED)
		
		# Populate the source directory.
        if(NOT ${name_lower}_POPULATED)
		
			# Fetch the content using previously declared details
            FetchContent_Populate(${name})

			message(STATUS "${name} source dir: ${${name_lower}_SOURCE_DIR}")
			message(STATUS "${name} binary dir: ${${name_lower}_BINARY_DIR}")
			
			# Bring the populated content into the build
			add_subdirectory(${${name_lower}_SOURCE_DIR} ${${name_lower}_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()

		#FetchContent_MakeAvailable(${name})
		
        set(${name_upper}_SOURCE_DIR ${${name_lower}_SOURCE_DIR})
        set(${name_upper}_BINARY_DIR ${${name_lower}_BINARY_DIR})
		
        set(${name}_FETCHED YES)
    endif()
	
endmacro()
