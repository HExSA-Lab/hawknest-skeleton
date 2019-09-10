.include "test_hdr.inc"

; DESC: Basic store from X register works

test:
    ldx #100
    stx $2000
    TEST_END
