# To re-generate the files:
# $ brew install protobuf-c
# $ ./tools/idf_esp32.sh   protos protos_go config_protos commands_protos
# $ ./tools/idf_esp32s2.sh protos protos_go config_protos commands_protos
#
# To upload the GreenMeanMachine.pb:
# $ gcloud iot devices configs update --config-file=components/protos/GreenMeanMachine.pb --registry=registry --region=us-central1 --device=GreenMeanMachine-test
# $ gcloud iot devices configs update --config-file=components/protos/GreenMeanMachine.pb --registry=registry --region=us-central1 --device=GreenMeanMachine
# $ gcloud iot devices configs update --config-file=components/protos/RedMeanMachine.pb --registry=registry --region=us-central1 --device=RedMeanMachine-test
# $ gcloud iot devices configs update --config-file=components/protos/RedMeanMachine.pb --registry=registry --region=us-central1 --device=RedMeanMachine
#
# To send a command:
# $ gcloud iot devices commands send --command-file=components/protos/Commands.pb --registry=registry --region=us-central1 --device=GreenMeanMachine-test
# $ gcloud iot devices commands send --command-file=components/protos/Commands.pb --registry=registry --region=us-central1 --device=GreenMeanMachine
# $ gcloud iot devices commands send --command-file=components/protos/Commands.pb --registry=registry --region=us-central1 --device=RedMeanMachine-test
# $ gcloud iot devices commands send --command-file=components/protos/Commands.pb --registry=registry --region=us-central1 --device=RedMeanMachine

idf_component_register(SRC_DIRS "." INCLUDE_DIRS "." REQUIRES "protobuf-c")

set(PROTO_C_COMPILER "protoc-c")
set(PROTO_COMPILER "protoc")

set(PROTO_SRCS "commands.proto" "config.proto" "state.proto")
set(PROTO_C_OUTS
        "commands.pb-c.c" "commands.pb-c.h"
        "config.pb-c.c" "config.pb-c.h"
        "state.pb-c.c" "state.pb-c.h")
set(PROTO_GO_OUTS
        "go/commands.pb.go"
        "go/config.pb.go"
        "go/state.pb.go")

set(CONFIG_GM_TEXTPROTO_SRC "GreenMeanMachine.textproto")
set(CONFIG_GM_TEXTPROTO_OUT "GreenMeanMachine.pb")
set(CONFIG_RM_TEXTPROTO_SRC "RedMeanMachine.textproto")
set(CONFIG_RM_TEXTPROTO_OUT "RedMeanMachine.pb")
set(COMMANDS_TEXTPROTO_SRC "Commands.textproto")
set(COMMANDS_TEXTPROTO_OUT "Commands.pb")

# Generate the protobuf-c sources from the protos.
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --c_out=${COMPONENT_DIR} -I . ${PROTO_SRCS}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${PROTO_SRCS}
        OUTPUT ${PROTO_C_OUTS}
        COMMENT "Generating protobuf-c for ${PROTO_SRCS}."
        VERBATIM)
add_custom_target(protos DEPENDS ${PROTO_C_OUTS})

# Generate the GoLang protobuf sources from the protos.
add_custom_command(
        COMMAND ${PROTO_COMPILER} --go_out=${COMPONENT_DIR}/go -I . --go_opt=paths=source_relative ${PROTO_SRCS}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${PROTO_SRCS}
        OUTPUT ${PROTO_GO_OUTS}
        COMMENT "Generating protobuf-go for ${PROTO_SRCS}."
        VERBATIM)
add_custom_target(protos_go DEPENDS ${PROTO_GO_OUTS})

# Generate the hydroponics.Config binary proto.
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --encode=hydroponics.Config -I . ${PROTO_SRCS} < ${CONFIG_GM_TEXTPROTO_SRC} > ${CONFIG_GM_TEXTPROTO_OUT}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${CONFIG_GM_TEXTPROTO_SRC}
        OUTPUT "${CONFIG_GM_TEXTPROTO_OUT}"
        COMMENT "Generating hydroponics.Config for ${CONFIG_GM_TEXTPROTO_SRC}."
        VERBATIM)
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --encode=hydroponics.Config -I . ${PROTO_SRCS} < ${CONFIG_RM_TEXTPROTO_SRC} > ${CONFIG_RM_TEXTPROTO_OUT}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${CONFIG_RM_TEXTPROTO_SRC}
        OUTPUT "${CONFIG_RM_TEXTPROTO_OUT}"
        COMMENT "Generating hydroponics.Config for ${CONFIG_RM_TEXTPROTO_SRC}."
        VERBATIM)
add_custom_target(config_protos DEPENDS "${CONFIG_GM_TEXTPROTO_OUT}" "${CONFIG_RM_TEXTPROTO_OUT}" protos)

# Generate the hydroponics.Commands binary proto.
add_custom_command(
        COMMAND ${PROTO_C_COMPILER} --encode=hydroponics.Commands -I . ${PROTO_SRCS} < ${COMMANDS_TEXTPROTO_SRC} > ${COMMANDS_TEXTPROTO_OUT}
        WORKING_DIRECTORY ${COMPONENT_DIR}
        DEPENDS ${COMMANDS_TEXTPROTO_SRC}
        OUTPUT "${COMMANDS_TEXTPROTO_OUT}"
        COMMENT "Generating hydroponics.Commands for ${COMMANDS_TEXTPROTO_SRC}."
        VERBATIM)
add_custom_target(commands_protos DEPENDS "${COMMANDS_TEXTPROTO_OUT}" protos)
