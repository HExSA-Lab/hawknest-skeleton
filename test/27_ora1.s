.include "test_hdr.inc"

; DESC: ORA works with absolute addressing

test:
    lda #2
    sta $2000
    lda #1
    ora $2000 ; should produce 3 at $2000
    TEST_END
