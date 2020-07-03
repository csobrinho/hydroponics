/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: config.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "config.pb-c.h"
void   hydroponics__sampling__init
                     (Hydroponics__Sampling         *message)
{
  static const Hydroponics__Sampling init_value = HYDROPONICS__SAMPLING__INIT;
  *message = init_value;
}
size_t hydroponics__sampling__get_packed_size
                     (const Hydroponics__Sampling *message)
{
  assert(message->base.descriptor == &hydroponics__sampling__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t hydroponics__sampling__pack
                     (const Hydroponics__Sampling *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &hydroponics__sampling__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t hydroponics__sampling__pack_to_buffer
                     (const Hydroponics__Sampling *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &hydroponics__sampling__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Hydroponics__Sampling *
       hydroponics__sampling__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Hydroponics__Sampling *)
     protobuf_c_message_unpack (&hydroponics__sampling__descriptor,
                                allocator, len, data);
}
void   hydroponics__sampling__free_unpacked
                     (Hydroponics__Sampling *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &hydroponics__sampling__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   hydroponics__controller__entry__pid__init
                     (Hydroponics__Controller__Entry__Pid         *message)
{
  static const Hydroponics__Controller__Entry__Pid init_value = HYDROPONICS__CONTROLLER__ENTRY__PID__INIT;
  *message = init_value;
}
void   hydroponics__controller__entry__init
                     (Hydroponics__Controller__Entry         *message)
{
  static const Hydroponics__Controller__Entry init_value = HYDROPONICS__CONTROLLER__ENTRY__INIT;
  *message = init_value;
}
void   hydroponics__controller__init
                     (Hydroponics__Controller         *message)
{
  static const Hydroponics__Controller init_value = HYDROPONICS__CONTROLLER__INIT;
  *message = init_value;
}
size_t hydroponics__controller__get_packed_size
                     (const Hydroponics__Controller *message)
{
  assert(message->base.descriptor == &hydroponics__controller__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t hydroponics__controller__pack
                     (const Hydroponics__Controller *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &hydroponics__controller__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t hydroponics__controller__pack_to_buffer
                     (const Hydroponics__Controller *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &hydroponics__controller__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Hydroponics__Controller *
       hydroponics__controller__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Hydroponics__Controller *)
     protobuf_c_message_unpack (&hydroponics__controller__descriptor,
                                allocator, len, data);
}
void   hydroponics__controller__free_unpacked
                     (Hydroponics__Controller *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &hydroponics__controller__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   hydroponics__task__cron__init
                     (Hydroponics__Task__Cron         *message)
{
  static const Hydroponics__Task__Cron init_value = HYDROPONICS__TASK__CRON__INIT;
  *message = init_value;
}
void   hydroponics__task__init
                     (Hydroponics__Task         *message)
{
  static const Hydroponics__Task init_value = HYDROPONICS__TASK__INIT;
  *message = init_value;
}
size_t hydroponics__task__get_packed_size
                     (const Hydroponics__Task *message)
{
  assert(message->base.descriptor == &hydroponics__task__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t hydroponics__task__pack
                     (const Hydroponics__Task *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &hydroponics__task__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t hydroponics__task__pack_to_buffer
                     (const Hydroponics__Task *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &hydroponics__task__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Hydroponics__Task *
       hydroponics__task__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Hydroponics__Task *)
     protobuf_c_message_unpack (&hydroponics__task__descriptor,
                                allocator, len, data);
}
void   hydroponics__task__free_unpacked
                     (Hydroponics__Task *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &hydroponics__task__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   hydroponics__config__init
                     (Hydroponics__Config         *message)
{
  static const Hydroponics__Config init_value = HYDROPONICS__CONFIG__INIT;
  *message = init_value;
}
size_t hydroponics__config__get_packed_size
                     (const Hydroponics__Config *message)
{
  assert(message->base.descriptor == &hydroponics__config__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t hydroponics__config__pack
                     (const Hydroponics__Config *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &hydroponics__config__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t hydroponics__config__pack_to_buffer
                     (const Hydroponics__Config *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &hydroponics__config__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Hydroponics__Config *
       hydroponics__config__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Hydroponics__Config *)
     protobuf_c_message_unpack (&hydroponics__config__descriptor,
                                allocator, len, data);
}
void   hydroponics__config__free_unpacked
                     (Hydroponics__Config *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &hydroponics__config__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor hydroponics__sampling__field_descriptors[5] =
{
  {
    "humidity",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Sampling, humidity),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "temperature",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Sampling, temperature),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "ec_probe",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Sampling, ec_probe),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "ec_probe_temp",
    4,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Sampling, ec_probe_temp),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "ph_probe",
    5,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Sampling, ph_probe),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned hydroponics__sampling__field_indices_by_name[] = {
  2,   /* field[2] = ec_probe */
  3,   /* field[3] = ec_probe_temp */
  0,   /* field[0] = humidity */
  4,   /* field[4] = ph_probe */
  1,   /* field[1] = temperature */
};
static const ProtobufCIntRange hydroponics__sampling__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 5 }
};
const ProtobufCMessageDescriptor hydroponics__sampling__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "hydroponics.Sampling",
  "Sampling",
  "Hydroponics__Sampling",
  "hydroponics",
  sizeof(Hydroponics__Sampling),
  5,
  hydroponics__sampling__field_descriptors,
  hydroponics__sampling__field_indices_by_name,
  1,  hydroponics__sampling__number_ranges,
  (ProtobufCMessageInit) hydroponics__sampling__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor hydroponics__controller__entry__pid__field_descriptors[4] =
{
  {
    "sampling",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller__Entry__Pid, sampling),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "p",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_FLOAT,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller__Entry__Pid, p),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "i",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_FLOAT,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller__Entry__Pid, i),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "d",
    4,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_FLOAT,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller__Entry__Pid, d),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned hydroponics__controller__entry__pid__field_indices_by_name[] = {
  3,   /* field[3] = d */
  2,   /* field[2] = i */
  1,   /* field[1] = p */
  0,   /* field[0] = sampling */
};
static const ProtobufCIntRange hydroponics__controller__entry__pid__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 4 }
};
const ProtobufCMessageDescriptor hydroponics__controller__entry__pid__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "hydroponics.Controller.Entry.Pid",
  "Pid",
  "Hydroponics__Controller__Entry__Pid",
  "hydroponics",
  sizeof(Hydroponics__Controller__Entry__Pid),
  4,
  hydroponics__controller__entry__pid__field_descriptors,
  hydroponics__controller__entry__pid__field_indices_by_name,
  1,  hydroponics__controller__entry__pid__number_ranges,
  (ProtobufCMessageInit) hydroponics__controller__entry__pid__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor hydroponics__controller__entry__field_descriptors[4] =
{
  {
    "target",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_FLOAT,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller__Entry, target),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "min",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_FLOAT,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller__Entry, min),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "max",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_FLOAT,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller__Entry, max),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "pid",
    4,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller__Entry, pid),
    &hydroponics__controller__entry__pid__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned hydroponics__controller__entry__field_indices_by_name[] = {
  2,   /* field[2] = max */
  1,   /* field[1] = min */
  3,   /* field[3] = pid */
  0,   /* field[0] = target */
};
static const ProtobufCIntRange hydroponics__controller__entry__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 4 }
};
const ProtobufCMessageDescriptor hydroponics__controller__entry__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "hydroponics.Controller.Entry",
  "Entry",
  "Hydroponics__Controller__Entry",
  "hydroponics",
  sizeof(Hydroponics__Controller__Entry),
  4,
  hydroponics__controller__entry__field_descriptors,
  hydroponics__controller__entry__field_indices_by_name,
  1,  hydroponics__controller__entry__number_ranges,
  (ProtobufCMessageInit) hydroponics__controller__entry__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor hydroponics__controller__field_descriptors[4] =
{
  {
    "eca",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller, eca),
    &hydroponics__controller__entry__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "pha",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller, pha),
    &hydroponics__controller__entry__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "ecb",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller, ecb),
    &hydroponics__controller__entry__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "phb",
    4,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Controller, phb),
    &hydroponics__controller__entry__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned hydroponics__controller__field_indices_by_name[] = {
  0,   /* field[0] = eca */
  2,   /* field[2] = ecb */
  1,   /* field[1] = pha */
  3,   /* field[3] = phb */
};
static const ProtobufCIntRange hydroponics__controller__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 4 }
};
const ProtobufCMessageDescriptor hydroponics__controller__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "hydroponics.Controller",
  "Controller",
  "Hydroponics__Controller",
  "hydroponics",
  sizeof(Hydroponics__Controller),
  4,
  hydroponics__controller__field_descriptors,
  hydroponics__controller__field_indices_by_name,
  1,  hydroponics__controller__number_ranges,
  (ProtobufCMessageInit) hydroponics__controller__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor hydroponics__task__cron__field_descriptors[2] =
{
  {
    "action",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_ENUM,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Task__Cron, action),
    &hydroponics__task__output_action__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "expression",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_STRING,
    offsetof(Hydroponics__Task__Cron, n_expression),
    offsetof(Hydroponics__Task__Cron, expression),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned hydroponics__task__cron__field_indices_by_name[] = {
  0,   /* field[0] = action */
  1,   /* field[1] = expression */
};
static const ProtobufCIntRange hydroponics__task__cron__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor hydroponics__task__cron__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "hydroponics.Task.Cron",
  "Cron",
  "Hydroponics__Task__Cron",
  "hydroponics",
  sizeof(Hydroponics__Task__Cron),
  2,
  hydroponics__task__cron__field_descriptors,
  hydroponics__task__cron__field_indices_by_name,
  1,  hydroponics__task__cron__number_ranges,
  (ProtobufCMessageInit) hydroponics__task__cron__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCEnumValue hydroponics__task__output__enum_values_by_number[16] =
{
  { "EXT_GPIO_A_0", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_A_0", 0 },
  { "EXT_GPIO_A_1", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_A_1", 1 },
  { "EXT_GPIO_A_2", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_A_2", 2 },
  { "EXT_GPIO_A_3", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_A_3", 3 },
  { "EXT_GPIO_A_4", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_A_4", 4 },
  { "EXT_GPIO_A_5", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_A_5", 5 },
  { "EXT_GPIO_A_6", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_A_6", 6 },
  { "EXT_GPIO_A_7", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_A_7", 7 },
  { "EXT_GPIO_B_0", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_B_0", 8 },
  { "EXT_GPIO_B_1", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_B_1", 9 },
  { "EXT_GPIO_B_2", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_B_2", 10 },
  { "EXT_GPIO_B_3", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_B_3", 11 },
  { "EXT_GPIO_B_4", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_B_4", 12 },
  { "EXT_GPIO_B_5", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_B_5", 13 },
  { "EXT_GPIO_B_6", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_B_6", 14 },
  { "EXT_GPIO_B_7", "HYDROPONICS__TASK__OUTPUT__EXT_GPIO_B_7", 15 },
};
static const ProtobufCIntRange hydroponics__task__output__value_ranges[] = {
{0, 0},{0, 16}
};
static const ProtobufCEnumValueIndex hydroponics__task__output__enum_values_by_name[16] =
{
  { "EXT_GPIO_A_0", 0 },
  { "EXT_GPIO_A_1", 1 },
  { "EXT_GPIO_A_2", 2 },
  { "EXT_GPIO_A_3", 3 },
  { "EXT_GPIO_A_4", 4 },
  { "EXT_GPIO_A_5", 5 },
  { "EXT_GPIO_A_6", 6 },
  { "EXT_GPIO_A_7", 7 },
  { "EXT_GPIO_B_0", 8 },
  { "EXT_GPIO_B_1", 9 },
  { "EXT_GPIO_B_2", 10 },
  { "EXT_GPIO_B_3", 11 },
  { "EXT_GPIO_B_4", 12 },
  { "EXT_GPIO_B_5", 13 },
  { "EXT_GPIO_B_6", 14 },
  { "EXT_GPIO_B_7", 15 },
};
const ProtobufCEnumDescriptor hydroponics__task__output__descriptor =
{
  PROTOBUF_C__ENUM_DESCRIPTOR_MAGIC,
  "hydroponics.Task.Output",
  "Output",
  "Hydroponics__Task__Output",
  "hydroponics",
  16,
  hydroponics__task__output__enum_values_by_number,
  16,
  hydroponics__task__output__enum_values_by_name,
  1,
  hydroponics__task__output__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
static const ProtobufCEnumValue hydroponics__task__output_action__enum_values_by_number[2] =
{
  { "OFF", "HYDROPONICS__TASK__OUTPUT_ACTION__OFF", 0 },
  { "ON", "HYDROPONICS__TASK__OUTPUT_ACTION__ON", 1 },
};
static const ProtobufCIntRange hydroponics__task__output_action__value_ranges[] = {
{0, 0},{0, 2}
};
static const ProtobufCEnumValueIndex hydroponics__task__output_action__enum_values_by_name[2] =
{
  { "OFF", 0 },
  { "ON", 1 },
};
const ProtobufCEnumDescriptor hydroponics__task__output_action__descriptor =
{
  PROTOBUF_C__ENUM_DESCRIPTOR_MAGIC,
  "hydroponics.Task.OutputAction",
  "OutputAction",
  "Hydroponics__Task__OutputAction",
  "hydroponics",
  2,
  hydroponics__task__output_action__enum_values_by_number,
  2,
  hydroponics__task__output_action__enum_values_by_name,
  1,
  hydroponics__task__output_action__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
static const ProtobufCFieldDescriptor hydroponics__task__field_descriptors[3] =
{
  {
    "name",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Task, name),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "output",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_ENUM,
    offsetof(Hydroponics__Task, n_output),
    offsetof(Hydroponics__Task, output),
    &hydroponics__task__output__descriptor,
    NULL,
    0 | PROTOBUF_C_FIELD_FLAG_PACKED,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "cron",
    3,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(Hydroponics__Task, n_cron),
    offsetof(Hydroponics__Task, cron),
    &hydroponics__task__cron__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned hydroponics__task__field_indices_by_name[] = {
  2,   /* field[2] = cron */
  0,   /* field[0] = name */
  1,   /* field[1] = output */
};
static const ProtobufCIntRange hydroponics__task__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor hydroponics__task__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "hydroponics.Task",
  "Task",
  "Hydroponics__Task",
  "hydroponics",
  sizeof(Hydroponics__Task),
  3,
  hydroponics__task__field_descriptors,
  hydroponics__task__field_indices_by_name,
  1,  hydroponics__task__number_ranges,
  (ProtobufCMessageInit) hydroponics__task__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor hydroponics__config__field_descriptors[3] =
{
  {
    "sampling",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Config, sampling),
    &hydroponics__sampling__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "controller",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Hydroponics__Config, controller),
    &hydroponics__controller__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "task",
    3,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(Hydroponics__Config, n_task),
    offsetof(Hydroponics__Config, task),
    &hydroponics__task__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned hydroponics__config__field_indices_by_name[] = {
  1,   /* field[1] = controller */
  0,   /* field[0] = sampling */
  2,   /* field[2] = task */
};
static const ProtobufCIntRange hydroponics__config__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor hydroponics__config__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "hydroponics.Config",
  "Config",
  "Hydroponics__Config",
  "hydroponics",
  sizeof(Hydroponics__Config),
  3,
  hydroponics__config__field_descriptors,
  hydroponics__config__field_indices_by_name,
  1,  hydroponics__config__number_ranges,
  (ProtobufCMessageInit) hydroponics__config__init,
  NULL,NULL,NULL    /* reserved[123] */
};
