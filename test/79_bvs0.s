.include "test_hdr.inc"

; DESC: Branch if overflow set works (taken condition)

test:
    lda #64
    adc #64    ;; sets overflow flag
    bvs set_it ;; should be taken, producing 42 in X
    jmp dont
set_it:
    ldx #42
dont:
    nop
    TEST_END
