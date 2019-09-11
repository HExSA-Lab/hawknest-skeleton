.include "test_hdr.inc"

; DESC: Bit test works

test:
    lda #2
    sta $2000
    lda #$fe
    bit $2000
    TEST_END
