cmake_minimum_required (VERSION 3.16)
project (OXYGINE)

#OPTIONS
set(OX_HAVE_LIBJPEG 0)
set(OX_HAVE_LIBPNG 0)
set(OX_HAVE_LIBCURL 0)
set(OX_HAVE_HTTP 0)
set(OX_USE_SDL2 1)
option(OX_FORCE_HPC "Force half pixel correction as the default in ResAtlasPrebuilt" OFF)

#PACKAGES
find_package(OpenGL)
find_package(CURL)

#VARS
set(OXYGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/oxygine)
set(OXYGINE_SRC ${OXYGINE_ROOT}/src/oxygine)

set(FOLDERS oxygine oxygine/closure oxygine/minizip oxygine/core
			oxygine/core/gl oxygine/dev_tools oxygine/minizip
			oxygine/math oxygine/actor oxygine/tween oxygine/pugixml oxygine/json oxygine/res
			oxygine/text_utils oxygine/utils oxygine/winnie_alloc)


if (EMSCRIPTEN)
	set(PLATFORM emscripten)
	set(SOURCES ${OXYGINE_SRC}/core/emscripten/HttpRequestEmscriptenTask.cpp)
else()
	set(FOLDERS ${FOLDERS} oxygine/core/curl)
	if (${CMAKE_SYSTEM_NAME} MATCHES "Linux") 	
		set(PLATFORM linux)
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(PLATFORM ios)
	elseif(MSVC)
		set(PLATFORM win32)
	elseif(MINGW)	
		set(PLATFORM win32_mingw)
	endif()
endif()

set(THIRD_PARTY ${OXYGINE_ROOT}/third_party/${PLATFORM})

