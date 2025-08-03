.global main
main:
	# li t0, 1
	# li t1, 2
	# li t2, 3

	# move a0, t0
	# move v0, t1
	# move t2, zero
	
	# li zero, 10
	# li t0, 0xF00D
	# li a0, 666
	# li v0, 1
	# syscall
	
	li v0, 5
	syscall
	
	li t0, 1
	add a0, v0, t0
	
	li v0, 1
	syscall 