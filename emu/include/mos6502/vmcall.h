#pragma once

#include <base.h>
#include <mos6502/mos6502.h>

// VMCALL numbers
#define VMCALL_ARGS  0
#define VMCALL_EXIT  1
#define VMCALL_OPEN  2
#define VMCALL_CLOSE 3
#define VMCALL_READ  4
#define VMCALL_WRITE 5
#define VMCALL_BREAK 6
#define VMCALL_DUMP  7

// Handles a VMCALL opcode
mos6502_step_result_t handle_vmcall (mos6502_t * cpu, uint8_t call_num);
