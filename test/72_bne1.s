.include "test_hdr.inc"

; DESC: Branch if not equal works (not taken condition)

test:
    ldy #20
    sty $2000
    tya
    cmp $2000        ;; set zero flag
    bne add_another  ;; should not be taken, producing 20 in A
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
