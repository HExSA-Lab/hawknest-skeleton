.include "test_hdr.inc"

; DESC: Rotate left with carry

test:
    lda #$7e
    sec ; set carry flag
    rol  ; should produce $ef in A
    TEST_END
