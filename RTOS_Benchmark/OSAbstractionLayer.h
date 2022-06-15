/* __________________________________________________________________________
* MODULE DESCRIPTION:
* -------------------
* Filename : OSAbstractionLayer.h
*
* Author: Alexy Torres Aurora Dugo
*
* Description: This file contains the abstraction layer (functions and
* configuration) to be updated and integrated in the RTOS to probe.
* __________________________________________________________________________
*/

/* __________________________________________________________________________
* PREPROCESSOR DIRECTIVES:
* ------------------------
*/
#ifndef __OS_ABSTRACTION_LAYER_H__
#define __OS_ABSTRACTION_LAYER_H__

#include <stdint.h>

/******************************************************************************
 * CONFIGURATION
 ******************************************************************************/

/* AMP partitions shared memory base address, must be updated according to the
 * project's configuration.
 */
#define INT_BENCH_SHARED_MEM_BASE 0x70001000

/* The ready mask that tells which partitions to wait during synchronization */
#define INT_BENCH_RDYMASK_VAL 0x00020002000203FFULL

/* Number of samples taken before ending the sampling */
#define INT_BENCH_SAMPLE_COUNT 10000

/* Size of the dumb region for every interrupt types */
#define INT_BENCH_DUMP_REG_SIZE 0x200000

/* Magic value put at the begining of the extraction region (8B) */
#define INT_BENCH_DUMP_REG_HEADER_MAGIC_VAL "INTBDUMP"

/* Magic value put at the begining of the part dump region (4B) */
#define INT_BENCH_DUMP_PART_HEADER_MAGIC_VAL "PART"

/* Magic value put at the begining of the sc dump region (4B) */
#define INT_BENCH_DUMP_SC_HEADER_MAGIC_VAL "SC  "

/* Magic value put at the begining of the internal int dump region (4B) */
#define INT_BENCH_DUMP_INTINT_HEADER_MAGIC_VAL "IINT"

/* Magic value put at the begining of the external int dump region (4B) */
#define INT_BENCH_DUMP_EXTINT_HEADER_MAGIC_VAL "EINT"

/* Magic value put at the begining of the ipi dump region (4B) */
#define INT_BENCH_DUMP_IPI_HEADER_MAGIC_VAL "IPI "

/* Defines the interrupt vector used by the external interrupt. */
#define INT_BENCH_EXTERNAL_INT_VECTOR 80

/*******************************************************************************
 * CONFIGURATION END
 ******************************************************************************/

/*******************************************************************************
 * MEMORY LAYOUT
 ******************************************************************************/
/* NOTE: Here the base is configured to be 0x70001000, but can be changed in
 * the configuration.
 * Regions marked as FREE can be used for future data storage.
 *
 * #------------#---------------------------------------#
 * | 0x70001000 | READY MASK POINTER (8B)               |
 * | 0x70001008 | READY MASK LOCK (4B)                  |
 * | 0x7000100B | FREE                                  |
 * |     ...    | FREE                                  |
 * #------------#---------------------------------------#
 * | 0x70002000 | MAGIC NUMBER                          | <- Extraction start
 * | 0x70002008 | FREE                                  |
 * |     ...    | FREE                                  |
 * #------------#---------------------------------------#
 * | 0x70002100 | PART DUMP MAGIC (4B)                  |
 * | 0x70002104 | PART DUMP REGION SIZE (4B)            |
 * | 0x70002108 | PART DUMP MEMORY REGION (2M - 8B)     |
 * |     ...    | PART DUMP MEMORY REGION (2M - 8B)     |
 * #------------#---------------------------------------#
 * | 0x70202100 | SC DUMP MAGIC (4B)                    |
 * | 0x70202104 | SC DUMP REGION SIZE (4B)              |
 * | 0x70202108 | SC DUMP MEMORY REGION (2M - 8B)       |
 * |     ...    | SC DUMP MEMORY REGION (2M - 8B)       |
 * #------------#---------------------------------------#
 * | 0x70402100 | IntINT DUMP MAGIC (4B)                |
 * | 0x70402104 | IntINT DUMP REGION SIZE (4B)          |
 * | 0x70402108 | IntINT DUMP MEMORY REGION (2M - 8B)   |
 * |     ...    | IntINT DUMP MEMORY REGION (2M - 8B)   |
 * #------------#---------------------------------------#
 * | 0x70602100 | ExtINT DUMP MAGIC (4B)                |
 * | 0x70602104 | ExtINT DUMP REGION SIZE (4B)          |
 * | 0x70602108 | ExtINT DUMP MEMORY REGION (2M - 8B)   |
 * |     ...    | ExtINT DUMP MEMORY REGION (2M - 8B)   |
 * #------------#---------------------------------------#
 * | 0x70802100 | IPI DUMP MAGIC (4B)                   |
 * | 0x70802104 | IPI DUMP REGION SIZE (4B)             |
 * | 0x70802108 | IPI DUMP MEMORY REGION (2M - 8B)      |
 * |     ...    | IPI DUMP MEMORY REGION (2M - 8B)      |
 * #------------#---------------------------------------#
 * | 0x70A02100 | END                                   | <- Extraction end
 * #------------#---------------------------------------#
 *
 * NOTE: This memory region is retreived with a probe (T32 Probe) and dumped as
 * a binary file. If no probe is available, the user will have to implement a
 * way to extract the data stored in this region.
 */

