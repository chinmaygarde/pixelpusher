
if(NOT WINDOWS)
	add_compile_options(
	  -Wall
	  -Wextra
	  -pedantic
	  -Werror

	  -Wno-unused-parameter
	  -Wno-gnu-statement-expression
	)

	add_compile_options(
	  -fno-exceptions
	  -fno-rtti
	)
endif(NOT WINDOWS)

add_subdirectory(core)
add_subdirectory(machine)
# add_subdirectory(test)
