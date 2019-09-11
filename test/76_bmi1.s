.include "test_hdr.inc"

; DESC: Branch if minus works (not taken condition)

test:
    ldx #0
    dex
    inx
    bmi add_another ;; should not be taken, producing 0 in X
    jmp dont
add_another:
    ldx #42
dont:
    nop
    TEST_END
