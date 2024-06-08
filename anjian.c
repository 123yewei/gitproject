#define __SYLIXOS_STDIO
#define __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <module.h>
#include "anjian.h"
#include "dtrace.h"


#define KEY_A GPIO_H_05
#define KEY_B GPIO_H_04
//#define  KEY_NUM        GPIO_H_05                                          /*  GPIO5--00                   */
PVOID _G_pvWorkQueue;
static INT _A_iIrqNum;
static INT _B_iIrqNum;
static long lCnt            = 0;

static int  iPhaseDirection = 0;
typedef enum {
    PHASE_STEP_00 = 0x00,
    PHASE_STEP_01 = 0x02,
    PHASE_STEP_02 = 0x03,
    PHASE_STEP_03 = 0x01,
}phaseStepEnum_t;
static int  iPhase          = PHASE_STEP_01;



static irqreturn_t AGpioIsr (PVOID pvArg, ULONG ulVector)
{
    int iFeedback = (API_GpioGetValue(GPIO_H_05) << 1) |
                            (API_GpioGetValue(GPIO_H_04) );
//    static INT ANum=0;
//    ANum++;
//    printk("ANum=%d\n",ANum);
    int value = 0;
    value=API_GpioSvrIrq(KEY_A);
    if(value==PX_ERROR)
    {
        printk(" AGpioIsr 1fault.\n");
        return (LW_IRQ_NONE);

    }else if(value==LW_IRQ_NONE){
        printk(" AGpioIsr 2fault.\n");
        return (LW_IRQ_NONE);
    }else {
        printk(" AGpioIsr.\n");
             API_InterVectorDisable(ulVector);
             API_GpioClearIrq(KEY_A);

             switch(iPhase)
                                     {
                         case PHASE_STEP_01 :
                             iPhaseDirection = 0;
                             if(iFeedback == PHASE_STEP_01){
                                 printk(" A PHASE_STEP_01.\n");
                                 iPhase = PHASE_STEP_02;
                                 break;
            //                        iPhaseDirection++;
            //                        iPhase = PHASE_STEP_01;
                             }
                             else if(iFeedback == PHASE_STEP_03){
                                 iPhase = PHASE_STEP_03;
                                 printk(" B PHASE_STEP_03.\n");
            //                        iPhaseDirection--;
                                 break;
                             }
                             break;
                         case PHASE_STEP_03 :
                             if(iFeedback == PHASE_STEP_03){
                                 printk(" A PHASE_STEP_03.\n");
                                 iPhase = PHASE_STEP_00;
                                 break;
                                         }
                             else if(iFeedback == PHASE_STEP_01)
                             {
                                 printk(" B PHASE_STEP_01.\n");
                                 iPhase = PHASE_STEP_01;
                                 break;
                             }
                             break;
                                     }

             API_InterVectorEnable(ulVector);
             return (LW_IRQ_HANDLED);
    }
}


static irqreturn_t BGpioIsr (PVOID pvArg, ULONG ulVector)
{
    int iFeedback = (API_GpioGetValue(GPIO_H_05) << 1) |
                                (API_GpioGetValue(GPIO_H_04) );
    int value=0;
//    static INT Bnum=0;
//        Bnum++;
//        printk("BNum=%d\n",Bnum);
        value=API_GpioSvrIrq(KEY_B);
        if(value==PX_ERROR)
        {
            printk(" BGpioIsr 1fault.\n");
            return (LW_IRQ_NONE);
        }else if(value==LW_IRQ_NONE){
            printk(" BGpioIsr 2fault.\n");

            return (LW_IRQ_NONE);
        }else {
            printk(" BGpioIsr.\n");
            API_InterVectorDisable(ulVector);
                 API_GpioClearIrq(KEY_B);

     switch(iPhase)
             {
             case PHASE_STEP_02 :
                 iPhaseDirection = 0;
                 if(iFeedback == PHASE_STEP_02){
                     printk(" A PHASE_STEP_02.\n");
                     iPhase = PHASE_STEP_03;
                     break;
//                        iPhaseDirection++;
//                        iPhase = PHASE_STEP_01;
                 }
                 else if(iFeedback == PHASE_STEP_03){
                     iPhase = PHASE_STEP_03;
                     printk(" B PHASE_STEP_03.\n");
//                        iPhaseDirection--;
                     break;
                 }
                 break;
             case PHASE_STEP_00 :
                 if(iFeedback == PHASE_STEP_00){
                     printk(" A PHASE_STEP_00.\n");
                     iPhase = PHASE_STEP_01;
                     break;
                             }
                 else if(iFeedback == PHASE_STEP_01)
                 {
                     printk(" B PHASE_STEP_01.\n");
                     iPhase = PHASE_STEP_01;
                     break;
                 }
                 break;
                         }

                 API_InterVectorEnable(ulVector);
                 return (LW_IRQ_HANDLED);
        }
}
void module_init (void)
{
    INT AError;
    INT BError;
    AError = API_GpioRequestOne(KEY_A, LW_GPIOF_IN, "KEYA");
    _A_iIrqNum = API_GpioSetupIrq(KEY_A, LW_FALSE, 2);
    API_InterVectorSetPriority(_A_iIrqNum,160);
    API_InterVectorSetFlag(_A_iIrqNum, LW_IRQ_FLAG_QUEUE);
    AError = API_InterVectorConnect((ULONG)_A_iIrqNum,
                                    (PINT_SVR_ROUTINE)AGpioIsr,
                                    (PVOID)LW_NULL,
                                    "AGpioIsr");
    API_InterVectorEnable(_A_iIrqNum);

    BError = API_GpioRequestOne(KEY_B, LW_GPIOF_IN, "KEYB");
    _B_iIrqNum = API_GpioSetupIrq(KEY_B, LW_FALSE, 2);
    API_InterVectorSetPriority(_B_iIrqNum,160);
    API_InterVectorSetFlag(_B_iIrqNum, LW_IRQ_FLAG_QUEUE);
    BError = API_InterVectorConnect((ULONG)_B_iIrqNum,
                                           (PINT_SVR_ROUTINE)BGpioIsr,
                                           (PVOID)LW_NULL,
                                           "BGpioIsr");

    API_InterVectorEnable(_B_iIrqNum);
    printk("interrupt_module init!\n");
}

void module_exit (void)
{
    API_InterVectorDisconnect((ULONG)_A_iIrqNum,
                              (PINT_SVR_ROUTINE)AGpioIsr,
                              (PVOID)LW_NULL);
    API_GpioFree(KEY_A);
    API_InterVectorDisconnect((ULONG)_B_iIrqNum,
                                 (PINT_SVR_ROUTINE)BGpioIsr,
                                 (PVOID)LW_NULL);
       API_GpioFree(KEY_B);

    printk("interrupt_module exit!\n");
}
