add_library(gui INTERFACE)

target_sources(gui INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/gui_button.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gui_page.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gui_slider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gui_widget.cpp
)

target_include_directories(gui INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(gui INTERFACE
    pico_stdlib
    framebuffer
    touchscreen
)

add_subdirectory(test)