/* The ready mask works as follows:
 * bits[0-15] partitions 0 to 15 on core 0
 * bits[16-31] partitions 0 to 15 on core 1
 * bits[32-47] partitions 0 to 15 on core 2
 * bits[48-63] partitions 0 to 15 on core 3
 */
#define INT_BENCH_RDYMASK_PTR  ((volatile uint64_t*)INT_BENCH_SHARED_MEM_BASE)
#define INT_BENCH_RDYMASK_LOCK ((volatile uint32_t*)(INT_BENCH_SHARED_MEM_BASE + 8))
#define INT_BENCH_INT_WAIT_PTR ((volatile uint32_t*)(INT_BENCH_SHARED_MEM_BASE + 12))

#define INT_BENCH_DUMP_REG_HEADER_ADDR      (INT_BENCH_SHARED_MEM_BASE + 0x1000)
#define INT_BENCH_DUMP_REG_ADDR             (INT_BENCH_SHARED_MEM_BASE + 0x1100)

#define INT_BENCH_DUMP_REG_HEADER_MAGIC     INT_BENCH_DUMP_REG_HEADER_ADDR

#define INT_BENCH_DUMP_PART_MAGIC_ADDR      (INT_BENCH_DUMP_REG_ADDR)
#define INT_BENCH_DUMP_PART_SIZE_ADDR       (INT_BENCH_DUMP_REG_ADDR + 4)
#define INT_BENCH_DUMP_PART_CURSOR_ADDR     (INT_BENCH_DUMP_PART_SIZE_ADDR + 4)
#define INT_BENCH_DUMP_PART_CURSOR_PTR      (INT_BENCH_DUMP_PART_CURSOR_ADDR + *(uint32_t*)INT_BENCH_DUMP_PART_SIZE_ADDR)

#define INT_BENCH_DUMP_SC_MAGIC_ADDR        (INT_BENCH_DUMP_REG_ADDR + INT_BENCH_DUMP_REG_SIZE)
#define INT_BENCH_DUMP_SC_SIZE_ADDR         (INT_BENCH_DUMP_SC_MAGIC_ADDR + 4)
#define INT_BENCH_DUMP_SC_CURSOR_ADDR       (INT_BENCH_DUMP_SC_SIZE_ADDR + 4)
#define INT_BENCH_DUMP_SC_CURSOR_PTR        (INT_BENCH_DUMP_SC_CURSOR_ADDR + *(uint32_t*)INT_BENCH_DUMP_SC_SIZE_ADDR)

#define INT_BENCH_DUMP_INTINT_MAGIC_ADDR    (INT_BENCH_DUMP_REG_ADDR + INT_BENCH_DUMP_REG_SIZE * 2)
#define INT_BENCH_DUMP_INTINT_SIZE_ADDR     (INT_BENCH_DUMP_INTINT_MAGIC_ADDR + 4)
#define INT_BENCH_DUMP_INTINT_CURSOR_ADDR   (INT_BENCH_DUMP_INTINT_SIZE_ADDR + 4)
#define INT_BENCH_DUMP_INTINT_CURSOR_PTR    (INT_BENCH_DUMP_INTINT_CURSOR_ADDR + *(uint32_t*)INT_BENCH_DUMP_INTINT_SIZE_ADDR)

