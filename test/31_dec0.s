.include "test_hdr.inc"

; DESC: Basic decrement works

test:
    lda #2
    sta $2000
    inc $2000
    dec $2000 ; should produce 2
    TEST_END
