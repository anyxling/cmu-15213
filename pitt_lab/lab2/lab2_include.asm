
# stripped down version of the display driver, with just enough
# stuff to make lab 2 possible.

# use like:
#   lstr a0, "whatever"
# it will put "whatever" in the data segment, and use "la" to put
# its address in a0.
# you can use any register, not just a0.
.macro lstr %rd, %str
	.data
	lstr_message: .asciiz %str
	.text
	la %rd, lstr_message
.end_macro

# classic color palette indexes in the default palette
.eqv COLOR_BLACK       64
.eqv COLOR_RED         65
.eqv COLOR_ORANGE      66
.eqv COLOR_YELLOW      67
.eqv COLOR_GREEN       68
.eqv COLOR_BLUE        69
.eqv COLOR_MAGENTA     70
.eqv COLOR_WHITE       71
.eqv COLOR_DARK_GREY   72
.eqv COLOR_DARK_GRAY   72
.eqv COLOR_BRICK       73
.eqv COLOR_BROWN       74
.eqv COLOR_TAN         75
.eqv COLOR_DARK_GREEN  76
.eqv COLOR_DARK_BLUE   77
.eqv COLOR_PURPLE      78
.eqv COLOR_LIGHT_GREY  79
.eqv COLOR_LIGHT_GRAY  79

# neat, we can specify the address to put these vars at!
# ...with a caveat
.data 0xFFFF0000
	display_ctrl:           .word 0 # 0xFFFF0000
	display_sync:           .word 0 # 0xFFFF0004
	display_reset:          .word 0 # 0xFFFF0008
	display_frame_counter:  .word 0 # 0xFFFF000C

	display_fb_clear:       .word 0 # 0xFFFF0010
	display_fb_in_front:    .word 0 # 0xFFFF0014
	display_fb_pal_offs:    .word 0 # 0xFFFF0018
	display_fb_scx:         .word 0 # 0xFFFF001C
	display_fb_scy:         .word 0 # 0xFFFF0020

.data 0xFFFF1000
	display_fb_ram:         .byte 0:16384 # 0xFFFF1000-0xFFFF4FFF

# because there is no way to know the previous .data location,
# this file must be included as the very first thing in any
# program that uses it. buuuut whatever. this resets the data
# location to its default value, so that the user variables
# end up where they should.
.data 0x10010000
.text

# initialize the display so it's set up right for lab 2.
display_init:
	li t0, 0x000f0101 # 15 ms/frame, framebuffer on, enhanced mode on
	sw t0, display_ctrl # ~ magical store ~

	# reset everything
	sw zero, display_reset

	# force a display update to clear the display to black
	sw zero, display_sync
jr ra

# should be called at the end of each frame. displays everything
# and then waits the right amount of time for the next frame.
display_finish_frame:
	sw zero, display_sync # display
	lw zero, display_sync # wait (~ magical load ~)
jr ra

# void display_set_pixel(int x, int y, int color)
#   sets 1 pixel to a given color. valid colors are in the range [0, 255].
#   (0, 0) is in the top LEFT, and Y increases DOWNWARDS!
display_set_pixel:
	blt a0, 0,   _return
	bge a0, 128, _return
	blt a1, 0,   _return
	bge a1, 128, _return

	mul t0, a1, 128
	add t0, t0, a0
	sb  a2, display_fb_ram(t0) # ??? not a function call, no...
_return:
jr  ra