add_executable(prototype
  main.c
)

target_link_libraries(prototype pico_stdlib)
pico_add_extra_outputs(prototype)
