# This file provides the functions;
#   "PostBuild_WindowsCopyFiles"
#   "PreBuild_WindowsCopyFiles"
#
# NOTE: These are only valid on Windows platforms!
#
# ALSO NOTE: This will most likely fail when trying to copy to a network
#            share or drive. A work-around for this is to create a symbolic
#            link to the share, then pass this path into the functions.

# Include guard
#if(PostBuild_WindowsCopyFiles)
#	return()
#endif()
#set(PostBuild_WindowsCopyFiles YES)

# Any number of files to copy from SOURCE_DIR to DEST_DIR can be specified after DEST_DIR.
# This copying happens as an post-build event.
function(PostBuild_WindowsCopyFiles TARGET SOURCE_DIR DEST_DIR)

    # Windows commandline expects the / to be \ so switch them.
    string(REPLACE "/" "\\\\" SOURCE_DIR ${SOURCE_DIR})
    string(REPLACE "/" "\\\\" DEST_DIR ${DEST_DIR})

    # /NJH /NJS /NDL /NFL /NC /NS /NP - Silence any output
    # CMake adds an extra check for command success which doesn't work too well
	# with robocopy so trick it into thinking the command was successful with
	# the '|| cmd /c "exit /b 0"'.
	# We also call "make_directory" to be sure the output directory exists.
    add_custom_command(TARGET ${TARGET} POST_BUILD
#       COMMAND ${CMAKE_COMMAND} -E make_directory ${DEST_DIR}
        COMMAND robocopy ${SOURCE_DIR} ${DEST_DIR} ${ARGN} /NJH /NJS /NDL /NFL /NC /NS || cmd /c "exit /b 0"
		COMMENT "Copying ${ARGN} from ${SOURCE_DIR} to ${DEST_DIR}"
    )
	
#	message(STATUS "Created post build script to copy files")
endfunction()

# Any number of files to copy from SOURCE_DIR to DEST_DIR can be specified after DEST_DIR.
# This copying happens as an pre-build event.
function(PreBuild_WindowsCopyFiles TARGET SOURCE_DIR DEST_DIR)

    # Windows commandline expects the / to be \ so switch them.
    string(REPLACE "/" "\\\\" SOURCE_DIR ${SOURCE_DIR})
    string(REPLACE "/" "\\\\" DEST_DIR ${DEST_DIR})

    # /NJH /NJS /NDL /NFL /NC /NS /NP - Silence any output
    # CMake adds an extra check for command success which doesn't work too well
	# with robocopy so trick it into thinking the command was successful with
	# the '|| cmd /c "exit /b 0"'.
	# We also call "make_directory" to be sure the output directory exists.
    add_custom_command(TARGET ${TARGET} PRE_BUILD
#       COMMAND ${CMAKE_COMMAND} -E make_directory ${DEST_DIR}
        COMMAND robocopy ${SOURCE_DIR} ${DEST_DIR} ${ARGN} /NJH /NJS /NDL /NFL /NC /NS || cmd /c "exit /b 0"
		COMMENT "Copying ${ARGN} from ${SOURCE_DIR} to ${DEST_DIR}"
    )
	
#	message(STATUS "Created post build script to copy files")
endfunction()
