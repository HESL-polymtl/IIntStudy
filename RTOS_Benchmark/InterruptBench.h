/* __________________________________________________________________________
* MODULE DESCRIPTION:
* -------------------
* Filename : InterruptBench.h
*
* Original Author: Alexy Torres Aurora Dugo
*
* Description: This file contains benchmark routine defined to study interrupt
* interferences in multicore ARINC653 systems.
* __________________________________________________________________________
*/

#ifndef __INTERRUPT_BENCH_H__
#define __INTERRUPT_BENCH_H__

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <stdint.h>
#include <ARINC653.h>           /* ARINC653 API (GET_TIME, etc.) */
#include <OSAbstractionLayer.h> /* API, print, etc. */
#include <PMCDriver.h>          /* Performance monitoring counters driver */

/*******************************************************************************
 * CONFIGURATION
 ******************************************************************************/

/* None */

/*******************************************************************************
 * TYPES
 ******************************************************************************/

/* Benchmark data structure that contains the metrics and error codes used in
 * the benchmarks
 */
typedef struct {
    RETURN_CODE_TYPE errCode;
    SYSTEM_TIME_TYPE startTime;
    SYSTEM_TIME_TYPE endTime;
    uint32_t         l2Miss;
    uint32_t         tlbMiss;
    uint32_t         samples;
} int_bench_measure_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* Generates the dump header */
#define INT_BENCH_DUMP_HADER() {                                               \
    memcpy((char*)INT_BENCH_DUMP_REG_HEADER_MAGIC,                             \
           INT_BENCH_DUMP_REG_HEADER_MAGIC_VAL, 8);                            \
    memcpy((char*)INT_BENCH_DUMP_PART_MAGIC_ADDR,                              \
           (char*)INT_BENCH_DUMP_PART_HEADER_MAGIC_VAL, 4);                    \
    memcpy((char*)INT_BENCH_DUMP_SC_MAGIC_ADDR,                                \
           (char*)INT_BENCH_DUMP_SC_HEADER_MAGIC_VAL, 4);                      \
    memcpy((char*)INT_BENCH_DUMP_INTINT_MAGIC_ADDR,                            \
           (char*)INT_BENCH_DUMP_INTINT_HEADER_MAGIC_VAL, 4);                  \
    memcpy((char*)INT_BENCH_DUMP_EXTINT_MAGIC_ADDR,                            \
           (char*)INT_BENCH_DUMP_EXTINT_HEADER_MAGIC_VAL, 4);                  \
    memcpy((char*)INT_BENCH_DUMP_IPI_MAGIC_ADDR,                               \
           (char*)INT_BENCH_DUMP_IPI_HEADER_MAGIC_VAL, 4);                     \
}

