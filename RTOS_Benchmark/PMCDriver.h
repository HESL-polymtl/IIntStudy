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

#ifndef __PMC_DRIVER_H__
#define __PMC_DRIVER_H__


#define stringify(s) tostring(s)
#define tostring(s) #s

#define mfpmr(rn) {                                             \
    uint32_t rval;                                              \
    __asm__ volatile("mfpmr %0," stringify(rn) : "=r" (rval));  \
    rval;                                                       \
}

#define mtpmr(rn, v) __asm__ volatile("mtpmr " stringify(rn) ",%0" : : "r" (v))

#define E6500_PMC_ID_MAX_VALUE 5

typedef enum
{
  NO_ERROR        = 0,
  INVALID_PARAM   = 1,
} ERROR_CODE_E;

/* PMC Trace entity: select the elevation privilege that will be able to trace
 * PMC events.
 */
typedef enum
{
  PMC_SUPERVISOR,
  PMC_USER,
  PMC_ALL
} PMC_TRACE_ENTITY_E;

/*------------------------------------------------------*/
/* PMC MSR: PMGC0 Global PMC MSR setting register flags */
/*------------------------------------------------------*/

/* FAC flag: globaly enable PMCs */
#define PMR_PMGC0_FAC     ((uint32_t)(1 << 31))
/* PMIE flag: globaly enable PMCs interrupt capability */
#define PMR_PMGC0_PMIE     ((uint32_t)(1 << 30))
/* FCECE flag: globaly enable PMCs count event capability */
#define PMR_PMGC0_FCECE    ((uint32_t)(1 << 29))
/* Clear mask, used to clear bits that should be cleared according to the
 * core manuals.
 */
#define PMR_PMGC0_CLEARED 0x3FFFE6FF

/*---------------------------------------------------------*/
/* PMC MSR: PMLCaX PMC specific MSR setting register flags */
/*---------------------------------------------------------*/

/* FC flag: freeze the counter. */
#define PMR_PMLCAX_FC      ((uint32_t)(1 << 31))
/* FCS flag: Freeze counter when in supervisor mode. */
#define PMR_PMLCAX_FCS     ((uint32_t)(1 << 30))
/* FCU flag: Freeze counter when in user mode. */
#define PMR_PMLCAX_FCU     ((uint32_t)(1 << 29))
/* FCM1 flag: Freeze counter when MSR[PMM] is cleared. */
#define PMR_PMLCAX_FCM1    ((uint32_t)(1 << 28))
/* FCM0 flag: Freeze counter when MSR[PMM] is set. */
#define PMR_PMLCAX_FCM0    ((uint32_t)(1 << 27))
/* CE flag: Enables PMC interrupt on overflow. */
#define PMR_PMLCAX_CE      ((uint32_t)(1 << 26))
/* Clear mask, used to clear bits that should be cleared according to the
 * core manual.
 */
#define PMR_PMLCAX_CLEARED 0x0200FFFC
/* Event position shift in the the PMLCaX register. */
#define PMR_PMLCaX_EVENT_SHIFT 16         /* PMLCaX event first bit */
/* Event position placeholder in the the PMLCaX register. */
#define PMR_PMLCaX_EVENT_MASK  0x01FF0000 /* PMLCaX event placeholder */

/*---------------------------------------------------------*/
/* PMC MSR: PMLCbX PMC specific MSR setting register flags */
/*---------------------------------------------------------*/

/* Clear mask, used to clear bits that should be cleared according to the
 * core manual.
 */
#define PMR_PMLCBX_CLEARED 0xFFFFF8C0   /* PMLCbX bits to be cleared */

/*---------------------------------------------------------*/
/* PMC registers                                           */
/*---------------------------------------------------------*/
#define PMR_PMC  0x010   /* PMC Register base id */
#define PMR_PMC0 0x010   /* PMC Register 0 id */
#define PMR_PMC1 0x011   /* PMC Register 1 id */
#define PMR_PMC2 0x012   /* PMC Register 2 id */
#define PMR_PMC3 0x013   /* PMC Register 3 id */
#define PMR_PMC4 0x014   /* PMC Register 4 id */
#define PMR_PMC5 0x015   /* PMC Register 5 id */

#define PMR_PMLCA  0x090 /* PMC Local Settings A Register base id  */
#define PMR_PMLCA0 0x090 /* PMC Local Settings A Register 0 id  */
#define PMR_PMLCA1 0x091 /* PMC Local Settings A Register 1 id  */
#define PMR_PMLCA2 0x092 /* PMC Local Settings A Register 2 id  */
#define PMR_PMLCA3 0x093 /* PMC Local Settings A Register 3 id  */
#define PMR_PMLCA4 0x094 /* PMC Local Settings A Register 4 id  */
#define PMR_PMLCA5 0x095 /* PMC Local Settings A Register 5 id  */

