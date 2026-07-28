find_package(edbus-rp2040 REQUIRED)

add_executable(prototype
  main.c
)
target_link_libraries(prototype PRIVATE
  pico_stdlib 
  edbus-rp2040::edbus-rp2040
)

pico_enable_stdio_uart(prototype 1)
pico_enable_stdio_usb(prototype 1)

pico_add_extra_outputs(prototype)
pico_set_program_url(prototype "https://github.com/headblockhead/interchange-firmware")

install(TARGETS prototype)
foreach(ext "bin" "dis" "elf.map" "hex" "uf2")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/prototype.${ext}" TYPE BIN)
endforeach()
