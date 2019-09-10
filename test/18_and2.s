.include "test_hdr.inc"

; DESC: Bitwise AND works with zero page

test:
    lda #3
    sta $20
    lda #2
    and $20
    TEST_END
