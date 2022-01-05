// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.25.0
// 	protoc        v3.14.0
// source: state.proto

package hydroponics

import (
	proto "github.com/golang/protobuf/proto"
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

// This is a compile-time assertion that a sufficiently up-to-date version
// of the legacy proto package is being used.
const _ = proto.ProtoPackageIsVersion4

// Task state that directly matches task.h eTaskState enumeration.
type StateTask_State int32

const (
	// A task is querying the state of itself, so must be running.
	StateTask_RUNNING StateTask_State = 0
	// The task being queried is in a read or pending ready list.
	StateTask_READY StateTask_State = 1
	// The task being queried is in the Blocked state.
	StateTask_BLOCKED StateTask_State = 2
	// The task being queried is in the Suspended state, or is in the Blocked state with an infinite time out.
	StateTask_SUSPENDED StateTask_State = 3
	// The task being queried has been deleted, but its TCB has not yet been freed.
	StateTask_DELETED StateTask_State = 4
)

// Enum value maps for StateTask_State.
var (
	StateTask_State_name = map[int32]string{
		0: "RUNNING",
		1: "READY",
		2: "BLOCKED",
		3: "SUSPENDED",
		4: "DELETED",
	}
	StateTask_State_value = map[string]int32{
		"RUNNING":   0,
		"READY":     1,
		"BLOCKED":   2,
		"SUSPENDED": 3,
		"DELETED":   4,
	}
)

func (x StateTask_State) Enum() *StateTask_State {
	p := new(StateTask_State)
	*p = x
	return p
}

func (x StateTask_State) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (StateTask_State) Descriptor() protoreflect.EnumDescriptor {
	return file_state_proto_enumTypes[0].Descriptor()
}

func (StateTask_State) Type() protoreflect.EnumType {
	return &file_state_proto_enumTypes[0]
}

func (x StateTask_State) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use StateTask_State.Descriptor instead.
func (StateTask_State) EnumDescriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{0, 0}
}

type StateTelemetry_Type int32

const (
	StateTelemetry_UNKNOWN     StateTelemetry_Type = 0
	StateTelemetry_TEMP_INDOOR StateTelemetry_Type = 1
	StateTelemetry_TEMP_PROBE  StateTelemetry_Type = 2
	StateTelemetry_HUMIDITY    StateTelemetry_Type = 3
	StateTelemetry_PRESSURE    StateTelemetry_Type = 4
	StateTelemetry_EC_A        StateTelemetry_Type = 5
	StateTelemetry_EC_B        StateTelemetry_Type = 6
	StateTelemetry_PH_A        StateTelemetry_Type = 7
	StateTelemetry_PH_B        StateTelemetry_Type = 8
	StateTelemetry_TANK_A      StateTelemetry_Type = 9
	StateTelemetry_TANK_B      StateTelemetry_Type = 10
)

// Enum value maps for StateTelemetry_Type.
var (
	StateTelemetry_Type_name = map[int32]string{
		0:  "UNKNOWN",
		1:  "TEMP_INDOOR",
		2:  "TEMP_PROBE",
		3:  "HUMIDITY",
		4:  "PRESSURE",
		5:  "EC_A",
		6:  "EC_B",
		7:  "PH_A",
		8:  "PH_B",
		9:  "TANK_A",
		10: "TANK_B",
	}
	StateTelemetry_Type_value = map[string]int32{
		"UNKNOWN":     0,
		"TEMP_INDOOR": 1,
		"TEMP_PROBE":  2,
		"HUMIDITY":    3,
		"PRESSURE":    4,
		"EC_A":        5,
		"EC_B":        6,
		"PH_A":        7,
		"PH_B":        8,
		"TANK_A":      9,
		"TANK_B":      10,
	}
)

func (x StateTelemetry_Type) Enum() *StateTelemetry_Type {
	p := new(StateTelemetry_Type)
	*p = x
	return p
}

func (x StateTelemetry_Type) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (StateTelemetry_Type) Descriptor() protoreflect.EnumDescriptor {
	return file_state_proto_enumTypes[1].Descriptor()
}

func (StateTelemetry_Type) Type() protoreflect.EnumType {
	return &file_state_proto_enumTypes[1]
}

func (x StateTelemetry_Type) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use StateTelemetry_Type.Descriptor instead.
func (StateTelemetry_Type) EnumDescriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{3, 0}
}

