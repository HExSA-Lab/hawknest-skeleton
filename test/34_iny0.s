.include "test_hdr.inc"

; DESC: Increment with Y reg works

test:
    ldy #2
    iny
    TEST_END
