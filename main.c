/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "main.h" // Bu dosya CubeMX tarafından oluşturulur ve pin tanımlarını içerir.

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h> // strcpy, strcat vb. için
#include <stdio.h>  // sprintf için
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// --- Buffer Boyutları ve Zaman Aşımı ---
#define RS232_RX_BUFFER_SIZE 256
#define ESP_TX_BUFFER_SIZE   256
#define WIFI_ACTIVITY_TIMEOUT 10000 // Wi-Fi'yi kapatmak için ms cinsinden zaman aşımı (10 saniye)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1; // CubeMX tarafından tanımlanır
UART_HandleTypeDef huart2; // CubeMX tarafından tanımlanır

/* USER CODE BEGIN PV */
// --- UART Bufferları ve Bayraklar ---
uint8_t rs232_rx_buffer[RS232_RX_BUFFER_SIZE];
volatile uint16_t rs232_rx_index = 0;
volatile uint8_t rs232_data_ready_flag = 0;

char esp_tx_buffer[ESP_TX_BUFFER_SIZE];

// --- Durum Değişkenleri ---
volatile uint8_t wifi_module_enabled = 0;
volatile uint32_t last_rs232_activity_time = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void process_rs232_data(uint8_t* data, uint16_t len);
void send_to_esp(const char* message);
void enable_wifi_module(void);
void disable_wifi_module(void);
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  // Başlangıçta Wi-Fi modülü kapalı
  disable_wifi_module();
  // WIFI_OUT_LED_PIN CubeMX'te User Label olarak tanımlandıysa:
  // main.h içinde #define WIFI_OUT_LED_PIN_Pin GPIO_PIN_1 gibi tanımlar olmalı
  // ve #define WIFI_OUT_LED_PIN_GPIO_Port GPIOA gibi
  HAL_GPIO_WritePin(WIFI_OUT_LED_PIN_GPIO_Port, WIFI_OUT_LED_PIN_Pin, GPIO_PIN_RESET); // D6 LED'i kapalı

  // RS232'den kesme ile veri almaya başla
  if(HAL_UART_Receive_IT(&huart1, &rs232_rx_buffer[rs232_rx_index], 1) != HAL_OK)
  {
    Error_Handler(); // Hata durumunda
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (rs232_data_ready_flag) {
        if (!wifi_module_enabled) {
            enable_wifi_module();
            send_to_esp("COM_SERVICE_STARTED\n");
        }

        process_rs232_data(rs232_rx_buffer, rs232_rx_index);

        rs232_data_ready_flag = 0; // Bayrağı sıfırla
        rs232_rx_index = 0;        // Buffer index'ini sıfırla
        memset(rs232_rx_buffer, 0, RS232_RX_BUFFER_SIZE); // Buffer'ı temizle (isteğe bağlı ama iyi pratik)

        // Tekrar RS232'den kesme ile veri almaya başla
        if(HAL_UART_Receive_IT(&huart1, &rs232_rx_buffer[rs232_rx_index], 1) != HAL_OK)
        {
          Error_Handler();
        }
    }

    // Wi-Fi aktivite zaman aşımı kontrolü
    // Sadece buffer boşsa ve timeout olduysa Wi-Fi'yi kapat.
    // Eğer buffer'da veri varsa ve timeout olduysa, bu verinin son parça olduğunu varsayıp
    // rs232_data_ready_flag'i set edip işlenmesini sağlayabiliriz.
    // Şimdilik basit tutalım: Sadece buffer boşken ve timeout olduğunda kapat.
    if (wifi_module_enabled && (HAL_GetTick() - last_rs232_activity_time > WIFI_ACTIVITY_TIMEOUT)) {
        if(rs232_rx_index == 0) { // Eğer buffer'da bekleyen veri yoksa
            send_to_esp("COM_SERVICE_ENDED_TIMEOUT\n");
            disable_wifi_module();
        }
    }
    HAL_Delay(10); // CPU'yu çok yormamak için küçük bir gecikme
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2; // (16MHz / 2) = 8MHz
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9; // 8MHz * 9 = 72MHz
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2; // Max 36MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1; // Max 72MHz

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) // 72MHz için Latency 2
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  /* USER CODE BEGIN USART1_Init 0 */
  /* USER CODE END USART1_Init 0 */
  /* USER CODE BEGIN USART1_Init 1 */
  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600; // VEYA KULLANILACAK BAUD RATE
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  /* USER CODE END USART1_Init 2 */
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  /* USER CODE BEGIN USART2_Init 0 */
  /* USER CODE END USART2_Init 0 */
  /* USER CODE BEGIN USART2_Init 1 */
  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200; // VEYA ESP32 İLE KULLANILACAK BAUD RATE
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
  /* USER CODE END USART2_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE(); // Genellikle PC13 için (kullanıcı LED/buton)
  __HAL_RCC_GPIOD_CLK_ENABLE(); // HSE için (PD0/PD1 olabilir, STM32F103C8'de değil ama genel bir açma)
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(WIFI_OUT_LED_PIN_GPIO_Port, WIFI_OUT_LED_PIN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(WIFI_EN_PIN_GPIO_Port, WIFI_EN_PIN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : WIFI_OUT_LED_PIN_Pin (User Label: WIFI_OUT_LED_PIN) */
  GPIO_InitStruct.Pin = WIFI_OUT_LED_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WIFI_OUT_LED_PIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WIFI_EN_PIN_Pin (User Label: WIFI_EN_PIN) */
  GPIO_InitStruct.Pin = WIFI_EN_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WIFI_EN_PIN_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void enable_wifi_module(void) {
    HAL_GPIO_WritePin(WIFI_EN_PIN_GPIO_Port, WIFI_EN_PIN_Pin, GPIO_PIN_SET);
    wifi_module_enabled = 1;
    HAL_Delay(200); // Modülün açılması için bekleme süresi (gerekirse ayarlayın)
}

