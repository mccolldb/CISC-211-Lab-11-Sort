
/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project. It is intended to
    be used as the starting point for CISC-211 Curiosity Nano Board
    programming projects. After initializing the hardware, it will
    go into a 0.5s loop that calls an assembly function specified in a separate
    .s file. It will print the iteration number and the result of the assembly 
    function call to the serial port.
    As an added bonus, it will toggle the LED on each iteration
    to provide feedback that the code is actually running.
  
    NOTE: PC serial port should be set to 115200 rate.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include <float.h>
#include "definitions.h"                // SYS function prototypes
#include "projGlobalDefinitions.h"  // lab print funcs
#include "printFuncs.h"  // lab print funcs
#include "testFuncs.h"  // lab print funcs

/* RTC Time period match values for input clock of 1 KHz */
#define PERIOD_50MS                             51
#define PERIOD_500MS                            512
#define PERIOD_1S                               1024
#define PERIOD_2S                               2048
#define PERIOD_4S                               4096

#define NUM_ELEM_SIZES 3  // element sizes 1,2, or 4 bytes

static volatile bool isRTCExpired = false;
static volatile bool changeTempSamplingRate = false;
static volatile bool isUSARTTxComplete = true;
static uint8_t uartTxBuffer[MAX_PRINT_LEN] = {0};

// static char * pass = "PASS";
// static char * fail = "FAIL";
// static char * oops = "OOPS";

// PROF COMMENT:
// The ARM calling convention permits the use of up to 4 registers, r0-r3
// to pass data into a function. Only one value can be returned from a C
// function call. It will be stored in r0, which will be automatically
// used by the C compiler as the function's return value.
//
// Function signature
// Create references to the assembly functions to be tested
extern int32_t asmSwap(void *, int32_t sign, int32_t elemSize);
extern int32_t asmSort(void *, int32_t sign, int32_t elemSize);

// externs defined in the assembly file:
// NONE

// test cases for swap
static int32_t swapTestCases[][2] = 
{
    {0x0203,0x0302},         // swap TC 0
    {0x80000001,0x00000001}, // swap TC 1
    {0,0},                   // swap TC 2
    {1,2},                   // swap TC 3
    {2,1}                    // swap TC 4
};

// test cases for sort
static int32_t sortTestCases[][MAX_SORT_ARRAY_SIZE] = {
    {79,69,59,78,68,58,75,65,45,-1,0}, // sort TC 0
    {2,1,0,6,0,1,2,3,4,5,6},           // sort TC 1
    {9,8,7,6,0,1,2,3,4,5,6},           // sort TC 2
    {0,0,0,0,0,0,0,0,0,0,0},           // sort TC 3
    {32768,1,0,0x001122AA,0x001122AA,
        0x001122AA,0x001122AA,0x001122AA,
        0x001122AA,0x001122AA,0x001122AA},   // sort TC 4
    {0x80000001,0x80011000,0x80010001,0x80011001,
        0x700109A0,0x7FED4000,0x60000000,0,
        0x11111111,0x22222222,0x33333333},   // sort TC 5
    {1,2,3,4,5,6,7,8,9,0,0},           // sort TC 6
    {-1,-2,-3,0,9,10,11,12,13,15}      // sort TC 7
};


