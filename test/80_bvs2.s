.include "test_hdr.inc"

; DESC: Branch if overflow set works (not taken condition)

test:
    lda #64
    adc #1    ;; does not set overflow flag
    bvs set_it ;; should not be taken, producing 0 in X
    jmp dont
set_it:
    ldx #42
dont:
    nop
    TEST_END
