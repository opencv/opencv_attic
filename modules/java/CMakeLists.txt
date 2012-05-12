# ----------------------------------------------------------------------------
#  Root CMake file for java module.
#
# ----------------------------------------------------------------------------

if(ANDROID)

  add_subdirectory(api)

  if(BUILD_ANDROID_SERVICE)
    add_subdirectory(android_service)
  endif()

  if(BUILD_ANDROID_BIN_PACKAGES)
    add_subdirectory(android_bin_package)
  endif()

endif()
