.include "test_hdr.inc"

; DESC: exclusive OR using abs Y indexing

test:
    lda #7
    sta $303
    ldy #3
    lda #2
    eor $300,Y ; should produce 7 in A
    TEST_END
