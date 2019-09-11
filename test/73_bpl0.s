.include "test_hdr.inc"

; DESC: Branch if positive works (taken condition)

test:
    ldy #1
    iny
    tya
    bpl add_another  ;; should be taken, producing 3 in A
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
