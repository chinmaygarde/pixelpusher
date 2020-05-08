
if(WINDOWS)
	add_compile_options(
		/W3
		/WX
		/wd4267
		/wd4305
	)

	add_definitions(
		"-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS=1"
	)
else()
	add_compile_options(
		-Wall
		-Werror

		-Wno-deprecated-declarations
	)

	add_compile_options(
		-fdiagnostics-color=always

		-fno-exceptions
		-fno-rtti
	)
endif()

if(WINDOWS)
	add_compile_definitions(NOMINMAX)
endif(WINDOWS)

add_subdirectory(core)
add_subdirectory(machine)