/* Dumps the data gathered for the calling internal interrupt */
#define INT_BENCH_DUMP(PARTID, BENCH_DATA, TYPE) {                             \
    /* Here there is not need to protect the data with a lock as only one */   \
    /* partition should execute this at a time.*/                              \
                                                                               \
    *((uint32_t*)INT_BENCH_DUMP_ ## TYPE ## _CURSOR_PTR) = PARTID;             \
    *(uint32_t*)INT_BENCH_DUMP_ ## TYPE ## _SIZE_ADDR += sizeof(uint32_t);     \
                                                                               \
    *((SYSTEM_TIME_TYPE*)INT_BENCH_DUMP_ ## TYPE ## _CURSOR_PTR) =             \
        BENCH_DATA.endTime - BENCH_DATA.startTime;                             \
    *(uint32_t*)INT_BENCH_DUMP_ ## TYPE ## _SIZE_ADDR +=                       \
        sizeof(SYSTEM_TIME_TYPE);                                              \
                                                                               \
    *((uint32_t*)INT_BENCH_DUMP_ ## TYPE ## _CURSOR_PTR) = BENCH_DATA.l2Miss;  \
    *(uint32_t*)INT_BENCH_DUMP_ ## TYPE ## _SIZE_ADDR += sizeof(uint32_t);     \
                                                                               \
    *((uint32_t*)INT_BENCH_DUMP_ ## TYPE ## _CURSOR_PTR) = BENCH_DATA.tlbMiss; \
    *(uint32_t*)INT_BENCH_DUMP_ ## TYPE ## _SIZE_ADDR += sizeof(uint32_t);     \
}

/* Initializes the benchmark data for the calling partition. This routine also
 * synchronize all the cores and partitions before starting the sampling
 */
#define INT_BENCH_INIT(CORE, PARTID, MAFCOUNT, BENCH_DATA) {                   \
    (void)MAFCOUNT;                                                            \
    /* Core 0 Part 0 resets everything, we cannot have an initialization */    \
    /* function that is synchronized with all cores thus we enforce update */  \
    /* of the mask by other partitions until everyone is ready */              \
    while(0 != __TestAndSet(INT_BENCH_RDYMASK_LOCK));                          \
    if(0 == CORE && PARTID == 1)                                               \
    {                                                                          \
        INT_BENCH_DUMP_HADER();                                                \
        *INT_BENCH_RDYMASK_PTR = 0x0000000000000001ULL;                        \
    }                                                                          \
                                                                               \
    while(INT_BENCH_RDYMASK_VAL != *INT_BENCH_RDYMASK_PTR)                     \
    {                                                                          \
        *INT_BENCH_RDYMASK_PTR = (*INT_BENCH_RDYMASK_PTR) |                    \
                                 ((1ULL << PARTID) << (CORE * 16));            \
        __LockRelease(INT_BENCH_RDYMASK_LOCK);                                 \
        PERIODIC_WAIT(&BENCH_DATA.errCode);                                    \
        if (BENCH_DATA.errCode != NO_ERROR)                                    \
        {                                                                      \
            printf("Cannot wait during synchronization: %d\n\r",               \
                   BENCH_DATA.errCode);                                        \
        }                                                                      \
        while(0 != __TestAndSet(INT_BENCH_RDYMASK_LOCK));                      \
    }                                                                          \
    /* Every partitions on every cores are now synchronized */                 \
    __LockRelease(INT_BENCH_RDYMASK_LOCK);                                     \
    MAFCOUNT = 0;                                                              \
    BENCH_DATA.samples = 0;                                                    \
}

/* Starts a sampling iteration. We discard the first and last MAF every 10 MAFs
 * to let other cores initialize and finalize their execution. This way, no
 * interference is unintentionaly generated by the other cores.
 */
#define INT_BENCH_PAYLOAD_PROLOGUE(MAF_COUNT, BENCH_DATA)                      \
    if(0 < MAF_COUNT && 9 > MAF_COUNT)                                         \
    {                                                                          \
        __PMCDrvWrite(1, 0);                                                   \
        __PMCDrvWrite(2, 0);                                                   \
        __PMCDrvEnable(1, E6500_PMC_EVENT_THREAD_L2_MISS, M_PMC_ALL, 0);       \
        __PMCDrvEnable(2, E6500_PMC_EVENT_L2MMU_MISS, M_PMC_ALL, 0);           \
        GET_TIME(&BENCH_DATA.startTime, &BENCH_DATA.errCode);                  \
        if (BENCH_DATA.errCode == NO_ERROR)                                    \
        {

/* Stops a sampling iteration. The gathered data are dumped to memory and the
 * MAF counter is incremented.
 */
#define INT_BENCH_PAYLOAD_EPILOGUE(MAF_COUNT, BENCH_DATA, CORE, PARTID)        \
            GET_TIME(&BENCH_DATA.endTime, &BENCH_DATA.errCode);                \
            if (BENCH_DATA.errCode == NO_ERROR)                                \
            {                                                                  \
                __PMCDrvDisable(1);                                            \
                __PMCDrvDisable(2);                                            \
                __PMCDrvRead(1, &BENCH_DATA.l2Miss);                           \
                __PMCDrvRead(2, &BENCH_DATA.tlbMiss);                          \
                if(INT_BENCH_SAMPLE_COUNT > BENCH_DATA.samples)                \
                {                                                              \
                    INT_BENCH_DUMP(PARTID, BENCH_DATA, PART);                  \
                    printf("C%dP%d %llius\n\r", CORE, PARTID,                  \
                           (BENCH_DATA.endTime - BENCH_DATA.startTime) / 1000);\
                    ++BENCH_DATA.samples;                                      \
                }                                                              \
                else if(INT_BENCH_SAMPLE_COUNT == BENCH_DATA.samples)          \
                {                                                              \
                    printf("[C%dP%d] Sample count reached\n\r",                \
                                       CORE, PARTID);                          \
                    ++BENCH_DATA.samples;                                      \
                }                                                              \
            }                                                                  \
            else                                                               \
            {                                                                  \
                printf("Cannot get end execution time: %d\n\r",                \
                                  BENCH_DATA.errCode);                         \
            }                                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            printf("Cannot get initial execution time: %d\n\r",                \
                               BENCH_DATA.errCode);                            \
        }                                                                      \
    }                                                                          \
    MAF_COUNT = (MAF_COUNT + 1) % 10;

/* Executes payload */
#define INT_BENCH_EXEC_PAYLOAD(PAYLOAD) PAYLOAD();

/* Generate an internal interrupt.
 * This function enters an infinite loop until the interrupt occur. The
 * interrupt handler is responsible of changing the interrupt return IP value to
 * jump over the infinite loop. */
#define INT_BENCH_GEN_INT_INT(COREID, PARTID, BENCH_DATA)                      \
{                                                                              \
    /* Init measurements */                                                    \
    __PMCDrvWrite(3, 0);                                                       \
    __PMCDrvWrite(4, 0);                                                       \
    __PMCDrvEnable(3, E6500_PMC_EVENT_THREAD_L2_MISS, M_PMC_ALL, 0);           \
    __PMCDrvEnable(4, E6500_PMC_EVENT_L2MMU_MISS, M_PMC_ALL, 0);               \
                                                                               \
    GET_TIME(&BENCH_DATA.startTime, &BENCH_DATA.errCode);                      \
    if (BENCH_DATA.errCode == NO_ERROR)                                        \
    {                                                                          \
        /* Generate interrupt */                                               \
        __IntBenchGenerateInternalInt();                                       \
        GET_TIME(&BENCH_DATA.endTime, &BENCH_DATA.errCode);                    \
        if(BENCH_DATA.errCode == NO_ERROR)                                     \
        {                                                                      \
            __PMCDrvDisable(3);                                                \
            __PMCDrvDisable(4);                                                \
            __PMCDrvRead(3, &BENCH_DATA.l2Miss);                               \
            __PMCDrvRead(4, &BENCH_DATA.tlbMiss);                              \
            if(INT_BENCH_SAMPLE_COUNT > BENCH_DATA.samples)                    \
            {                                                                  \
                INT_BENCH_DUMP(PARTID, BENCH_DATA, INTINT);                    \
                ++BENCH_DATA.samples;                                          \
            }                                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            amp_printf_unsafe("Cannot get end execution time: %d\n\r",         \
                                BENCH_DATA.errCode);                           \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        amp_printf_unsafe("Cannot get start execution time: %d\n\r",           \
                            BENCH_DATA.errCode);                               \
    }                                                                          \
}

/* Generate a syscall interrupt */
#define INT_BENCH_GEN_SC(COREID, PARTID, BENCH_DATA)                           \
{                                                                              \
    /* Init measurements */                                                    \
    __PMCDrvWrite(3, 0);                                                       \
    __PMCDrvWrite(4, 0);                                                       \
    __PMCDrvEnable(3, E6500_PMC_EVENT_THREAD_L2_MISS, M_PMC_ALL, 0);           \
    __PMCDrvEnable(4, E6500_PMC_EVENT_L2MMU_MISS, M_PMC_ALL, 0);               \
                                                                               \
    GET_TIME(&BENCH_DATA.startTime, &BENCH_DATA.errCode);                      \
    if (BENCH_DATA.errCode == NO_ERROR)                                        \
    {                                                                          \
        /* Generate syscall */                                                 \
        __IntBenchGenerateSyscall();                                           \
        GET_TIME(&BENCH_DATA.endTime, &BENCH_DATA.errCode);                    \
        if(BENCH_DATA.errCode == NO_ERROR)                                     \
        {                                                                      \
            __PMCDrvDisable(3);                                                \
            __PMCDrvDisable(4);                                                \
            __PMCDrvRead(3, &BENCH_DATA.l2Miss);                               \
            __PMCDrvRead(4, &BENCH_DATA.tlbMiss);                              \
            if(INT_BENCH_SAMPLE_COUNT > BENCH_DATA.samples)                    \
            {                                                                  \
                INT_BENCH_DUMP(PARTID, BENCH_DATA, SC);                        \
                ++BENCH_DATA.samples;                                          \
            }                                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            amp_printf_unsafe("Cannot get end execution time: %d\n\r",         \
                                BENCH_DATA.errCode);                           \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        amp_printf_unsafe("Cannot get start execution time: %d\n\r",           \
                            BENCH_DATA.errCode);                               \
    }                                                                          \
}

/* Generate an IPI.
 * This function enters an infinite loop until the interrupt occur. The
 * interrupt handler is responsible of changing the interrupt return IP value to
 * jump over the infinite loop. */
#define INT_BENCH_GEN_IPI(COREID, PARTID, BENCH_DATA, DSTID)                   \
{                                                                              \
    /* Init measurements */                                                    \
    __PMCDrvWrite(3, 0);                                                       \
    __PMCDrvWrite(4, 0);                                                       \
    __PMCDrvEnable(3, E6500_PMC_EVENT_THREAD_L2_MISS, M_PMC_ALL, 0);           \
    __PMCDrvEnable(4, E6500_PMC_EVENT_L2MMU_MISS, M_PMC_ALL, 0);               \
                                                                               \
    GET_TIME(&BENCH_DATA.startTime, &BENCH_DATA.errCode);                      \
    if (BENCH_DATA.errCode == NO_ERROR)                                        \
    {                                                                          \
        /* Generate interrupt */                                               \
        __IntBenchGenerateIPI(DSTID);                                          \
        GET_TIME(&BENCH_DATA.endTime, &BENCH_DATA.errCode);                    \
        if(BENCH_DATA.errCode == NO_ERROR)                                     \
        {                                                                      \
            __PMCDrvDisable(3);                                                \
            __PMCDrvDisable(4);                                                \
            __PMCDrvRead(3, &BENCH_DATA.l2Miss);                               \
            __PMCDrvRead(4, &BENCH_DATA.tlbMiss);                              \
            if(INT_BENCH_SAMPLE_COUNT > BENCH_DATA.samples)                    \
            {                                                                  \
                INT_BENCH_DUMP(PARTID, BENCH_DATA, IPI);                       \
                ++BENCH_DATA.samples;                                          \
            }                                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            amp_printf_unsafe("Cannot get end execution time: %d\n\r",         \
                                BENCH_DATA.errCode);                           \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        amp_printf_unsafe("Cannot get start execution time: %d\n\r",           \
                            BENCH_DATA.errCode);                               \
    }                                                                          \
}

/* Generate an external interrupt.
 * This function enters an infinite loop until the interrupt occur. The
 * interrupt handler is responsible of changing the interrupt return IP value to
 * jump over the infinite loop. */
#define INT_BENCH_GEN_EXT_INT(COREID, PARTID, BENCH_DATA, DSTID)               \
{                                                                              \
    /* Init measurements */                                                    \
    __PMCDrvWrite(3, 0);                                                       \
    __PMCDrvWrite(4, 0);                                                       \
    __PMCDrvEnable(3, E6500_PMC_EVENT_THREAD_L2_MISS, M_PMC_ALL, 0);           \
    __PMCDrvEnable(4, E6500_PMC_EVENT_L2MMU_MISS, M_PMC_ALL, 0);               \
                                                                               \
    GET_TIME(&BENCH_DATA.startTime, &BENCH_DATA.errCode);                      \
    if (BENCH_DATA.errCode == NO_ERROR)                                        \
    {                                                                          \
        /* Generate interrupt */                                               \
        __IntBenchGenerateExternalInt(DSTID);                                  \
        GET_TIME(&BENCH_DATA.endTime, &BENCH_DATA.errCode);                    \
        if(BENCH_DATA.errCode == NO_ERROR)                                     \
        {                                                                      \
            __PMCDrvDisable(3);                                                \
            __PMCDrvDisable(4);                                                \
            __PMCDrvRead(3, &BENCH_DATA.l2Miss);                               \
            __PMCDrvRead(4, &BENCH_DATA.tlbMiss);                              \
            if(INT_BENCH_SAMPLE_COUNT > BENCH_DATA.samples)                    \
            {                                                                  \
                INT_BENCH_DUMP(PARTID, BENCH_DATA, EXTINT);                    \
                ++BENCH_DATA.samples;                                          \
            }                                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            amp_printf_unsafe("Cannot get end execution time: %d\n\r",         \
                                BENCH_DATA.errCode);                           \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        amp_printf_unsafe("Cannot get start execution time: %d\n\r",           \
                            BENCH_DATA.errCode);                               \
    }                                                                          \
}

#endif  /* ifndef _INTERRUPT_BENCH_H_ */
/* __________________________________________________________________________
* END OF FILE:
* -------------
* ___________________________________________________________________________
*/