#define PMR_PMLCB  0x110 /* PMC Local Settings B Register base id  */
#define PMR_PMLCB0 0x110 /* PMC Local Settings B Register 0 id  */
#define PMR_PMLCB1 0x111 /* PMC Local Settings B Register 1 id  */
#define PMR_PMLCB2 0x112 /* PMC Local Settings B Register 2 id  */
#define PMR_PMLCB3 0x113 /* PMC Local Settings B Register 3 id  */
#define PMR_PMLCB4 0x114 /* PMC Local Settings B Register 4 id  */
#define PMR_PMLCB5 0x115 /* PMC Local Settings B Register 5 id  */

#define PMR_PMGC0 0x190 /* PMC Global Settings Register */



#define E6500_PMC_EVENT_CPU_CYCLES      1
#define E6500_PMC_EVENT_INSTR_COMPLTD   2
#define E6500_PMC_EVENT_TOTAL_TRANSL    26
#define E6500_PMC_EVENT_CACHE_INHIBIT   31
#define E6500_PMC_EVENT_LSU_STALL       110
#define E6500_PMC_EVENT_L2MMU_MISS      264
#define E6500_PMC_EVENT_L2_HIT          456
#define E6500_PMC_EVENT_L2_MISS         457
#define E6500_PMC_EVENT_THREAD_L2_HIT   465
#define E6500_PMC_EVENT_THREAD_L2_MISS  466
#define E6500_PMC_EVENT_THREAD_L2_ACC   467


/* FUNCTIONS DEFINITIONS */
static void __PMCDrvWritePMR(const uint32_t pmrId, const uint32_t value)
{
    switch(pmrId)
    {
        /* PMR Value */
        case PMR_PMC:
            mtpmr(PMR_PMC0, value);
            break;
        case PMR_PMC + 1:
            mtpmr(PMR_PMC1, value);
            break;
        case PMR_PMC + 2:
            mtpmr(PMR_PMC2, value);
            break;
        case PMR_PMC + 3:
            mtpmr(PMR_PMC3, value);
            break;
        case PMR_PMC + 4:
            mtpmr(PMR_PMC4, value);
            break;
        case PMR_PMC + 5:
            mtpmr(PMR_PMC5, value);
            break;

        /* PMR_PMLCA Value */
        case PMR_PMLCA:
            mtpmr(PMR_PMLCA0, value);
            break;
        case PMR_PMLCA + 1:
            mtpmr(PMR_PMLCA1, value);
            break;
        case PMR_PMLCA + 2:
            mtpmr(PMR_PMLCA2, value);
            break;
        case PMR_PMLCA + 3:
            mtpmr(PMR_PMLCA3, value);
            break;
        case PMR_PMLCA + 4:
            mtpmr(PMR_PMLCA4, value);
            break;
        case PMR_PMLCA + 5:
            mtpmr(PMR_PMLCA5, value);
            break;

        /* PMR_PMLCB Value */
        case PMR_PMLCB:
            mtpmr(PMR_PMLCB0, value);
            break;
        case PMR_PMLCB + 1:
            mtpmr(PMR_PMLCB1, value);
            break;
        case PMR_PMLCB + 2:
            mtpmr(PMR_PMLCB2, value);
            break;
        case PMR_PMLCB + 3:
            mtpmr(PMR_PMLCB3, value);
            break;
        case PMR_PMLCB + 4:
            mtpmr(PMR_PMLCB4, value);
            break;
        case PMR_PMLCB + 5:
            mtpmr(PMR_PMLCB5, value);
            break;

        /* PMR_PMGC0 Value */
        case PMR_PMGC0:
            mtpmr(PMR_PMGC0, value);
            break;

        default:
            break;
    }
}

static uint32_t __PMCDrvReadPMR(const uint32_t pmrId)
{
    uint32_t value;

    switch(pmrId)
    {
        /* PMR Value */
        case PMR_PMC:
            value = mfpmr(PMR_PMC0);
            break;
        case PMR_PMC + 1:
            value = mfpmr(PMR_PMC1);
            break;
        case PMR_PMC + 2:
            value = mfpmr(PMR_PMC2);
            break;
        case PMR_PMC + 3:
            value = mfpmr(PMR_PMC3);
            break;
        case PMR_PMC + 4:
            value = mfpmr(PMR_PMC4);
            break;
        case PMR_PMC + 5:
            value = mfpmr(PMR_PMC5);
            break;

        /* PMR_PMLCA Value */
        case PMR_PMLCA:
            value = mfpmr(PMR_PMLCA0);
            break;
        case PMR_PMLCA + 1:
            value = mfpmr(PMR_PMLCA1);
            break;
        case PMR_PMLCA + 2:
            value = mfpmr(PMR_PMLCA2);
            break;
        case PMR_PMLCA + 3:
            value = mfpmr(PMR_PMLCA3);
            break;
        case PMR_PMLCA + 4:
            value = mfpmr(PMR_PMLCA4);
            break;
        case PMR_PMLCA + 5:
            value = mfpmr(PMR_PMLCA5);
            break;

        /* PMR_PMLCB Value */
        case PMR_PMLCB:
            value = mfpmr(PMR_PMLCB0);
            break;
        case PMR_PMLCB + 1:
            value = mfpmr(PMR_PMLCB1);
            break;
        case PMR_PMLCB + 2:
            value = mfpmr(PMR_PMLCB2);
            break;
        case PMR_PMLCB + 3:
            value = mfpmr(PMR_PMLCB3);
            break;
        case PMR_PMLCB + 4:
            value = mfpmr(PMR_PMLCB4);
            break;
        case PMR_PMLCB + 5:
            value = mfpmr(PMR_PMLCB5);
            break;

        /* PMR_PMGC0 Value */
        case PMR_PMGC0:
            value = mfpmr(PMR_PMGC0);
            break;

        default:
            value = 0xFFFFFFFF;
    }

    return value;
}

