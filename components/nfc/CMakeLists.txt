set(CPPFLAGS -DHAVE_CONFIG_H)

set(COMPONENT_SRCDIRS libnfc libnfc/buses libnfc/chips libnfc/drivers)
set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_PRIV_INCLUDEDIRS libnfc libnfc/buses libnfc/chips libnfc/drivers)

set(COMPONENT_REQUIRES main)

register_component()

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format-truncation)
