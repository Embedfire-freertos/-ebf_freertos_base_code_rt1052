/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   �¼�
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  i.MXRT1052������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************
  */
#include "fsl_debug_console.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "./led/bsp_led.h"  
#include "./key/bsp_key.h"   

/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
static TaskHandle_t AppTaskCreate_Handle = NULL;/* ���������� */
static TaskHandle_t LED_Task_Handle = NULL;/* LED_Task������ */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY_Task������ */

/********************************** �ں˶����� *********************************/
/*
 * �ź�������Ϣ���У��¼���־�飬�����ʱ����Щ�������ں˵Ķ���Ҫ��ʹ����Щ�ں�
 * ���󣬱����ȴ����������ɹ�֮��᷵��һ����Ӧ�ľ����ʵ���Ͼ���һ��ָ�룬������
 * �ǾͿ���ͨ��������������Щ�ں˶���
 *
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ��ͨ����Щ���ݽṹ���ǿ���ʵ��������ͨ�ţ�
 * �������¼�ͬ���ȸ��ֹ��ܡ�������Щ���ܵ�ʵ��������ͨ��������Щ�ں˶���ĺ���
 * ����ɵ�
 * 
 */
static EventGroupHandle_t Event_Handle =NULL;

/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */


/******************************* �궨�� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩ�궨�塣
 */
#define KEY1_EVENT  (0x01 << 0)//�����¼������λ0
#define KEY2_EVENT  (0x01 << 1)//�����¼������λ1

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */

static void LED_Task(void* pvParameters);/* LED_Task ����ʵ�� */
static void KEY_Task(void* pvParameters);/* KEY_Task ����ʵ�� */

static void BSP_Init(void);/* ���ڳ�ʼ�����������Դ */

/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  /* ������Ӳ����ʼ�� */
  BSP_Init();
	PRINTF("����һ��[Ұ��]-ȫϵ�п�����-FreeRTOS�¼���־��ʵ�飡\n");
   /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */    
}


/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  taskENTER_CRITICAL();           //�����ٽ���
  
  /* ���� Event_Handle */
  Event_Handle = xEventGroupCreate();	 
  if(NULL != Event_Handle)
    PRINTF("Event_Handle �¼������ɹ�!\r\n");
    
  /* ����LED_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )LED_Task, /* ������ں��� */
                        (const char*    )"LED_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )2,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&LED_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    PRINTF("����LED_Task����ɹ�!\r\n");
  
  /* ����KEY_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )KEY_Task,  /* ������ں��� */
                        (const char*    )"KEY_Task",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )3, /* ��������ȼ� */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* ������ƿ�ָ�� */ 
  if(pdPASS == xReturn)
    PRINTF("����KEY_Task����ɹ�!\n");
  
  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}



/**********************************************************************
  * @ ������  �� LED_Task
  * @ ����˵���� LED_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void LED_Task(void* parameter)
{	
  EventBits_t r_event;  /* ����һ���¼����ձ��� */
  /* ������һ������ѭ�������ܷ��� */
  while (1)
	{
    /*******************************************************************
     * �ȴ������¼���־ 
     * 
     * ���xClearOnExit����ΪpdTRUE����ô��xEventGroupWaitBits()����֮ǰ��
     * �������ȴ�����������������ص�ԭ���ǳ�ʱ������ô���¼���������
     * ��uxBitsToWaitFor�е��κ�λ����������� 
     * ���xClearOnExit����ΪpdFALSE��
     * ���ڵ���xEventGroupWaitBits()ʱ����������¼��������õ�λ��
     *
     * xWaitForAllBits���xWaitForAllBits����ΪpdTRUE����uxBitsToWaitFor��
     * ������λ�����û�ָ���Ŀ�ʱ�䵽��ʱ��xEventGroupWaitBits()�ŷ��ء� 
     * ���xWaitForAllBits����ΪpdFALSE��������uxBitsToWaitFor�����õ��κ�
     * һ��λ��1 ��ָ���Ŀ�ʱ�䵽��ʱ��xEventGroupWaitBits()���᷵�ء� 
     * ����ʱ����xTicksToWait����ָ����          
      *********************************************************/
    r_event = xEventGroupWaitBits(Event_Handle,  /* �¼������� */
                                  KEY1_EVENT|KEY2_EVENT,/* �����̸߳���Ȥ���¼� */
                                  pdTRUE,   /* �˳�ʱ����¼�λ */
                                  pdTRUE,   /* �������Ȥ�������¼� */
                                  portMAX_DELAY);/* ָ����ʱ�¼�,һֱ�� */
                        
    if((r_event & (KEY1_EVENT|KEY2_EVENT)) == (KEY1_EVENT|KEY2_EVENT)) 
    {
      /* ���������ɲ�����ȷ */
      PRINTF ( "KEY1��KEY2������\n");		
      LED1_TOGGLE;       //LED1	��ת
    }
    else
      PRINTF ( "�¼�����\n");	
  }
}

/**********************************************************************
  * @ ������  �� KEY_Task
  * @ ����˵���� KEY_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void KEY_Task(void* parameter)
{	 
    /* ������һ������ѭ�������ܷ��� */
  while (1)
  {
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_PIN) == KEY_ON )       //���KEY2������
		{
      PRINTF ( "KEY1������\n" );
			/* ����һ���¼�1 */
			xEventGroupSetBits(Event_Handle,KEY1_EVENT);  					
		}
    
		if( Key_Scan(KEY2_GPIO_PORT,KEY2_PIN) == KEY_ON )       //���KEY2������
		{
      PRINTF ( "KEY2������\n" );	
			/* ����һ���¼�2 */
			xEventGroupSetBits(Event_Handle,KEY2_EVENT); 				
		}
		vTaskDelay(20);     //ÿ20msɨ��һ��		
  }
}


/***********************************************************************
  * @ ������  �� BSP_Init
  * @ ����˵���� �弶�����ʼ�������а����ϵĳ�ʼ�����ɷ��������������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  *********************************************************************/
static void BSP_Init(void)
{
  /* ��ʼ���ڴ汣����Ԫ */
  BOARD_ConfigMPU();
  /* ��ʼ������������ */
  BOARD_InitPins();
  /* ��ʼ��������ʱ�� */
  BOARD_BootClockRUN();
  /* ��ʼ�����Դ��� */
  BOARD_InitDebugConsole();
  /* ��ӡϵͳʱ�� */
  PRINTF("\r\n");
  PRINTF("*****��ӭʹ�� Ұ��i.MX RT1052 ������*****\r\n");
  PRINTF("CPU:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
  PRINTF("AHB:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
  PRINTF("SEMC:            %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));
  PRINTF("SYSPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
  PRINTF("SYSPLLPFD0:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
  PRINTF("SYSPLLPFD1:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
  PRINTF("SYSPLLPFD2:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
  PRINTF("SYSPLLPFD3:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));  
  
  /* ��ʼ��SysTick */
  SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);
  
	/* Ӳ��BSP��ʼ��ͳͳ�����������LED�����ڣ�LCD�� */
    
	/* LED �˿ڳ�ʼ�� */
	LED_GPIO_Config();	
	

  /* KEY �˿ڳ�ʼ�� */
  Key_GPIO_Config();
  
}
/****************************END OF FILE**********************/
