#include "main.h"
#include "OLED_I2C.h"
#include "led.h"
#include "uart.h"
#include "pwm_curr_fdbk.h"
#include "drv8323.h"
#include "encoder.h"
#include "systick.h"
#include "foc.h"
#include "hw_config.h"
#include "usr_config.h"
#include "calibration.h"
#include "util.h"
#include "arm_math.h"
#include "controller.h"
#include "anticogging.h"
#include "heap.h"
#include "fdcan.h"
#include "fsm.h"


void SystemClock_Config(void);
void MX_GPIO_Init(void);


int main(void)
{
	 SystemClock_Config(); 
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	HAL_Init();

	SYSTICK_init();
	LED_init();
	FDCAN_init();
	MX_GPIO_Init();
	MX_I2C2_Init();
	
	
	/* OLED CODE */
	HAL_Delay(1000);
	OLED_Init();
	OLED_Fill(0x00);// all screem light off
	HAL_Delay(2000);
	OLED_ShowStr(0,1,"HAL_Delay()" ,2);//²âÊÔ6*8×Ö·û
	
#ifdef __DEBUG__
	UART_init(115200);
#endif
	ENCODER_init();		// Encoder chip power up time 260ms
	PWMC_init();
//	if(0 != DRV8323_init()){
//		DEBUG("\n\rDRV8323 init fail!\n\r");
//		while(1);
//	}
	
	DEBUG("\n\r");
	if(0 == USR_CONFIG_read_config()){
		DEBUG("Config loaded\n\r");
	}else{
		USR_CONFIG_set_default_config();
		DEBUG("Config loaded faile set to default\n\r");
	}
	
	if(0 == USR_CONFIG_read_cogging_map()){
		AnticoggingValid = true;
		DEBUG("Cogging map loaded\n\r\n\r");
	}else{
		USR_CONFIG_set_default_cogging_map();
		DEBUG("Cogging map loaded faile set to default\n\r\n\r");
	}
	
	FSM_input(CMD_MENU);
	
	while(1){
		LED_loop();
		FDCAN_timeout_check_loop();
		ANTICOGGING_loop(&Controller);
		
		HAL_Delay(1);
	}
}



/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOF);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

}


////HSE() 24M
#ifdef OS24M
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
  {
  }
  LL_PWR_EnableRange1BoostMode();
  LL_RCC_HSE_Enable();
   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {
  }

  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_6, 85, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_EnableDomain_SYS();
  LL_RCC_PLL_Enable();
   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Insure 1?s transition state at intermediate medium speed clock based on DWT */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
  while(DWT->CYCCNT < 100);
  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_SetSystemCoreClock(170000000);

   /* Update the time base */
//  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
//  {
//    Error_Handler();
//  }
  LL_RCC_SetI2CClockSource(LL_RCC_I2C2_CLKSOURCE_PCLK1); 
	LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
	LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_SYSCLK);
	LL_RCC_SetFDCANClockSource(LL_RCC_FDCAN_CLKSOURCE_PCLK1);
}
#endif


#ifdef OS8M
//HSE() 8M
void SystemClock_Config(void)
{
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
	while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4){}
	LL_PWR_EnableRange1BoostMode();
	LL_RCC_HSE_Enable();
	/* Wait till HSE is ready */
	while(LL_RCC_HSE_IsReady() != 1){}

	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 85, LL_RCC_PLLR_DIV_2);
	LL_RCC_PLL_EnableDomain_SYS();
	LL_RCC_PLL_Enable();
	/* Wait till PLL is ready */
	while(LL_RCC_PLL_IsReady() != 1){}

	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
	/* Wait till System clock is ready */
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL){}

	/* Insure 1?s transition state at intermediate medium speed clock based on DWT */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	DWT->CYCCNT = 0;
	while(DWT->CYCCNT < 100);

	/* Set AHB prescaler*/
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	LL_SetSystemCoreClock(170000000);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C2_CLKSOURCE_PCLK1); 
	LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
	LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_SYSCLK);
	LL_RCC_SetFDCANClockSource(LL_RCC_FDCAN_CLKSOURCE_PCLK1);
}

#endif

#ifdef HSI
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
  {
  }
  LL_PWR_EnableRange1BoostMode();
  LL_RCC_HSI_Enable();
   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  LL_RCC_HSI_SetCalibTrimming(64);
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_4, 85, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_EnableDomain_SYS();
  LL_RCC_PLL_Enable();
   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Insure 1?s transition state at intermediate medium speed clock based on DWT */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
  while(DWT->CYCCNT < 100);
  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_SetSystemCoreClock(170000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetI2CClockSource(LL_RCC_I2C2_CLKSOURCE_PCLK1); 
	LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
	LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_SYSCLK);
	LL_RCC_SetFDCANClockSource(LL_RCC_FDCAN_CLKSOURCE_PCLK1);
}



#endif

void Error_Handler(void)
{
	__disable_irq();
	
	FOC_disarm();
	
	while(1){}
}
