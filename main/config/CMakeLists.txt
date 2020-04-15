# To re-generate the files:
# $ cmake --build build --target config_pb config_textpb

idf_component_register(SRC_DIRS "." INCLUDE_DIRS "." REQUIRES "protobuf-c")

set(PROTO_C_COMPILER "protoc-c")

set(PROTO_SRC "config.proto")
set(PROTO_OUTS "config.pb-c.c" "config.pb-c.h")
set(CONFIG_TEXTPROTO_SRC "GreenMeanMachine.textproto")
set(CONFIG_TEXTPROTO_OUT "GreenMeanMachine.pb")

# Generate the protobuf-c sources for "config.proto".
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --c_out=${COMPONENT_DIR} -I . ${PROTO_SRC}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${PROTO_SRC}
        OUTPUT ${PROTO_OUTS}
        COMMENT "Generating protobuf-c for ${PROTO_SRC}."
        VERBATIM)
add_custom_target(config_pb DEPENDS ${PROTO_OUTS})

# Generate the hydroponics.Config binary proto.
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --encode=hydroponics.Config -I . ${PROTO_SRC} < ${CONFIG_TEXTPROTO_SRC} > ${CONFIG_TEXTPROTO_OUT}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${CONFIG_TEXTPROTO_SRC}
        OUTPUT "${CONFIG_TEXTPROTO_OUT}"
        COMMENT "Generating hydroponics.Config for ${CONFIG_TEXTPROTO_SRC}."
        VERBATIM)
add_custom_target(config_textpb DEPENDS "${CONFIG_TEXTPROTO_OUT}")
