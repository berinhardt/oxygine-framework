cmake_minimum_required (VERSION 3.12)
project (Demo)

find_package( Python COMPONENTS Interpreter)

add_subdirectory(../../../ oxygine-framework)
add_definitions(${OXYGINE_DEFINITIONS})
include_directories(${OXYGINE_INCLUDE_DIRS})
link_directories(${OXYGINE_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OXYGINE_CXX_FLAGS}")

add_executable(Demo ../src/Counter.cpp ../src/example.cpp ../src/main.cpp ../src/test.cpp  ../src/Counter.h ../src/TestAlphaHitTest.h ../src/TestBox9Sprite.h ../src/TestCamera.h ../src/TestClipRect.h ../src/TestColorFont.h ../src/TestCounter.h ../src/TestDrag.h ../src/TestEdges.h ../src/TestHttp.h ../src/TestInputText.h ../src/TestManageRes.h ../src/TestMask.h ../src/TestOpenGL.h ../src/TestPerf.h ../src/TestPolygon.h ../src/TestProgressBar.h ../src/TestRender2Texture.h ../src/TestSignedDistanceFont.h ../src/TestSliding.h ../src/TestTexel2Pixel.h ../src/TestText.h ../src/TestTextureFormat.h ../src/TestTiled.h ../src/TestTouches.h ../src/TestTweenPostProcessing.h ../src/TestTweenShine.h ../src/TestTweenText.h ../src/TestTweens.h ../src/TestUserShader.h ../src/example.h ../src/test.h )
target_link_libraries(Demo ${OXYGINE_CORE_LIBS})

if (WIN32) #disable console mode for VC++
	set_target_properties(Demo PROPERTIES WIN32_EXECUTABLE TRUE)
endif(WIN32)

if (EMSCRIPTEN)
	add_custom_target(Demo_Atlas 
		OUTPUT_NAME ${CMAKE_CURRENT_BINARY_DIR}/data.js
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/data/
		COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/oxyresbuild.py
		-v 1
		--src_data ${CMAKE_CURRENT_SOURCE_DIR}/../data
		--dest_data ${CMAKE_CURRENT_BINARY_DIR}/data
		-x xmls/res.xml
		COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/others/embed_folder_js.py -s ${CMAKE_CURRENT_SOURCE_DIR}/../data
	)

	SET(CMAKE_EXECUTABLE_SUFFIX ".html")	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WARN_ON_UNDEFINED_SYMBOLS=1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s TOTAL_MEMORY=50331648")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -s FORCE_FILESYSTEM=1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s FULL_ES2")
	em_link_pre_js(Demo  ${OXYGINE_JS_LIBRARIES}  ${CMAKE_CURRENT_BINARY_DIR}/data.js)
endif(EMSCRIPTEN)