#*******************************************************************************
#*                 O P E N  S O U R C E  --  V I N I F E R A                  **
#*******************************************************************************
#*
#*  @project       Vinifera
#*
#*  @file          CMAKELISTS.TXT
#*
#*  @author        CCHyper
#*
#*  @brief         Main CMake configuration for the project.
#*
#*  @license       Vinifera is free software: you can redistribute it and/or
#*                 modify it under the terms of the GNU General Public License
#*                 as published by the Free Software Foundation, either version
#*                 3 of the License, or (at your option) any later version.
#*
#*                 Vinifera is distributed in the hope that it will be
#*                 useful, but WITHOUT ANY WARRANTY; without even the implied
#*                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#*                 PURPOSE. See the GNU General Public License for more details.
#*
#*                 You should have received a copy of the GNU General Public
#*                 License along with this program.
#*                 If not, see <http://www.gnu.org/licenses/>.
#*
#******************************************************************************/
 
 # Visual Studio 2017 requires at least 3.7.
cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(CMAKE_SUPPRESS_REGENERATION true) # Remove ZERO_CHECK project.
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Organize projects into folders within the VS solution.

# Prevent incremental linking.
# This line has to go before "project"!
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND MSVC)
	set(MSVC_INCREMENTAL_DEFAULT OFF)
endif()

# Set available project configurations.
SET(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo" CACHE STRING "Configs" FORCE)

# Declare the project.
project(Vinifera VERSION 1.00 LANGUAGES CXX)

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) #policy for <PackageName>_ROOT variables.
endif()

# Check supported generators
if(NOT MSVC)
    message(FATAL_ERROR "This project can only be built using Visual Studio!")
endif()

if(MSVC AND MSVC_VERSION LESS 1910)
    message(FATAL_ERROR "This version of Visual Studio is not supported: ${CMAKE_GENERATOR}.")
endif()

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    message(FATAL_ERROR "This project can only be built as Win32!")
endif()


################################################################################
# Set a default build type if none was specified.
################################################################################

# Default to a Release build.
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug RelWithDebInfo." FORCE)
    message(STATUS "Setting build type default to RelWithDebInfo")
endif()


################################################################################
# Configure C++ standard
################################################################################

# Require that we have at least C++14.
# We can not use C++ 17 due to a issue with MSVC and COM
# See: https://developercommunity.visualstudio.com/content/problem/297914/c-intellisense-not-working-for-com-ptr-t-with-c17.html
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#enable_language(C)
enable_language(CXX)


################################################################################
# Set custom module path.
################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_NO_SYSTEM_FROM_IMPORTED TRUE) # Needed to prevent FindDirectX screwing up a MinGW build.


################################################################################
# Hack to get around the "Debug", "Release", etc directories CMake tries to add on Windows.
################################################################################

set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set output directory for LIB files.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

# Set output directory for DLL files.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

# Set output directory for executable files.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})


################################################################################
# We don't support in tree builds, so help people make the right choice.
################################################################################
if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build directory and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()


################################################################################
# Available CMake options.
################################################################################

# Option for producing Release builds.
# DO NOT USE: This is for the CI build system only.
option(OPTION_TS_CLIENT_BUILD "Produce configurations for building a TS Client compatibility build." OFF)
option(OPTION_RELEASE_BUILD "Produce configurations for building a release build." OFF)

# These are for debugging purposes only.
option(OPTION_CUSTOM_DEBUGGER_DIRECTORY "Define custom debugger working path and target to copy binaries to on build." OFF)
option(OPTION_CUSTOM_DEBUGGER_TARGET "Define custom debugger target binary name (only use with modified EXE which loads the DLL via an import)." OFF)

# Both of these options must be enabled to setup the post-build script and debugger target.
if(${OPTION_CUSTOM_DEBUGGER_DIRECTORY} AND ${OPTION_CUSTOM_DEBUGGER_TARGET})
	set(OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH " " CACHE STRING "")
	set(OPTION_CUSTOM_DEBUGGER_TARGET_NAME " " CACHE STRING "")
	
	if(${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH} STREQUAL " " OR ${OPTION_CUSTOM_DEBUGGER_TARGET_NAME})
		message(NOTICE "Warning: Please set the custom debugging path and target name for Tiberian Sun!")
	endif()
endif()

if(NOT ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH} STREQUAL " ")
	message(STATUS " - Custom debugging path: ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}")
endif()

