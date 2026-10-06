cmake_minimum_required(VERSION 3.16)
project(ban_preview C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(FW_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
set(LVGL_DIR ${FW_DIR}/managed_components/lvgl__lvgl)

file(GLOB_RECURSE LVGL_SRC CONFIGURE_DEPENDS ${LVGL_DIR}/src/*.c)
list(FILTER LVGL_SRC EXCLUDE REGEX "/drivers/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/libs/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/nxp/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/renesas/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/dave2d/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/eve/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/vg_lite/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/espressif/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/sdl/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/nema/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/nanovg/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/opengles/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/dma2d/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/draw/vglite/")
list(FILTER LVGL_SRC EXCLUDE REGEX "/others/")
# lv_init() always calls the built-in binary image decoder.
list(APPEND LVGL_SRC ${LVGL_DIR}/src/libs/bin_decoder/lv_bin_decoder.c)

add_executable(ban_preview
    main.c
    port_host.c
    png_write.c
    ${FW_DIR}/main/ban_logic.c
    ${FW_DIR}/main/ban_ui.c
    ${FW_DIR}/main/ban_ui_home.c
    ${FW_DIR}/main/ban_ui_scan.c
    ${FW_DIR}/main/ban_ui_event.c
    ${FW_DIR}/main/ban_ui_ban.c
    ${FW_DIR}/main/ban_ui_appeal.c
    ${FW_DIR}/main/ban_ui_over.c
    ${FW_DIR}/main/ban_assets.c
    ${FW_DIR}/main/ban_font_zh_12.c
    ${FW_DIR}/main/ban_font_zh_16.c
    ${FW_DIR}/main/ban_font_zh_24.c
    ${FW_DIR}/main/ban_font_latin_8.c
    ${LVGL_SRC}
)

target_include_directories(ban_preview PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${FW_DIR}/main
    ${LVGL_DIR}
    ${LVGL_DIR}/src
)

target_compile_definitions(ban_preview PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_compile_options(ban_preview PRIVATE -Wall -Wno-unused-parameter -Wno-unused-function)
target_link_libraries(ban_preview PRIVATE z m)

if(APPLE)
    target_compile_options(ban_preview PRIVATE -Wno-deprecated-declarations)
endif()
