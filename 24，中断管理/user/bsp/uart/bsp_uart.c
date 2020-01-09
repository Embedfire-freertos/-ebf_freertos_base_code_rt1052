/**
  ******************************************************************
  * @file    bsp_usart.c
  * @author  fire
  * @version V1.1
  * @date    2018-xx-xx
  * @brief   uartӦ�ú����ӿ�
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  i.MXRT1052������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************
  */

#include "board.h"
#include "fsl_iomuxc.h"
#include "fsl_gpio.h"  
#include "fsl_lpuart.h"
  
#include "pad_config.h"  
#include "fsl_debug_console.h"
#include "./bsp/nvic/bsp_nvic.h"
#include "./bsp/uart/bsp_uart.h"
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

uint8_t RX_BUFF[USART_RBUFF_SIZE]={0};

/**
* @brief  ��ʼ��uart���ò���
* @param  ��
* @retval ��
*/
void UART_ModeConfig(void)
{
  /*���崮�����ò����ṹ����������ڱ��洮�ڵ�������Ϣ*/
  lpuart_config_t config;
  
  /*���ù̼��⺯���õ�Ĭ�ϵĴ������ò�������Ĭ�ϵ����ò����������޸�*/
  LPUART_GetDefaultConfig(&config);
  config.baudRate_Bps = DEBUG_UART_BAUDRATE;  //������
  config.enableRx = DEBUG_UART_ENABLE_RESIVE; //�Ƿ������������
  config.enableTx = DEBUG_UART_ENABLE_SEND;   //�Ƿ�����������
  
  /*���ù̼��⺯�������޸ĺõ�������Ϣд�뵽���ڵ����üĴ�����*/
  LPUART_Init(DEBUG_UARTx, &config, BOARD_DEBUG_UART_CLK_FREQ);


  /*��������ж�*/
  LPUART_EnableInterrupts(DEBUG_UARTx, kLPUART_RxDataRegFullInterruptEnable);
  
  /*�����ж����ȼ�,*/
  set_IRQn_Priority(DEBUG_UART_IRQ,Group4_PreemptPriority_6, Group4_SubPriority_0);
  /*ʹ���ж�*/
  EnableIRQ(DEBUG_UART_IRQ);
  
  //LPUART_EnableRx(DEBUG_USARTx, true);  
  //LPUART_EnableTx(DEBUG_USARTx, false);
}

 /**
* @brief  ��ʼ��uart���Ź���
* @param  ��
* @retval ��
*/
void UART_IOMUXC_MUX_Config(void)
{
  /* RX��TX���� */
  IOMUXC_SetPinMux(UART_RX_IOMUXC, 0U);                                      
  IOMUXC_SetPinMux(UART_TX_IOMUXC, 0U); 
}
 
 /**
* @brief  ��ʼ��uart���IOMUXC��PAD��������
* @param  ��
* @retval ��
*/
void UART_IOMUXC_PAD_Config(void)
{
  IOMUXC_SetPinConfig(UART_RX_IOMUXC, UART_RX_PAD_CONFIG_DATA);
  IOMUXC_SetPinConfig(UART_TX_IOMUXC, UART_TX_PAD_CONFIG_DATA);
}
  /**
* @brief  ��ʼ��uart,���������շ�����
* @param  ��
* @retval ��
*/
void UART_Config(void)
{
  UART_IOMUXC_MUX_Config();
  UART_IOMUXC_PAD_Config();
  UART_ModeConfig();
}


  /**
* @brief  ����һ���ַ� 
* @param  base:ѡ��˿�
* @param  data:��Ҫ���͵�����
* @retval ��
*/
void Uart_SendByte(LPUART_Type *base, uint8_t data)
{
  LPUART_WriteByte( base, data);
  while (!(base->STAT & LPUART_STAT_TDRE_MASK));
}

  /**
* @brief  ����һ���ַ��� 
* @param  base:ѡ��˿�
* @param  data:��Ҫ���͵�����
* @retval ��
*/
void Uart_SendString( LPUART_Type *base,  const char *str)
{
  LPUART_WriteBlocking( base, (const uint8_t *)str, strlen(str));
}
 

  /**
* @brief  ����һ��16λ��  
* @param  base:ѡ��˿�
* @param  data:��Ҫ���͵�����
* @retval ��
*/
void Uart_SendHalfWord(LPUART_Type *base, uint16_t ch)
{
  uint8_t temp_h, temp_l;
  
  /* ȡ���߰�λ */
  temp_h = (ch&0XFF00)>>8;
  /* ȡ���Ͱ�λ */
  temp_l = ch&0XFF; 

  /* ���͸߰�λ */
  LPUART_WriteByte( base, temp_h);
  while (!(base->STAT & LPUART_STAT_TDRE_MASK));
  
  /* ���͵Ͱ�λ */ 
  LPUART_WriteByte( base, temp_l);
  while (!(base->STAT & LPUART_STAT_TDRE_MASK));  
}
extern SemaphoreHandle_t BinarySem_Handle;

/******************���ڽ����жϷ�����********************/
extern SemaphoreHandle_t BinarySem_Handle;
void UART_IdelCallback(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
  
  //������ֵ�ź��� �����ͽ��յ������ݱ�־����ǰ̨�����ѯ
  xSemaphoreGiveFromISR(BinarySem_Handle,&pxHigherPriorityTaskWoken);	//�ͷŶ�ֵ�ź���
  //�����Ҫ�Ļ�����һ�������л���ϵͳ���ж��Ƿ���Ҫ�����л�
  portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}
int index_num=0;
uint8_t arrary[30];
uint8_t str_c;//���Ե��ַ���
void DEBUG_UART_IRQHandler(void)
{
	uint32_t ulReturn;
  /* �����ٽ�Σ��ٽ�ο���Ƕ�� */
  ulReturn = taskENTER_CRITICAL_FROM_ISR();

	uint8_t ucTemp;
  /*���ڽ��յ�����*/
  if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(DEBUG_UARTx))
  {
			/*��ȡ����*///��仰һ��Ҫ�У������岻�˱�־���������ж�����
			ucTemp = LPUART_ReadByte(DEBUG_UARTx);
			RX_BUFF[index_num++]=ucTemp;
			//��������ʱ ĩβ �ӻس�����
			if((RX_BUFF[index_num-2]==0x0D)&&(RX_BUFF[index_num-1]==0x0A))
			{
				UART_IdelCallback();	/* �ͷ�һ���ź�������ʾ�����ѽ��� */
			}
   }			
 
  /* �˳��ٽ�� */
  taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}




