.include "test_hdr.inc"

; DESC: Add with negative result works (negative flag is set)

test:
    lda #<-2
    sta $2000
    lda #0
    adc $2000
    TEST_END
