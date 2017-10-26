cmake_minimum_required (VERSION 2.6)
project (O3D_COLLADA)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   message(STATUS "Setting build type to 'Debug' as none was specified.")
   set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
   # Set the possible values of build type for cmake-gui
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
     "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT MINGW AND ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /D_DEBUG /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_RELEASE        "/O2 /DNDEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER /DO3D_DISABLE_GPU_MEMORY_MANAGER /DO3D_DISABLE_SPU_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_DEBUG          "/D_DEBUG /Zi /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_DEBUGFULL      "/D_DEBUG /Zi /EHsc /MP /MDd")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO   "/O2 /Zi /D_DEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_RELEASE          "/O2 /DNDEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER /DO3D_DISABLE_GPU_MEMORY_MANAGER /DO3D_DISABLE_SPU_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_DEBUG            "/D_DEBUG /Zi /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_DEBUGFULL        "/D_DEBUG /Zi /EHsc /MP /MDd")
else()
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER -DO3D_DISABLE_GPU_MEMORY_MANAGER -DO3D_DISABLE_SPU_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_DEBUGFULL      "-O0 -ggdb -D_DEBUG")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_RELEASE          "-O2 -DNDEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER -DO3D_DISABLE_GPU_MEMORY_MANAGER -DO3D_DISABLE_SPU_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_DEBUG            "-O0 -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_DEBUGFULL        "-O0 -ggdb -D_DEBUG")
endif()

#----------------------------------------------------------
# externals libraries
#----------------------------------------------------------

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(FindOpenGL)
include(FindOpenAL)

find_package(COLLADA_DOM COMPONENTS 1.4 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COLLADA_DOM_CXX_FLAGS}")
set(COLLADA_INCLUDE_DIRS ${COLLADA_DOM_INCLUDE_DIRS})
set(COLLADA_LIBRARIES ${COLLADA_DOM_LIBRARIES})

# or uses
# set(COLLADADOM_15 0)
# include(FindCOLLADA)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOLLADA_DOM_SUPPORT141")

# or uses
# include(FindPkgConfig)
# pkg_check_modules(collada-dom collada-dom)

# set(COLLADA_CFLAGS "-DCOLLADA_DOM_USING_141")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COLLADA_CFLAGS}")
# set(COLLADA_NAMESPACE ON)
# set(COLLADA_DEFINITIONS "-DCOLLADA_DOM_NAMESPACE -DCOLLADA_DOM_DAEFLOAT_IS64")
# set(COLLADA_NAMESPACE_141 ON)
# set(COLLADA_NAMESPACE_150 OFF)
# set(COLLADA_DEFINITIONS "${COLLADA_DEFINITIONS} -DCOLLADA_DOM_SUPPORT141")

#include(Bullet)

# objective3d Config.h
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if(MINGW)
		set(LIB_PATH $ENV{CROSSROAD_PREFIX})
	else()
		set(LIB_PATH $ENV{LIB_PATH})
	endif()
