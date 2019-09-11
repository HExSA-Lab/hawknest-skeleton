.include "test_hdr.inc"

; DESC: Branch if positive works (not taken condition)

test:
    ldx #0
    dex
    bpl add_another  ;; should not be taken, producing ff (-1) in X
    jmp dont
add_another:
    inx
dont:
    nop
    TEST_END
