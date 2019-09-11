.include "test_hdr.inc"

; DESC: Branch if overflow clear works (not taken condition)

test:
    lda #64
    adc #64    ;; sets overflow flag
    bvc set_it ;; should not be taken, producing 0 in X
    jmp dont
set_it:
    ldx #42
dont:
    nop
    TEST_END
