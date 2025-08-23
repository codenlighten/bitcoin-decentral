# Minimal compatibility shim for try_append_cxx_flags
include(CheckCXXCompilerFlag)

function(try_append_cxx_flags flag)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "SKIP_LINK" "TARGET;RESULT_VAR" "")
  
  string(MAKE_C_IDENTIFIER "CXX_SUPPORTS_${flag}" var_name)
  check_cxx_compiler_flag("${flag}" ${var_name})
  
  if(${var_name})
    if(ARG_TARGET)
      target_compile_options(${ARG_TARGET} INTERFACE ${flag})
    else()
      add_compile_options(${flag})
    endif()
  endif()
  
  if(ARG_RESULT_VAR)
    set(${ARG_RESULT_VAR} ${${var_name}} PARENT_SCOPE)
  endif()
endfunction()
