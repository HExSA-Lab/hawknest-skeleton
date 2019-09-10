.include "test_hdr.inc"

; DESC: Add with zero result works (zero flag set)

test:
    lda #<-2
    sta $2000
    lda #2
    adc $2000
    TEST_END
