.include "test_hdr.inc"

; DESC: Buggy read16 - Indirect jump with NOP sled and page-unaligned address

; "An original 6502 has does not (sic.) correctly fetch the target address if the
; indirect vector falls on a page boundary (e.g. $xxFF where xx is any value from
; $00 to $FF). In this case fetches the LSB from $xxFF as expected but takes the
; MSB from $xx00. This is fixed in some later chips like the 65SC02 so for
; compatibility always ensure the indirect vector is not at the end of the page."

.macro gen_nops count
    .repeat count, I
    nop
    .endrepeat
.endmacro

test:
    ldy #1
    lda #$20
    sta $10ff
    lda #$a1
    sta $1100   ; proper target MSB location
    lda #$a0
    sta $1000   ; buggy target MSB location (one we should use)
    jmp ($10ff) ; (correctly) buggy version should jump to $a020 (this code) producing 2 in Y, not $a120 (which will incorrectly produce 1 in Y)
    gen_nops 20
    iny 
    gen_nops 256
    TEST_END