#define INT_BENCH_DUMP_EXTINT_MAGIC_ADDR    (INT_BENCH_DUMP_REG_ADDR + INT_BENCH_DUMP_REG_SIZE * 3)
#define INT_BENCH_DUMP_EXTINT_SIZE_ADDR     (INT_BENCH_DUMP_EXTINT_MAGIC_ADDR + 4)
#define INT_BENCH_DUMP_EXTINT_CURSOR_ADDR   (INT_BENCH_DUMP_EXTINT_SIZE_ADDR + 4)
#define INT_BENCH_DUMP_EXTINT_CURSOR_PTR    (INT_BENCH_DUMP_EXTINT_CURSOR_ADDR + *(uint32_t*)INT_BENCH_DUMP_EXTINT_SIZE_ADDR)

#define INT_BENCH_DUMP_IPI_MAGIC_ADDR       (INT_BENCH_DUMP_REG_ADDR + INT_BENCH_DUMP_REG_SIZE * 4)
#define INT_BENCH_DUMP_IPI_SIZE_ADDR        (INT_BENCH_DUMP_IPI_MAGIC_ADDR + 4)
#define INT_BENCH_DUMP_IPI_CURSOR_ADDR      (INT_BENCH_DUMP_IPI_SIZE_ADDR + 4)
#define INT_BENCH_DUMP_IPI_CURSOR_PTR       (INT_BENCH_DUMP_IPI_CURSOR_ADDR + *(uint32_t*)INT_BENCH_DUMP_IPI_SIZE_ADDR)

/*******************************************************************************
 * MEMORY LAYOUT END
 ******************************************************************************/

/******************************************************************************
 * API
 ******************************************************************************/

/*******************************************************************************
 * API REQUIREMENTS
 * The following functions must be implemented by the OS and sometimes depend
 * on the architecture.
 ******************************************************************************/

/* Test and set function that returns the previous value read in memory */
extern int32_t __TestAndSet(volatile int32_t * lock);

/* Releases a previously acquired lock */
extern void __LockRelease(volatile int32_t * lock);

/* This function should be implemented inside the OS to generate the probing
 * system call.
 */
extern void __IntBenchGenerateSyscall(void);

/*******************************************************************************
 * API REQUIREMENTS END
 ******************************************************************************/

/*******************************************************************************
 * API IMPLEMENTATION
 * The following function are part of the API and can be implemented in the
 * partition itself.
 * The rest of the unimplemented API must be put added to the OS. See additional
 * files to get our implementation of those function in the RTOS.
 ******************************************************************************/

/* Generate an internal interrupt. FIT interrupt on the e6500, the interrupt
 * should happen instantly. Our measurement shown that the interrupt takes no
 * more than 0.19us.
 *
 * @Warning This function enters an infinite loop until the interrupt occur. The
 * interrupt handler is responsible of changing the interrupt return IP value to
 * jump over the infinite loop.
 *
 * @Warning The OS is responsible of handling the interrupt and disabling the FIT
 * after acknowleding the interrupt.
 */
static void __IntBenchGenerateInternalInt(void)
{
    __asm__ __volatile__("__IntBenchGenIntINT:\n\t"
                         "mfspr 3, 340\n\t"
                         "lis 4, 0x0381\n\t"
                         "ori 4, 4, 0xF000\n\t"
                         "or 3, 3, 4\n\t"
                         "mtspr 340, 3\n\t"
                         "__IntBenchGenIntINT_END:\n\t"
                         "b __IntBenchGenIntINT_END\n\t"
                         ::: "3", "4");
}

/* Generate an IPI. On the e6500, the IPI is generated by a doorbell.
 * This function enters an infinite loop until the interrupt occur. The
 * interrupt handler is responsible of changing the interrupt return IP value to
 * jump over the infinite loop.  */
static void __IntBenchGenerateIPI(uint32_t coreId)
{
    /* On T2080 to get the Core ID we must multiply by two (because each core
     * has two threads).
     */
    coreId = coreId << 3;
    __asm__ __volatile__("__IntBenchGenIPI:\n\t"
                         "mr 3, %0\n\t"
                         "msgsnd 3\n\t"
                         "__IntBenchGenIPI_END:\n\t"
                         "b __IntBenchGenIPI_END\n\t"
                         : "=r" (coreId)
                         :: "3");

}

