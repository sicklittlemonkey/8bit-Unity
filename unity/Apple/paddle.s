;
; Copyright (c) 2018 Anthony Beaucamp.
;
; This software is provided 'as-is', without any express or implied warranty.
; In no event will the authors be held liable for any damages arising from
; the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
;   1. The origin of this software must not be misrepresented; you must not
;   claim that you wrote the original software. If you use this software in a
;   product, an acknowledgment in the product documentation would be
;   appreciated but is not required.
;
;   2. Altered source versions must be plainly marked as such, and must not
;   be misrepresented as being the original software.
;
;   3. This notice may not be removed or altered from any distribution.
;
;   4. The names of this software and/or it's copyright holders may not be
;   used to endorse or promote products derived from this software without
;   specific prior written permission.
;

	.include  "apple2.inc"
	
	.export _GetPaddle
	.export _GetButton
	
PREAD = $FB1E   ; Read paddle in X, return AD conv. value in Y

	.segment	"CODE"		; DO NOT RELOCATE TO OTHER SEGMENTS!!! (app crashes)

; ---------------------------------------------------------------
; unsigned char __near__ _GetPaddle (unsigned char)
; ---------------------------------------------------------------		
	
.proc _GetPaddle: near
	; Read a particular paddle passed in A.
	bit     $C082           ; Switch in ROM
	
	; Read Paddle Potentiometer
	tax                     
	jsr     PREAD           ; Read paddle x
	tya
	ldx     #$00
	bit     $C080           ; Switch in LC bank 2 for R/O
	rts
.endproc	

; ---------------------------------------------------------------
; unsigned char __near__ _GetButton (unsigned char)
; ---------------------------------------------------------------		
	
.proc _GetButton: near
	tax
	lda     BUTN0,x
	and		#$80	; Keep only bit 7
	asl				; Shift it to carry
	rol				; Roll carry to bit 0
	rts
.endproc	
