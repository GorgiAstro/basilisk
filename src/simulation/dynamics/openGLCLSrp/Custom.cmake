if(BUILD_OPENGLCL_SRP)
  find_package(glfw3 REQUIRED)
  find_package(glm REQUIRED)
  find_library(OPENGL_LIBRARY OpenGL REQUIRED)
  find_library(OPENCL_LIBRARY OpenCL REQUIRED)
#  target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libs/glload/include")
#  target_include_directories(${TARGET_NAME} PRIVATE "${CONAN_INCLUDE_DIRS_GLM}")
#  target_include_directories(${TARGET_NAME} PRIVATE "${CONAN_INCLUDE_DIRS_GLFW}")
#  get_cmake_property(_variableNames VARIABLES)
#  list (SORT _variableNames)
#  foreach (_variableName ${_variableNames})
#      message(STATUS "${_variableName}=${${_variableName}}")
#  endforeach()
#  message(FATAL_ERROR "stop for pat")
#  if(glm_FOUND)
#      include_directories(${glm_INCLUDE_DIRS})
#      target_link_libraries(${TARGET_NAME} ${glm_LIBRARIES})
#  else()
#      message(FATAL_ERROR "glm not found")
#  endif()
#
#  if(glfw_FOUND)
#      include_directories(${glfw_INCLUDE_DIRS})
#      target_link_libraries(${TARGET_NAME} ${glfw_LIBRARIES})
#  else()
#      message(FATAL_ERROR "glfw not found")
#  endif()

   set(CUSTOM_INCLUDES
           ${glm_INCLUDE_DIRS_DEBUG}
           ${glfw_INCLUDE_DIRS_DEBUG}
           "${CMAKE_CURRENT_LIST_DIR}/libs/glload/include"
           "${CMAKE_CURRENT_LIST_DIR}/libs/assimp/include")

  set(CUSTOM_DEPENDENCIES
          ${OPENCL_LIBRARY}
          ${glm_LIBRARIES_DEBUG}
          ${glfw_LIBRARIES_DEBUG}
          ${CMAKE_CURRENT_LIST_DIR}/libs/glload/lib/libglloadD.a
          ${CMAKE_CURRENT_LIST_DIR}/libs/assimp/lib/libassimp.dylib)

else()
  MESSAGE("SKIPPED: ${TARGET_NAME}")
  set(CUSTOM_DEPENDENCIES_HANDLED 1)
endif()