void disable_wifi_module(void) {
    HAL_GPIO_WritePin(WIFI_EN_PIN_GPIO_Port, WIFI_EN_PIN_Pin, GPIO_PIN_RESET);
    wifi_module_enabled = 0;
    HAL_GPIO_WritePin(WIFI_OUT_LED_PIN_GPIO_Port, WIFI_OUT_LED_PIN_Pin, GPIO_PIN_RESET);
}

// Bu fonksiyon, RS232'den gelen veriyi işler.
// "verim hesabı, pid'den gelen bilgiler" gibi matematiksel işlemler burada yapılır.
void process_rs232_data(uint8_t* data, uint16_t len) {
    if (len > 0) {
        char local_temp_buffer[RS232_RX_BUFFER_SIZE + 1]; // Yerel buffer, global buffer'ı etkilemez
        memcpy(local_temp_buffer, data, len);
        local_temp_buffer[len] = '\0'; // String sonlandırıcısı

        // Veriyi işle (Örnek: basitçe logla veya belirli bir formatta ESP'ye gönder)
        // Matematiksel işlemlerinizi burada yapın.
        // Sonucu esp_tx_buffer'a yazın.
        // Örneğin:
        // float verim = calculate_verim(local_temp_buffer);
        // snprintf(esp_tx_buffer, ESP_TX_BUFFER_SIZE, "VERIM:%.2f;RAW:%s", verim, local_temp_buffer);

        // Şimdilik sadece gelen veriyi bir ön ekle gönderelim:
        snprintf(esp_tx_buffer, ESP_TX_BUFFER_SIZE, "DATA_FROM_RS232:%s", local_temp_buffer);
        send_to_esp(esp_tx_buffer);

        // Özel bitiş komutu kontrolü (Örnek: "END_PROCESS")
        if (strstr(local_temp_buffer, "END_PROCESS") != NULL) {
            send_to_esp("COM_SERVICE_ENDED_CMD\n");
            disable_wifi_module();
        }
    }
}

// ESP32 modülüne (UART2 üzerinden) mesaj gönderir.
void send_to_esp(const char* message) {
    if (wifi_module_enabled) {
        HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        HAL_GPIO_WritePin(WIFI_OUT_LED_PIN_GPIO_Port, WIFI_OUT_LED_PIN_Pin, GPIO_PIN_SET);
        HAL_Delay(20); // LED'in kısa süre yanması için
        HAL_GPIO_WritePin(WIFI_OUT_LED_PIN_GPIO_Port, WIFI_OUT_LED_PIN_Pin, GPIO_PIN_RESET);
    }
}

// UART Receive Complete Callback
// Bu fonksiyon, HAL_UART_Receive_IT ile her byte alındığında çağrılır.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) { // USART1'den (RS232) veri geldi
        last_rs232_activity_time = HAL_GetTick(); // Son aktivite zamanını güncelle

        // Gelen byte rs232_rx_buffer[rs232_rx_index]'e zaten konuldu.
        // Index'i bir sonraki pozisyona ilerlet.
        if (rs232_rx_index < RS232_RX_BUFFER_SIZE - 1) {
            rs232_rx_index++;
        } else {
            // Buffer doldu, veriyi işlemeye zorla (idealde bu durum olmamalı, mesajlar daha kısa olmalı)
            rs232_data_ready_flag = 1;
            return; // Bir sonraki Receive_IT ana döngüde çağrılacak
        }

        // Satır sonu karakteri ('\n') geldi mi diye kontrol et.
        // Veya belirli bir paket sonu karakteri/dizisi.
        if (rs232_rx_buffer[rs232_rx_index - 1] == '\n') {
            rs232_data_ready_flag = 1; // Ana döngüde işlenmek üzere bayrağı set et
            // Bir sonraki HAL_UART_Receive_IT ana döngüde rs232_data_ready_flag işlendikten sonra çağrılacak.
        } else {
            // Henüz mesaj tamamlanmadı, bir sonraki byte'ı bekle
            if(HAL_UART_Receive_IT(&huart1, &rs232_rx_buffer[rs232_rx_index], 1) != HAL_OK)
            {
              Error_Handler();
            }
        }
    }
    // ESP32'den (USART2) veri almak için de benzer bir yapı kurulabilir.
    // else if (huart->Instance == USART2) { /* ESP'den gelen veriyi işle */ }
}

// UART Hata Callback'i
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        uint32_t error_code = HAL_UART_GetError(huart);
        // Olası hatalar: HAL_UART_ERROR_PE (Parity), HAL_UART_ERROR_NE (Noise),
        // HAL_UART_ERROR_FE (Frame), HAL_UART_ERROR_ORE (Overrun), HAL_UART_ERROR_DMA (DMA)

        if(error_code & HAL_UART_ERROR_ORE) { // Overrun error
          __HAL_UART_CLEAR_OREFLAG(huart); // Overrun bayrağını temizle
        }
        // Diğer hatalar için de loglama veya özel işlemler yapılabilir.

        // Hata sonrası alıcıyı yeniden başlat
        rs232_rx_index = 0;
        memset(rs232_rx_buffer, 0, RS232_RX_BUFFER_SIZE);
        if(HAL_UART_Receive_IT(&huart1, &rs232_rx_buffer[rs232_rx_index], 1) != HAL_OK)
        {
          Error_Handler();
        }
    }
    // USART2 için de benzer bir hata yönetimi eklenebilir.
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

#ifdef  USE_FULL_ASSERT
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