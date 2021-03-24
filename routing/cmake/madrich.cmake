include_guard(GLOBAL)

set(MADRICH_SRC_PATH ${CMAKE_CURRENT_LIST_DIR}/..)
###############################################################################
# Brief:
#   First agrument is always must be name of target.
# Parameters:
#   HEADERS - list of headers
#   SOURCES - list of source files
###############################################################################
function(add_madrich_executable name)
  cmake_parse_arguments(ARG
    # true_false_options
    ""
    # one_value_options
    ""
    # multi_value_options
    "HEADERS;SOURCES"
    ${ARGN}
  )

  set(${name}_HEADERS ${ARG_HEADERS} PARENT_SCOPE)
  set(${name}_SOURCES ${ARG_SOURCES} PARENT_SCOPE)

  add_executable(${name}
    ${ARG_HEADERS}
    ${ARG_SOURCES}
  )

  target_link_libraries(${name} generators
                                local_search
                                local_search_operators
                                base_model
  )
  target_include_directories(${name} PRIVATE ${MADRICH_SRC_PATH})
endfunction(add_madrich_executable)

###############################################################################
# Brief:
#   First agrument is always must be name of target.
# Parameters:
#   HEADERS - list of headers
#   SOURCES - list of source files
###############################################################################
function(add_madrich_library name)
  cmake_parse_arguments(ARG
    # true_false_options
    ""
    # one_value_options
    ""
    # multi_value_options
    "HEADERS;SOURCES"
    ${ARGN}
  )

  set(${name}_HEADERS ${ARG_HEADERS} PARENT_SCOPE)
  set(${name}_SOURCES ${ARG_SOURCES} PARENT_SCOPE)

  add_library(${name} STATIC
    ${ARG_HEADERS}
    ${ARG_SOURCES}
  )

  target_include_directories(${name} PRIVATE ${MADRICH_SRC_PATH})
endfunction(add_madrich_library)

