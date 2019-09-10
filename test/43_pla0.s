.include "test_hdr.inc"

; DESC: Basic pull from stack into A

test:
    lda #1
    pha
    lda #3
    pla ; should have #1 in A
    TEST_END