/* Generate an external interrupt. Global timer B (timer 0) interrupt on the
 * T2080, the interrupt happen instantly.
 *
 * @Warning This function enters an infinite loop until the interrupt occur. The
 * interrupt handler is responsible of changing the interrupt return IP value to
 * jump over the infinite loop.
 *
 * @Warning The OS is responsible of handling the interrupt and disabling the
 * Timer after acknowleding the interrupt.
 */

#define BSP_MPIC_GLBL_REG_ADDR (0xFE040000)

#define BSP_MPIC_TFRRB_ADDR    (BSP_MPIC_GLBL_REG_ADDR + 0x20F0)
#define BSP_MPIC_GTBCRB_ADDR   (BSP_MPIC_GLBL_REG_ADDR + 0x2110)
#define BSP_MPIC_GTVPRB_ADDR   (BSP_MPIC_GLBL_REG_ADDR + 0x2120)
#define BSP_MPIC_GTDRB_ADDR    (BSP_MPIC_GLBL_REG_ADDR + 0x2130)
#define BSP_MPIC_TCRB_ADDR     (BSP_MPIC_GLBL_REG_ADDR + 0x2300)

#define MPIC_GTBCR_CI      0x80000000

#define MPIC_TCR_ROVR_MASK 0x07000000
#define MPIC_TCR_RTM_MASK  0x00010000
#define MPIC_TCR_CLKR_MASK 0x00000300
#define MPIC_TCR_CASC_MASK 0x00000007
#define MPIC_TCR_RTM_CCB   0x00000000
#define MPIC_TCR_CLKR_8    0x00000000

#define MPIC_GTVPR_PRIO_MASK   0x000F0000
#define MPIC_GTVPR_VECTOR_MASK 0x0000FFFF
#define MPIC_GTVPR_PRIO_15     0x000F0000
#define MPIC_GTVPR_MSK         0x80000000
#define MPIC_GTVPR_A           0x40000000

#define MPIC_CLOCK_FREQUENCY_HZ 299970000

void __IntBenchGenerateExternalInt(const uint32_t coreId)
{
    volatile uint32_t * MPICReg;

    *(uint32_t*)INT_BENCH_INT_WAIT_PTR = 1;

    /* Make sure the timer count is disabled */
    MPICReg  = (uint32_t *)BSP_MPIC_GTBCRB_ADDR;
    *MPICReg = MPIC_GTBCR_CI;

    /* Set 8 clock ratio based on CCB */
    MPICReg  = (uint32_t *)BSP_MPIC_TCRB_ADDR;
    *MPICReg &= ~(uint32_t)MPIC_TCR_ROVR_MASK & ~(uint32_t)MPIC_TCR_RTM_MASK &
               ~(uint32_t)MPIC_TCR_CLKR_MASK & ~(uint32_t)MPIC_TCR_CASC_MASK;
    *MPICReg |= MPIC_TCR_RTM_CCB | MPIC_TCR_CLKR_8;

    /* Update information register */
    MPICReg  = (uint32_t *)BSP_MPIC_TFRRB_ADDR;
    *MPICReg = MPIC_CLOCK_FREQUENCY_HZ;

    /* Set interrupt to be sent to desired core */
    MPICReg  = (uint32_t *)BSP_MPIC_GTDRB_ADDR;
    *MPICReg = (uint32_t)(1 << (coreId * 2));

    /* Wait to be able to change the values */
    MPICReg  =  (uint32_t *)BSP_MPIC_GTVPRB_ADDR;
    while(0 != (*MPICReg & MPIC_GTVPR_A));

    /* Set priority and vector, unmask interrupt */
    *MPICReg  &= ~(uint32_t)MPIC_GTVPR_PRIO_MASK & ~(uint32_t)MPIC_GTVPR_VECTOR_MASK & ~(uint32_t)MPIC_GTVPR_MSK;
    *MPICReg |= MPIC_GTVPR_PRIO_15 | INT_BENCH_EXTERNAL_INT_VECTOR;

    /* Enable count and set timer period as smallest as possible */
    MPICReg  = (uint32_t *)BSP_MPIC_GTBCRB_ADDR;
    *MPICReg = 0;

    while(1 == *(volatile uint32_t*)INT_BENCH_INT_WAIT_PTR){}
}

#endif  /* ifndef _OS_ABSTRACTION_LAYER_H_ */
/* __________________________________________________________________________
* END OF FILE:
* -------------
* ___________________________________________________________________________
*/