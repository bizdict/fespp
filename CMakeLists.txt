cmake_minimum_required(VERSION 3.1.0)

project(Fespp)

set (FESPP_ROOT_DIR ${CMAKE_SOURCE_DIR})
set (FESPP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# -----------------------------------------------------------------------------
# Disable deprecation warnings for standard C and STL functions in VS2005 and
# later (no, we don't need IF(CMAKE_COMPILER_2005) ... )
# -----------------------------------------------------------------------------
add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
add_definitions(-D_SCL_SECURE_NO_DEPRECATE)

IF (UNIX)
	if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.6 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.6)
		add_compile_options(-std=c++11)
	else ()
		add_compile_options(-std=gnu++0x)
	endif()
	add_compile_options(-fPIC)
ENDIF (UNIX)

IF (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	SET (CMAKE_INSTALL_PREFIX ${FESPP_BINARY_DIR}/install CACHE PATH "Fespp install prefix" FORCE)
ENDIF (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# ============================================================================
# checking for required dependencies
# ============================================================================

# -- Fesapi --
# path to custom find_package modules
SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# -- FESAPI --
SET(FESAPI_DIR "FESAPI_DIR-NOTFOUND" CACHE PATH "Fesapi install PATH")
find_package(Fesapi REQUIRED)


# -- ParaView --
IF (ParaView_SOURCE_DIR)
  # we are building from within ParaView Source. The environment is already set.
  # Nothing more to do.
ELSE (ParaView_SOURCE_DIR)
  find_package(ParaView REQUIRED)
  include("${PARAVIEW_USE_FILE}")
  include (ParaViewPlugins)

  # Load information about VTK/ParaView modules loaded/available.
  pv_setup_module_environment("Fespp")
ENDIF (ParaView_SOURCE_DIR)


# -----------------------------------------------------------------------------
# Process all vtk-modules defined in this plugin. This generates the necessary
# wrappings for all these modules as well.
# -----------------------------------------------------------------------------
pv_process_modules()

# ============================================================================
# compilation options
# ============================================================================

# Build with Multiple Processes on windows
IF (WIN32)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
ENDIF (WIN32)

# -----------------------------------------------------------------------------
# Build the Paraview plugins
# -----------------------------------------------------------------------------
add_subdirectory(src)
#
