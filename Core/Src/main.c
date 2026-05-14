#include "main.h"

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

#define FLASH_ADDR 0x0801FC00

uint16_t blink_count = 4;
uint16_t blink_step = 0;
uint8_t wait_mode = 0;

/* button control */
uint32_t button_press_start = 0;

/* USER CODE END PV */

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

/* ================= FLASH ================= */

uint16_t Flash_Read(uint32_t address)
{
    return *(volatile uint16_t*)address;
}

void Flash_Write(uint32_t address, uint16_t data)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseStruct;
    uint32_t pageError;

    eraseStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseStruct.PageAddress = address;
    eraseStruct.NbPages = 1;

    HAL_FLASHEx_Erase(&eraseStruct, &pageError);

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);

    HAL_FLASH_Lock();
}

/* ================= TIMER ================= */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        if(wait_mode == 0)
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            blink_step++;

            if(blink_step >= (blink_count * 2))
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                wait_mode = 1;
                blink_step = 0;
            }
        }
        else
        {
            blink_step++;

            if(blink_step >= 5)
            {
                wait_mode = 0;
                blink_step = 0;
            }
        }
    }
}

/* ================= MAIN ================= */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    /* Flash read */
    blink_count = Flash_Read(FLASH_ADDR);

    if(blink_count < 4 || blink_count > 7)
    {
        blink_count = 4;
        Flash_Write(FLASH_ADDR, blink_count);
    }

    /* ================= POWER ON RESET ================= */
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
    {
        HAL_Delay(3000);

        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
        {
            blink_count = 4;
            Flash_Write(FLASH_ADDR, blink_count);
            HAL_Delay(20);
            NVIC_SystemReset();
        }
    }

    HAL_TIM_Base_Start_IT(&htim2);

    /* ================= LOOP ================= */
    while(1)
    {
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
        {
            if(button_press_start == 0)
            {
                button_press_start = HAL_GetTick();
            }
        }
        else
        {
            if(button_press_start != 0)
            {
                uint32_t press_time = HAL_GetTick() - button_press_start;

                /* SHORT or LONG press -> same action (+1) */
                if(press_time > 50)   // debounce
                {
                    blink_count++;

                    if(blink_count > 7)
                        blink_count = 4;

                    Flash_Write(FLASH_ADDR, blink_count);
                }

                button_press_start = 0;
            }
        }
    }
}

/* ================= CLOCK ================= */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/* ================= TIM2 ================= */

static void MX_TIM2_Init(void)
{
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7199;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 9999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&htim2);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/* ================= GPIO ================= */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* LED PC13 */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* BUTTON PA0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA1 LOW */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}