.include "test_hdr.inc"

; DESC: Put X into stack pointer register

test:
    ldx #$f4
    txs ; SP should now have $f4
    TEST_END
