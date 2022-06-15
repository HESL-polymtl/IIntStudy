/* __________________________________________________________________________
* MODULE DESCRIPTION:
* -------------------
* Filename : ApplicativePartition.c
*
* Description: This file contains the application partition.
* __________________________________________________________________________
*/

/* INCLUDE SECTION */
#include <stdio.h>
#include <string.h>
#include "InterruptBench.h"

//#define GENERATE_INT

static void bench_routine(void)
{
    /* None, here you can add whatever application you want */
}

void process1(void)
{
    RETURN_CODE_TYPE    retCode;
    T_uint32            mafCount;
    int_bench_measure_t benchData;
    int_bench_measure_t intIntBenchData;
    int_bench_measure_t extIntBenchData;
    int_bench_measure_t scBenchData;
    int_bench_measure_t ipiBenchData;

    memset(&benchData, 0, sizeof(int_bench_measure_t));
    memset(&intIntBenchData, 0, sizeof(int_bench_measure_t));
    memset(&extIntBenchData, 0, sizeof(int_bench_measure_t));
    memset(&scBenchData, 0, sizeof(int_bench_measure_t));
    memset(&ipiBenchData, 0, sizeof(int_bench_measure_t));

    INT_BENCH_INIT(0, 0, mafCount, benchData);

    while(1)
    {
        INT_BENCH_PAYLOAD_PROLOGUE(mafCount, benchData);

        INT_BENCH_EXEC_PAYLOAD(bench_routine);

        INT_BENCH_PAYLOAD_EPILOGUE(mafCount, benchData, 0, 0);

#ifdef GENERATE_INT
        INT_BENCH_GEN_INT_INT(0, 0, intIntBenchData);
        INT_BENCH_GEN_SC(0, 0, scBenchData);
        INT_BENCH_GEN_IPI(0, 0, ipiBenchData, 0);
        INT_BENCH_GEN_EXT_INT(0, 0, extIntBenchData, 0);
#endif

        PERIODIC_WAIT(&retCode);
        if(NO_ERROR != retCode)
        {
            printf("[C0P0] Cannot periodic wait: %d\n", retCode);
        }
    }
}

/* __________________________________________________________________________
 *
 * FUNCTION NAME : main_process
 * DESCRIPTION   : Partition WR1Partition_T2080RDB_PCA main function.
 * PARAMETERS :    None.
 * INPUT :         None.
 * OUTPUT :        None.
 * RETURN :        None.
 * __________________________________________________________________________
 */
void main_process(void)
{
    RETURN_CODE_TYPE       retCode;
    PROCESS_ID_TYPE        thOutput1;
    PROCESS_ATTRIBUTE_TYPE thAttrOutput1;

    char* errorMessage = "Failed to transition to NORMAL mode";

    /* Set processes */
    printf("[CORE0][P0] Initialize P0 processes\n");

    thAttrOutput1.ENTRY_POINT   = process1;
    thAttrOutput1.DEADLINE      = SOFT;
    thAttrOutput1.PERIOD        = 100000000;
    thAttrOutput1.STACK_SIZE    = 0x1000;
    thAttrOutput1.TIME_CAPACITY = 100000000;
    thAttrOutput1.BASE_PRIORITY = 2;
    memcpy(thAttrOutput1.NAME, "Process1\0", 9 * sizeof(char));

    printf("[CORE0][P0] Initialize P0\n");
    CREATE_PROCESS(&thAttrOutput1, &thOutput1, &retCode);
    if(retCode != NO_ERROR)
    {
        printf("[CORE0][P0] ERROR Creating Process1: %d\n", retCode);
        while(1);
    }

    START(thOutput1, &retCode);
    if(retCode != NO_ERROR)
    {
        printf("[CORE0][P0] ERROR: Starting Process1: %d\n", retCode);
        while(1);
    }

    SET_PARTITION_MODE (NORMAL, &retCode);
    if(retCode != NO_ERROR)
    {
        printf("[CORE0][P0] ERROR: Switching to normal mode: %d\n", retCode);
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
