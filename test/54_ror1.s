.include "test_hdr.inc"

; DESC: Rotate right of A with wraparound and flag set

test:
    lda #1
    ror  ; should produce 0, setting both carry and zero flags
    TEST_END
