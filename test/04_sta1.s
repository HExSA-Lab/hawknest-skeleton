.include "test_hdr.inc"

; DESC: store accumulator to zero page works

test:
    lda #1 
    sta $0
    TEST_END
