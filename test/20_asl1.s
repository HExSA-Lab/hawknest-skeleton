.include "test_hdr.inc"

; DESC: Arithmetic left shift works with absolute address

test:
    lda #$ff
    sta $2000
    asl $2000 ; should set carry flag to 1 and produce #$fe
    TEST_END
