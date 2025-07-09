# Fixup the list for use with "target_include_directories".
#string(REGEX REPLACE ";" " " COMMON_HEADERS "${COMMON_HEADERS}")
#string(REGEX REPLACE ";" " " COMMON_SOURCES "${COMMON_SOURCES}")
#string(REPLACE REGEX ";" "\n$" COMMON_HEADERS ${COMMON_HEADERS})
#string(REPLACE REGEX ";" "\n$" COMMON_SOURCES ${COMMON_SOURCES})

# Fixup the list for use with "target_include_directories".
function(Glob_Internal_Stringify_CR var line)
    if (ARGN)
        Glob_Internal_Stringify_CR(${var} ${ARGN})
    endif()
    set(${var} "${line}\n${${var}}" PARENT_SCOPE)
endfunction()


# Glob all child directories of the input path into a list.
function(GlobDirectories GLOBTYPE RESULT ROOT_PATH)

#	message(NOTICE "GlobDirectories!")
#	message(NOTICE "Input: ${ROOT_PATH}")

	file(${GLOBTYPE} children LIST_DIRECTORIES true RELATIVE ${ROOT_PATH} ${ROOT_PATH}/*)
	
	# Add the root path as a include directory at least.
	set(dirlist "${ROOT_PATH}")
	
	# Loop over all globbed child directories.
	foreach(child ${children})
	
		# Make sure this child directory exists. If it does, add it to
		# the dirlist (separated with a ";" delimiter).
		if(IS_DIRECTORY ${ROOT_PATH}/${child})
			list(APPEND dirlist ${ROOT_PATH}/${child})
#			message(NOTICE "Added Child: ${ROOT_PATH}/${child}")
		endif()

	endforeach()
	
	# Fixup the list for use with the likes of "target_include_directories".
	if(NOT "${dirlist}" STREQUAL "")
#		Glob_Internal_Stringify_CR(dirlist ${dirlist})
	endif()
	
	# This fails without PARENT_SCOPE!
	set(${RESULT} ${dirlist} PARENT_SCOPE)
	
#	message(NOTICE "Output List: ${dirlist}")

endfunction()


# Glob all source files in child directories of the input path into a list.
function(GlobSources GLOBTYPE RESULT ROOT_PATH)

#	message(NOTICE "GlobSources!")
#	message(NOTICE "Input: ${ROOT_PATH}")
	
	set(dirlist "")

#	message(NOTICE "GlobSources - Searching: ${ROOT_PATH}")

	# Glob all source files in this directory. If the glob was
	# successful, add the globbed files to the dirlist (separated with a ";" delimiter).
	file(GLOB files "${ROOT_PATH}/*.cpp")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobSources - Added Files: ${files}")
	endif()
	
	file(GLOB files "${ROOT_PATH}/*.c")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobSources - Added Files: ${files}")
	endif()
	
	file(GLOB files "${ROOT_PATH}/*.asm")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobSources - Added Files: ${files}")
	endif()

	file(GLOB children RELATIVE ${ROOT_PATH} ${ROOT_PATH}/*)
	
	# Glob all files in the root and subdirectories.
	if(${GLOBTYPE} STREQUAL "GLOB_RECURSE")
		
		# Loop over all globbed child directories.
		foreach(child ${children})
		
			# Make sure this child directory exists.
			if(IS_DIRECTORY ${ROOT_PATH}/${child})
#				message(NOTICE "GlobSources - Searching: ${ROOT_PATH}/${child}")
				
				# Glob all source files in this child directory. If the glob was
				# successful, add the globbed files to the dirlist (separated with a ";" delimiter).
				file(GLOB_RECURSE files "${ROOT_PATH}/${child}/*.cpp")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobSources - Added Files: ${files}")
				endif()
				
				file(GLOB_RECURSE files "${ROOT_PATH}/${child}/*.c")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobSources - Added Files: ${files}")
				endif()
				
				file(GLOB_RECURSE files "${ROOT_PATH}/${child}/*.asm")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobSources - Added Files: ${files}")
				endif()
			endif()

		endforeach()

	endif()

	# Fixup the list for use with the likes of "target_include_directories".
	if(NOT "${dirlist}" STREQUAL "")
#		Glob_Internal_Stringify_CR(dirlist ${dirlist})
	endif()

	# This fails without PARENT_SCOPE!
	set(${RESULT} ${dirlist} PARENT_SCOPE)
	
#	message(NOTICE "Output List: ${dirlist}")

endfunction()


# Glob all header files in child directories of the input path into a list.
function(GlobHeaders GLOBTYPE RESULT ROOT_PATH)

#	message(NOTICE "GlobHeaders!")
#	message(NOTICE "Input: ${ROOT_PATH}")
	
	set(dirlist "")
	
#	message(NOTICE "GlobHeaders - Searching: ${ROOT_PATH}")
			
	# Glob all header files in this directory. If the glob was
	# successful, add the globbed files to the dirlist (separated with a ";" delimiter).
	file(GLOB files "${ROOT_PATH}/*.hpp")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobHeaders - Added Files: ${files}")
	endif()
	
	file(GLOB files "${ROOT_PATH}/*.hh")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobHeaders - Added Files: ${files}")
	endif()
	
	file(GLOB files "${ROOT_PATH}/*.h")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobHeaders - Added Files: ${files}")
	endif()
	
	# Glob all files in the root and subdirectories.
	if(${GLOBTYPE} STREQUAL "GLOB_RECURSE")
	
		file(GLOB children RELATIVE ${ROOT_PATH} ${ROOT_PATH}/*)
		
		# Loop over all globbed child directories.
		foreach(child ${children})
		
			# Make sure this child directory exists.
			if(IS_DIRECTORY ${ROOT_PATH}/${child})
#				message(NOTICE "GlobHeaders - Searching: ${ROOT_PATH}/${child}")
				
				# Glob all header files in this child directory. If the glob was
				# successful, add the globbed files to the dirlist (separated with a ";" delimiter).
				file(GLOB_RECURSE files "${ROOT_PATH}/${child}/*.hpp")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobHeaders - Added Files: ${files}")
				endif()
				
				file(GLOB_RECURSE files "${ROOT_PATH}/${child}/*.hh")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobHeaders - Added Files: ${files}")
				endif()
				
				file(GLOB_RECURSE files "${ROOT_PATH}/${child}/*.h")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobHeaders - Added Files: ${files}")
				endif()
			endif()

		endforeach()

	endif()
	
	# Fixup the list for use with the likes of "target_include_directories".
	if(NOT "${dirlist}" STREQUAL "")
#		Glob_Internal_Stringify_CR(dirlist ${dirlist})
	endif()
	
	# This fails without PARENT_SCOPE!
	set(${RESULT} ${dirlist} PARENT_SCOPE)
	
#	message(NOTICE "Output List: ${dirlist}")

endfunction()


# Glob all resource files in child directories of the input path into a list.
function(GlobResources GLOBTYPE RESULT ROOT_PATH)

#	message(NOTICE "GlobResources!")
#	message(NOTICE "Input: ${ROOT_PATH}")
	
	file(GLOB children RELATIVE ${ROOT_PATH} ${ROOT_PATH}/*)
	
	set(dirlist "")
	
	# Glob all header files in this child directory. If the glob was
	# successful, add the globbed files to the dirlist (separated with a ";" delimiter).
	file(${GLOBTYPE} files "${ROOT_PATH}/*.h")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobResources - Added Files: ${files}")
	endif()
	
	file(${GLOBTYPE} files "${ROOT_PATH}/*.hm")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobResources - Added Files: ${files}")
	endif()
	
	file(${GLOBTYPE} files "${ROOT_PATH}/*.ico")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobResources - Added Files: ${files}")
	endif()
	
	file(${GLOBTYPE} files "${ROOT_PATH}/*.cur")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobResources - Added Files: ${files}")
	endif()
	
	file(${GLOBTYPE} files "${ROOT_PATH}/*.rc")
	if(NOT "${files}" STREQUAL "")
		list(APPEND dirlist ${files})
#		message(NOTICE "GlobResources - Added Files: ${files}")
	endif()
	
	# Glob all files in the root and subdirectories.
	if(${GLOBTYPE} STREQUAL "GLOB_RECURSE")
	
		# Loop over all globbed child directories.
		foreach(child ${children})
		
			# Make sure this child directory exists.
			if(IS_DIRECTORY ${ROOT_PATH}/${child})
	#			message(NOTICE "GlobResources - Searching: ${ROOT_PATH}/${child}")
				
				# Glob all header files in this child directory. If the glob was
				# successful, add the globbed files to the dirlist (separated with a ";" delimiter).
				file(${GLOBTYPE} files "${ROOT_PATH}/${child}/*.h")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobResources - Added Files: ${files}")
				endif()
				
				file(${GLOBTYPE} files "${ROOT_PATH}/${child}/*.hm")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobResources - Added Files: ${files}")
				endif()
				
				file(${GLOBTYPE} files "${ROOT_PATH}/${child}/*.ico")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobResources - Added Files: ${files}")
				endif()
				
				file(${GLOBTYPE} files "${ROOT_PATH}/${child}/*.cur")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobResources - Added Files: ${files}")
				endif()
				
				file(${GLOBTYPE} files "${ROOT_PATH}/${child}/*.rc")
				if(NOT "${files}" STREQUAL "")
					list(APPEND dirlist ${files})
#					message(NOTICE "GlobResources - Added Files: ${files}")
				endif()
			endif()

		endforeach()
	
	endif()
	
	# Fixup the list for use with the likes of "target_include_directories".
	if(NOT "${dirlist}" STREQUAL "")
#		Glob_Internal_Stringify_CR(dirlist ${dirlist})
	endif()
	
	# This fails without PARENT_SCOPE!
	set(${RESULT} ${dirlist} PARENT_SCOPE)
	
#	message(NOTICE "Output List: ${dirlist}")

endfunction()