if(NOT ${OPTION_CUSTOM_DEBUGGER_TARGET_NAME} STREQUAL " ")
	message(STATUS " - Custom debugging target: ${OPTION_CUSTOM_DEBUGGER_TARGET_NAME}")
endif()

option(OPTION_CHECK_TARGET_BINARIES "This option controls if the launcher should check target binaries before injection." ON)


################################################################################
# Some general Windows definitions to keep the MSVC compiler happy with our code.
################################################################################
if(WIN32)
	add_compile_definitions(__STDC_FORMAT_MACROS)
	add_compile_definitions(__STDC_LIMIT_MACROS)
	add_compile_definitions(_CRT_SECURE_NO_DEPRECATE)
	add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)
	add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
	add_compile_definitions(_USE_32BIT_TIME_T) # This is for ABI compatibility with a few functions.
	add_compile_definitions(WIN32_LEAN_AND_MEAN) # Go lean and mean on windows.
	add_compile_definitions(NOMINMAX) # Windows min/max clashes with std::min/max
endif()


################################################################################
# Disable warnings.
################################################################################
# Set warning level 4
add_compile_options(/W4)

# For catching unreferenced local variables, we do not need to worry about this when compiling debug builds.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4189")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4101")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4700") # uninitialized local variable 'x' used.

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244") # disable C4244: conversion from 'double' to 'float', possible loss of data
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4800") # disable C4800: 'BOOL' : forcing value to bool 'true' or 'false' (performance warning)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4351") # disable C4351: "new behavior: elements of array will be default initialized"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4514") # disable C4514: "unreferenced inline function has been removed" Yea, so what?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4065") # disable C4065: "switch statement contains 'default' but no 'case' labels"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4514") # disable C4514: "unreferenced inline function has been removed" Yea, so what?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4065") # disable C4065: "switch statement contains 'default' but no 'case' labels"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244") # disable C4244: "conversion from 'double' to 'float', possible loss of data" Yea, so what?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4530") # disable C4530: Disable warning about exception handling not being enabled. It's used as part of STL - in a part of STL we don't use.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4056") # disable C4056: "overflow in floating-point constant arithmetic" This warning occurs even if the loss of precision is insignificant.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4710") # disable C4710: "function not inlined" This warning is typically useless. The inline keyword only serves as a suggestion to the compiler and it may or may not inline a function on a case by case basis. No need to be told of this.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4355") # disable C4355: "'this' used in base member initializer list" Using "this" in a base member initializer is valid -- no need for this warning.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4097") # disable C4097: "typedef-name used as a synonym for class-name". This is by design and should not be a warning.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4091") # disable C4091: 'typedef ': ignored on left of '' when no variable is declared
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4505") # disable C4505: Unreferenced local function removed.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4511") # disable C4511: 'copy constructor could not be generated'
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4711") # disable C4711: 'function selected for automatic inline expansion'. Cool, but since we're treating warnings as errors, don't warn us about this!
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4512") # disable C4512: 'assignment operator could not be generated' 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4100") # disable C4100: 'unreferenced formal parameter'
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4786") # disable C4786: "identifier was truncated to '255' characters in the browser information" Templates create long identifiers...
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4800") # disable C4800: 'BOOL' : forcing value to bool 'true' or 'false' (performance warning)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996") # disable C4996: 'This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.'
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4324") # disable C4324: 'structure was padded due to alignment specifier.' Safe to ignore this.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4102") # disable C4102: 'unreferenced label.' Yea, so what?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4646") # disable C4646: 'function declared with 'noreturn' has non-void return type' Yea, so what?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4731") # disable C4731: 'frame pointer register 'ebp' modified by inline assembly code' Nothing to worry about.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4740") # disable C4740: 'flow in or out of inline asm code suppresses global optimization' Nothing to worry about.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4239") # disable C4239: 'nonstandard extension used: 'default argument': conversion from 'value' to 'value &'. Shoosh...

# disable D9035: 'option 'QIfist' has been deprecated and will be removed in a future release.' Yeah yeah, we need it for now though...
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd9035") # Not allowed to disable command line warnings.


################################################################################
# Set global configuration definitions.
################################################################################

# Define the string build type.
if(CMAKE_BUILD_TYPE)
	string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPERCASE)
	add_compile_definitions(BUILD_TYPE="${CMAKE_BUILD_TYPE_UPPERCASE}")
endif()


