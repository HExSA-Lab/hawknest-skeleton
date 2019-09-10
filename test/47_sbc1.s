.include "test_hdr.inc"

; DESC: Subtract with carry (testing correct borrowing semantics)

test:
    lda #2
    sta $2000
    lda #3
    sbc $2000 ; should produce 0 (A - M - (1 - C))
    TEST_END