else()
	set(LIB_PATH $ENV{PREFIX})
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	include_directories(${LIB_PATH}/lib/objective3d-dbg)
elseif(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
	include_directories(${LIB_PATH}/lib/objective3d-odbg)
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
	include_directories(${LIB_PATH}/lib/objective3d)
endif()

# Windows MSVC
if(NOT MINGW AND ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set (O3D_PATH $ENV{O3D_PATH})

	set(O3D_INCLUDES "${O3D_PATH}/include")
	set(O3D_LIBRARIES_DIR "${O3D_PATH}/dist/lib/${CMAKE_BUILD_TYPE}")
	
	include_directories(${O3D_INCLUDES})
	include_directories(${O3D_LIBRARIES_DIR}/objective3d)
	link_directories(${O3D_LIBRARIES_DIR})
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	include(FindX11)
	# set(COLLADA_INCLUDE_DIRS 
        #"/usr/local/include/collada-dom2.4"
	#	"/usr/include/collada-dom2.4"
        # "/usr/local/include/collada-dom2.4/1.4")
	#	"/usr/include/collada-dom2.4/1.4")
	# set(COLLADA_LIBRARY_DIRS "/usr/local/lib")
	# set(COLLADA_LIBRARIES "collada14dom")
	# set(COLLADA_LIBRARIES "collada-dom2.4-dp")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include(FindX11)
	set(COLLADA_INCLUDE_DIRS 
		"/usr/local/include/colladadom"
		"/usr/local/include/colladadom/1.4")
	# set(COLLADA_LIBRARY_DIRS "/usr/local/lib")
	# set(COLLADA_LIBRARIES "collada14dom")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if (MINGW) # TODO
		set(COLLADA_INCLUDE_DIRS 
			"/usr/local/include/colladadom"
			"/usr/local/include/colladadom/1.4")
	else()
		set(COLLADA_INCLUDE_DIRS 
			"D:/lib/colladadom/include/colladadom"
			"D:/lib/colladadom/include/colladadom/1.4")
		set(COLLADA_LIBRARY_DIRS "D:/lib/colladadom/lib")
		set(O3D_INCLUDES "D:/lib/o3d/api/include")
		set(O3D_LIBRARIES_DIR "D:/lib/o3d/api/dist/RelWithDebInfo/bin")
		include_directories(${O3D_INCLUDES})
		link_directories(${O3D_LIBRARIES_DIR})
	endif()
	set(COLLADA_LIBRARIES "collada14dom")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(LIBRARY_OUTPUT_PATH ${CMAKE_BUILD_TYPE}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BUILD_TYPE}/bin)

include_directories(
		"${PROJECT_SOURCE_DIR}/include"
		${COLLADA_INCLUDE_DIRS})

link_directories(${COLLADA_LIBRARY_DIRS})

#----------------------------------------------------------
# options
#----------------------------------------------------------

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter -std=c++14 -fexceptions -Wextra")
	if(O3D_USE_SSE2)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
	endif(O3D_USE_SSE2)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter -std=c++14 -fexceptions -Wextra")
	if (O3D_USE_SSE2)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
	endif(O3D_USE_SSE2)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if(MINGW)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter -std=c++11 -fexceptions -Wextra")
		if (O3D_USE_SSE2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
		endif(O3D_USE_SSE2)
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DO3D_IMPORT_DLL")
		if (O3D_USE_SSE2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
		endif(O3D_USE_SSE2)
	endif()
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if (O3D_USE_SSE2)
	set(O3D_USE_SSE2 "")
endif (O3D_USE_SSE2)

#----------------------------------------------------------
# resources
#----------------------------------------------------------

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    file(COPY ${LIB_PATH}/share/o3d/shaders.zip DESTINATION ${CMAKE_BINARY_DIR})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	file(COPY ${LIB_PATH}/share/o3d/shaders.zip DESTINATION ${CMAKE_BINARY_DIR})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    file(COPY ${LIB_PATH}/share/o3d/shaders.zip DESTINATION ${CMAKE_BINARY_DIR})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#----------------------------------------------------------
# targets
#----------------------------------------------------------

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(O3D_COLLADA_LIB_NAME o3dcollada-dbg)
	set(O3D_COLLADA_TEST_NAME testcollada1-dbg)
elseif (${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
	set(O3D_COLLADA_LIB_NAME o3dcollada-odbg)
	set(O3D_COLLADA_TEST_NAME testcollada1-odbg)
elseif (${CMAKE_BUILD_TYPE} MATCHES "Release")
	set(O3D_COLLADA_LIB_NAME o3dcollada)
	set(O3D_COLLADA_TEST_NAME testcollada1)
endif()

add_library(${O3D_COLLADA_LIB_NAME} STATIC
		src/animation.cpp
	    src/camera.cpp
	    src/collada.cpp
	    src/controller.cpp
	    src/geometry.cpp
	    src/global.cpp
	    src/light.cpp
	    src/material.cpp
	    src/node.cpp)

add_executable(${O3D_COLLADA_TEST_NAME} test/main.cpp)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
		target_link_libraries(${O3D_COLLADA_TEST_NAME} objective3d-dbg o3dcollada-dbg ${COLLADA_LIBRARIES} boost_filesystem boost_system ${OPENGL_gl_LIBRARY})
	elseif(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
		target_link_libraries(${O3D_COLLADA_TEST_NAME} objective3d-odbg o3dcollada-odbg ${COLLADA_LIBRARIES} boost_filesystem boost_system ${OPENGL_gl_LIBRARY})
	elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
		target_link_libraries(${O3D_COLLADA_TEST_NAME} objective3d o3dcollada ${COLLADA_LIBRARIES} boost_filesystem boost_system ${OPENGL_gl_LIBRARY})
	endif()
ELSE()
	if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
		target_link_libraries(${O3D_COLLADA_TEST_NAME} objective3d-dbg o3dcollada-dbg ${COLLADA_LIBRARIES} boost_filesystem boost_system ${OPENGL_gl_LIBRARY})
	elseif(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
		target_link_libraries(${O3D_COLLADA_TEST_NAME} objective3d-odbg o3dcollada-odbg ${COLLADA_LIBRARIES} boost_filesystem boost_system ${OPENGL_gl_LIBRARY})
	elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
		target_link_libraries(${O3D_COLLADA_TEST_NAME} objective3d o3dcollada ${COLLADA_LIBRARIES} boost_filesystem boost_system ${OPENGL_gl_LIBRARY})
	endif()
ENDIF()

#----------------------------------------------------------
# install
#----------------------------------------------------------

file(GLOB COLLADA_HXX include/o3d/collada/*.h)

install (FILES ${COLLADA_HXX} DESTINATION include/o3d/collada)
install (TARGETS ${O3D_COLLADA_LIB_NAME} DESTINATION lib)
