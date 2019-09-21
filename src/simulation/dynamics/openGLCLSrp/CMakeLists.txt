cmake_minimum_required(VERSION 3.0)

file(GLOB module_file "${CMAKE_CURRENT_LIST_DIR}/*.i")
get_filename_component(module_name ${module_file} NAME_WE)
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")


# Manually create list of libraries depending on system
if(WIN32)
    message("Open CL SRP is not supported on the Windows platform")
    set(swig_module_${module_name}_use_opencl FALSE)
	#set(swig_module_libraries
#          ${CMAKE_SOURCE_DIR}/../libs/cspice/lib/cspice_win${_arch_suffix}.lib
#        ${Boost_LIBRARIES}
#    )
elseif(APPLE)
    set(X11_LIBS "")
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    set(X11_LIBS ${X11_LIBRARIES})
        
    set(swig_module_${module_name}_use_opencl TRUE)
    set(swig_module_${module_name}_includes
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glload/include
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glfw/include
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glmesh/include
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glutil/include
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glm/include
	/usr/local/include/
        #${CMAKE_SOURCE_DIR}/../libs/openglSupport/assimp_3_3_1/include
    )
    find_library(COCOA_LIBRARY Cocoa)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(AGL_LIBRARY AGL)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(OPENCL_LIBRARY OpenCL)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(GLUT_LIBRARY GLUT)
    set(swig_module_${module_name}_libraries 
        ${COCOA_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${AGL_LIBRARY}
        ${OPENGL_LIBRARY}
        ${OPENCL_LIBRARY}
        ${IOKIT_LIBRARY}
        ${GLUT_LIBRARY}
        ${X11_LIBS}    
        #${CMAKE_SOURCE_DIR}/../libs/openglSupport/assimp_3_3_1/lib/libassimp.dylib
	/usr/local/lib/libassimp.4.1.0.dylib
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glfw/lib/libglfw3.a
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glutil/lib/libglutilD.a
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glmesh/lib/libglmeshD.a
        ${CMAKE_SOURCE_DIR}/../libs/openglSupport/glload/lib/libglloadD.a
    )
elseif(UNIX)
    message("Open CL SRP is not supported on the Linux platform")
    set(swig_module_${module_name}_use_opencl FALSE)
	#set(swig_module_libraries
#        ${CMAKE_SOURCE_DIR}/../libs/cspice/lib/cspice.a
#        ${Boost_LIBRARIES}
#    )
endif()


#add_executable(tutorial14_render_to_texture
#    resources/shaders/debugShader.fsh
#    resources/shaders/debugShader.gsh
#    resources/shaders/debugShader.vsh
#    resources/shaders/depthShader.fsh
#    resources/shaders/depthShader.vsh
#    resources/shaders/screenTexture.fsh
#    resources/shaders/screenTexture.vsh
#    resources/shaders/shadowMapFragmentShader.glsl
#    resources/shaders/shadowMapVertexShader.glsl
#    resources/shaders/simple.fsh
#    resources/shaders/simple.vsh
#    resources/shaders/simpleDebug.fsh
#    resources/shaders/simpleDebug.vsh
#)
    file(GLOB shader_files
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.vsh"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.fsh"
    )
SOURCE_GROUP(shaders shader_files)
