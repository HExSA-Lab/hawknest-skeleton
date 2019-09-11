.include "test_hdr.inc"

; DESC: 16-bit by 16-bit unsigned multiply

; Multiply $22 (low) and $23 (high) by $20 (low) and
; $21 (high) producing a 32-bit result in $24 (low) to $27 (high)
;
mlt16:
    LDA #$00     ; clear p2 and p3 of product
    STA $26
    STA $27
    LDX #$16     ; multiplier bit count = 16
nxtbt:
    LSR $21      ; shift two-byte multiplier right
    ROR $20
    BCC align    ; multiplier = 1?
    LDA $26      ; yes. fetch p2
    CLC
    ADC $22      ; and add m0 to it
    STA $26      ; store new p2
    LDA $27      ; fetch p3
    ADC $23      ; and add m1 to it
align:
    ROR A        ; rotate four-byte product right
    STA $27      ; store new p3
    ROR $26
    ROR $25
    ROR $24
    DEX          ; decrement bit count
    BNE nxtbt    ; loop until 16 bits are done
    RTS
       
test:
    lda #0
    sta $22
    lda #12
    sta $23
    lda #0
    sta $20
    lda #13
    sta $21
    jsr mlt16
    TEST_END

