/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define P1_UP_Pin GPIO_PIN_2
#define P1_UP_GPIO_Port GPIOE
#define P1_DOWN_Pin GPIO_PIN_3
#define P1_DOWN_GPIO_Port GPIOE
#define P1_LEFT_Pin GPIO_PIN_4
#define P1_LEFT_GPIO_Port GPIOE
#define P1_RIGHT_Pin GPIO_PIN_5
#define P1_RIGHT_GPIO_Port GPIOE
#define P1_A_Pin GPIO_PIN_6
#define P1_A_GPIO_Port GPIOE
#define FPGA_CS_Pin GPIO_PIN_0
#define FPGA_CS_GPIO_Port GPIOB
#define P1_B_Pin GPIO_PIN_7
#define P1_B_GPIO_Port GPIOE
#define P2_UP_Pin GPIO_PIN_8
#define P2_UP_GPIO_Port GPIOE
#define P2_DOWN_Pin GPIO_PIN_9
#define P2_DOWN_GPIO_Port GPIOE
#define P2_LEFT_Pin GPIO_PIN_10
#define P2_LEFT_GPIO_Port GPIOE
#define P2_RIGHT_Pin GPIO_PIN_11
#define P2_RIGHT_GPIO_Port GPIOE
#define P2_A_Pin GPIO_PIN_12
#define P2_A_GPIO_Port GPIOE
#define P2_B_Pin GPIO_PIN_13
#define P2_B_GPIO_Port GPIOE
#define AUDIO_EN_Pin GPIO_PIN_14
#define AUDIO_EN_GPIO_Port GPIOE
#define SD_DETECT_Pin GPIO_PIN_15
#define SD_DETECT_GPIO_Port GPIOE
#define WB_NRST_Pin GPIO_PIN_8
#define WB_NRST_GPIO_Port GPIOD
#define WB_HOST_IRQ_Pin GPIO_PIN_9
#define WB_HOST_IRQ_GPIO_Port GPIOD
#define WB_HOST_IRQ_EXTI_IRQn EXTI9_IRQn
#define FPGA_READY_Pin GPIO_PIN_3
#define FPGA_READY_GPIO_Port GPIOD
#define FPGA_IRQ_Pin GPIO_PIN_4
#define FPGA_IRQ_GPIO_Port GPIOD
#define FPGA_IRQ_EXTI_IRQn EXTI4_IRQn
#define FPGA_FRAME_Pin GPIO_PIN_5
#define FPGA_FRAME_GPIO_Port GPIOD
#define FPGA_FRAME_EXTI_IRQn EXTI5_IRQn
#define FPGA_AUX0_Pin GPIO_PIN_6
#define FPGA_AUX0_GPIO_Port GPIOD
#define FPGA_AUX1_Pin GPIO_PIN_7
#define FPGA_AUX1_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
