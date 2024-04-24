/*** asmSort.s   ***/
#include <xc.h>
.syntax unified

@ Declare the following to be in data memory
.data
.align    

@ Define the globals so that the C code can access them
/* define and initialize global variables that C can access */
/* create a string */
.global nameStr
.type nameStr,%gnu_unique_object
    
/*** STUDENTS: Change the next line to your name!  **/
nameStr: .asciz "David McColl"  
.align   /* realign so that next mem allocations are on word boundaries */
 
/* initialize a global variable that C can access to print the nameStr */
.global nameStrPtr
.type nameStrPtr,%gnu_unique_object
nameStrPtr: .word nameStr   /* Assign the mem loc of nameStr to nameStrPtr */

@ Tell the assembler that what follows is in instruction memory    
.text
.align

/********************************************************************
function name: asmSwap(inpAddr,signed,elementSize)
function description:
    Checks magnitude of each of two input values 
    v1 and v2 that are stored in adjacent in 32bit memory words.
    v1 is located in memory location (inpAddr)
    v2 is located at mem location (inpAddr + M4 word size)
    
    If v1 or v2 is 0, this function immediately
    places 0 in r0 and returns to the caller.
    
    Else, if v1 <= v2, this function 
    does not modify memory, and returns 0 in r0. 

    Else, if v1 > v2, this function 
    swaps the values and returns 1 in r0

Inputs: r0: inpAddr: Address of v1 to be examined. 
	             Address of v2 is: inpAddr + M4 word size
	r1: signed: 1 indicates values are signed, 
	            0 indicates values are unsigned
	r2: size: number of bytes for each input value.
                  Valid values: 1, 2, 4
                  The values v1 and v2 are stored in
                  the least significant bits at locations
                  inpAddr and (inpAddr + M4 word size).
                  Any bits not used in the word may be
                  set to random values. They should be ignored
                  and must not be modified.
Outputs: r0 returns: -1 If either v1 or v2 is 0
                      0 If neither v1 or v2 is 0, 
                        and a swap WAS NOT made
                      1 If neither v1 or v2 is 0, 
                        and a swap WAS made             
             
         Memory: if v1>v2:
			swap v1 and v2.
                 Else, if v1 == 0 OR v2 == 0 OR if v1 <= v2:
			DO NOT swap values in memory.

NOTE: definitions: "greater than" means most positive number
********************************************************************/     
.global asmSwap
.type asmSwap,%function     
asmSwap:
    /* YOUR asmSwap CODE BELOW THIS LINE! VVVVVVVVVVVVVVVVVVVVV  */
    // uint32 swapped = asmSwap(int32* addr, uint32 signed, uint32 size) 
    //        R0=0,1,-1                R0           R1=0,1   R2=1,2,4
    // note: R1,R2 used but not changed  -- R0 is return value .EQU UNSIGNED, 0
.EQU   SIGNED, 1
.EQU   BYTES,  1
.EQU   HALFS,  2
.EQU   WORDS,  4
.EQU ZERO_DETECT, -1

    push {r4-r11,LR} /* save the caller's registers */
    MOV R6,ZERO_DETECT    // default return value
check_8bit:
    CMP R2,BYTES        // check size
    BNE check_16bit
    CMP R1,SIGNED        // select signed/unsigned load
    LDRSBEQ R4,[R0]      // load signed first
    LDRSBEQ R5,[R0,4]    // load signed second
    LDRBNE R4,[R0]       // load unsigned first
    LDRBNE R5,[R0,4]     // load unsigned second
    CBZ R4,set_swap_return   // check for end-of-list = 0
    CBZ R5,set_swap_return   // check for end-of-list = 0
    CMP R4,R5        // check if first > second
    STRBGT R5,[R0]   // write back in reverse order
    STRBGT R4,[R0,4]
    MOVGT R6,1       // mark as swapped
    MOVLE R6,0       // else not swapped
    B set_swap_return
    
