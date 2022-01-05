/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: commands.proto */

#ifndef PROTOBUF_C_commands_2eproto__INCLUDED
#define PROTOBUF_C_commands_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

#include "config.pb-c.h"

typedef struct _Hydroponics__CommandReboot Hydroponics__CommandReboot;
typedef struct _Hydroponics__CommandSet Hydroponics__CommandSet;
typedef struct _Hydroponics__CommandImpulse Hydroponics__CommandImpulse;
typedef struct _Hydroponics__CommandI2c Hydroponics__CommandI2c;
typedef struct _Hydroponics__Command Hydroponics__Command;
typedef struct _Hydroponics__Commands Hydroponics__Commands;


/* --- enums --- */


/* --- messages --- */

struct  _Hydroponics__CommandReboot
{
  ProtobufCMessage base;
};
#define HYDROPONICS__COMMAND_REBOOT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&hydroponics__command_reboot__descriptor) \
     }


struct  _Hydroponics__CommandSet
{
  ProtobufCMessage base;
  size_t n_output;
  Hydroponics__Output *output;
  Hydroponics__OutputState state;
};
#define HYDROPONICS__COMMAND_SET__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&hydroponics__command_set__descriptor) \
    , 0,NULL, HYDROPONICS__OUTPUT_STATE__OFF }


/*
 * Defines an output impulse response.
 * The output will start on the initial `state`, then it will be valid for `delay_ms` and finally it will transition to
 * the final `!state`.
 */
struct  _Hydroponics__CommandImpulse
{
  ProtobufCMessage base;
  size_t n_output;
  Hydroponics__Output *output;
  Hydroponics__OutputState state;
  uint32_t delay_ms;
};
#define HYDROPONICS__COMMAND_IMPULSE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&hydroponics__command_impulse__descriptor) \
    , 0,NULL, HYDROPONICS__OUTPUT_STATE__OFF, 0 }


struct  _Hydroponics__CommandI2c
{
  ProtobufCMessage base;
  uint32_t address;
  uint32_t reg_address;
  ProtobufCBinaryData write;
  uint32_t read_len;
};
#define HYDROPONICS__COMMAND_I2C__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&hydroponics__command_i2c__descriptor) \
    , 0, 0, {0,NULL}, 0 }


typedef enum {
  HYDROPONICS__COMMAND__COMMAND__NOT_SET = 0,
  HYDROPONICS__COMMAND__COMMAND_REBOOT = 1,
  HYDROPONICS__COMMAND__COMMAND_SET = 2,
  HYDROPONICS__COMMAND__COMMAND_IMPULSE = 3,
  HYDROPONICS__COMMAND__COMMAND_I2C = 4
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(HYDROPONICS__COMMAND__COMMAND)
} Hydroponics__Command__CommandCase;

struct  _Hydroponics__Command
{
  ProtobufCMessage base;
  Hydroponics__Command__CommandCase command_case;
  union {
    Hydroponics__CommandReboot *reboot;
    Hydroponics__CommandSet *set;
    Hydroponics__CommandImpulse *impulse;
    Hydroponics__CommandI2c *i2c;
  };
};
#define HYDROPONICS__COMMAND__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&hydroponics__command__descriptor) \
    , HYDROPONICS__COMMAND__COMMAND__NOT_SET, {0} }


struct  _Hydroponics__Commands
{
  ProtobufCMessage base;
  size_t n_command;
  Hydroponics__Command **command;
};
#define HYDROPONICS__COMMANDS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&hydroponics__commands__descriptor) \
    , 0,NULL }


/* Hydroponics__CommandReboot methods */
void   hydroponics__command_reboot__init
                     (Hydroponics__CommandReboot         *message);
size_t hydroponics__command_reboot__get_packed_size
                     (const Hydroponics__CommandReboot   *message);
size_t hydroponics__command_reboot__pack
                     (const Hydroponics__CommandReboot   *message,
                      uint8_t             *out);
size_t hydroponics__command_reboot__pack_to_buffer
                     (const Hydroponics__CommandReboot   *message,
                      ProtobufCBuffer     *buffer);
Hydroponics__CommandReboot *
       hydroponics__command_reboot__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   hydroponics__command_reboot__free_unpacked
                     (Hydroponics__CommandReboot *message,
                      ProtobufCAllocator *allocator);
/* Hydroponics__CommandSet methods */
void   hydroponics__command_set__init
                     (Hydroponics__CommandSet         *message);
size_t hydroponics__command_set__get_packed_size
                     (const Hydroponics__CommandSet   *message);