type StateTask struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Name      string          `protobuf:"bytes,1,opt,name=name,proto3" json:"name,omitempty"`
	State     StateTask_State `protobuf:"varint,2,opt,name=state,proto3,enum=hydroponics.StateTask_State" json:"state,omitempty"`
	Priority  uint32          `protobuf:"varint,3,opt,name=priority,proto3" json:"priority,omitempty"`
	Runtime   uint64          `protobuf:"varint,4,opt,name=runtime,proto3" json:"runtime,omitempty"`
	Stats     uint32          `protobuf:"varint,5,opt,name=stats,proto3" json:"stats,omitempty"`
	Highwater uint32          `protobuf:"varint,6,opt,name=highwater,proto3" json:"highwater,omitempty"`
}

func (x *StateTask) Reset() {
	*x = StateTask{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StateTask) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StateTask) ProtoMessage() {}

func (x *StateTask) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StateTask.ProtoReflect.Descriptor instead.
func (*StateTask) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{0}
}

func (x *StateTask) GetName() string {
	if x != nil {
		return x.Name
	}
	return ""
}

func (x *StateTask) GetState() StateTask_State {
	if x != nil {
		return x.State
	}
	return StateTask_RUNNING
}

func (x *StateTask) GetPriority() uint32 {
	if x != nil {
		return x.Priority
	}
	return 0
}

func (x *StateTask) GetRuntime() uint64 {
	if x != nil {
		return x.Runtime
	}
	return 0
}

func (x *StateTask) GetStats() uint32 {
	if x != nil {
		return x.Stats
	}
	return 0
}

func (x *StateTask) GetHighwater() uint32 {
	if x != nil {
		return x.Highwater
	}
	return 0
}

type StateTasks struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Task []*StateTask `protobuf:"bytes,1,rep,name=task,proto3" json:"task,omitempty"`
}

func (x *StateTasks) Reset() {
	*x = StateTasks{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StateTasks) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StateTasks) ProtoMessage() {}

func (x *StateTasks) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StateTasks.ProtoReflect.Descriptor instead.
func (*StateTasks) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{1}
}

func (x *StateTasks) GetTask() []*StateTask {
	if x != nil {
		return x.Task
	}
	return nil
}

type StateMemory struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	MinFree uint32 `protobuf:"varint,1,opt,name=min_free,json=minFree,proto3" json:"min_free,omitempty"`
	Free    uint32 `protobuf:"varint,2,opt,name=free,proto3" json:"free,omitempty"`
}

func (x *StateMemory) Reset() {
	*x = StateMemory{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StateMemory) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StateMemory) ProtoMessage() {}

func (x *StateMemory) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StateMemory.ProtoReflect.Descriptor instead.
func (*StateMemory) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{2}
}

func (x *StateMemory) GetMinFree() uint32 {
	if x != nil {
		return x.MinFree
	}
	return 0
}

func (x *StateMemory) GetFree() uint32 {
	if x != nil {
		return x.Free
	}
	return 0
}

type StateTelemetry struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	TempIndoor float32 `protobuf:"fixed32,1,opt,name=temp_indoor,json=tempIndoor,proto3" json:"temp_indoor,omitempty"`
	TempProbe  float32 `protobuf:"fixed32,2,opt,name=temp_probe,json=tempProbe,proto3" json:"temp_probe,omitempty"`
	Humidity   float32 `protobuf:"fixed32,3,opt,name=humidity,proto3" json:"humidity,omitempty"`
	Pressure   float32 `protobuf:"fixed32,4,opt,name=pressure,proto3" json:"pressure,omitempty"`
	EcA        float32 `protobuf:"fixed32,5,opt,name=ec_a,json=ecA,proto3" json:"ec_a,omitempty"`
	EcB        float32 `protobuf:"fixed32,6,opt,name=ec_b,json=ecB,proto3" json:"ec_b,omitempty"`
	PhA        float32 `protobuf:"fixed32,7,opt,name=ph_a,json=phA,proto3" json:"ph_a,omitempty"`
	PhB        float32 `protobuf:"fixed32,8,opt,name=ph_b,json=phB,proto3" json:"ph_b,omitempty"`
	TankA      float32 `protobuf:"fixed32,9,opt,name=tank_a,json=tankA,proto3" json:"tank_a,omitempty"`
	TankB      float32 `protobuf:"fixed32,10,opt,name=tank_b,json=tankB,proto3" json:"tank_b,omitempty"`
}

