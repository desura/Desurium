function(add_copy_target_file_step target target_loc)
  get_filename_component(target_name "${target_loc}" NAME)

  if(${target_name} MATCHES ".*\\.exe")
    if(NOT IS_ABSOLUTE ${BINDIR})
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/${BINDIR}")
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_BINARY_DIR}/${BINDIR}/${target_name}")
    endif()
  elseif(${target_name} MATCHES ".*\\.dll" OR ${target_name} MATCHES ".*\\.so" OR ${target_name} MATCHES ".*\\.dylib" OR ${target_name} MATCHES ".*\\.sh")
    if(NOT IS_ABSOLUTE ${RUNTIME_LIBDIR})
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/${RUNTIME_LIBDIR}")
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_BINARY_DIR}/${RUNTIME_LIBDIR}/${target_name}")
    endif()
  elseif(${target_name} MATCHES ".*\\.lib" OR ${target_name} MATCHES ".*\\.a")
    # this is a placeholder for desurium API and import libs, also the static preloader for games
  else()
    # handle UNIX executeables here
    # get the extension first
    get_filename_component(target_ext ${target_name} EXT)
    if("${target_ext}" STREQUAL "")
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/${RUNTIME_LIBDIR}")
      add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_BINARY_DIR}/${RUNTIME_LIBDIR}/${target_name}")
    endif()
  endif()
endfunction()

function(add_copy_target_step target)
  get_target_property(target_loc ${target} LOCATION)
  add_copy_target_file_step(${target} ${target_loc})
endfunction()

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
      set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${flags_list}")
    endif()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_CXX)
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${flags_list}")
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${flags_list}")
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

function(add_gtest_test binaryname arg)
  if(WITH_GTEST)
    add_test(NAME ${binaryname}_gtest COMMAND "${CMAKE_BINARY_DIR}/${BINDIR}/${binaryname}" ${arg})
  endif()
endfunction()

function(__internal_install_binary target category create_wrapper)
  add_copy_target_step(${target})
  set_property(TARGET ${target} PROPERTY FOLDER "${category}")

  if(WIN32)
    install(TARGETS ${target}
            RUNTIME DESTINATION "${BIN_INSTALL_DIR}")
  else()
    install(TARGETS ${target}
            RUNTIME DESTINATION "${LIB_INSTALL_DIR}")
    if(create_wrapper)
      # install script to launch binary
      set(CURRENT_TARGET "${target}")
      configure_file("${CMAKE_SCRIPT_PATH}/run.sh" "${CMAKE_BINARY_DIR}/${CURRENT_TARGET}" @ONLY)
      install(FILES "${CMAKE_BINARY_DIR}/${CURRENT_TARGET}"
              DESTINATION "${BIN_INSTALL_DIR}"
              PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ             GROUP_EXECUTE
                          WORLD_READ             WORLD_EXECUTE)
    endif()
  endif()
endfunction()

function(install_executable target)
  __internal_install_binary(${target} "Executable" TRUE)
endfunction()

function(install_tool target)
  __internal_install_binary(${target} "Tools" TRUE)
endfunction()

function(install_internal_tool target)
  __internal_install_binary(${target} "Tools" FALSE)
endfunction()

function(install_internal_tool_script file)
  install(FILES ${file}
          DESTINATION ${LIB_INSTALL_DIR}
          PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                      GROUP_READ             GROUP_EXECUTE
                      WORLD_READ             WORLD_EXECUTE
  )
endfunction()

function(install_external_library target)
  foreach(file ${ARGN})
    install(CODE "get_filename_component(resolved_file \"${file}\" REALPATH)\n get_filename_component(file_name \"${file}\" NAME)\n file(INSTALL DESTINATION ${LIB_INSTALL_DIR} TYPE FILE RENAME \${file_name} FILES \"\${resolved_file}\" )")
    set_property(TARGET ${target} PROPERTY FOLDER "ThirdParty")
    add_copy_target_file_step(${target} "${file}")
  endforeach()
endfunction()

function(install_third_party_library target)
  install(TARGETS ${target} DESTINATION ${LIB_INSTALL_DIR})
  set_property(TARGET ${target} PROPERTY FOLDER "ThirdParty")
  add_copy_target_step(${target})
endfunction()

function(install_library target)
  # dlls are runtime targets, import libs are archive, but we don't need them
  install(TARGETS "${target}"
          RUNTIME DESTINATION "${LIB_INSTALL_DIR}"
          LIBRARY DESTINATION "${LIB_INSTALL_DIR}")
  add_copy_target_step(${target})
  set_property(TARGET ${target} PROPERTY FOLDER "Shared")  
endfunction()

macro(generate_current_data_copy_target target)
  file(RELATIVE_PATH __rel_current_dir "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  set(${target} "internal_data_copy_${__rel_current_dir}")
endmacro()

function(__create__data_copy_target)
  generate_current_data_copy_target(target)
  if(NOT TARGET ${target})
    add_custom_target(${target} ALL)
  endif()
endfunction()

function(add_copy_data_file_step file new_name path)
  if(NOT IS_ABSOLUTE "${DATADIR}")
    generate_current_data_copy_target(target)
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${file}" "${CMAKE_BINARY_DIR}/${DATADIR}/${path}/${new_name}" COMMENT "copy ${file} to ${DATADIR}/${path}/${new_name}")
  endif()
endfunction()

function(install_data_files target_path)
  __create__data_copy_target()
  foreach(file ${ARGN})
    get_filename_component(file_name "${file}" NAME)
    set(new_name "${file_name}")
    # remove _win and _lin stuff
    macro(__if_body)
      string(REGEX REPLACE "_[wl]in" "" new_name "${file_name}")
      add_copy_data_file_step("${file}" "${new_name}" "${target_path}")
      install(FILES "${file}" RENAME "${new_name}" DESTINATION "${DATA_INSTALL_DIR}/${target_path}")
    endmacro()
    if("${file_name}" MATCHES ".*_win.*" AND WIN32)
      __if_body()
    elseif("${file_name}" MATCHES ".*_lin.*" AND NOT WIN32)
      __if_body()
    elseif(NOT "${file_name}" MATCHES ".*_[wl]in.*")
      __if_body()
    endif()
  endforeach()
endfunction()

function(install_data_directories target_path)
  __create__data_copy_target()
  foreach(dir ${ARGN})
    get_filename_component(target_dir_name "${dir}" NAME)
    file(GLOB_RECURSE files "${dir}" "${dir}/*")
    foreach(file ${files})
      get_filename_component(file_name "${file}" NAME)
      file(RELATIVE_PATH rel_file "${dir}" "${file}")
      get_filename_component(rel_path "${rel_file}" PATH)
      add_copy_data_file_step("${file}" "${file_name}" "${target_path}/${target_dir_name}/${rel_path}")
    endforeach()
    install(DIRECTORY ${dir}
            DESTINATION "${DATA_INSTALL_DIR}/${target_path}")
  endforeach()
endfunction()

macro(LinkWithGTest target)
  if(WITH_GTEST)
    if(MSVC11)
      add_definitions(-D_VARIADIC_MAX=10)
    endif()
    include_directories(${GTEST_INCLUDE_DIRS})
    target_link_libraries(${target} ${GTEST_LIBRARIES})
  endif()
endmacro()

MACRO(setup_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(SourcesInternal ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${SourcesInternal}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(setup_precompiled_header)
