# Shared compiler/output settings applied to every gtl target.
# Mirrors the settings that used to live in each .vcxproj (LanguageStandard=stdcpplatest,
# ConformanceMode=true, /bigobj /utf-8, AVX2, WarningLevel3, DisableSpecificWarnings 4251).

function(gtl_apply_common_options target)
	target_compile_features(${target} PRIVATE cxx_std_23)

	if(MSVC)
		# Real cl.exe and clang-cl (Clang's cl-compatible driver) both parse these, but
		# clang-cl warns "argument unused" on /std:c++latest (the cxx_std_23 compile feature
		# above already covers it), /Zc:preprocessor, and /MP, so those three stay cl.exe-only.
		target_compile_options(${target} PRIVATE
			/utf-8 /bigobj /permissive-
			/W3 /wd4251
			/arch:AVX2
		)
		if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
			target_compile_options(${target} PRIVATE /std:c++latest /Zc:preprocessor /MP)
		endif()
	else()
		# GCC (MSYS2 ucrt64) and Clang's GNU-style driver (MSYS2 clang64, Linux/macOS) both
		# accept these.
		target_compile_options(${target} PRIVATE -mavx2 -fexec-charset=UTF-8)
	endif()

	if(WIN32)
		target_compile_definitions(${target} PRIVATE UNICODE _UNICODE)
	endif()

	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:_DEBUG>
		$<$<NOT:$<CONFIG:Debug>>:NDEBUG>
	)
endfunction()

# Reproduces the vcxproj TargetName pattern: <base_name>.x64<R|D>
function(gtl_set_output_name target base_name)
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME_DEBUG            "${base_name}.x64D"
		OUTPUT_NAME_RELEASE          "${base_name}.x64R"
		OUTPUT_NAME_RELWITHDEBINFO   "${base_name}.x64R"
		OUTPUT_NAME_MINSIZEREL       "${base_name}.x64R"
		RUNTIME_OUTPUT_DIRECTORY     "${GTL_BIN_DIR}"
		ARCHIVE_OUTPUT_DIRECTORY     "${GTL_LIB_DIR}"
		LIBRARY_OUTPUT_DIRECTORY     "${GTL_BIN_DIR}"
	)
	foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
		string(TOUPPER "${cfg}" cfg_upper)
		set_target_properties(${target} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${GTL_BIN_DIR}"
			ARCHIVE_OUTPUT_DIRECTORY_${cfg_upper} "${GTL_LIB_DIR}"
			LIBRARY_OUTPUT_DIRECTORY_${cfg_upper} "${GTL_BIN_DIR}"
		)
	endforeach()
endfunction()

# Executables (test/bench/...) just get the plain target name, but still land in bin/.
function(gtl_set_exe_output_dir target)
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${GTL_BIN_DIR}"
	)
	foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
		string(TOUPPER "${cfg}" cfg_upper)
		set_target_properties(${target} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${GTL_BIN_DIR}"
		)
	endforeach()
endfunction()
