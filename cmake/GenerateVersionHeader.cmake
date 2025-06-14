if(GIT_EXECUTABLE)
  get_filename_component(SRC_DIR ${SRC} DIRECTORY)

  message(WARNING "Failed to determine DYNAMIC_VERSION from Git tags. Using default version \"${DYNAMIC_VERSION}\".")
  # Generate a git-describe version string from Git repository tags
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v*"
    WORKING_DIRECTORY ${SRC_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
    RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(NOT GIT_DESCRIBE_ERROR_CODE)
    set(DYNAMIC_VERSION ${GIT_DESCRIBE_VERSION})
  endif()
endif()

# Final fallback: Just use a bogus version string that is semantically older
# than anything else and spit out a warning to the developer.
if(NOT DEFINED DYNAMIC_VERSION)
  set(DYNAMIC_VERSION v0.0.0-unknown)
  message(WARNING "Failed to determine DYNAMIC_VERSION from Git tags. Using default version \"${DYNAMIC_VERSION}\".")
endif()

configure_file(${SRC} ${DST} @ONLY)