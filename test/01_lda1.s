.include "test_hdr.inc"

; DESC: LDA properly sets zero flag

test:
    lda #0 ;; should set zero flag
    TEST_END
