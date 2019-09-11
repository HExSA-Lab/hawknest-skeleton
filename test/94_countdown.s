.include "test_hdr.inc"

; DESC: countdown loop routine

loopit:
    ldy #$ff
loop1:
    ldx #$ff
loop2:
    dex
    bne loop2
    dey
    bne loop1
    rts

test:
    jsr loopit
    TEST_END

