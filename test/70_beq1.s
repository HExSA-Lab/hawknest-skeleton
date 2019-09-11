.include "test_hdr.inc"

; DESC: Branch if equal works (not taken condition)

test:
    ldy #20
    sty $2000
    lda #22
    cmp $2000        ;; does not set zero flag
    beq add_another  ;; should not be taken, producing 22 in A
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
