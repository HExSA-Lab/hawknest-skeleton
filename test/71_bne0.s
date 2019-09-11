.include "test_hdr.inc"

; DESC: Branch if not equal works (taken condition)

test:
    ldy #20
    sty $2000
    lda #22
    cmp $2000        ;; does not set zero flag
    bne add_another  ;; should be taken, producing 23 in A
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
