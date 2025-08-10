set(SRC_DIRS_LIST ".")

idf_component_register(SRC_DIRS ${SRC_DIRS_LIST})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
