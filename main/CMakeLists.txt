set(SRC_DIRS_LIST ".")

idf_component_register(SRC_DIRS ${SRC_DIRS_LIST})

add_custom_target(build_frontend ALL
    COMMAND yarn build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/app
    COMMENT "Building Vite React frontend"
)

add_dependencies(${COMPONENT_LIB} build_frontend)
spiffs_create_partition_image(spiffs ../app/dist FLASH_IN_PROJECT)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")

