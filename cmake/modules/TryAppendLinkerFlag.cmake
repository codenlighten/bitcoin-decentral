# Minimal compatibility shim for try_append_linker_flag
include(CheckLinkerFlag)

function(try_append_linker_flag flag)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "NO_CACHE_IF_FAILED" "TARGET;VAR;SOURCE;RESULT_VAR" "")
  
  string(MAKE_C_IDENTIFIER "LINKER_SUPPORTS_${flag}" var_name)
  
  if(ARG_SOURCE)
    # Custom source provided, use it for the test
    check_linker_flag(CXX "${flag}" ${var_name})
  else()
    check_linker_flag(CXX "${flag}" ${var_name})
  endif()
  
  if(${var_name})
    if(ARG_TARGET)
      target_link_options(${ARG_TARGET} INTERFACE ${flag})
    elseif(ARG_VAR)
      set(${ARG_VAR} ${flag} PARENT_SCOPE)
    else()
      add_link_options(${flag})
    endif()
  endif()
  
  if(ARG_RESULT_VAR)
    set(${ARG_RESULT_VAR} ${${var_name}} PARENT_SCOPE)
  endif()
endfunction()
