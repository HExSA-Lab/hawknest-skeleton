.include "test_hdr.inc"

; DESC: ORA works

test:
    lda #2
    ora #1 ; should produce 3 in a
    TEST_END
