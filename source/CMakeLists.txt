
add_compile_options(
  -Wall
  -Wextra
  -pedantic
  -Werror

  -Wno-unused-parameter
)

add_subdirectory(core)
add_subdirectory(machine)
add_subdirectory(test)