################################################################################
# Set some handy defines we can use for error messages etc.
################################################################################
set(PROJECT_DLL "${PROJECT_NAME}.dll")
set(UNSUPPORTED_TARGET_ERROR "(Only TS203EN, Freeware-TS, and TUC-TS (2024 release) is currently supported)")
set(TARGET_EXE "GAME.EXE") # Define the EXE to launch.
set(TARGET_PDB "GAME.PDB") # Define the PDB for the target.
set(TARGET_EDB "GAME.EDB") # Define the EDB (exception database) for the target.
set(TARGET_NAME "Tiberian Sun")

# Standard TS203EN 'GAME.EXE'.
set(TARGET_SIZE 3457296)
set(TARGET_ENTRY 0x006B7E21) # Entry point (start).
set(TARGET_HASH "E1B997B4A83B84B5481F773B0B001E3186D02990") # SHA hash

# "The First Decade" 'GAME.EXE'.
set(TARGET_TFD_SIZE 4677437)
set(TARGET_TFD_ENTRY 0x006B7E21) # Entry point (start).
set(TARGET_TFD_HASH "2309D215AC10A0F331009A02EEBFAC4001755A99") # SHA hash

# "The Ultimate Collection (2012 release)" 'GAME.EXE'.
set(TARGET_OLD_TUC_SIZE 3465936)
set(TARGET_OLD_TUC_ENTRY 0x0086D830) # Origin activation changes the entry point (start).
set(TARGET_OLD_TUC_HASH "BEE10E9A55614D134D513909A44BBCC2830B607F") # SHA hash

# "The Ultimate Collection (2024 release)" 'GAME.EXE'.
set(TARGET_TUC_SIZE 3850240)
set(TARGET_TUC_ENTRY 0x006B7E21) # Entry point (start).
set(TARGET_TUC_HASH "AC408535113C221E26C7CF2AAF69AF06FB447CD2") # SHA hash

# "Freeware TS" 'GAME.EXE'.
set(TARGET_FTS_SIZE 3457296)
set(TARGET_FTS_ENTRY 0x006B7E21) # Entry point (start).
set(TARGET_FTS_HASH "846F0077129CB390BB206431A63E9246E3D8DA72") # SHA hash

# Debug option for the launcher to skip target checksum.
if (OPTION_CHECK_TARGET_BINARIES)
set(CHECK_TARGET_BINARIES "true")
else()
set(CHECK_TARGET_BINARIES "false")
endif()

# Compatibility mode overrides this.
if(OPTION_TS_CLIENT_BUILD)
set(CHECK_TARGET_BINARIES "false")
endif()


################################################################################
# Glob all the headers, source files and include directories.
################################################################################
include(GlobUtil)

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

message(STATUS "Pulling in Vinifera sources.")
GlobDirectories(GLOB_RECURSE VINIFERA_INCLUDE_DIRS ${PROJECT_SOURCE_DIR})
GlobHeaders(GLOB_RECURSE VINIFERA_HEADERS ${PROJECT_SOURCE_DIR})
GlobSources(GLOB_RECURSE VINIFERA_SOURCES ${PROJECT_SOURCE_DIR})