size_t hydroponics__command_set__pack
                     (const Hydroponics__CommandSet   *message,
                      uint8_t             *out);
size_t hydroponics__command_set__pack_to_buffer
                     (const Hydroponics__CommandSet   *message,
                      ProtobufCBuffer     *buffer);
Hydroponics__CommandSet *
       hydroponics__command_set__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   hydroponics__command_set__free_unpacked
                     (Hydroponics__CommandSet *message,
                      ProtobufCAllocator *allocator);
/* Hydroponics__CommandImpulse methods */
void   hydroponics__command_impulse__init
                     (Hydroponics__CommandImpulse         *message);
size_t hydroponics__command_impulse__get_packed_size
                     (const Hydroponics__CommandImpulse   *message);
size_t hydroponics__command_impulse__pack
                     (const Hydroponics__CommandImpulse   *message,
                      uint8_t             *out);
size_t hydroponics__command_impulse__pack_to_buffer
                     (const Hydroponics__CommandImpulse   *message,
                      ProtobufCBuffer     *buffer);
Hydroponics__CommandImpulse *
       hydroponics__command_impulse__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   hydroponics__command_impulse__free_unpacked
                     (Hydroponics__CommandImpulse *message,
                      ProtobufCAllocator *allocator);
/* Hydroponics__CommandI2c methods */
void   hydroponics__command_i2c__init
                     (Hydroponics__CommandI2c         *message);
size_t hydroponics__command_i2c__get_packed_size
                     (const Hydroponics__CommandI2c   *message);
size_t hydroponics__command_i2c__pack
                     (const Hydroponics__CommandI2c   *message,
                      uint8_t             *out);
size_t hydroponics__command_i2c__pack_to_buffer
                     (const Hydroponics__CommandI2c   *message,
                      ProtobufCBuffer     *buffer);
Hydroponics__CommandI2c *
       hydroponics__command_i2c__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   hydroponics__command_i2c__free_unpacked
                     (Hydroponics__CommandI2c *message,
                      ProtobufCAllocator *allocator);
/* Hydroponics__Command methods */
void   hydroponics__command__init
                     (Hydroponics__Command         *message);
size_t hydroponics__command__get_packed_size
                     (const Hydroponics__Command   *message);
size_t hydroponics__command__pack
                     (const Hydroponics__Command   *message,
                      uint8_t             *out);
size_t hydroponics__command__pack_to_buffer
                     (const Hydroponics__Command   *message,
                      ProtobufCBuffer     *buffer);
Hydroponics__Command *
       hydroponics__command__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   hydroponics__command__free_unpacked
                     (Hydroponics__Command *message,
                      ProtobufCAllocator *allocator);
/* Hydroponics__Commands methods */
void   hydroponics__commands__init
                     (Hydroponics__Commands         *message);
size_t hydroponics__commands__get_packed_size
                     (const Hydroponics__Commands   *message);
size_t hydroponics__commands__pack
                     (const Hydroponics__Commands   *message,
                      uint8_t             *out);
size_t hydroponics__commands__pack_to_buffer
                     (const Hydroponics__Commands   *message,
                      ProtobufCBuffer     *buffer);
Hydroponics__Commands *
       hydroponics__commands__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   hydroponics__commands__free_unpacked
                     (Hydroponics__Commands *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Hydroponics__CommandReboot_Closure)
                 (const Hydroponics__CommandReboot *message,
                  void *closure_data);
typedef void (*Hydroponics__CommandSet_Closure)
                 (const Hydroponics__CommandSet *message,
                  void *closure_data);
typedef void (*Hydroponics__CommandImpulse_Closure)
                 (const Hydroponics__CommandImpulse *message,
                  void *closure_data);
typedef void (*Hydroponics__CommandI2c_Closure)
                 (const Hydroponics__CommandI2c *message,
                  void *closure_data);
typedef void (*Hydroponics__Command_Closure)
                 (const Hydroponics__Command *message,
                  void *closure_data);
typedef void (*Hydroponics__Commands_Closure)
                 (const Hydroponics__Commands *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor hydroponics__command_reboot__descriptor;
extern const ProtobufCMessageDescriptor hydroponics__command_set__descriptor;
extern const ProtobufCMessageDescriptor hydroponics__command_impulse__descriptor;
extern const ProtobufCMessageDescriptor hydroponics__command_i2c__descriptor;
extern const ProtobufCMessageDescriptor hydroponics__command__descriptor;
extern const ProtobufCMessageDescriptor hydroponics__commands__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_commands_2eproto__INCLUDED */