ERROR_CODE_E __PMCDrvRead(const uint32_t pmcID, uint32_t* pPmcValue)
{
  ERROR_CODE_E retCode;

  retCode = NO_ERROR;

  if(E6500_PMC_ID_MAX_VALUE < pmcID)
  {
    retCode = INVALID_PARAM;
  }
  else
  {
    *pPmcValue = __PMCDrvReadPMR(PMR_PMC + pmcID);
  }

  return retCode;
}

ERROR_CODE_E __PMCDrvWrite(const uint32_t pmcID, const uint32_t pmcValue)
{
  ERROR_CODE_E retCode;

  retCode = NO_ERROR;

  if(E6500_PMC_ID_MAX_VALUE < pmcID)
  {
    retCode = INVALID_PARAM;
  }
  else
  {
    __PMCDrvWritePMR(PMR_PMC + pmcID, pmcValue);
  }

  return retCode;
}

ERROR_CODE_E __PMCDrvEnable(const uint32_t pmcID, const int32_t event,
                            const PMC_TRACE_ENTITY_E traceEntity,
                            const uint32_t intEnabled)
{
    uint32_t     pmrValue;
    ERROR_CODE_E retCode;

    retCode = NO_ERROR;

    if(E6500_PMC_ID_MAX_VALUE < pmcID)
    {
        /* Check if the pmcID is valid detected an error */
        retCode = INVALID_PARAM;
    }
    else
    {

        pmrValue = (uint32_t) (PMR_PMLCaX_EVENT_MASK & (event << PMR_PMLCaX_EVENT_SHIFT));

        /* Clear to un-freeze counter */
        pmrValue &= (uint32_t) (~PMR_PMLCAX_FC);

        if(PMC_SUPERVISOR == traceEntity || PMC_ALL == traceEntity)
        {
            /* Un-freeze for supervisor */
            pmrValue &= (uint32_t) (~PMR_PMLCAX_FCS);

        }
        if(PMC_USER == traceEntity || PMC_ALL == traceEntity)
        {
            /* Un-freeze for user */
            pmrValue &= (uint32_t) (~PMR_PMLCAX_FCU);
        }

            /* For the moment we don't use MSR[PMM] to set the PMC trace,
            * so clear these two bits.
            */
            pmrValue &= (uint32_t) (~PMR_PMLCAX_FCM1);
            pmrValue &= (uint32_t) (~PMR_PMLCAX_FCM0);

        if(0 != intEnabled)
        {
            /* Clear to freeze counter on event (interrupt are then enabled) */
            pmrValue |= (uint32_t) PMR_PMLCAX_CE;
        }

        else
        {
            /* Clear to avoid freeze counter (interrupt are then disabled) */
            pmrValue &= (uint32_t) (~PMR_PMLCAX_CE);
        }

        /* Write value to PMR */
        __PMCDrvWritePMR(PMR_PMLCA + pmcID, pmrValue);

        /*************************************
         * PMLCbX Settings
         ************************************/
        pmrValue = 0x00000000;
        __PMCDrvWritePMR(PMR_PMLCB + pmcID, pmrValue);

        /*************************************
         * PMGC0 Settings
         ************************************/
        pmrValue = __PMCDrvReadPMR(PMR_PMGC0);

        pmrValue &= ~PMR_PMGC0_FAC;
        /* Set to enable global PMC interrupt */
        pmrValue |= PMR_PMGC0_PMIE;
        /* Set to freeze counters conditionally */
        pmrValue |= PMR_PMGC0_FCECE;

        __PMCDrvWritePMR(PMR_PMGC0, pmrValue);
    }

    return retCode;
}

ERROR_CODE_E __PMCDrvDisable(const uint32_t pmcID)
{
    uint32_t     pmrValue;
    ERROR_CODE_E retCode;

    retCode = NO_ERROR;

    if(E6500_PMC_ID_MAX_VALUE < pmcID)
    {
        /* Check if the pmcID is valid detected an error */
        retCode = INVALID_PARAM;
    }
    else
    {
        /*************************************
         * PMLCaX Settings
         ************************************/
        /* Set to freeze counter */
        pmrValue = (uint32_t) PMR_PMLCAX_FC;
        __PMCDrvWritePMR(PMR_PMLCA + pmcID, pmrValue);
    }

    return retCode;
}

#endif /* __PMC_DRIVER_H__ */

/* __________________________________________________________________________
* END OF FILE:
* -------------
* ___________________________________________________________________________
*/

