add_executable(prototype)

pico_generate_pio_header(prototype ${CMAKE_CURRENT_LIST_DIR}/edbus.pio)

target_sources(prototype PRIVATE 
  main.c
)
target_link_libraries(prototype PRIVATE
  pico_stdlib 
  hardware_pio
)

pico_add_extra_outputs(prototype)
pico_set_program_url(prototype "https://github.com/headblockhead/interchange-firmware/tree/${GIT_REVISION}/prototype")
