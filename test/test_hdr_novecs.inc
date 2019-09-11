.macro TEST_END
    .byte $80, $07 ; DUMP paravirt call
.endmacro

.export __HEADER__ : absolute = 1 ; Linker referenced
.segment "HEADER"
.byte "HKNS"

.segment "CODE"

