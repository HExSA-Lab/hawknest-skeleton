.include "test_hdr.inc"

; DESC: Bitwise AND works with memory address

test:
    lda #3
    sta $2000
    lda #2
    and $2000
    TEST_END
