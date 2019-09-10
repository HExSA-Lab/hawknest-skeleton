.include "test_hdr.inc"

; DESC: Transfer from A to X with zero flag

test:
    lda #0
    tax ; should set zero flag
    TEST_END
