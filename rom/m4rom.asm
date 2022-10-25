save "m4rom.rom",#c000,#4000

TXT_OUTPUT	equ #bb5a

COMMANDPORT equ #ffff

M4_DATAPORT equ #fe00
M4_ACKPORT 	equ #fc00

ROM_RESPONSE equ #e800

org #c000

start:

	db 1,1,0,0
	dw name_table
	jp init_rom
	jp exit
	jp menu
	jp time
	jp 0

name_table:
	str "M4 BOARD"
	str "EXIT"
	str "MENU"
	str "TIME"
	str "RESET"
	db 0

init_rom:
	push hl
	ld hl,rom_message
	call print_text
	pop hl
	scf
	ret

exit:
	or a
	jr z,no_params	
	ld a,(ix)
no_params:
	ld bc,COMMANDPORT
	out (c),a
	ret

menu:
	ld bc,COMMANDPORT
	out (c),c
	scf
	ret

time:
	ld hl,time_command
	call m4_sendcmd
	ld hl,ROM_RESPONSE+3
	jp print_text

m4_sendcmd:
	ld	bc,M4_DATAPORT
	ld	d,(hl)
	inc	d
m4_sendloop:
	inc	b
	outi
	dec	d
	jr	nz,m4_sendloop
	ld	bc,M4_ACKPORT
	out	(c),c
	ret

print_text:
	ld a,(hl)
	or a
	ret z
	call TXT_OUTPUT
	inc hl
	jr print_text

time_command:
	db 2
	dw #4324

rom_message:
	db " M4 Board",13,10,10,0

org ROM_RESPONSE
	ds 2048

org #ff00
	dw 0
	dw ROM_RESPONSE