func (x *StateTelemetry) Reset() {
	*x = StateTelemetry{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StateTelemetry) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StateTelemetry) ProtoMessage() {}

func (x *StateTelemetry) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StateTelemetry.ProtoReflect.Descriptor instead.
func (*StateTelemetry) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{3}
}

func (x *StateTelemetry) GetTempIndoor() float32 {
	if x != nil {
		return x.TempIndoor
	}
	return 0
}

func (x *StateTelemetry) GetTempProbe() float32 {
	if x != nil {
		return x.TempProbe
	}
	return 0
}

func (x *StateTelemetry) GetHumidity() float32 {
	if x != nil {
		return x.Humidity
	}
	return 0
}

func (x *StateTelemetry) GetPressure() float32 {
	if x != nil {
		return x.Pressure
	}
	return 0
}

func (x *StateTelemetry) GetEcA() float32 {
	if x != nil {
		return x.EcA
	}
	return 0
}

func (x *StateTelemetry) GetEcB() float32 {
	if x != nil {
		return x.EcB
	}
	return 0
}

func (x *StateTelemetry) GetPhA() float32 {
	if x != nil {
		return x.PhA
	}
	return 0
}

func (x *StateTelemetry) GetPhB() float32 {
	if x != nil {
		return x.PhB
	}
	return 0
}

func (x *StateTelemetry) GetTankA() float32 {
	if x != nil {
		return x.TankA
	}
	return 0
}

func (x *StateTelemetry) GetTankB() float32 {
	if x != nil {
		return x.TankB
	}
	return 0
}

type StateOutput struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Output []Output    `protobuf:"varint,1,rep,packed,name=output,proto3,enum=hydroponics.Output" json:"output,omitempty"`
	State  OutputState `protobuf:"varint,2,opt,name=state,proto3,enum=hydroponics.OutputState" json:"state,omitempty"`
}

func (x *StateOutput) Reset() {
	*x = StateOutput{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[4]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StateOutput) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StateOutput) ProtoMessage() {}

func (x *StateOutput) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[4]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StateOutput.ProtoReflect.Descriptor instead.
func (*StateOutput) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{4}
}

func (x *StateOutput) GetOutput() []Output {
	if x != nil {
		return x.Output
	}
	return nil
}

func (x *StateOutput) GetState() OutputState {
	if x != nil {
		return x.State
	}
	return OutputState_OFF
}

type StateOutputs struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Output []*StateOutput `protobuf:"bytes,1,rep,name=output,proto3" json:"output,omitempty"`
}

func (x *StateOutputs) Reset() {
	*x = StateOutputs{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[5]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StateOutputs) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StateOutputs) ProtoMessage() {}

func (x *StateOutputs) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[5]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StateOutputs.ProtoReflect.Descriptor instead.
func (*StateOutputs) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{5}
}

func (x *StateOutputs) GetOutput() []*StateOutput {
	if x != nil {
		return x.Output
	}
	return nil
}

type StateReboot struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields
}

func (x *StateReboot) Reset() {
	*x = StateReboot{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[6]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StateReboot) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StateReboot) ProtoMessage() {}

func (x *StateReboot) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[6]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StateReboot.ProtoReflect.Descriptor instead.
func (*StateReboot) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{6}
}

type State struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Timestamp uint64 `protobuf:"varint,1,opt,name=timestamp,proto3" json:"timestamp,omitempty"`
	// Types that are assignable to State:
	//	*State_Telemetry
	//	*State_Tasks
	//	*State_Memory
	//	*State_Outputs
	//	*State_Reboot
	State isState_State `protobuf_oneof:"state"`
}

func (x *State) Reset() {
	*x = State{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[7]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *State) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*State) ProtoMessage() {}

func (x *State) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[7]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use State.ProtoReflect.Descriptor instead.
func (*State) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{7}
}

func (x *State) GetTimestamp() uint64 {
	if x != nil {
		return x.Timestamp
	}
	return 0
}

func (m *State) GetState() isState_State {
	if m != nil {
		return m.State
	}
	return nil
}

func (x *State) GetTelemetry() *StateTelemetry {
	if x, ok := x.GetState().(*State_Telemetry); ok {
		return x.Telemetry
	}
	return nil
}

func (x *State) GetTasks() *StateTasks {
	if x, ok := x.GetState().(*State_Tasks); ok {
		return x.Tasks
	}
	return nil
}

