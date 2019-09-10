EMU_SRC += mos6502/vmcall.c mos6502/mos6502-common.c

ifndef REFERENCE
EMU_SRC += mos6502/mos6502-skeleton.c
else
EMU_SRC += mos6502/mos6502.c
endif
