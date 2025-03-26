@ This ARM Assembler code should implement a matching function, for use in the MasterMind program, as
@ described in the CW2 specification. It should produce as output 2 numbers, the first for the
@ exact matches (peg of right colour and in right position) and approximate matches (peg of right
@ color but not in right position). Make sure to count each peg just once!
    
@ Example (first sequence is secret, second sequence is guess):
@ 1 2 1
@ 3 1 3 ==> 0 1
@ You can return the result as a pointer to two numbers, or two values
@ encoded within one number
@
@ -----------------------------------------------------------------------------

.text
@ this is the matching fct that should be called from the C part of the CW	
.global         matches
@ use the name `main` here, for standalone testing of the assembler code
@ when integrating this code into `master-mind.c`, choose a different name
@ otw there will be a clash with the main function in the C code
.global         main2
main2: 
    @LDR  R2, =secret	@ pointer to secret sequence
    @LDR  R3, =guess		@ pointer to guess sequence
    @MOV R0, R2			@ Pass secret to matches function as a function paramter (needs to be in R0)
    @MOV R1, R3			@ Pass guess to matches function
    LDR R10, =NAN1
    LDR R11, =NAN2
    BL matches			@ Call matches function

    @ Print the result
    LDR R1, =result_str @ Load format string
    MOV R2, R0          @ Move result to R2 (argument for printf)
    BL printf           @ Call printf


    @ you probably need to initialise more values here

    @ ... COMPLETE THE CODE BY ADDING YOUR CODE HERE, you should use sub-routines to structure your code

exit:	
    MOV R7, #1		@ load system call code
                    @ The value 1 typically corresponds to the "exit" system call, which tells the operating system to terminate the program.
    SWI 0			@ return this value
                    @ triggers the system call to terminate the program and return the value in R0

@ -----------------------------------------------------------------------------
@ sub-routines

@ this is the matching fct that should be callable from C	
matches:			@ Input: R0, R1 ... ptr to int arrays to match ; Output: R0 ... exact matches (10s) and approx matches (1s) of base COLORS
    @ COMPLETE THE CODE HERE
    PUSH {LR}          @ Save return address
    MOV R4, #0 @ R4 stores exact matches count
    MOV R5, #0 @ R5 stores approximate matches count
    MOV R6, #0 @ loop counter (i)

exact_match_loop:
    CMP R6, #LEN 
    BEQ approximate_match_loop

    @ Load the value from the memory address calculated by adding the base address of secret in R0 
    @ and the offset (R6 shifted left by 2 bits, which means multiplying R6 (i counter) by 4). 
    @ to load secret[1] : R6 would have 1 so we do 1 << 2 = 1 * 4 = 4 
    @ if memory address of secret points to 0x1000
    @ R0 + R6 << 2 = R0 + R6 * 4 = 1000 + 1 * 4 = 1004 which has secret[1]
    @ 0x1000 - secret[0]
    @ 0x1000 - secret[1] ..
    LDR R7, [R0,R6, LSL #2] @load secret[i]
    LDR R8, [R1, R6, LSL #2] @load guess[i]

    CMP R7, R8
    BNE not_exact

    @it is an exact match
    ADD R4, R4, #1 @increment exact matches count
    STR R10, [R0, R6, LSL #2] @ mark as counted in secret
    STR R10, [R1, R6, LSL #2] @ mark as counted in guess

not_exact:
    ADD R6, R6, #1 @ move to the next index of the secret sequence
    B exact_match_loop @check if the next index digit is an exact match

approximate_match_loop:
    MOV R6, #0 @counter i for outer loop
    MOV R9, #0  @counter j for inner loop

    outer_loop : 
    CMP R6, #LEN
    BGE done 
    LDR R8, [R1, R6, LSL #2] @load guess[i]
    CMP R8, #NAN1
    BGE skip_outer

    MOV R9, #0
    inner_loop:
    CMP R9, #LEN
    BGE skip_outer
    LDR R7, [R0,R9, LSL #2] @load secret[j]
    CMP R7, R8
    BNE skip_inner

    ADD R5, R5, #1 @increment approximate matches count
    STR R11, [R0,R9, LSL #2] @ mark as counted in secret
    STR R11, [R1, R6, LSL #2] @ mark as counted in guess
    B skip_outer

    skip_inner:
        ADD R9, R9, #1
        B inner_loop

    skip_outer:
        ADD R6, R6, #1
        B outer_loop

    done:
        MOV R12, #10
     	MUL R0, R4, R12       @ Multiply exact matches (R4) by 10 and store in R0
        ADD R0, R0, R5        @ Store approximate matches in R1
        POP {LR}          @ Restore return address
        BX LR             @ Return	

    



@ show the sequence in R0, use a call to printf in libc to do the printing, a useful function when debugging 
showseq: 			@ Input: R0 = pointer to a sequence of 3 int values to show
    @ COMPLETE THE CODE HERE (OPTIONAL)
    
    
@ =============================================================================

.data
result_str: .asciz "Result: %d\n"

@ constants about the basic setup of the game: length of sequence and number of colors	
.equ LEN, 3
.equ COL, 3
.equ NAN1, 8
.equ NAN2, 9

@ a format string for printf that can be used in showseq
f4str: .asciz "Seq:    %d %d %d\n"

@ a memory location, initialised as 0, you may need this in the matching fct
n: .word 0x00
    
@ INPUT DATA for the matching function
.align 4
secret: .word 1 @ defines a seq of three 32-bit ( word is 4 bytes ) integers and labels it as secret
    .word 2 
    .word 1 

.align 4
guess:	.word 3 
    .word 1 
    .word 3 

@ Not strictly necessary, but can be used to test the result	
@ Expect Answer: 0 1
.align 4
expect: .byte 0
    .byte 1

.align 4
secret1: .word 1 
     .word 2 
     .word 3 

.align 4
guess1:	.word 1 
    .word 1 
    .word 2 

@ Not strictly necessary, but can be used to test the result	
@ Expect Answer: 1 1
.align 4
expect1: .byte 1
     .byte 1

.align 4
secret2: .word 2 
     .word 3
     .word 2 

.align 4
guess2:	.word 3 
    .word 3 
    .word 1 

@ Not strictly necessary, but can be used to test the result	
@ Expect Answer: 1 0
.align 4
expect2: .byte 1
     .byte 0