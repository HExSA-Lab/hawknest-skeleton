.include "test_hdr.inc"

; DESC: Branch if overflow clear works (taken condition)

test:
    lda #64
    adc #1     ;; overflow flag not set
    bvc set_it ;; should be taken, producing 42 in X
    jmp dont
set_it:
    ldx #42
dont:
    nop
    TEST_END
