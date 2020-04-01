# ┌──────────────────────────────────────────────────────────────────┐
# │  Retrieve Global Properties                                      │
# └──────────────────────────────────────────────────────────────────┘
vidi_global_property_get_lib_output_path(in_LIB_OUTPUT_PATH)
vidi_global_property_get_exe_output_path(in_EXE_OUTPUT_PATH)
vidi_global_property_get_install_incdir(in_INSTALL_INCDIR)
vidi_global_property_get_install_bindir(in_INSTALL_BINDIR)
vidi_global_property_get_install_libdir(in_INSTALL_LIBDIR)
vidi_global_property_get_install_dev_headers(in_INSTALL_DEV_HEADERS)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Configure Source Files                                          │
# └──────────────────────────────────────────────────────────────────┘
set_source_files_properties(src/glad.c PROPERTIES LANGUAGE CXX)
add_library(GLAD SHARED
  include/glad/glad.h
  include/KHR/khrplatform.h
  src/glad.c)
if(TARGET OpenGL::OpenGL)
  target_link_libraries(GLAD PUBLIC OpenGL::OpenGL)
else()
  target_link_libraries(GLAD PUBLIC OpenGL::GL)
endif()
if(TARGET OpenGL::GLU)
  target_link_libraries(GLAD PUBLIC OpenGL::GLU)
endif()
target_link_libraries(GLAD PUBLIC ${CMAKE_DL_LIBS})
target_include_directories(GLAD PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include;>"
  "$<INSTALL_INTERFACE:${in_INSTALL_INCDIR}>")
target_compile_definitions(GLAD PUBLIC GLAD_GLAPI_EXPORT=1)
set_target_properties(GLAD PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${in_LIB_OUTPUT_PATH}
  LIBRARY_OUTPUT_DIRECTORY ${in_LIB_OUTPUT_PATH}
  RUNTIME_OUTPUT_DIRECTORY ${in_EXE_OUTPUT_PATH})

# ┌──────────────────────────────────────────────────────────────────┐
# │  Configure Install                                               │
# └──────────────────────────────────────────────────────────────────┘
# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS GLAD EXPORT GLADConfig
  ARCHIVE  DESTINATION ${in_INSTALL_LIBDIR}
  LIBRARY  DESTINATION ${in_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${in_INSTALL_BINDIR}) # Windows

if(in_INSTALL_DEV_HEADERS)

  install(DIRECTORY include/ DESTINATION ${in_INSTALL_INCDIR})

  # This makes the project importable from the install directory
  # Put config file in per-project dir (name MUST match), can also
  # just go into 'cmake'.
  install(EXPORT GLADConfig
    NAMESPACE GLAD::
    DESTINATION ${in_INSTALL_LIBDIR}/cmake)

  # This makes the project importable from the build directory
  export(TARGETS GLAD
    NAMESPACE GLAD::
    FILE ${CMAKE_BINARY_DIR}/cmake/GLADConfig.cmake)

  # Register configure files
  set_property(GLOBAL APPEND PROPERTY VIDI_INSTALL_TARGET GLADConfig)

endif()
