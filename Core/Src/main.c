/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
	off = 0,
	on  = 1
}type_on_off;

typedef enum
{
	False = 0,
	True  = 1
}type_bool;

typedef struct
{
	uint32_t initial_time;
	uint32_t elapsed_time;
	uint32_t delay_time;
}type_ST; // tipo soft timer

typedef enum
{
	Inactive = 0,
	Active  = 1
}type_bool_state;

typedef struct
{
	type_bool_state state;
	uint32_t T;
	uint32_t t_act;
	uint32_t t_ina;
	float duty_cycle;
	GPIO_TypeDef *Port;
	uint16_t Pin;
	type_ST timer;
	uint32_t T_shadow;
	float duty_cicle_shadow;
	type_bool_state shadow;

}type_PWM; // tipo PWM

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

type_ST ST_Timer1;
type_PWM PWM1;
type_on_off LED_B_state;

type_bool_state BOT_B_atu, BOT_B_ant;
type_ST ST_Timer_db_BOT_B;
type_transition_state BOT_B_Rising_Transition;


gpio_edge_t USER_BUTTON_Rising_Edge;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void ST_Init(type_ST *pST, uint32_t time_lapse);
type_bool ST(type_ST *pST);
void ST_Lapse(type_ST *pST);

void PWM_Run(type_PWM *pPWM);
void PWM_Init(type_PWM *pPWM, GPIO_TypeDef* GPIO_Port, uint16_t GPIO_Pin, 
              uint32_t Period, float Duty); 
void PWM_Update(type_PWM *pPWM, uint32_t Period, 
                float Duty, type_bool_state shadow);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	/* USER CODE BEGIN 2 */
	
	ST_Init(&ST_Timer1, 500);
	HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
	
	PWM_Init(&PWM1, LED_O_GPIO_Port, LED_O_Pin, 1000, 0.5);
	
	LED_B_state = off;
	
	int i = 0;
	
	BOT_B_atu = Active;
	BOT_B_ant = Active;
	
	BOT_B_Rising_Transition = Detecting;
	
	GPIO_Edge_Init(&USER_BUTTON_Rising_Edge, BOT_B_GPIO_Port, BOT_B_Pin, DEBOUNCE_DELAY);
	
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		
		if (ST(&ST_Timer1)) {
			ST_Lapse(&ST_Timer1);
			HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
			i++;
		 
		}
	  
		PWM_Run(&PWM1);
	  
		if (i == 20)
		{
			i = 0;
			PWM_Update(&PWM1, 1000, 0.05, Active);
		}
		else if (i == 10)
		{
			PWM_Update(&PWM1, 1000, 0.5, Inactive);
		}
	    
		if (GPIO_Edge_Detected(&USER_BUTTON_Rising_Edge)){
 
			HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);
		}
	
	} // fim da baleia
	/* USER CODE END 3 */
	
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void ST_Init(type_ST *pST, uint32_t time_lapse)
{
	pST->initial_time = HAL_GetTick();
	pST->elapsed_time = 0;
	pST->delay_time = time_lapse;
}
type_bool ST(type_ST *pST)
{
	pST->elapsed_time = HAL_GetTick() - pST->initial_time;
	if (pST->elapsed_time >= pST->delay_time)
	{
		return True;
	}
	else return False;
}
void ST_Lapse(type_ST *pST)
{
	pST->initial_time = pST->initial_time + pST->delay_time;
	pST->elapsed_time = 0;
}

void PWM_Init(type_PWM *pPWM,
	GPIO_TypeDef* GPIO_Port,
	uint16_t GPIO_Pin, 
	uint32_t Period,
	float Duty)
{
	pPWM->T = Period;
	if (Duty <= 0.0f)
		pPWM->duty_cycle = 0;
	else if (Duty >= 1.0f)
		pPWM->duty_cycle = 1.0;
	else
		pPWM->duty_cycle = Duty;
	pPWM->t_act = pPWM->T * pPWM->duty_cycle;
	pPWM->t_ina = pPWM->T - pPWM->t_act;
	
	pPWM->Port = GPIO_Port;
	pPWM->Pin = GPIO_Pin;
	
	pPWM->T_shadow = pPWM->T;
	pPWM->duty_cicle_shadow = pPWM->duty_cycle;
	pPWM->shadow = Inactive;
	
	pPWM->state = Active;
	ST_Init(&pPWM->timer, pPWM->t_act);
	HAL_GPIO_WritePin(pPWM->Port, pPWM->Pin, GPIO_PIN_SET);
}

