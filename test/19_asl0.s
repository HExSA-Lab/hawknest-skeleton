.include "test_hdr.inc"

; DESC: Arithmetic left shift works

test:
    lda #3
    asl a  ; should produce 6
    TEST_END
