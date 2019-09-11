.include "test_hdr.inc"

; DESC: Basic rotate right of A

test:
    lda #8
    ror  ; should produce 4
    TEST_END
