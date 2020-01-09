#ifndef __BSP_PIT_H
#define __BSP_PIT_H

#include "fsl_common.h"
#include "fsl_pit.h"

/*��ʱ��ʱ��Ƶ�ʺ궨��*/
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_OscClk) 

/*�жϺ궨��*/
#define PIT_IRQ_ID PIT_IRQn

/*��ʱ��ͨ���궨��*/
#define PIT_CHANNEL_X kPIT_Chnl_0

/*��ʱʱ��궨�壨��λ��us)*/
#define TIME_0 10000U

void PIT_TIMER_Init(void);


#endif /* __BSP_PIT_H */

