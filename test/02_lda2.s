.include "test_hdr.inc"

; DESC: LDA properly sets negative flag

test:
    lda #<-1 ;; should set negative flag
    TEST_END