message(STATUS "Pulling in Hooker sources.")
set(HOOKER_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/hooker")
GlobHeaders(GLOB HOOKER_HEADERS ${HOOKER_INCLUDE_DIR})
GlobSources(GLOB HOOKER_SOURCES ${HOOKER_INCLUDE_DIR})


################################################################################
# Include the TS++ submodule in the solution.
################################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/TSpp)


################################################################################
# Embed some git version information in the binary.
################################################################################
message(STATUS "Fetching git repository information.")

# Locate git binary to provide information to the build environment.
# find_package(Git) # Removed, GitWatcher does this quietly.

# The following two defines are automatically picked up by GitWatcher.
set(GIT_PRE_CONFIGURE_FILE "${CMAKE_SOURCE_DIR}/src/vinifera/vinifera_gitinfo.h.in")
set(GIT_POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/vinifera_gitinfo.h")
set(GIT_STATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/vinifera-git-state-hash")
set(GIT_WORKING_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include(GitWatcher)
GitWatcher_SetupInitial()  # Running the script function again forces the file to be
					       # created locally so the file can be added to the filters.


################################################################################
# Build the launcher.
################################################################################
set(LAUNCHER_NAME Launch${PROJECT_NAME})

message(STATUS "Configuring launcher as ${LAUNCHER_NAME} using entry at ${TARGET_ENTRY} to inject ${PROJECT_DLL} into ${TARGET_EXE}")

configure_file(${PROJECT_SOURCE_DIR}/hooker/launcher.cpp.in
		${CMAKE_CURRENT_BINARY_DIR}/generated/launcher.cpp @ONLY
)

add_executable(${LAUNCHER_NAME} WIN32
		${CMAKE_CURRENT_BINARY_DIR}/generated/launcher.cpp # Configured launcher source file.
		${PROJECT_SOURCE_DIR}/core/sha.h
		${PROJECT_SOURCE_DIR}/core/sha.cpp
		${CMAKE_SOURCE_DIR}/res/resource.h
		${CMAKE_SOURCE_DIR}/res/icon.rc
		${CMAKE_SOURCE_DIR}/res/vinifera.ico
)

target_include_directories(${LAUNCHER_NAME} PUBLIC
		${VINIFERA_INCLUDE_DIRS}
		${CMAKE_SOURCE_DIR}/res/
)
	
# Filter the configured git version source file manually.
include(ProjectFilters)

# Filter the configured launcher config file manually.
SetCustomProjectFilters("Generated Files" # "Source Files"
		${CMAKE_CURRENT_BINARY_DIR}/launcher.cpp
)

SetCustomProjectFilters("Resource Files"
		${CMAKE_SOURCE_DIR}/res/resource.h
		${CMAKE_SOURCE_DIR}/res/icon.rc
		${CMAKE_SOURCE_DIR}/res/vinifera.ico
)

# Set custom debugging target name (only works if the custom path option has been set!).
if(NOT ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH} STREQUAL " " AND
	NOT ${OPTION_CUSTOM_DEBUGGER_TARGET_NAME} STREQUAL " ")
	set_target_properties(${LAUNCHER_NAME} PROPERTIES
			VS_DEBUGGER_COMMAND "${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}/${LAUNCHER_NAME}.exe")
#		message(STATUS "Custom VS_DEBUGGER_COMMAND set to '${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}/${LAUNCHER_NAME}.exe'")
endif()

# Set the custom debugging target directory.
if(NOT ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH} STREQUAL " ")
	set_target_properties(${LAUNCHER_NAME} PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}")
#		message(STATUS "Custom VS_DEBUGGER_WORKING_DIRECTORY set to '${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}'")
endif()

# Create post build script to copy the DLL to the custom debugger directory.
if(NOT ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH} STREQUAL " ")

	# Set input and output paths.
	set(COPY_SOURCE_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/)
	set(COPY_DESTINATION_PATH ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH})

	# Create the post build script.
	include(WindowsCopyFiles)
	PostBuild_WindowsCopyFiles(${LAUNCHER_NAME}
		${COPY_SOURCE_FILE}
		${COPY_DESTINATION_PATH}
		"${LAUNCHER_NAME}.exe"
	)
endif()


################################################################################
# Build the DLL.
################################################################################
message(STATUS "Configuring ${PROJECT_NAME} DLL.")

add_library(${PROJECT_NAME} SHARED
		${VINIFERA_HEADERS} 
		${VINIFERA_SOURCES}
		${HOOKER_HEADERS} ${HOOKER_SOURCES}
		${VINIFERA_RESOURCES}
		${GIT_POST_CONFIGURE_FILE}
		${CMAKE_SOURCE_DIR}/res/resource.h
		${CMAKE_SOURCE_DIR}/res/dialog.rc
		${CMAKE_SOURCE_DIR}/res/icon.rc
		${CMAKE_SOURCE_DIR}/res/version.rc
)
target_include_directories(${PROJECT_NAME} BEFORE PUBLIC
		${VINIFERA_INCLUDE_DIRS}
		${PROJECT_SOURCE_DIR}/hooker/
		${CMAKE_SOURCE_DIR}/res
		${CMAKE_CURRENT_BINARY_DIR}/generated
)

# Set custom debugging target name (only works if the custom path option has been set!).
if(NOT ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH} STREQUAL " " AND
	NOT ${OPTION_CUSTOM_DEBUGGER_TARGET_NAME} STREQUAL " ")
	set_target_properties(${PROJECT_NAME} PROPERTIES
			VS_DEBUGGER_COMMAND "${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}/${OPTION_CUSTOM_DEBUGGER_TARGET_NAME}")
