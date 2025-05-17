/* main.h */
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
// Bu, STM32F1 serisi için ana HAL başlık dosyasıdır.
// Mikrodenetleyici serinize göre değişir (örn: stm32f4xx_hal.h, stm32l0xx_hal.h vb.)
// Bizim durumumuzda STM32F103C8T6 için:
#include "stm32f1xx_hal.h"

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
void Error_Handler(void); // Bu fonksiyon genellikle main.c'de tanımlanır

/* USER CODE BEGIN EFP */
// main.c'de tanımladığımız ve global olarak erişilmesi gerekebilecek
// diğer fonksiyonların prototiplerini buraya ekleyebilirsiniz.
// Örneğin, HAL_UART_RxCpltCallback gibi callback'ler HAL tarafından çağrıldığı için
// prototiplerinin HAL başlıklarında olması gerekir.
// UART handle'larımız main.c içinde global tanımlı olduğu için
// ve stm32f1xx_it.c gibi yerlerden erişilmesi gerekebileceği için
// burada extern olarak bildirebiliriz (CubeMX genellikle bunu main.c'de yapar).
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

// Eğer CubeMX ile User Label verdiyseniz ve bunları kullanmak istiyorsanız,
// o pin tanımlarını buraya eklemeniz gerekir.
// Örnek (PINLERİ KENDİ TANIMLARINIZA GÖRE GÜNCELLEYİN!):
#define WIFI_EN_PIN_Pin GPIO_PIN_0
#define WIFI_EN_PIN_GPIO_Port GPIOB
#define WIFI_OUT_LED_PIN_Pin GPIO_PIN_1
#define WIFI_OUT_LED_PIN_GPIO_Port GPIOA
// Eğer User Label kullanmıyorsanız, bu pin tanımlarına ihtiyacınız yok,
// kodda doğrudan HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, ...); şeklinde kullanırsınız.

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Projeye özel #define'ları buraya veya doğrudan main.c'ye koyabilirsiniz.
// CubeMX, User Label pinlerini genellikle burada tanımlar.
/* USER CODE END PD */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */