.include "test_hdr.inc"

; DESC: Branch if carry clear works (taken condition)

test:
    ldy #1
    clc              ;; carry is clear
    bcc add_another
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