#		message(STATUS "Custom VS_DEBUGGER_COMMAND set to '${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}/${OPTION_CUSTOM_DEBUGGER_TARGET_NAME}'")
endif()

# Set the custom debugging target directory.
if(NOT ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH} STREQUAL " ")
	set_target_properties(${PROJECT_NAME} PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}")
#		message(STATUS "Custom VS_DEBUGGER_WORKING_DIRECTORY set to '${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH}'")
endif()

# Set input and output paths.
set(PDB_COPY_SOURCE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/_deps/tspp-src/pdb/)
set(EDB_COPY_SOURCE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/_deps/tspp-src/edb/)
set(PDB_COPY_DESTINATION_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/)
set(EDB_COPY_DESTINATION_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/)

# Create post build script to copy the Target Fake PDB to the build output directory.
include(WindowsCopyFiles)
PostBuild_WindowsCopyFiles(${PROJECT_NAME}
	${PDB_COPY_SOURCE_PATH}
	${PDB_COPY_DESTINATION_PATH}
	"${TARGET_PDB}"
)

# Create post build script to copy the EDB (exception database) to the build output directory.
include(WindowsCopyFiles)
PostBuild_WindowsCopyFiles(${PROJECT_NAME}
	${EDB_COPY_SOURCE_PATH}
	${EDB_COPY_DESTINATION_PATH}
	"${TARGET_EDB}"
)

# Create post build script to copy the DLL to the custom debugger directory.
if(NOT ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH} STREQUAL " ")

	# Set input and output paths.
	set(COPY_SOURCE_FILE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/)
	set(COPY_DESTINATION_PATH ${OPTION_CUSTOM_DEBUGGER_DIRECTORY_PATH})

	# Create the post build script.
	include(WindowsCopyFiles)
	PostBuild_WindowsCopyFiles(${PROJECT_NAME}
		${COPY_SOURCE_FILE_PATH}
		${COPY_DESTINATION_PATH}
		"${PROJECT_NAME}.dll"
	)
	
	# Create the post build script.
	include(WindowsCopyFiles)
	PostBuild_WindowsCopyFiles(${PROJECT_NAME}
		${COPY_SOURCE_FILE_PATH}
		${COPY_DESTINATION_PATH}
		"${PROJECT_NAME}.pdb"
	)
	
	# Create the post build script.
	include(WindowsCopyFiles)
	PostBuild_WindowsCopyFiles(${PROJECT_NAME}
		${COPY_SOURCE_FILE_PATH}
		${COPY_DESTINATION_PATH}
		"${TARGET_PDB}"
	)
	
	# Create the post build script.
	include(WindowsCopyFiles)
	PostBuild_WindowsCopyFiles(${PROJECT_NAME}
		${COPY_SOURCE_FILE_PATH}
		${COPY_DESTINATION_PATH}
		"${TARGET_EDB}"
	)
endif()


# Set the custom project name.
set_target_properties("${PROJECT_NAME}" PROPERTIES PROJECT_LABEL "${PROJECT_NAME}")

# Set some handy defines we can use for error messages etc.
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_TARGET_NAME="${TARGET_NAME}")
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_UNSUPPORTED_TARGET_ERROR="${UNSUPPORTED_TARGET_ERROR}")
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_DLL="${PROJECT_DLL}")
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_TARGET_EXE="${TARGET_EXE}")
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_TARGET_EDB="${TARGET_EDB}")
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_TARGET_PDB="${TARGET_PDB}")
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_TARGET_SIZE=${TARGET_SIZE})
target_compile_definitions(${PROJECT_NAME} PUBLIC VINIFERA_TARGET_HASH="${TARGET_HASH}")

# Set the build type defines.
if(OPTION_TS_CLIENT_BUILD)
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RelWithDebInfo>:TS_CLIENT>")
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:TS_CLIENT>")
message(STATUS "Build: TS Client Compatibility")
endif()
if(RELEASE OR OPTION_RELEASE_BUILD)
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RelWithDebInfo>:RELEASE>")
message(STATUS "Build: Release")
elseif(NIGHTLY)
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RelWithDebInfo>:NIGHTLY>")
message(STATUS "Build: Nightly")
elseif(PREVIEW)
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RelWithDebInfo>:PREVIEW>")
message(STATUS "Build: Preview")
else()
message(STATUS "Build: Unofficial")
endif()


#
# Add/Remove flags required for trying matching pre-compiled game code.
#

