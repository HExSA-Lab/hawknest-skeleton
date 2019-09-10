.include "test_hdr.inc"

; DESC: Basic increment works

test:
    lda #2
    sta $2000
    inc $2000
    TEST_END
