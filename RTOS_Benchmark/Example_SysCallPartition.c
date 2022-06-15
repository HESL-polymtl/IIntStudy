/* __________________________________________________________________________
* MODULE DESCRIPTION:
* -------------------
* Filename : SysCallPartition.c
*
* Description: This file contains the system call generation partition.
* __________________________________________________________________________
*/

/* INCLUDE SECTION */
#include <stdio.h>
#include <string.h>
#include "ARINC653.h"
#include "InterruptBench.h"


#define GENERATE_INT

static void process1(void)
{
    RETURN_CODE_TYPE    retCode;
    int_bench_measure_t benchData;
    int_bench_measure_t extIntBenchData;
    uint32_t            mafCount;
    uint32_t            i;

    memset(&benchData, 0, sizeof(int_bench_measure_t));
    memset(&extIntBenchData, 0, sizeof(int_bench_measure_t));

    INT_BENCH_INIT(1, 1, mafCount, benchData);

    while(1)
    {
        printf("[C1P1] Executes\n");
        GET_TIME(&benchData.startTime, &retCode);
        if (retCode == NO_ERROR)
        {
#ifndef GENERATE_INT
                (void)i;
                (void)extIntBenchData;
                /* IDLE partition for more than 900ms */
                __asm__ __volatile__ (
                    "lis 3, 0x2000\n\t"
                    "p_idle_loop:\n\t"
                    "addi 3, 3, -1\n\t"
                    "cmpwi 3, 0\n\t"
                    "bne p_idle_loop\n\t"
                :::"3");
#else
            for(i = 0; i < 1800; ++i)
            {

                /* Generates an interrupt every 0.5ms aproximately */
                __asm__ __volatile__ (
                        "lis 3, 0x0006\n\t"
                        "p_idle_loop:\n\t"
                        "addi 3, 3, -1\n\t"
                        "cmpwi 3, 0\n\t"
                        "bne p_idle_loop\n\t"
                    :::"3");
                INT_BENCH_GEN_SC(1, 1, extIntBenchData);
            }
#endif
            GET_TIME(&benchData.endTime, &retCode);
            if (retCode == NO_ERROR)
            {
                printf("[C1P1] %uus\n",
                       ((benchData.endTime - benchData.startTime) / 1000));
            }
            else
            {
                printf("[C1P1] Cannot get end iteration time: error %d\n",
                       retCode);
            }
        }
        else
        {
            printf("[C1P1] Cannot get initial iteration time: error %d\n",
                   retCode);
        }
        PERIODIC_WAIT(&retCode);
    }
}

void main_process(void)
{
    RETURN_CODE_TYPE       retCode;
    PROCESS_ID_TYPE        thOutput1;
    PROCESS_ATTRIBUTE_TYPE thAttrOutput1;

    char* errorMessage = "Failed to transition to NORMAL mode";

    /* Set processes */
    printf("[CORE1][P1] Initialize P1 processes\n");

    thAttrOutput1.ENTRY_POINT   = process1;
    thAttrOutput1.DEADLINE      = SOFT;
    thAttrOutput1.PERIOD        = 1000000000;
    thAttrOutput1.STACK_SIZE    = 0x1000;
    thAttrOutput1.TIME_CAPACITY = 1000000000;
    thAttrOutput1.BASE_PRIORITY = 2;
    memcpy(thAttrOutput1.NAME, "Process1\0", 9 * sizeof(char));

    printf("[CORE1][P1] Initialize P1\n");
    CREATE_PROCESS(&thAttrOutput1, &thOutput1, &retCode);
    if(retCode != NO_ERROR)
    {
        printf("[CORE1][P1] ERROR Creating Process1: %d\n", retCode);
        while(1);
    }

    START(thOutput1, &retCode);
    if(retCode != NO_ERROR)
    {
        printf("[CORE1][P1] ERROR: Starting Process1: %d\n", retCode);
        while(1);
    }

    SET_PARTITION_MODE (NORMAL, &retCode);
    if(retCode != NO_ERROR)
    {
        printf("[CORE1][P1] ERROR: Switching to normal mode: %d\n", retCode);
        while(1);
    }

    RAISE_APPLICATION_ERROR(APPLICATION_ERROR,
                            (MESSAGE_ADDR_TYPE)errorMessage,
                            (ERROR_MESSAGE_SIZE_TYPE)strlen(errorMessage) + 1,
                            &retCode);
}

/* __________________________________________________________________________
 * END OF FILE:
 * -------------
 * ___________________________________________________________________________
 */
