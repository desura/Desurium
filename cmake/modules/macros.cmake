

function(CopyOutputFiles target target_loc)
  if (WIN32)
    if(${target_loc} MATCHES ".*exe")
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_OUTPUT}\\.")
    endif()
	
    if(${target_loc} MATCHES ".*dll")
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_OUTPUT}\\bin")
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_OUTPUT}\\bin\\.")
    endif()
  else()
	  get_filename_component(target_loc "${target_loc}" NAME)
  
	  if(NOT IS_ABSOLUTE ${BINDIR})
		if(${target_loc} MATCHES ".*exe")
		  add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_BINARY_DIR}/${target_loc}")
		endif()
	  endif()

	  if(RUNTIME_LIBDIR)
		if(${target_loc} MATCHES ".*so" OR ${target_loc} MATCHES ".*dylib")
		  add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_OUTPUT}/${RUNTIME_LIBDIR}/${target_loc}")
		  add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_BINARY_DIR}/${RUNTIME_LIBDIR}/${target_loc}")
		endif()
	  endif()
	endif()
endfunction(CopyOutputFiles)

function(CopyTargetFiles target)
  get_target_property(target_loc ${target} LOCATION)
  CopyOutputFiles(${target} ${target_loc})
endfunction(CopyTargetFiles)

macro(add_compiler_flags)
  set(flags_list "")
  parse_arguments(ARG "" "C;CXX;DEBUG;RELEASE" ${ARGN})

  if(NOT ARG_DEBUG AND NOT ARG_RELEASE)
    foreach(flag ${ARG_DEFAULT_ARGS})
      set(flags_list "${flags_list} ${flag}")
    endforeach()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_C)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flags_list}")
    endif()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_CXX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags_list}")
    endif()
  elseif(ARG_DEBUG)
    foreach(flag ${ARG_DEFAULT_ARGS})
      set(flags_list "${flags_list} ${flag}")
    endforeach()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_C)
      set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${flags_list}")
    endif()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_CXX)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${flags_list}")
    endif()
  elseif(ARG_RELEASE)
    foreach(flag ${ARG_DEFAULT_ARGS})
      set(flags_list "${flags_list} ${flag}")
    endforeach()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_C)
      set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${flags_list}")
    endif()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_CXX)
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${flags_list}")
    endif()
  endif()
endmacro()

macro(add_linker_flags)
  foreach(flag ${ARGN})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flag}")
  endforeach()
endmacro()

macro(set_up_32bit_env)
  if(CMAKE_COMPILER_IS_GNUCXX)
    add_compiler_flags(-m32)
    add_linker_flags(-m32)
  endif()
endmacro()

macro(pair)
  set(_aList ${ARGN})
  list(GET _aList 0 _CMD)
  
  if(_CMD STREQUAL "FIRST")
    list(GET _aList 1 _in)
    list(GET _aList 2 _out)
    string(FIND "${_in}" "->" _pos)
    string(SUBSTRING "${_in}" 0 ${_pos} ${_out})
    
    unset(_in)
    unset(_out)
    unset(_pos)
  elseif(_CMD STREQUAL "SECOND")
    list(GET _aList 1 _in)
    list(GET _aList 2 _out)
    string(FIND "${_in}" "->" _pos)
    string(LENGTH "${_in}" _len)

    math(EXPR _pos "${_pos} + 2")
    math(EXPR _len "${_len} - ${_pos}")

    string(SUBSTRING "${_in}" ${_pos} ${_len} ${_out})
    
    unset(_in)
    unset(_out)
    unset(_pos)
    unset(_len)
  else()
    message(FATAL_ERROR "${_CMD} is not implemented for pair")
  endif()
  
  unset(_CMD)
  unset(_aList)
endmacro()

macro(add_desura_test name category neededLibs)
  if(BUILD_TESTS)
    file(GLOB ${name}_SRC ${CMAKE_SOURCE_DIR}/src/tests/${category}/${name}*.c
                          ${CMAKE_SOURCE_DIR}/src/tests/${category}/${name}*.C
                          ${CMAKE_SOURCE_DIR}/src/tests/${category}/${name}*.cc
                          ${CMAKE_SOURCE_DIR}/src/tests/${category}/${name}*.cpp
                          ${CMAKE_SOURCE_DIR}/src/tests/${category}/${name}*.cxx
                          ${CMAKE_SOURCE_DIR}/src/tests/${category}/${name}*.h
                          ${CMAKE_SOURCE_DIR}/src/tests/*.h)
    add_executable(${name} ${${name}_SRC})
    # have to be defined to generate a main function
    add_definitions(-DBOOST_TEST_DYN_LINK)
    include_directories(
      ${CMAKE_SOURCE_DIR}/src/tests
      ${Boost_INCLUDE_DIR}
    )
    target_link_libraries(${name}
      ${neededLibs}
      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    )
    add_test(${name} ${CMAKE_BINARY_DIR}/src/tests/${name})
  endif()
endmacro()

function(install_executable target)
  set(CURRENT_TARGET "${target}")
  install(TARGETS "${CURRENT_TARGET}"
          RUNTIME DESTINATION "${LIB_INSTALL_DIR}")
  # install script to launch binary
  if(NOT WIN32)
    configure_file("${CMAKE_SCRIPT_PATH}/run.sh" "${CMAKE_GEN_SRC_DIR}/build_out/${CURRENT_TARGET}" @ONLY)
    install(FILES "${CMAKE_GEN_SRC_DIR}/build_out/${CURRENT_TARGET}"
            DESTINATION "${BIN_INSTALL_DIR}"
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ             GROUP_EXECUTE
                        WORLD_READ             WORLD_EXECUTE)
  endif()
endfunction()

function(desurium_install_library target category)
  # dlls are runtime targets, import libs are archive, but we don't need them
  install(TARGETS "${target}"
          RUNTIME DESTINATION "${LIB_INSTALL_DIR}"
          LIBRARY DESTINATION "${LIB_INSTALL_DIR}")
  CopyTargetFiles(${target})
  set_property(TARGET ${target} PROPERTY FOLDER ${category})
endfunction()
