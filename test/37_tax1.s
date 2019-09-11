.include "test_hdr.inc"

; DESC: Transfer from A to X with negative flag

test:
    lda #$ff
    tax ; should set neg reg
    TEST_END
