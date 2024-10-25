/* Includes ------------------------------------------------------------------*/
#include "main.h"  // Librería principal para funciones HAL

/* Private defines -----------------------------------------------------------*/
#define SW_1 GPIO_PIN_5     // Pin del botón SW_1 (PB5)
#define SW_2 GPIO_PIN_6     // Pin del botón SW_2 (PB6)
#define LED_1 GPIO_PIN_13   // Pin de la luz roja (PC13)
#define LED_2 GPIO_PIN_12   // Pin de la luz verde (PB12)

/* Private variables ---------------------------------------------------------*/
uint32_t last_blink_time = 0;   // Guarda el tiempo de la última acción de parpadeo
uint32_t last_state_change = 0; // Guarda el tiempo de la última vez que cambió de estado
uint8_t blinking = 0;           // Cuenta cuántas veces ha parpadeado
uint8_t state = 0;              // Estado del semáforo: 0 = verde encendido, 1 = verde parpadeando, 2 = rojo encendido, 3 = rojo parpadeando
uint8_t request_crossing = 0;   // Variable que indica si se ha solicitado el cruce

/* Function Prototypes -------------------------------------------------------*/
void SystemClock_Config(void);  // Configura el reloj del sistema
static void MX_GPIO_Init(void); // Inicializa los pines de entrada/salida (GPIO)

/* Main program --------------------------------------------------------------*/
int main(void)
{
    /* Inicializa los periféricos y el reloj del sistema */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* Estado inicial: luz verde encendida, luz roja apagada */
    HAL_GPIO_WritePin(GPIOB, LED_2, GPIO_PIN_RESET);   // Enciende la luz verde (LED_2)
    HAL_GPIO_WritePin(GPIOC, LED_1, GPIO_PIN_SET);     // Apaga la luz roja (LED_1)

    while (1)  // Bucle infinito para el funcionamiento del semáforo
    {
        uint32_t current_time = HAL_GetTick();  // Obtiene el tiempo actual en milisegundos

        // Revisa si alguno de los botones SW_1 o SW_2 fue presionado y la luz verde está encendida
        if ((HAL_GPIO_ReadPin(GPIOB, SW_1) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOB, SW_2) == GPIO_PIN_SET) && state == 0)
        {
            // Solo aceptamos la solicitud si la luz verde está encendida (estado 0)
            request_crossing = 1;  // Se marca la solicitud de cruce
        }

        // Control del semáforo basado en el estado actual
        switch (state)
        {
            case 0: // Estado 0: luz verde encendida
                if (request_crossing)
                {
                    request_crossing = 0;   // Resetea la solicitud de cruce
                    state = 1;              // Cambia al estado de parpadeo verde
                    last_blink_time = current_time; // Guarda el tiempo actual
                    blinking = 0;           // Reinicia el contador de parpadeos
                }
                break;

            case 1: // Estado 1: luz verde parpadeando
                if (current_time - last_blink_time >= 200)
                {
                    // Cambia el estado del LED verde cada 200 ms (parpadeo)
                    HAL_GPIO_TogglePin(GPIOB, LED_2);
                    last_blink_time = current_time; // Actualiza el tiempo del último parpadeo

                    // Después de 5 parpadeos (1 segundo), cambia al estado rojo
                    if (++blinking >= 5)
                    {
                        blinking = 0; // Reinicia el contador de parpadeos
                        state = 2;    // Cambia al estado rojo encendido
                        HAL_GPIO_WritePin(GPIOB, LED_2, GPIO_PIN_SET);   // Apaga la luz verde
                        HAL_GPIO_WritePin(GPIOC, LED_1, GPIO_PIN_RESET); // Enciende la luz roja
                        last_state_change = current_time;  // Guarda el tiempo del cambio de estado
                    }
                }
                break;

            case 2: // Estado 2: luz roja encendida
                if (current_time - last_state_change >= 3000)
                {
                    // Después de 3 segundos en rojo, cambia a parpadeo rojo
                    state = 3;  // Cambia al estado de parpadeo rojo
                    last_blink_time = current_time; // Guarda el tiempo actual para el parpadeo
                    blinking = 0;  // Reinicia el contador de parpadeos
                }
                break;

            case 3: // Estado 3: luz roja parpadeando
                if (current_time - last_blink_time >= 200)
                {
                    // Cambia el estado del LED rojo cada 200 ms (parpadeo)
                    HAL_GPIO_TogglePin(GPIOC, LED_1);
                    last_blink_time = current_time; // Actualiza el tiempo del último parpadeo

                    // Después de 5 parpadeos (1 segundo), vuelve al estado verde
                    if (++blinking >= 5)
                    {
                        blinking = 0; // Reinicia el contador de parpadeos
                        state = 0;   // Cambia al estado verde encendido
                        HAL_GPIO_WritePin(GPIOC, LED_1, GPIO_PIN_SET);   // Apaga la luz roja
                        HAL_GPIO_WritePin(GPIOB, LED_2, GPIO_PIN_RESET); // Enciende la luz verde
                    }
                }
                break;
        }
    }
}

/* Function Definitions ------------------------------------------------------*/
/**
  * @brief Configuración del reloj del sistema
  * @retval None
  */
void SystemClock_Config(void)
{
    // Configuración del reloj del sistema (ajustar según el microcontrolador)
}

/**
  * @brief Inicialización de los pines de entrada/salida (GPIO)
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Habilita los puertos GPIOB y GPIOC */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configura los pines SW_1 y SW_2 como entrada (botones) */
    GPIO_InitStruct.Pin = SW_1 | SW_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // Configura con resistencia de pull-down
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Configura el pin de la luz roja (LED_1) como salida */
    GPIO_InitStruct.Pin = LED_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Salida push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;         // Sin resistencia pull-up/pull-down
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Baja frecuencia para ahorro de energía
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Configura el pin de la luz verde (LED_2) como salida */
    GPIO_InitStruct.Pin = LED_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Salida push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;         // Sin resistencia pull-up/pull-down
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Baja frecuencia para ahorro de energía
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

