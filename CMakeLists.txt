cmake_minimum_required (VERSION 3.16)

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
elseif (ANDROID)
	set(PLATFORM android)
	# Android compiles the folder set below. core/gl is not in it:
	# those six sources are named explicitly below. Android uses the Java HTTP
	# task, so core/curl must not be added to FOLDERS.
	set(FOLDERS oxygine/closure oxygine/core oxygine/math oxygine/actor
			oxygine/tween oxygine/pugixml oxygine/json oxygine/res
			oxygine/text_utils oxygine/dev_tools oxygine/utils
			oxygine/winnie_alloc oxygine/minizip oxygine)
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

if (ANDROID)
	# The JNI sources and the core/gl set, which FOLDERS does not cover.
	set(SOURCES ${SOURCES}
		${OXYGINE_SRC}/core/android/jniUtils.cpp
		${OXYGINE_SRC}/core/android/jniHelper.cpp
		${OXYGINE_SRC}/core/android/HttpRequestJavaTask.cpp
		${OXYGINE_SRC}/core/gl/NativeTextureGLES.cpp
		${OXYGINE_SRC}/core/gl/oxgl.cpp
		${OXYGINE_SRC}/core/gl/VertexDeclarationGL.cpp
		${OXYGINE_SRC}/core/gl/ShaderProgramGL.cpp
		${OXYGINE_SRC}/core/gl/VideoDriverGL.cpp
		${OXYGINE_SRC}/core/gl/VideoDriverGLES20.cpp)
endif()

add_library(oxygine-framework STATIC ${SOURCES})

if (ANDROID)
	set(OXYGINE_ANDROID_LIBS ${THIRD_PARTY}/libraries/${ANDROID_ABI})
	foreach(LIB png jpeg)
		if (NOT EXISTS ${OXYGINE_ANDROID_LIBS}/lib${LIB}.a)
			message(FATAL_ERROR
				"Missing prebuilt lib${LIB} for ${ANDROID_ABI}: ${OXYGINE_ANDROID_LIBS}/lib${LIB}.a")
		endif()
	endforeach()

	# png and jpeg are linked whole by the main module's ordered whole-archive
	# group (cmake/android.cmake), not here: a whole-archive pair declared in
	# this branch can only be emitted at this module's position in the link,
	# after the selective archives. Only the platform libraries stay here. The
	# headers still come from third_party/android/libpng and /libjpeg below; the
	# sources include png.h and jpeglib.h.
	target_link_libraries(oxygine-framework PUBLIC GLESv2 log)
endif()

target_link_libraries(oxygine-framework PUBLIC glm)

target_include_directories(oxygine-framework PRIVATE ${OXYGINE_SRC})
target_include_directories(oxygine-framework PUBLIC ${OXYGINE_ROOT}/src)
if (EMSCRIPTEN)
	# SDL headers come from emcc's -sUSE_SDL=2 port, declared below; this
	# branch deliberately adds no in-tree SDL include directory.
elseif (WIN32)	
	#hardcoded path to SDL2 on windows
	target_include_directories(oxygine-framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../SDL/include)
elseif (ANDROID)
	# Android builds against the in-tree SDL submodule, so this does not
	# depend on find_path locating a system SDL2.
	target_include_directories(oxygine-framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../SDL/include)
	# The framework headers include glm through '<module>/../glm', the spelling
	# clang records. The PUBLIC glm target below exports the collapsed absolute
	# path, so put this spelling first; it only affects this target's own
	# compile.
	target_include_directories(oxygine-framework BEFORE PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/../glm")
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
elseif (ANDROID)
	# The png/jpeg backends are used, and the 32-bit file API keeps minizip on
	# the same off_t as the rest of the build. The OX_HAVE_LIBPNG/LIBJPEG blocks
	# below add their own -D flags and the third_party/android include dirs; the
	# archives those headers belong to are linked by the main module, not here.
	set(OX_HAVE_LIBJPEG 1)
	set(OX_HAVE_LIBPNG 1)
	# OX_NO_HTTP must not be defined: the Java task supplies init() and
	# release(), which the null bodies compiled under OX_NO_HTTP would collide
	# with.
	set(OX_HAVE_HTTP 1)
	set(OX_DEFINITIONS	${OX_DEFINITIONS} -DUSE_FILE32API)
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
elseif (ANDROID)
	# third_party/android has no pthreads directory, so only zlib is added.
	target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/zlib)
else()
	target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/pthreads/include/ ${THIRD_PARTY}/zlib)
endif()		

if (FORCE_GLES)
	target_link_libraries(oxygine-framework PUBLIC libGLESv2.lib)
endif(FORCE_GLES)

if (OX_USE_SDL2)
	if (ANDROID)
		# android.mk's framework module declares no shared libraries; the main module
		# provides SDL2. Linking it here — PUBLIC or PRIVATE — still puts it on every
		# consumer's link line for a static library, so the framework must not name it.
		# The SDL include directories below stay PUBLIC, and OX_USE_SDL2 stays 1.
	else()
		target_link_libraries(oxygine-framework PUBLIC SDL2)
		if (NOT EMSCRIPTEN)
			target_link_libraries(oxygine-framework PUBLIC SDL2main)
		endif()
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
elseif(ANDROID)
	# Android supplies pthread through libc; there is nothing to link.
else()
	target_link_libraries(oxygine-framework PUBLIC pthread)
endif()

if (OX_HAVE_LIBPNG)
	set(OX_DEFINITIONS	${OX_DEFINITIONS} -DOX_HAVE_LIBPNG=1)

	if (NOT EMSCRIPTEN)
		# clang records the include directory text in the DWARF line table.
		# Android's image include directories carry a doubled slash, which CMake
		# collapses in an include directory, so they are INTERFACE for consumers
		# and a raw -I for this target's own compile, which CMake passes through
		# verbatim.
		if (ANDROID)
			target_include_directories(oxygine-framework INTERFACE ${THIRD_PARTY}//libpng)
			target_compile_options(oxygine-framework PRIVATE "-I${THIRD_PARTY}//libpng")
		else()
			target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/libpng)
		endif()
		# Android links the archive from the main module's whole-archive group;
		# see the ANDROID branch above. Only the headers come from here.
		if (NOT ANDROID)
			target_link_libraries(oxygine-framework PUBLIC png)
		endif(NOT ANDROID)
	endif(NOT EMSCRIPTEN)
endif(OX_HAVE_LIBPNG)


if (OX_HAVE_LIBJPEG)
	set(OX_DEFINITIONS	${OX_DEFINITIONS} -DOX_HAVE_LIBJPEG=1)
	if (NOT EMSCRIPTEN)
		if (ANDROID)
			target_include_directories(oxygine-framework INTERFACE ${THIRD_PARTY}//libjpeg)
			target_compile_options(oxygine-framework PRIVATE "-I${THIRD_PARTY}//libjpeg")
		else()
			target_include_directories(oxygine-framework PUBLIC ${THIRD_PARTY}/libjpeg)
		endif()
		if (NOT ANDROID)
			target_link_libraries(oxygine-framework PUBLIC jpeg)
		endif(NOT ANDROID)
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

