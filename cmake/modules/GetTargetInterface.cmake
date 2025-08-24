# cmake/modules/GetTargetInterface.cmake
# Minimal shim to extract interface properties from a target.

function(get_target_interface out_var prefix tgt property_type)
  if(NOT TARGET ${tgt})
    message(FATAL_ERROR "get_target_interface(): '${tgt}' is not a target")
  endif()

  set(_result "")
  
  if(property_type STREQUAL "COMPILE_OPTIONS")
    get_target_property(_props ${tgt} INTERFACE_COMPILE_OPTIONS)
  elseif(property_type STREQUAL "LINK_OPTIONS")
    get_target_property(_props ${tgt} INTERFACE_LINK_OPTIONS)
  elseif(property_type STREQUAL "INCLUDE_DIRECTORIES")
    get_target_property(_props ${tgt} INTERFACE_INCLUDE_DIRECTORIES)
  elseif(property_type STREQUAL "LINK_LIBRARIES")
    get_target_property(_props ${tgt} INTERFACE_LINK_LIBRARIES)
  else()
    message(FATAL_ERROR "get_target_interface(): Unknown property type '${property_type}'")
  endif()
  
  if(_props AND NOT _props STREQUAL "_props-NOTFOUND")
    set(_result "${_props}")
  endif()

  # Return to caller scope
  set(${out_var} "${_result}" PARENT_SCOPE)
endfunction()
