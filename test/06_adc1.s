.include "test_hdr.inc"

; DESC: Add with overflow works (overflow flag is set)

test:
    lda #254
    sta $2000
    lda #20
    adc $2000
    TEST_END
