if(DEFINED ENV{PICO_EDBUS_PATH})
    set(pico-edbus_DIR "$ENV{PICO_EDBUS_PATH}" CACHE PATH "Path to pico-edbus." FORCE)
endif()

find_package(pico-edbus REQUIRED)

add_executable(prototype
  main.c
)
target_link_libraries(prototype PRIVATE
  pico_stdlib 
  hardware_pio
  pico-edbus::pico-edbus
)

pico_enable_stdio_uart(prototype 1)
pico_enable_stdio_usb(prototype 1)

pico_add_extra_outputs(prototype)
pico_set_program_url(prototype "https://github.com/headblockhead/interchange-firmware")

install(TARGETS prototype)
foreach(ext "bin" "dis" "elf.map" "hex" "uf2")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/prototype.${ext}" TYPE BIN)
endforeach()
