# DO NOT REMOVE THIS LINE WHEN SUBMITTING
.globl LPS

strlen: # it acts like function strlen in <string.h>
        addi    sp, sp, -12     # allocate stack frame 
        sw      s0, 0(sp)       
        addi    s0, sp, 12      # set s0 to point to top of stack
        sw      a0, -4(s0)     # save str on stack at offset -4
        sw      x0, -8(s0)     # t = 0, to count the length of the string
        j       inner_loop

add_n: # label used for the loop in strlen
        lw      a2, -8(s0)     # load t into a2
        addi    a2, a2, 1       # ++t
        sw      a2, -8(s0)     # store the updated value back into the stack

inner_loop:
        lw      a2, -8(s0)     # load t into a2
        lw      t2, -4(s0)     # load str into t2
        add     a2, t2, a2      # calculate the memory address of the current character
        lbu     a2, 0(a2)       # to obtain str[t]
        bnez    a2, add_n       # chech if (str[t] == '\0'); 
        
        lw      a2, -8(s0)     # to give a2 the length
        mv      a0, a2          
        lw      s0, 0(sp)
        addi    sp, sp, 12
        jr      ra


LPS:
        # parameters that we will use:
        # @str: the given string, stored at stack -20
        # @n: to store the length of str; stored at 0
        # @maxlength: to store the value of the answer; stored at -4
        # @low: the pointer points to the left side of the substr; stored at -8
        # @high: the pointer points to the right side of the substr; stored at -12
        # @ra: stored at -24

        addi    sp, sp, -32     # allocate stack frame with enough space
        sw      s0, 4(sp)
        addi    s0, sp, 32      # set s0 to point to top of stack
        sw      ra, -24(s0)
        sw      a0, -20(s0)     # save str on stack at offset -20
           
        # to call function strlen
        lw      a0, -20(s0)
        call    strlen          
        sw      a0, 0(s0)       # to give n the value
        
        # after obtaining n, it will reach this part
        lw      t0, 0(s0)       # load n into t0
        li      t1, 1           # set t1 to 1
        bgt     t0, t1, init_maxlength # to chech if (n <= 1), jump to init_maxlength
        lw      t1, 0(s0)       # load n into t1, in this case maxlength = n
        j       done            

init_maxlength: # to discuss the occasion where n >= 2, i.e. str is quite long
        li      t1, 1           # to set maxlength = 1 and it is now at t1
        sw      t1, -4(s0)      # to store the value of maxlenth on stack at offset -4
        
        # prepare for going into the outer loop.
        sw      x0, -16(s0)     # i = 0 and store it on stack at offset -16
        j       outer_loop_condition  

set_low_and_high: # here we assign value to low and high 
        lw      t1, -16(s0)     # load i into t1
        addi    t1, t1, -1      #  low = i - 1
        sw      t1, -8(s0)      # save low on stack at offset -8
        lw      t1, -16(s0)     # load i into t1
        addi    t1, t1, 1       # high = i + 1
        sw      t1, -12(s0)     # save high on stack at offset -12
        
        j       increment_high

add_high:
        lw      t1, -12(s0)     # load high into t1
        addi    t1, t1, 1       # ++high
        sw      t1, -12(s0)     

increment_high:
        lw      t0, -12(s0)     # load high into t0
        lw      t1, 0(s0)       # load n into t1
        bge     t0, t1, decrement_low    # chech if (high >= n), if yes jump to decrement_low
        
        # otherwise, we will move on
        lw      t1, -12(s0)     # load high into t1
        lw      t0, -20(s0)     # load str into t0
        add     t1, t0, t1
        lbu     t0, 0(t1)       # str[high] in t0
        lw      t1, -16(s0)     # load i into t1
        lw      t2, -20(s0)     # load str into t2
        add     t1, t2, t1      
        lbu     t1, 0(t1)        # str[i] in t1
        beq     t0, t1, add_high # check str[high] == str[i] ?
        j       decrement_low

sub_low:
        lw      t1, -8(s0)      # load low into t1 
        addi    t1, t1, -1      # --low
        sw      t1, -8(s0)

decrement_low:
        lw      t1, -8(s0)      # load low into t1
        bltz    t1, check_ps    # check if (low < 0), if yes, jump to check_ps
        
        lw      t1, -8(s0)      # load low into t1
        lw      t0, -20(s0)     # load str into t0 
        add     t1, t0, t1      
        lbu     t0, 0(t1)       # str[low] in t0
        lw      t1, -16(s0)     # load i into t1
        lw      t2, -20(s0)     # load str into t2
        add     t1, t2, t1      
        lbu     t1, 0(t1)       # str[i] in t1
        beq     t0, t1, sub_low # chech if (str[low] == str[i]), if yes, jump to sub_low
        j       check_ps

change_low_and_high:
        # --low
        lw      t1, -8(s0)
        addi    t1, t1, -1
        sw      t1, -8(s0)
        # ++high
        lw      t1, -12(s0)
        addi    t1, t1, 1
        sw      t1, -12(s0)

check_ps:
        lw      t1, -8(s0)      # load low into t1
        bltz    t1, change_maxlength
        
        lw      t0, -12(s0)     # load high into t0
        lw      t1, 0(s0)       # load n into t1
        bge     t0, t1, change_maxlength
        
        lw      t1, -8(s0)      # load low into t1
        lw      t0, -20(s0)     # load str into t0
        add     t1, t0, t1
        lbu     t0, 0(t1)       # str[low]
        
        lw      t1, -12(s0)     # load high into t1      
        lw      t2, -20(s0)     # load str into t2
        add     t1, t2, t1
        lbu     t1, 0(t1)       # str[high]
        beq     t0, t1, change_low_and_high

change_maxlength:
        lw      t0, -12(s0)     # load high into t0
        lw      t1, -8(s0)      # load low into t1
        sub     t1, t0, t1      
        addi    t1, t1, -1      # length = high - low - 1 in t1    
        lw      t0, -4(s0)
        bge     t0, t1, outer_loop_add_i # check if (length < maxlength)
        sw      t1, -4(s0)

outer_loop_add_i:
        lw      t1, -16(s0)     # load i into t1
        addi    t1, t1, 1       # ++i
        sw      t1, -16(s0)

outer_loop_condition:
        lw      t0, -16(s0)     # load i into t0
        lw      t1, 0(s0)       # load n into t1
        bne     t0, t1, set_low_and_high # chech if (i < n), if yes jump to set_low_and_high
        lw      t1, -4(s0)      # load maxlength into t1

done:
        mv      a0, t1          # give a0 the return value
        lw      ra, -24(s0)
        lw      s0, 4(sp)
        addi    sp, sp, 32    
        jr      ra