#if USING_HW
static void rtcEventHandler (RTC_TIMER32_INT_MASK intCause, uintptr_t context)
{
    if (intCause & RTC_MODE0_INTENSET_CMP0_Msk)
    {            
        isRTCExpired    = true;
    }
}
static void usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        isUSARTTxComplete = true;
    }
}
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main ( void )
{
    
 
#if USING_HW
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
    RTC_Timer32CallbackRegister(rtcEventHandler, 0);
    RTC_Timer32Compare0Set(PERIOD_50MS);
    RTC_Timer32CounterSet(0);
    RTC_Timer32Start();

#else // using the simulator
    isRTCExpired = true;
    isUSARTTxComplete = true;
#endif //SIMULATOR

    int32_t passCount = 0;
    int32_t failCount = 0;
    int32_t totalPassCount = 0;
    int32_t totalFailCount = 0;
    int32_t totalTestCount = 0;
    int32_t numTestCases = 0;
    int32_t elemSizeArray[NUM_ELEM_SIZES] = {1,2,4};

    int32_t swapTotalTestCount = 0;
    int32_t swapTotalPassCount = 0;
    int32_t swapTotalFailCount = 0;
    
    int32_t testCaseCounter = 0;
    
    // test the swap func
    numTestCases = sizeof(swapTestCases)/sizeof(swapTestCases[0]);
    for(int32_t testCase = 0; testCase<numTestCases; ++testCase)
    {
        for (int32_t sign = 0; sign < 2; ++sign)
        {
            for (int32_t elemIndex = 0; elemIndex < NUM_ELEM_SIZES; ++elemIndex)
            {
                int32_t elemSize = elemSizeArray[elemIndex];
                LED0_Toggle();
                // reset the state variables for the timer and serial port funcs
                isRTCExpired = false;
                isUSARTTxComplete = false;

                // reset the counters for this test pass
                passCount = 0;
                failCount = 0;

                // Get the value for this test case
                int32_t tcCopy[2];
                tcCopy[0] = swapTestCases[testCase][0];
                tcCopy[1] = swapTestCases[testCase][1];

                // STUDENTS! If you need to debug a test case, change the
                // number in the line of code below to match. Then set a
                // breakpoint on the line between the curly braces...
                int32_t breakForTestNum = 6;
                if(testCaseCounter == breakForTestNum)
                {
                    // do nothing; set breakpoint on the following line
                    tcCopy[0] = swapTestCases[testCase][0];
                }

                // call the func
                int32_t swapResult = 0;
                swapResult = asmSwap((void *) tcCopy, sign, elemSize);
#if VERBOSE_DEBUG                
                snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
                    "========= Debug:\r\n"
                    "sign:      %ld\r\n"
                    "elemIndex: %ld\r\n"
                    "elemSize:  %ld\r\n"
                    "unsorted inputs:         %ld, %ld\r\n"
                    "sorted outputs from asm: %ld, %ld\r\n"
                    "\r\n",
                    sign,
                    elemIndex,
                    elemSize,
                    swapTestCases[testCase][0], swapTestCases[testCase][1],
                    tcCopy[0], tcCopy[1]); 
                printAndWait((char*)uartTxBuffer,&isUSARTTxComplete);
#endif
                
                // check the result
                testAsmSwap(testCaseCounter,
                        "",
                        (void *) swapTestCases[testCase], // copy of vals sent to asm
                        (void *) tcCopy, // asm stored sorted vals here
                        swapResult, // -1, 0, or 1
                        sign,
                        elemSize,
                        &passCount,
                        &failCount,
                        &isUSARTTxComplete);

                ++testCaseCounter;
                swapTotalPassCount += passCount;
                swapTotalFailCount += failCount;
                swapTotalTestCount += passCount + failCount;

                // wait for the toggle timer to expire
                while(isRTCExpired == false);
            } // end -- for elem size = 1,2, or 4
        } // end -- for sign 0 or 1
    } // end: for (swap testCase = 0; ...)
    
    // Print the overall results of the swap tests        
    snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
            "========= asmSwap Tests Summary\r\n"
            "tests passed: %ld \r\n"
            "tests failed: %ld \r\n"
            "total tests:  %ld \r\n"
            "\r\n",
            swapTotalPassCount,
            swapTotalFailCount,
            swapTotalTestCount); 
            
    isUSARTTxComplete = false;
    
    printAndWait((char*)uartTxBuffer,&isUSARTTxComplete);
    
    totalTestCount += swapTotalTestCount;
    totalFailCount += swapTotalFailCount;
    totalPassCount += swapTotalPassCount;
    
    // ===============================================
    // ===============================  SORT TESTS ===
    // ===============================================
    
    numTestCases = 0;

    int32_t sortTotalTestCount = 0;
    int32_t sortTotalPassCount = 0;
    int32_t sortTotalFailCount = 0;
    
    testCaseCounter = 0;

    numTestCases = sizeof(sortTestCases)/sizeof(sortTestCases[0]);
    for(int32_t testCase = 0; testCase<numTestCases; ++testCase)
    {
        for (int32_t sign = 0; sign < 2; ++sign)
        {
            for (int32_t elemIndex = 0; elemIndex < NUM_ELEM_SIZES; ++elemIndex)
            {
                int32_t elemSize = elemSizeArray[elemIndex];
                LED0_Toggle();
                // reset the state variables for the timer and serial port funcs
                isRTCExpired = false;
                isUSARTTxComplete = false;

                // reset the counters for this test pass
                passCount = 0;
                failCount = 0;
                
                // make copies of the test case array for the test funcs
                int32_t inpArrayCopy[MAX_SORT_ARRAY_SIZE];
                int32_t asmArrayCopy[MAX_SORT_ARRAY_SIZE];
                for(int32_t i = 0;  i<MAX_SORT_ARRAY_SIZE; i++)
                {
                    inpArrayCopy[i] = sortTestCases[testCase][i];
                    asmArrayCopy[i] = sortTestCases[testCase][i];
                }

                // STUDENTS! If you need to debug a test case, change the
                // number in the line of code below to match. Then set a
                // on the line between the curly braces...
                int32_t breakForTestNum = 7;
                if(testCaseCounter == breakForTestNum)
                {
                    // do nothing; set breakpoint on the following line
                    passCount = 0;
                }
                        
                // call the student's asmSort function
                int32_t asmNumSwaps = asmSort((void *)&asmArrayCopy[0],
                                                sign,
                                                elemSize);
                
                // call the test function to validate the results
                testAsmSort(testCaseCounter, testCase,
                    "", // optional description of test for printout
                    (void *) inpArrayCopy, // unmodified copy of unsorted input array sent to asm
                    // next ptr is where a copy of the unsorted test case input was made
                    // The asm code is supposed to sort them in-place at this same location
                    (void *) asmArrayCopy,
                    sign, // whether inp arr was signed or unsigned
                    elemSize, // num bytes for each element in array: 1,2, or 4
                    asmNumSwaps, // num swaps reported by asmSort to order the input array
                    &passCount,
                    &failCount,
                    &isUSARTTxComplete);
                
                ++testCaseCounter;
                sortTotalPassCount += passCount;
                sortTotalFailCount += failCount;
                sortTotalTestCount += passCount + failCount;

                // wait for the toggle timer to expire
                while(isRTCExpired == false);
            } // end -- for elem size = 1,2, or 4
        } // end -- for sign 0 or 1
    } // end: for ( sort testCase = 0; ...)

    
    
    // Print the overall results of the sort tests        
    snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
            "========= asmSort Tests Summary\r\n"
            "tests passed: %ld \r\n"
            "tests failed: %ld \r\n"
            "total tests:  %ld \r\n"
            "\r\n",
            sortTotalPassCount,
            sortTotalFailCount,
            sortTotalTestCount); 
            
    isUSARTTxComplete = false;
    
    printAndWait((char*)uartTxBuffer,&isUSARTTxComplete);
    
    totalTestCount += sortTotalTestCount;
    totalFailCount += sortTotalFailCount;
    totalPassCount += sortTotalPassCount;

    // Print the summary of all swap and sort tests
    