# Disable the random address rebasing on module load.
target_link_options(${PROJECT_NAME} PUBLIC /DYNAMICBASE:NO)

# Disable Data Execution Prevention.
target_link_options(${PROJECT_NAME} PUBLIC /NXCOMPAT:NO)

# Prevent incremental linking.
target_link_options(${PROJECT_NAME} PUBLIC /INCREMENTAL:NO)

# disable SAFESEH to avoid linker issues with 3rd party libraries.
target_link_options(${PROJECT_NAME} PUBLIC /SAFESEH:NO)

# Disable Run Time Checking.
foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL
				 CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
	if(${flag_var} MATCHES "/RTC")
		string(REGEX REPLACE "/RTC[^ ]*" "" ${flag_var} "${${flag_var}}")
	endif()
endforeach(flag_var)

# Specifies no enhanced instructions and also specifies x87 for floating-point calculations.
target_compile_options(${PROJECT_NAME} PUBLIC /arch:IA32)

# Specify floating-point behaviour.
target_compile_options(${PROJECT_NAME} PUBLIC /fp:fast)

# Suppresses _ftol when a conversion from a floating-point type to an integer type is required (x86 only).
#target_compile_options(${PROJECT_NAME} PUBLIC /QIfist) # This is no longer supported by the MSVC version that GitHub uses.

# Force generates fast transcendentals.
target_compile_options(${PROJECT_NAME} PUBLIC /Qfast_transcendentals)

# Disable thread-safe local static initialization.
target_compile_options(${PROJECT_NAME} PUBLIC /Zc:threadSafeInit-)

# forces __fastcall as default calling convention.
target_compile_options(${PROJECT_NAME} PUBLIC /Gr)

# Disables buffer security overrun checks.
target_compile_options(${PROJECT_NAME} PUBLIC /GS-)

# Disables buffer security overrun checks.
target_compile_options(${PROJECT_NAME} PUBLIC /GS-)

# Align structures to 4-byte boundaries.
#		target_compile_options(${PROJECT_NAME} PUBLIC /Zp4)

# Replaces some function calls with intrinsic or otherwise special forms of the function.
#		target_compile_options(${PROJECT_NAME} PUBLIC /Oi)

# (Non-debug only) Optimisation. (1) Minimize Size, (2) Maximize Speed.
string(REPLACE "/O2" "/O1" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
string(REPLACE "/O2" "/O1" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "/O2" "/O1" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
string(REPLACE "/O2" "/O1" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string(REPLACE "/O2" "/O1" CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")
string(REPLACE "/O2" "/O1" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")

# (Non-debug only) Disable frame pointer omission (FPO).
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oy-")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oy-")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /Oy-")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Oy-")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /Oy-")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /Oy-")

# Inline function expansion.
target_compile_options(${PROJECT_NAME} PUBLIC
		$<$<CONFIG:Debug>:/Ob0>
		$<$<CONFIG:Release>:/Ob1>
		$<$<CONFIG:RelWithDebInfo>:/Ob1>
		$<$<CONFIG:MinSizeRel>:/Ob1>)

# Build with multiple processes.
target_compile_options(${PROJECT_NAME} PUBLIC /MP2)

# Link to the TS++ static library.
target_link_libraries(${PROJECT_NAME} TSpp)
add_dependencies(${PROJECT_NAME} TSpp)

# Ensure the launcher is built with the main project.
add_dependencies(${PROJECT_NAME} Launch${PROJECT_NAME})

set(REQUIRED_LIBRARIES ws2_32 dbghelp shlwapi version comctl32)

# Link to any required libraries.
target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBRARIES})

# Make build check state of git to check for uncommitted changes.
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_check_git)


################################################################################
# Setup project IDE filters.
################################################################################
message(STATUS "Setting up project file filters.")

include(ProjectFilters)
SetupProjectFilters("Vinifera" ${VINIFERA_HEADERS} ${VINIFERA_SOURCES} ${VINIFERA_RESOURCES})

SetCustomProjectFilters("Hooker Files"
	${HOOKER_HEADERS} ${HOOKER_SOURCES}
)

SetCustomProjectFilters("Resource Files"
		${CMAKE_SOURCE_DIR}/res/resource.h
		${CMAKE_SOURCE_DIR}/res/dialog.rc
)

# Filter the configured git version source file manually.
SetCustomProjectFilters("Generated Files" ${GIT_POST_CONFIGURE_FILE})

# Force the default startup project.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Vinifera)