void PWM_Run(type_PWM *pPWM)
{
	if (ST(&pPWM->timer))
	{
		ST_Lapse(&pPWM->timer);
		if (pPWM->state == Active)
		{
			pPWM->state = Inactive;
			HAL_GPIO_WritePin(pPWM->Port, pPWM->Pin, GPIO_PIN_RESET);
			pPWM->timer.delay_time = pPWM->t_ina;
		}
		else
		{
			pPWM->state = Active;
			if (pPWM->shadow)
			{
				pPWM->duty_cycle = pPWM->duty_cicle_shadow;
				pPWM->T = pPWM->T_shadow;
				pPWM->t_act = pPWM->T * pPWM->duty_cycle;
				pPWM->t_ina = pPWM->T - pPWM->t_act;
			}
			HAL_GPIO_WritePin(pPWM->Port, pPWM->Pin, GPIO_PIN_SET);
			pPWM->timer.delay_time = pPWM->t_act;

		}
	}
}

void PWM_Update(type_PWM *pPWM, uint32_t Period, 
                float Duty, type_bool_state shadow)
{
#if (0)
	pPWM->T = Period;
	if (Duty <= 0.0f)
		pPWM->duty_cycle = 0;
	else if (Duty >= 1.0f)
		pPWM->duty_cycle = 1.0;
	else
		pPWM->duty_cycle = Duty;
	pPWM->t_act = pPWM->T * pPWM->duty_cycle;
	pPWM->t_ina = pPWM->T - pPWM->t_act;
	
	pPWM->T_shadow = pPWM->T;
	pPWM->duty_cicle_shadow = pPWM->duty_cycle;
#endif
	
	pPWM->shadow = shadow;
	pPWM->T_shadow = Period;
	if (Duty <= 0.0f)
		pPWM->duty_cicle_shadow = 0;
	else if (Duty >= 1.0f)
		pPWM->duty_cicle_shadow = 1.0;
	else
		pPWM->duty_cicle_shadow = Duty;	
	if (!shadow)
	{
		pPWM->duty_cycle = pPWM->duty_cicle_shadow;
		pPWM->T = pPWM->T_shadow;
		pPWM->t_act = pPWM->T * pPWM->duty_cycle;
		pPWM->t_ina = pPWM->T - pPWM->t_act;
	}
}


void GPIO_Edge_Init(gpio_edge_t *gpio, GPIO_TypeDef *port, uint16_t pin, uint32_t debounce) {
	gpio->port = port;
	gpio->pin = pin;
	gpio->Current_state = HAL_GPIO_ReadPin(port, pin);
	gpio->Previous_state = gpio->Current_state;
	gpio->debounce_time = debounce;
	gpio->State = Detecting;
}

type_bool GPIO_Edge_Detected(gpio_edge_t *gpio) {
	
	gpio->Current_state = HAL_GPIO_ReadPin(gpio->port, gpio->pin);	

	switch (gpio->State) {	
	  
		case Detecting:
		  
			if (gpio->Current_state != gpio->Previous_state) {
				gpio->State = Possible_Transition;
				ST_Init(&gpio->debounce_timer, gpio->debounce_time);		  
			}	  	  
			gpio->Previous_state = gpio->Current_state;
		
			break;
	
		case Possible_Transition:
		  
			if (ST(&gpio->debounce_timer)) {
			  
				if (gpio->Current_state == GPIO_PIN_SET) {
					gpio->State = Detected;			  
				}
				else {
				gpio->State = Detecting;
				}		  
			}	
			
			break;
		
		case Detected:
		  
			gpio->State = Detecting;
			
			break;
		
		default:
            gpio->State = Detecting;
			
            break;
	}
	
	if (gpio->State == Detected){
		
		return True;
	}
	else{
		
		return False;
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
