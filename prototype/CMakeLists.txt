add_executable(prototype)
install(TARGETS prototype)
foreach(ext "bin" "dis" "elf.map" "hex" "uf2")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/prototype.${ext}" TYPE BIN)
endforeach()

pico_generate_pio_header(prototype ${CMAKE_CURRENT_LIST_DIR}/edbus.pio)

target_sources(prototype PRIVATE 
  main.c
)
target_link_libraries(prototype PRIVATE
  pico_stdlib 
  hardware_pio
)
pico_enable_stdio_uart(prototype 1)
pico_enable_stdio_usb(prototype 1)

pico_add_extra_outputs(prototype)
pico_set_program_url(prototype "https://github.com/headblockhead/interchange-firmware")
