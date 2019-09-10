.include "test_hdr.inc"

; DESC: Basic add with carry works

test:
    lda #1 
    sta $2000
    adc $2000
    TEST_END