foreach(ITEM ${FOLDERS})
	file(GLOB FLS
		${OXYGINE_ROOT}/src/${ITEM}/*.cpp
		${OXYGINE_ROOT}/src/${ITEM}/*.c
		${OXYGINE_ROOT}/src/${ITEM}/*.h)
	set(SOURCES ${SOURCES} ${FLS})	
	string(REPLACE / \\ SGROUP ${ITEM})
	source_group(${SGROUP} FILES ${FLS})
endforeach(ITEM)

add_library(oxygine-framework STATIC ${SOURCES})

target_link_libraries(oxygine-framework PUBLIC glm)

target_include_directories(oxygine-framework PRIVATE ${OXYGINE_SRC})
target_include_directories(oxygine-framework PUBLIC ${OXYGINE_ROOT}/src)
if (EMSCRIPTEN)
	#don't need SDL2
elseif (WIN32)	
	#hardcoded path to SDL2 on windows
	target_include_directories(oxygine-framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../SDL/include)
else(WIN32)
	find_path(SDL2_INCLUDE_DIRS NAMES SDL2/SDL.h)

	if (SDL2_INCLUDE_DIRS)		
		target_include_directories(oxygine-framework PUBLIC ${SDL2_INCLUDE_DIRS}/SDL2)
	else()
		target_include_directories(oxygine-framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../SDL/include)
	endif()
endif(EMSCRIPTEN)

if (EMSCRIPTEN)
	set(OX_HAVE_LIBPNG 1)
	set(OX_HAVE_LIBJPEG 1)
	set(OX_HAVE_HTTP 1)
	set(OX_USE_SDL2 1)

	file(GLOB OXYGINE_JS_LIBRARIES ${OXYGINE_SRC}/core/emscripten/*.js)

	# The emscripten port settings this library needs. A port setting on the
	# compile line makes emcc install that port's headers into the sysroot, so
	# every port whose headers oxygine sources include (SDL.h, png.h, jpeglib.h)
	# has to be requested while compiling; otherwise a fresh sysroot fails the
	# compile before anything links. The link line keeps its own copy so linking
	# never depends on a sysroot some earlier compile happened to populate.
	# USE_WEBGL2 only selects the link-time GL binding, so it stays link-only.
	target_compile_options(oxygine-framework PUBLIC -sUSE_SDL=2)
	target_link_options(oxygine-framework PUBLIC -sUSE_SDL=2)
	target_compile_options(oxygine-framework PUBLIC -sUSE_LIBPNG=1)
	target_link_options(oxygine-framework PUBLIC -sUSE_LIBPNG=1)
	target_compile_options(oxygine-framework PUBLIC -sUSE_LIBJPEG=1)
	target_link_options(oxygine-framework PUBLIC -sUSE_LIBJPEG=1)
	target_link_options(oxygine-framework PUBLIC -sUSE_WEBGL2=1)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux") 		
	set(OX_HAVE_LIBJPEG 1)
	set(OX_HAVE_LIBPNG 1)
	if (CURL_FOUND)
		target_include_directories(oxygine-framework PUBLIC ${CURL_INCLUDE_DIRS})
		target_link_libraries(oxygine-framework PUBLIC ${CURL_LIBRARIES})
	endif()
	target_link_libraries(oxygine-framework PUBLIC ${OPENGL_LIBRARIES})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
elseif(MSVC)
	set(OX_HAVE_LIBJPEG 1)
	set(OX_HAVE_LIBPNG 1)
	set(OX_HAVE_LIBCURL 1)
	set(OX_HAVE_HTTP 1)

	set(libprefix lib)

	set(OX_DEFINITIONS	${OX_DEFINITIONS} -D_CRT_SECURE_NO_WARNINGS)
elseif(MINGW)	
	set(libprefix lib)

	set(OX_HAVE_LIBPNG 1)
	set(OX_HAVE_LIBCURL 1)
	set(OX_HAVE_HTTP 1)

	target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/curl/)
endif()

if (OX_HAVE_LIBCURL)	
	set(OX_DEFINITIONS ${OX_DEFINITIONS} -DOX_HAVE_LIBCURL)
endif(OX_HAVE_LIBCURL)



# NDEBUG is read by the framework itself (oxygine-include.h, oxygine.cpp) and
# propagates to consumers via the PUBLIC target_compile_definitions below.
set(OX_DEFINITIONS ${OX_DEFINITIONS} -DNDEBUG)

# FORCE_HPC sets the half pixel correction default in ResAtlasPrebuilt.h.
if (OX_FORCE_HPC)
	set(OX_DEFINITIONS ${OX_DEFINITIONS} -DFORCE_HPC)
endif(OX_FORCE_HPC)

if (NOT OX_HAVE_HTTP)
	set(OX_DEFINITIONS ${OX_DEFINITIONS} -DOX_NO_HTTP)
endif(NOT OX_HAVE_HTTP)

if (EMSCRIPTEN)
	set(OX_DEFINITIONS ${OX_DEFINITIONS} -DOX_NO_MT)
endif(EMSCRIPTEN)



if (MINGW)
	target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/zlib)
else()
	target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/pthreads/include/ ${THIRD_PARTY}/zlib)
endif()		

if (FORCE_GLES)
	target_link_libraries(oxygine-framework PUBLIC libGLESv2.lib)
endif(FORCE_GLES)

if (OX_USE_SDL2)
	target_link_libraries(oxygine-framework PUBLIC SDL2)
	if (NOT EMSCRIPTEN) 
		target_link_libraries(oxygine-framework PUBLIC SDL2main)
	endif()
endif(OX_USE_SDL2)	


if (MINGW) 
	target_link_libraries(oxygine-framework PUBLIC mingw32 pthread psapi libcurl_imp ws2_32)

elseif (WIN32)
	target_link_libraries(oxygine-framework PUBLIC 
		pthreadVCE2
		libcurl_imp
		ws2_32)

elseif(EMSCRIPTEN)
else()
	target_link_libraries(oxygine-framework PUBLIC pthread)
endif()

if (OX_HAVE_LIBPNG)
	set(OX_DEFINITIONS	${OX_DEFINITIONS} -DOX_HAVE_LIBPNG)

	if (NOT EMSCRIPTEN)
		target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/libpng)		
		target_link_libraries(oxygine-framework PUBLIC png)
	endif(NOT EMSCRIPTEN)
endif(OX_HAVE_LIBPNG)


if (OX_HAVE_LIBJPEG)
	set(OX_DEFINITIONS	${OX_DEFINITIONS} -DOX_HAVE_LIBJPEG)
	if (NOT EMSCRIPTEN)
		target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/libjpeg)		
		target_link_libraries(oxygine-framework PUBLIC jpeg)
	endif(NOT EMSCRIPTEN)
endif(OX_HAVE_LIBJPEG)


if (NOT EMSCRIPTEN)
	target_link_libraries(oxygine-framework PUBLIC z)
endif(NOT EMSCRIPTEN)


if (NOT MSVC)
	# A usage requirement, so consumers inherit the same dialect; extensions off
	# keeps the emitted flag -std=c++11 rather than -std=gnu++11.
	target_compile_features(oxygine-framework PUBLIC cxx_std_11)
	set_target_properties(oxygine-framework PROPERTIES CXX_EXTENSIONS OFF)
endif(NOT MSVC)

target_compile_definitions(oxygine-framework PUBLIC ${OX_DEFINITIONS})

set(OXYGINE_JS_LIBRARIES
	${OXYGINE_JS_LIBRARIES}
	PARENT_SCOPE)

