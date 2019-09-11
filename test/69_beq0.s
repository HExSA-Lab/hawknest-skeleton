.include "test_hdr.inc"

; DESC: Branch if equal works (taken condition)

test:
    ldy #20
    sty $2000
    tya
    cmp $2000        ;; sets zero flag
    beq add_another  ;; should be taken, producing 21 in A
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