func (x *State) GetMemory() *StateMemory {
	if x, ok := x.GetState().(*State_Memory); ok {
		return x.Memory
	}
	return nil
}

func (x *State) GetOutputs() *StateOutputs {
	if x, ok := x.GetState().(*State_Outputs); ok {
		return x.Outputs
	}
	return nil
}

func (x *State) GetReboot() *StateReboot {
	if x, ok := x.GetState().(*State_Reboot); ok {
		return x.Reboot
	}
	return nil
}

type isState_State interface {
	isState_State()
}

type State_Telemetry struct {
	Telemetry *StateTelemetry `protobuf:"bytes,2,opt,name=telemetry,proto3,oneof"`
}

type State_Tasks struct {
	Tasks *StateTasks `protobuf:"bytes,3,opt,name=tasks,proto3,oneof"`
}

type State_Memory struct {
	Memory *StateMemory `protobuf:"bytes,4,opt,name=memory,proto3,oneof"`
}

type State_Outputs struct {
	Outputs *StateOutputs `protobuf:"bytes,5,opt,name=outputs,proto3,oneof"`
}

type State_Reboot struct {
	Reboot *StateReboot `protobuf:"bytes,6,opt,name=reboot,proto3,oneof"`
}

func (*State_Telemetry) isState_State() {}

func (*State_Tasks) isState_State() {}

func (*State_Memory) isState_State() {}

func (*State_Outputs) isState_State() {}

func (*State_Reboot) isState_State() {}

type States struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	State []*State `protobuf:"bytes,1,rep,name=state,proto3" json:"state,omitempty"`
}

func (x *States) Reset() {
	*x = States{}
	if protoimpl.UnsafeEnabled {
		mi := &file_state_proto_msgTypes[8]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *States) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*States) ProtoMessage() {}

