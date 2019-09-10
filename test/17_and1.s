.include "test_hdr.inc"

; DESC: Bitwise AND works with immediate

test:
    lda #3
    and #2
    TEST_END