#if USING_HW
    int32_t swapPointsMax = 15;
    int32_t swapPoints = swapPointsMax*swapTotalPassCount/swapTotalTestCount;

    int32_t sortPointsMax = 15;
    int32_t sortPoints = sortPointsMax*sortTotalPassCount/sortTotalTestCount;
    
    snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
            "========= %s: asmSort.s ALL TESTS COMPLETE!\r\n"
            "swap tests passed: %ld \r\n"
            "swap tests failed: %ld \r\n"
            "swap total tests:  %ld \r\n"
            "swap score: %ld/%ld points \r\n\r\n"
            "sort tests passed: %ld \r\n"
            "sort tests failed: %ld \r\n"
            "sort total tests:  %ld \r\n"
            "sort score: %ld/%ld points \r\n\r\n"
            "FINAL SCORE: %ld\r\n\r\n",
            nameStrPtr,
            swapTotalPassCount,
            swapTotalFailCount,
            swapTotalTestCount,
            swapPoints,swapPointsMax, 
            sortTotalPassCount,
            sortTotalFailCount,
            sortTotalTestCount,
            sortPoints,sortPointsMax,
            (sortPoints+swapPoints)); 
            
    isUSARTTxComplete = false;
    
    printAndWait((char*)uartTxBuffer,&isUSARTTxComplete);

#endif

    snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
            "========= Going into idle mode; display will not scroll!\r\n");
    printAndWait((char*)uartTxBuffer,&isUSARTTxComplete);

    // change to 2s timer and start idle counter loop
    RTC_Timer32Compare0Set(PERIOD_2S);
    RTC_Timer32CounterSet(0);
    RTC_Timer32Start();
    isRTCExpired = false;
    uint32_t idleCounter = 0;

    while(1)
    {
        snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
                "========= Idle Cycle Counter: %6ld\r",
                idleCounter);

        printAndWait((char*)uartTxBuffer,&isUSARTTxComplete);

        // wait for the toggle timer to expire
        while(isRTCExpired == false);
        LED0_Toggle();
        isRTCExpired = false;
        ++idleCounter;
        break;
    }

    
    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}
/*******************************************************************************
 End of File
*/

