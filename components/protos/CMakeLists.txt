# To re-generate the files:
# $ brew install protobuf-c
# $ cmake --build build --target protos config_protos command_protos
#
# To upload the GreenMeanMachine.pb:
# $ gcloud iot devices configs update --config-file=components/protos/GreenMeanMachine.pb --registry=registry --region=us-central1 --device=GreenMeanMachine-test
# $ gcloud iot devices configs update --config-file=components/protos/GreenMeanMachine.pb --registry=registry --region=us-central1 --device=GreenMeanMachine
#
# To send a command:
# $ gcloud iot devices commands send --command-file=components/protos/Commands.pb --registry=registry --region=us-central1 --device=GreenMeanMachine-test
# $ gcloud iot devices commands send --command-file=components/protos/Commands.pb --registry=registry --region=us-central1 --device=GreenMeanMachine

idf_component_register(SRC_DIRS "." INCLUDE_DIRS "." REQUIRES "protobuf-c")

set(PROTO_C_COMPILER "protoc-c")

set(PROTO_SRCS "commands.proto" "config.proto")
set(PROTO_OUTS "commands.pb-c.c" "commands.pb-c.h" "config.pb-c.c" "config.pb-c.h")
set(CONFIG_TEXTPROTO_SRC "GreenMeanMachine.textproto")
set(CONFIG_TEXTPROTO_OUT "GreenMeanMachine.pb")
set(COMMANDS_TEXTPROTO_SRC "Commands.textproto")
set(COMMANDS_TEXTPROTO_OUT "Commands.pb")

# Generate the protobuf-c sources for "config.proto".
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --c_out=${COMPONENT_DIR} -I . ${PROTO_SRCS}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${PROTO_SRCS}
        OUTPUT ${PROTO_OUTS}
        COMMENT "Generating protobuf-c for ${PROTO_SRCS}."
        VERBATIM)
add_custom_target(protos DEPENDS ${PROTO_OUTS})

# Generate the hydroponics.Config binary proto.
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --encode=hydroponics.Config -I . ${PROTO_SRCS} < ${CONFIG_TEXTPROTO_SRC} > ${CONFIG_TEXTPROTO_OUT}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${CONFIG_TEXTPROTO_SRC}
        OUTPUT "${CONFIG_TEXTPROTO_OUT}"
        COMMENT "Generating hydroponics.Config for ${CONFIG_TEXTPROTO_SRC}."
        VERBATIM)
add_custom_target(config_protos DEPENDS "${CONFIG_TEXTPROTO_OUT}" protos)

# Generate the hydroponics.Commands binary proto.
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --encode=hydroponics.Commands -I . ${PROTO_SRCS} < ${COMMANDS_TEXTPROTO_SRC} > ${COMMANDS_TEXTPROTO_OUT}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${COMMANDS_TEXTPROTO_SRC}
        OUTPUT "${COMMANDS_TEXTPROTO_OUT}"
        COMMENT "Generating hydroponics.Commands for ${COMMANDS_TEXTPROTO_SRC}."
        VERBATIM)
add_custom_target(commands_protos DEPENDS "${COMMANDS_TEXTPROTO_OUT}" protos)
