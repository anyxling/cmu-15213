.include "lab2_include.asm"

.data
input_buffer: .space 10  # buffer for input string
color: .word COLOR_WHITE
left:    .word 0
top:     .word 0
right:   .word 0
bottom:  .word 0


.text
.global main
main:
	jal display_init
	
_loop:
	# print the command message
	lstr a0, "Command ([c]olor, [p]ixel, [l]ine, [r]ectangle, [q]uit): "
	li v0, 4
	syscall
	
	# read a string from the user
	la a0, input_buffer
	li a1, 10
	li v0, 8
	syscall
	
	# get first character of input_buffer with lb(load *byte*) and switch on it
	lb t0, input_buffer
	
	# branch to appropriate label
	beq t0, 'c', _color
	beq t0, 'p', _pixel
	beq t0, 'l', _line
	beq t0, 'r', _rectangle
	beq t0, 'q', _quit
	
	# jump to _break
	j _break

_color:
	lstr a0, "Enter a color in the range [0, 15]: "
	li v0, 4
	syscall
	
	li v0, 5
	syscall
	move t0, v0
	
	# Validate input range [0, 15]
	blt t0, 0, _color
	bgt t0, 15, _color
	
	li t1, COLOR_BLACK
	add t0, t0, t1
	sw t0, color

	j _break

_pixel:
	# Ask for X coordinate
	lstr a0, "Enter X coordinate: "
	li v0, 4
	syscall
	
	# Read X coordinate (int)
	li v0, 5
	syscall
	move a0, v0 # put X into a0 (argument register)
	
	# Ask for Y coordinate
	lstr a0, "Enter Y coordinate: "
	li v0, 4
	syscall
	
	# Read Y coordinate (int)
	li v0, 5
	syscall
	move a1, v0 # put Y into a1 (argument register)
	
	# Set color (white)
	lw a2, color # use provided constant (not hardcoded 71)
	
	# Call display_set_pixel(a0 = x, a1 = y, a2 = COLOR_WHITE)
	jal display_set_pixel
	
	# Call display_finish_frame to update display
    jal display_finish_frame
	
	j _break

_line:
	# Ask for X coordinate
	lstr a0, "Enter X coordinate: "
	li v0, 4
	syscall
	
	# Read X coordinate (int)
	li v0, 5
	syscall
	move s0, v0
	
	# Ask for Y coordinate
	lstr a0, "Enter Y coordinate: "
	li v0, 4
	syscall
	
	# Read Y coordinate (int)
	li v0, 5
	syscall
	move s1, v0
	
	# Ask for width
	lstr a0, "Enter width: "
	li v0, 4
	syscall
	
	# Read width(int)
	li v0, 5
	syscall
	move s2, v0 
	
	lw a2, color
	
	move t0, s0
	add t1, s0, s2  # t1 = X + width
	
_line_loop:
	bge t0, t1, _line_done
	
	move a0, t0  # x
	move a1, s1  # y
	jal display_set_pixel
	
	# Increment t0
	addi t0, t0, 1
	
	j _line_loop
	
_line_done:
	jal display_finish_frame
	j _break


_rectangle:
	# Ask for X coordinate
	lstr a0, "Enter X coordinate: "
	li v0, 4
	syscall
	
	# Read X coordinate (int)
	li v0, 5
	syscall
	sw v0, left
	
	# Ask for Y coordinate
	lstr a0, "Enter Y coordinate: "
	li v0, 4
	syscall
	
	# Read Y coordinate (int)
	li v0, 5
	syscall
	sw v0, top
	
	# Ask for width
	lstr a0, "Enter width: "
	li v0, 4
	syscall
	
	# Read width(int)
	li v0, 5
	syscall
	lw t0, left
	add v0, v0, t0
	sw v0, right  # right = left + width
	
	# Ask for height
	lstr a0, "Enter height: "
	li v0, 4
	syscall
	
	# Read height (int)
	li v0, 5
	syscall
	lw t0, top
	add v0, v0, t0
	sw v0, bottom  # bottom = top + height
	
	lw a2, color
	
	lw s0, left  # x = left
	lw t1, right  # t1 = right

_rect_outer_loop:
	bge s0, t1, _rect_done
	
	lw s1, top  # y = top
	lw t2, bottom  # t2 = bottom
	
_rect_inner_loop:
	bge s1, t2, _rect_outer_next
	
	# Set pixel at (x=s0, y=s1)
    move a0, s0
    move a1, s1
    jal display_set_pixel
    
    addi s1, s1, 1  # y++
    
    j _rect_inner_loop
    
_rect_outer_next:
	addi s0, s0, 1  # x++
	j _rect_outer_loop
	
_rect_done:
	jal display_finish_frame
	j _break


_quit:
	li v0, 10
	syscall

_break:
	j _loop

	
