.include "test_hdr.inc"

; DESC: Pull from stack into processor status flags

test:
    lda #0
    pha
    plp
    TEST_END
