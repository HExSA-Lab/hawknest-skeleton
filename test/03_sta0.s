.include "test_hdr.inc"

; DESC: store accumulator works

test:
    lda #1 
    sta $2000
    TEST_END
