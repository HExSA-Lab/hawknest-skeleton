.macro TEST_END
    .byte $80, $07 ; DUMP paravirt call
.endmacro

.segment "VECTORS"
    .word $0
    .word test
    .word $0

.export __HEADER__ : absolute = 1 ; Linker referenced
.segment "HEADER"
.byte "HKNS"

.segment "CODE"

