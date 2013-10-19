set(DATA ${CMAKE_CURRENT_SOURCE_DIR}/data)

install_data_directories("" "${DATA}/bindings" "${DATA}/language" "${DATA}/scripts" "${DATA}/themes")
install_data_files("" "${DATA}/ca-bundle_lin.crt" "${DATA}/cef_data_lin.pak")

if(NOT WIN32)
  install_internal_tool_script(util.sh)
  install_internal_tool_script(findlib.sh)
endif()
