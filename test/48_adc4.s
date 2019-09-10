.include "test_hdr.inc"

; DESC: Add with overflow

test:
    lda #64
    sta $2000
    lda #64
    adc $2000 ; should set overflow flag
    TEST_END
