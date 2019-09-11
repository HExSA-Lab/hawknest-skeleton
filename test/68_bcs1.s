.include "test_hdr.inc"

; DESC: Branch if carry set works (not taken condition)

test:
    ldy #1
    clc              ;; carry is cleared
    bcs add_another  ;; should not be taken
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
