add_library(SHCONFIG INTERFACE)
target_compile_definitions(SHCONFIG INTERFACE SHCONFIG_MVP_UBO_BINDING=0)
target_compile_definitions(SHCONFIG INTERFACE SHCONFIG_IN_POSITION_LOCATION=0)
target_compile_definitions(SHCONFIG INTERFACE SHCONFIG_IN_COLOR_LOCATION=1)
target_compile_definitions(SHCONFIG INTERFACE SHCONFIG_GL_VERSION_MAJOR=4)
target_compile_definitions(SHCONFIG INTERFACE SHCONFIG_GL_VERSION_MINOR=5)

if(REMOTE_BUILD)
  add_custom_target(compile_and_copy_shaders_bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}/bin
  )
else()
  set(COMPILE_SHADERS ninja -C ${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_target(compile_and_copy_shaders_bin
    COMMAND ${COMPILE_SHADERS}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}/bin
  )
endif()
