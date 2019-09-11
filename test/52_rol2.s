.include "test_hdr.inc"

; DESC: Rotate left with absolute addressing

test:
    lda #$7e
    sta $2000
    sec       ; set carry flag
    rol $2000 ; should produce $ef at $2000
    TEST_END
