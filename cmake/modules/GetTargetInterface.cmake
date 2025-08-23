# cmake/modules/GetTargetInterface.cmake
# Minimal shim to extract interface include dirs and link libs from a target.

function(get_target_interface tgt OUT_INCLUDES OUT_LIBS)
  if(NOT TARGET ${tgt})
    message(FATAL_ERROR "get_target_interface(): '${tgt}' is not a target")
  endif()

  # Interface include dirs
  get_target_property(_incs ${tgt} INTERFACE_INCLUDE_DIRECTORIES)
  if(NOT _incs)
    set(_incs "")
  endif()

  # Interface link libraries (may contain generator expressions and other targets)
  get_target_property(_libs ${tgt} INTERFACE_LINK_LIBRARIES)
  if(NOT _libs)
    set(_libs "")
  endif()

  # Return to caller scope
  set(${OUT_INCLUDES} "${_incs}" PARENT_SCOPE)
  set(${OUT_LIBS}     "${_libs}" PARENT_SCOPE)
endfunction()
