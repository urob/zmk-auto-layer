find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zmk_ruen)

if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
  target_sources(app PRIVATE src/lang.c)
  target_sources(app PRIVATE src/behaviors/behavior_ruen_key.c src/behaviors/behavior_ruen_macos.c src/behaviors/behavior_ruen_one_key.c src/behaviors/behavior_ruen_switch.c)
endif()