check_16bit:
    CMP R2,HALFS      // check size
    BNE check_32bit
    CMP R1,SIGNED      // select signed/unsigned load
    LDRSHEQ R4,[R0]    // load signed first
    LDRSHEQ R5,[R0,4]  // load signed second
    LDRHNE R4,[R0]     // load unsigned first
    LDRHNE R5,[R0,4]   // load unsigned second
    CBZ R4,set_swap_return
    CBZ R5,set_swap_return
    CMP R4,R5          // check is first>second
    STRHGT R5,[R0]     // write back in reverse order
    STRHGT R4,[R0,4]
    MOVGT R6,1         // mark as swapped
    MOVLE R6,0         // or not
    B set_swap_return
    
check_32bit:
    LDR R4,[R0]
    LDR R5,[R0,4]
    CBZ R4,set_swap_return
    CBZ R5,set_swap_return
    CMP R1,SIGNED
    BNE unsigned_compare
    CMP R4,R5
    STRGT R5,[R0]
    STRGT R4,[R0,4]
    MOVGT R6,1
    MOVLE R6,0
    B set_swap_return
unsigned_compare:
    CMP R4,R5
    STRHI R5,[R0]
    STRHI R4,[R0,4]
    MOVHI R6,1
    MOVLS R6,0
    B set_swap_return
    
set_swap_return:
    MOV R0,R6  // set return value = swapped flag
    pop  {r4-r11,PC} /* save the caller's registers */
    /* YOUR asmSwap CODE ABOVE THIS LINE! ^^^^^^^^^^^^^^^^^^^^^  */
    
    
/********************************************************************
function name: asmSort(startAddr,signed,elementSize)
function description:
    Sorts value in an array from lowest to highest.
    The end of the input array is marked by a value
    of 0.
    The values are sorted "in-place" (i.e. upon returning
    to the caller, the first element of the sorted array 
    is located at the original startAddr)
    The function returns the total number of swaps that were
    required to put the array in order in r0. 
    
         
Inputs: r0: startAddr: address of first value in array.
		      Next element will be located at:
                          inpAddr + M4 word size
	r1: signed: 1 indicates values are signed, 
	            0 indicates values are unsigned
	r2: elementSize: number of bytes for each input value.
                          Valid values: 1, 2, 4
Outputs: r0: number of swaps required to sort the array
         Memory: The original input values will be
                 sorted and stored in memory starting
		 at mem location startAddr
NOTE: definitions: "greater than" means most positive number    
********************************************************************/     
.global asmSort
.type asmSort,%function
asmSort:   
    /* YOUR asmSort CODE BELOW THIS LINE! VVVVVVVVVVVVVVVVVVVVV  */
    // int32 count = asmSort(int32* addr, uint32 signed, uint32 size)
    //         R0                    R0           R1=0,1   R2=1,2,4
    push {r4-r11,LR} /* save the caller's registers */
    MOV R3,R0       // save array base addr
    MOV R5,0        // init total swap counter
next_outer:
    MOV R4,R3       // reset array addr to base addr
    MOV R6,0        // reset pass swaps
next_inner:
    MOV R0,R4        // get addr array[i]
    BL  asmSwap      // call swap (note: R1, R2 same as passed in)
    CMP R0,ZERO_DETECT
    BEQ end_inner
    ADD R6,R0        // accumulate swaps this pass
    ADD R4,4         // point addr to next 32bit word
    B   next_inner
end_inner:
    CBZ R6, set_sort_return  // no new swaps -- we are done sorting
    ADD R5,R6                // accum total swaps
    b next_outer
    
set_sort_return:
    MOV R0,R5        // set return value = total swaps
    pop  {r4-r11,PC} /* save the caller's registers */
    /* YOUR asmSort CODE ABOVE THIS LINE! ^^^^^^^^^^^^^^^^^^^^^  */

   

/**********************************************************************/   
.end  /* The assembler will not process anything after this directive!!! */
           




