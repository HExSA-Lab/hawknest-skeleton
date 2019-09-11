.include "test_hdr.inc"

; DESC: exclusive OR

test:
    lda #7
    sta $300
    lda #2
    eor $300 ; should produce 7 in A
    TEST_END
