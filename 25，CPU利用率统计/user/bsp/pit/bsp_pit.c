#include "./bsp/nvic/bsp_nvic.h"
#include "./bsp/pit/bsp_pit.h"
#include "./bsp/led/bsp_led.h"


/**
  * @brief  PIT��ʱ����ʼ��
  * @param  ��
  * @retval ��
  */
void PIT_TIMER_Init(void)
{
  pit_config_t pitConfig; //����PIT��ʼ���ṹ��
  
  /* ����PIT��ʱ��ʱ�� OSC_CLK*/
  CLOCK_SetMux(kCLOCK_PerclkMux, 1U);
  /* ���� PERCLK_CLK ʱ�ӷ�ƵΪ 1 */
  CLOCK_SetDiv(kCLOCK_PerclkDiv, 0U);

  /*���PIT��ʱ��Ĭ�����ò���*/
  PIT_GetDefaultConfig(&pitConfig);
  /*��ʼ��PIT */
  PIT_Init(PIT, &pitConfig);
  
  /* ����PIT��ʱ��ͨ��0�Զ���װ��ֵ */
  PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(TIME_0, PIT_SOURCE_CLOCK));
  
  /*���ͨ��0���жϱ�־λ*/
  PIT_ClearStatusFlags(PIT, PIT_CHANNEL_X, kPIT_TimerFlag);
  /* ʹ��ͨ��0�ļ�ʱ����ж� */
  PIT_EnableInterrupts(PIT, PIT_CHANNEL_X, kPIT_TimerInterruptEnable);
  
  /*�����ж����ȼ�,*/
  set_IRQn_Priority(PIT_IRQ_ID,Group4_PreemptPriority_6, Group4_SubPriority_0);
  /* ʹ��PIT��ʱ���ж� */
  EnableIRQ(PIT_IRQ_ID);
}



volatile uint32_t CPU_RunTime = 0UL;
/**
  * @brief  PIT�жϷ�����
  * @param  ��
  * @retval ��
  */
void PIT_IRQHandler(void)
{
  /* ����жϱ�־λ.*/
  PIT_ClearStatusFlags(PIT, PIT_CHANNEL_X, kPIT_TimerFlag);
    
  CPU_RunTime++;
}