func (x *States) ProtoReflect() protoreflect.Message {
	mi := &file_state_proto_msgTypes[8]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use States.ProtoReflect.Descriptor instead.
func (*States) Descriptor() ([]byte, []int) {
	return file_state_proto_rawDescGZIP(), []int{8}
}

func (x *States) GetState() []*State {
	if x != nil {
		return x.State
	}
	return nil
}

var File_state_proto protoreflect.FileDescriptor

var file_state_proto_rawDesc = []byte{
	0x0a, 0x0b, 0x73, 0x74, 0x61, 0x74, 0x65, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x0b, 0x68,
	0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x1a, 0x0c, 0x63, 0x6f, 0x6e, 0x66,
	0x69, 0x67, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x22, 0x87, 0x02, 0x0a, 0x09, 0x53, 0x74, 0x61,
	0x74, 0x65, 0x54, 0x61, 0x73, 0x6b, 0x12, 0x12, 0x0a, 0x04, 0x6e, 0x61, 0x6d, 0x65, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x6e, 0x61, 0x6d, 0x65, 0x12, 0x32, 0x0a, 0x05, 0x73, 0x74,
	0x61, 0x74, 0x65, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0e, 0x32, 0x1c, 0x2e, 0x68, 0x79, 0x64, 0x72,
	0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x65, 0x54, 0x61, 0x73,
	0x6b, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x65, 0x52, 0x05, 0x73, 0x74, 0x61, 0x74, 0x65, 0x12, 0x1a,
	0x0a, 0x08, 0x70, 0x72, 0x69, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x18, 0x03, 0x20, 0x01, 0x28, 0x0d,
	0x52, 0x08, 0x70, 0x72, 0x69, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x12, 0x18, 0x0a, 0x07, 0x72, 0x75,
	0x6e, 0x74, 0x69, 0x6d, 0x65, 0x18, 0x04, 0x20, 0x01, 0x28, 0x04, 0x52, 0x07, 0x72, 0x75, 0x6e,
	0x74, 0x69, 0x6d, 0x65, 0x12, 0x14, 0x0a, 0x05, 0x73, 0x74, 0x61, 0x74, 0x73, 0x18, 0x05, 0x20,
	0x01, 0x28, 0x0d, 0x52, 0x05, 0x73, 0x74, 0x61, 0x74, 0x73, 0x12, 0x1c, 0x0a, 0x09, 0x68, 0x69,
	0x67, 0x68, 0x77, 0x61, 0x74, 0x65, 0x72, 0x18, 0x06, 0x20, 0x01, 0x28, 0x0d, 0x52, 0x09, 0x68,
	0x69, 0x67, 0x68, 0x77, 0x61, 0x74, 0x65, 0x72, 0x22, 0x48, 0x0a, 0x05, 0x53, 0x74, 0x61, 0x74,
	0x65, 0x12, 0x0b, 0x0a, 0x07, 0x52, 0x55, 0x4e, 0x4e, 0x49, 0x4e, 0x47, 0x10, 0x00, 0x12, 0x09,
	0x0a, 0x05, 0x52, 0x45, 0x41, 0x44, 0x59, 0x10, 0x01, 0x12, 0x0b, 0x0a, 0x07, 0x42, 0x4c, 0x4f,
	0x43, 0x4b, 0x45, 0x44, 0x10, 0x02, 0x12, 0x0d, 0x0a, 0x09, 0x53, 0x55, 0x53, 0x50, 0x45, 0x4e,
	0x44, 0x45, 0x44, 0x10, 0x03, 0x12, 0x0b, 0x0a, 0x07, 0x44, 0x45, 0x4c, 0x45, 0x54, 0x45, 0x44,
	0x10, 0x04, 0x22, 0x38, 0x0a, 0x0a, 0x53, 0x74, 0x61, 0x74, 0x65, 0x54, 0x61, 0x73, 0x6b, 0x73,
	0x12, 0x2a, 0x0a, 0x04, 0x74, 0x61, 0x73, 0x6b, 0x18, 0x01, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x16,
	0x2e, 0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e, 0x53, 0x74, 0x61,
	0x74, 0x65, 0x54, 0x61, 0x73, 0x6b, 0x52, 0x04, 0x74, 0x61, 0x73, 0x6b, 0x22, 0x3c, 0x0a, 0x0b,
	0x53, 0x74, 0x61, 0x74, 0x65, 0x4d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x12, 0x19, 0x0a, 0x08, 0x6d,
	0x69, 0x6e, 0x5f, 0x66, 0x72, 0x65, 0x65, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0d, 0x52, 0x07, 0x6d,
	0x69, 0x6e, 0x46, 0x72, 0x65, 0x65, 0x12, 0x12, 0x0a, 0x04, 0x66, 0x72, 0x65, 0x65, 0x18, 0x02,
	0x20, 0x01, 0x28, 0x0d, 0x52, 0x04, 0x66, 0x72, 0x65, 0x65, 0x22, 0x95, 0x03, 0x0a, 0x0e, 0x53,
	0x74, 0x61, 0x74, 0x65, 0x54, 0x65, 0x6c, 0x65, 0x6d, 0x65, 0x74, 0x72, 0x79, 0x12, 0x1f, 0x0a,
	0x0b, 0x74, 0x65, 0x6d, 0x70, 0x5f, 0x69, 0x6e, 0x64, 0x6f, 0x6f, 0x72, 0x18, 0x01, 0x20, 0x01,
	0x28, 0x02, 0x52, 0x0a, 0x74, 0x65, 0x6d, 0x70, 0x49, 0x6e, 0x64, 0x6f, 0x6f, 0x72, 0x12, 0x1d,
	0x0a, 0x0a, 0x74, 0x65, 0x6d, 0x70, 0x5f, 0x70, 0x72, 0x6f, 0x62, 0x65, 0x18, 0x02, 0x20, 0x01,
	0x28, 0x02, 0x52, 0x09, 0x74, 0x65, 0x6d, 0x70, 0x50, 0x72, 0x6f, 0x62, 0x65, 0x12, 0x1a, 0x0a,
	0x08, 0x68, 0x75, 0x6d, 0x69, 0x64, 0x69, 0x74, 0x79, 0x18, 0x03, 0x20, 0x01, 0x28, 0x02, 0x52,
	0x08, 0x68, 0x75, 0x6d, 0x69, 0x64, 0x69, 0x74, 0x79, 0x12, 0x1a, 0x0a, 0x08, 0x70, 0x72, 0x65,
	0x73, 0x73, 0x75, 0x72, 0x65, 0x18, 0x04, 0x20, 0x01, 0x28, 0x02, 0x52, 0x08, 0x70, 0x72, 0x65,
	0x73, 0x73, 0x75, 0x72, 0x65, 0x12, 0x11, 0x0a, 0x04, 0x65, 0x63, 0x5f, 0x61, 0x18, 0x05, 0x20,
	0x01, 0x28, 0x02, 0x52, 0x03, 0x65, 0x63, 0x41, 0x12, 0x11, 0x0a, 0x04, 0x65, 0x63, 0x5f, 0x62,
	0x18, 0x06, 0x20, 0x01, 0x28, 0x02, 0x52, 0x03, 0x65, 0x63, 0x42, 0x12, 0x11, 0x0a, 0x04, 0x70,
	0x68, 0x5f, 0x61, 0x18, 0x07, 0x20, 0x01, 0x28, 0x02, 0x52, 0x03, 0x70, 0x68, 0x41, 0x12, 0x11,
	0x0a, 0x04, 0x70, 0x68, 0x5f, 0x62, 0x18, 0x08, 0x20, 0x01, 0x28, 0x02, 0x52, 0x03, 0x70, 0x68,
	0x42, 0x12, 0x15, 0x0a, 0x06, 0x74, 0x61, 0x6e, 0x6b, 0x5f, 0x61, 0x18, 0x09, 0x20, 0x01, 0x28,
	0x02, 0x52, 0x05, 0x74, 0x61, 0x6e, 0x6b, 0x41, 0x12, 0x15, 0x0a, 0x06, 0x74, 0x61, 0x6e, 0x6b,
	0x5f, 0x62, 0x18, 0x0a, 0x20, 0x01, 0x28, 0x02, 0x52, 0x05, 0x74, 0x61, 0x6e, 0x6b, 0x42, 0x22,
	0x90, 0x01, 0x0a, 0x04, 0x54, 0x79, 0x70, 0x65, 0x12, 0x0b, 0x0a, 0x07, 0x55, 0x4e, 0x4b, 0x4e,
	0x4f, 0x57, 0x4e, 0x10, 0x00, 0x12, 0x0f, 0x0a, 0x0b, 0x54, 0x45, 0x4d, 0x50, 0x5f, 0x49, 0x4e,
	0x44, 0x4f, 0x4f, 0x52, 0x10, 0x01, 0x12, 0x0e, 0x0a, 0x0a, 0x54, 0x45, 0x4d, 0x50, 0x5f, 0x50,
	0x52, 0x4f, 0x42, 0x45, 0x10, 0x02, 0x12, 0x0c, 0x0a, 0x08, 0x48, 0x55, 0x4d, 0x49, 0x44, 0x49,
	0x54, 0x59, 0x10, 0x03, 0x12, 0x0c, 0x0a, 0x08, 0x50, 0x52, 0x45, 0x53, 0x53, 0x55, 0x52, 0x45,
	0x10, 0x04, 0x12, 0x08, 0x0a, 0x04, 0x45, 0x43, 0x5f, 0x41, 0x10, 0x05, 0x12, 0x08, 0x0a, 0x04,
	0x45, 0x43, 0x5f, 0x42, 0x10, 0x06, 0x12, 0x08, 0x0a, 0x04, 0x50, 0x48, 0x5f, 0x41, 0x10, 0x07,
	0x12, 0x08, 0x0a, 0x04, 0x50, 0x48, 0x5f, 0x42, 0x10, 0x08, 0x12, 0x0a, 0x0a, 0x06, 0x54, 0x41,
	0x4e, 0x4b, 0x5f, 0x41, 0x10, 0x09, 0x12, 0x0a, 0x0a, 0x06, 0x54, 0x41, 0x4e, 0x4b, 0x5f, 0x42,
	0x10, 0x0a, 0x22, 0x6a, 0x0a, 0x0b, 0x53, 0x74, 0x61, 0x74, 0x65, 0x4f, 0x75, 0x74, 0x70, 0x75,
	0x74, 0x12, 0x2b, 0x0a, 0x06, 0x6f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x18, 0x01, 0x20, 0x03, 0x28,
	0x0e, 0x32, 0x13, 0x2e, 0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e,
	0x4f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x52, 0x06, 0x6f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x12, 0x2e,
	0x0a, 0x05, 0x73, 0x74, 0x61, 0x74, 0x65, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0e, 0x32, 0x18, 0x2e,
	0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e, 0x4f, 0x75, 0x74, 0x70,
	0x75, 0x74, 0x53, 0x74, 0x61, 0x74, 0x65, 0x52, 0x05, 0x73, 0x74, 0x61, 0x74, 0x65, 0x22, 0x40,
	0x0a, 0x0c, 0x53, 0x74, 0x61, 0x74, 0x65, 0x4f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x73, 0x12, 0x30,
	0x0a, 0x06, 0x6f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x18, 0x01, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x18,
	0x2e, 0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e, 0x53, 0x74, 0x61,
	0x74, 0x65, 0x4f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x52, 0x06, 0x6f, 0x75, 0x74, 0x70, 0x75, 0x74,
	0x22, 0x0d, 0x0a, 0x0b, 0x53, 0x74, 0x61, 0x74, 0x65, 0x52, 0x65, 0x62, 0x6f, 0x6f, 0x74, 0x22,
	0xbb, 0x02, 0x0a, 0x05, 0x53, 0x74, 0x61, 0x74, 0x65, 0x12, 0x1c, 0x0a, 0x09, 0x74, 0x69, 0x6d,
	0x65, 0x73, 0x74, 0x61, 0x6d, 0x70, 0x18, 0x01, 0x20, 0x01, 0x28, 0x04, 0x52, 0x09, 0x74, 0x69,
	0x6d, 0x65, 0x73, 0x74, 0x61, 0x6d, 0x70, 0x12, 0x3b, 0x0a, 0x09, 0x74, 0x65, 0x6c, 0x65, 0x6d,
	0x65, 0x74, 0x72, 0x79, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x1b, 0x2e, 0x68, 0x79, 0x64,
	0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x65, 0x54, 0x65,
	0x6c, 0x65, 0x6d, 0x65, 0x74, 0x72, 0x79, 0x48, 0x00, 0x52, 0x09, 0x74, 0x65, 0x6c, 0x65, 0x6d,
	0x65, 0x74, 0x72, 0x79, 0x12, 0x2f, 0x0a, 0x05, 0x74, 0x61, 0x73, 0x6b, 0x73, 0x18, 0x03, 0x20,
	0x01, 0x28, 0x0b, 0x32, 0x17, 0x2e, 0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63,
	0x73, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x65, 0x54, 0x61, 0x73, 0x6b, 0x73, 0x48, 0x00, 0x52, 0x05,
	0x74, 0x61, 0x73, 0x6b, 0x73, 0x12, 0x32, 0x0a, 0x06, 0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x18,
	0x04, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x18, 0x2e, 0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e,
	0x69, 0x63, 0x73, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x65, 0x4d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x48,
	0x00, 0x52, 0x06, 0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x12, 0x35, 0x0a, 0x07, 0x6f, 0x75, 0x74,
	0x70, 0x75, 0x74, 0x73, 0x18, 0x05, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x19, 0x2e, 0x68, 0x79, 0x64,
	0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x65, 0x4f, 0x75,
	0x74, 0x70, 0x75, 0x74, 0x73, 0x48, 0x00, 0x52, 0x07, 0x6f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x73,
	0x12, 0x32, 0x0a, 0x06, 0x72, 0x65, 0x62, 0x6f, 0x6f, 0x74, 0x18, 0x06, 0x20, 0x01, 0x28, 0x0b,
	0x32, 0x18, 0x2e, 0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e, 0x53,
	0x74, 0x61, 0x74, 0x65, 0x52, 0x65, 0x62, 0x6f, 0x6f, 0x74, 0x48, 0x00, 0x52, 0x06, 0x72, 0x65,
	0x62, 0x6f, 0x6f, 0x74, 0x42, 0x07, 0x0a, 0x05, 0x73, 0x74, 0x61, 0x74, 0x65, 0x22, 0x32, 0x0a,
	0x06, 0x53, 0x74, 0x61, 0x74, 0x65, 0x73, 0x12, 0x28, 0x0a, 0x05, 0x73, 0x74, 0x61, 0x74, 0x65,
	0x18, 0x01, 0x20, 0x03, 0x28, 0x0b, 0x32, 0x12, 0x2e, 0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f,
	0x6e, 0x69, 0x63, 0x73, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x65, 0x52, 0x05, 0x73, 0x74, 0x61, 0x74,
	0x65, 0x42, 0x43, 0x0a, 0x1d, 0x70, 0x74, 0x2e, 0x73, 0x6f, 0x62, 0x72, 0x69, 0x6e, 0x68, 0x6f,
	0x2e, 0x68, 0x79, 0x64, 0x72, 0x6f, 0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x2e, 0x70, 0x72, 0x6f,
	0x74, 0x6f, 0x50, 0x01, 0x5a, 0x20, 0x67, 0x69, 0x74, 0x68, 0x75, 0x62, 0x2e, 0x63, 0x6f, 0x6d,
	0x2f, 0x63, 0x73, 0x6f, 0x62, 0x72, 0x69, 0x6e, 0x68, 0x6f, 0x2f, 0x68, 0x79, 0x64, 0x72, 0x6f,
	0x70, 0x6f, 0x6e, 0x69, 0x63, 0x73, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_state_proto_rawDescOnce sync.Once
	file_state_proto_rawDescData = file_state_proto_rawDesc
)

func file_state_proto_rawDescGZIP() []byte {
	file_state_proto_rawDescOnce.Do(func() {
		file_state_proto_rawDescData = protoimpl.X.CompressGZIP(file_state_proto_rawDescData)
	})
	return file_state_proto_rawDescData
}

var file_state_proto_enumTypes = make([]protoimpl.EnumInfo, 2)
var file_state_proto_msgTypes = make([]protoimpl.MessageInfo, 9)
var file_state_proto_goTypes = []interface{}{
	(StateTask_State)(0),     // 0: hydroponics.StateTask.State
	(StateTelemetry_Type)(0), // 1: hydroponics.StateTelemetry.Type
	(*StateTask)(nil),        // 2: hydroponics.StateTask
	(*StateTasks)(nil),       // 3: hydroponics.StateTasks
	(*StateMemory)(nil),      // 4: hydroponics.StateMemory
	(*StateTelemetry)(nil),   // 5: hydroponics.StateTelemetry
	(*StateOutput)(nil),      // 6: hydroponics.StateOutput
	(*StateOutputs)(nil),     // 7: hydroponics.StateOutputs
	(*StateReboot)(nil),      // 8: hydroponics.StateReboot
	(*State)(nil),            // 9: hydroponics.State
	(*States)(nil),           // 10: hydroponics.States
	(Output)(0),              // 11: hydroponics.Output
	(OutputState)(0),         // 12: hydroponics.OutputState
}
var file_state_proto_depIdxs = []int32{
	0,  // 0: hydroponics.StateTask.state:type_name -> hydroponics.StateTask.State
	2,  // 1: hydroponics.StateTasks.task:type_name -> hydroponics.StateTask
	11, // 2: hydroponics.StateOutput.output:type_name -> hydroponics.Output
	12, // 3: hydroponics.StateOutput.state:type_name -> hydroponics.OutputState
	6,  // 4: hydroponics.StateOutputs.output:type_name -> hydroponics.StateOutput
	5,  // 5: hydroponics.State.telemetry:type_name -> hydroponics.StateTelemetry
	3,  // 6: hydroponics.State.tasks:type_name -> hydroponics.StateTasks
	4,  // 7: hydroponics.State.memory:type_name -> hydroponics.StateMemory
	7,  // 8: hydroponics.State.outputs:type_name -> hydroponics.StateOutputs
	8,  // 9: hydroponics.State.reboot:type_name -> hydroponics.StateReboot
	9,  // 10: hydroponics.States.state:type_name -> hydroponics.State
	11, // [11:11] is the sub-list for method output_type
	11, // [11:11] is the sub-list for method input_type
	11, // [11:11] is the sub-list for extension type_name
	11, // [11:11] is the sub-list for extension extendee
	0,  // [0:11] is the sub-list for field type_name
}

func init() { file_state_proto_init() }
func file_state_proto_init() {
	if File_state_proto != nil {
		return
	}
	file_config_proto_init()
	if !protoimpl.UnsafeEnabled {
		file_state_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StateTask); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_state_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StateTasks); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_state_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StateMemory); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_state_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StateTelemetry); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_state_proto_msgTypes[4].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StateOutput); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_state_proto_msgTypes[5].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StateOutputs); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_state_proto_msgTypes[6].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StateReboot); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_state_proto_msgTypes[7].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*State); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_state_proto_msgTypes[8].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*States); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	file_state_proto_msgTypes[7].OneofWrappers = []interface{}{
		(*State_Telemetry)(nil),
		(*State_Tasks)(nil),
		(*State_Memory)(nil),
		(*State_Outputs)(nil),
		(*State_Reboot)(nil),
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_state_proto_rawDesc,
			NumEnums:      2,
			NumMessages:   9,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_state_proto_goTypes,
		DependencyIndexes: file_state_proto_depIdxs,
		EnumInfos:         file_state_proto_enumTypes,
		MessageInfos:      file_state_proto_msgTypes,
	}.Build()
	File_state_proto = out.File
	file_state_proto_rawDesc = nil
	file_state_proto_goTypes = nil
	file_state_proto_depIdxs = nil
}
