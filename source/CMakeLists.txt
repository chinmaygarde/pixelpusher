
if(NOT WINDOWS)
	add_compile_options(
		-Wall
		-Werror
	)

	add_compile_options(
		-fno-exceptions
		-fno-rtti
	)
endif(NOT WINDOWS)

if(WINDOWS)
	add_compile_definitions(NOMINMAX)
endif(WINDOWS)

add_subdirectory(core)
add_subdirectory(machine)

# Conflicts with shaderc dependencies.
# add_subdirectory(test)
