.include "test_hdr.inc"

; DESC: Branch if minus works (taken condition)

test:
    ldx #0
    dex
    bmi add_another  ;; should not be taken, producing 0 in X
    jmp dont
add_another:
    inx
dont:
    nop
    TEST_END
