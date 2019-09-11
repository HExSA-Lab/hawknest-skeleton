.include "test_hdr.inc"

; DESC: Zero-page,Y addressing works (with wrap-around)

test:
    ldy #5
    ldx #2
    stx $ff,Y ;; should wrap around to $04
    TEST_END
