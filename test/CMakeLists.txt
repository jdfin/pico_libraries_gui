
add_compile_options(-Wall -Wextra -Werror)

# gui_test

add_executable(gui_test
    gui_test.cpp
)

pico_enable_stdio_uart(gui_test 0)
pico_enable_stdio_usb(gui_test 1)
target_link_libraries(gui_test PRIVATE
    pico_stdlib
    pico_stdio_usb
    gui
    framebuffer
    touchscreen
    misc
)

pico_add_extra_outputs(gui_test)
