#include <delay.h>
#include <gpio.h>
#include <stm32.h>

// ============================ LEDY ===================================
#define RED_LED_GPIO GPIOA
#define GREEN_LED_GPIO GPIOA
#define BLUE_LED_GPIO GPIOB
#define GREEN2_LED_GPIO GPIOA
#define RED_LED_PIN 6
#define GREEN_LED_PIN 7
#define BLUE_LED_PIN 0
#define GREEN2_LED_PIN 5

#define RedLEDon()  RED_LED_GPIO->BSRRH = 1 << RED_LED_PIN
#define RedLEDoff() RED_LED_GPIO->BSRRL = 1 << RED_LED_PIN

#define GreenLEDon() GREEN_LED_GPIO->BSRRH = 1 << GREEN_LED_PIN
#define GreenLEDoff() GREEN_LED_GPIO->BSRRL = 1 << GREEN_LED_PIN

#define BlueLEDon() BLUE_LED_GPIO->BSRRH = 1 << BLUE_LED_PIN
#define BlueLEDoff() BLUE_LED_GPIO->BSRRL = 1 << BLUE_LED_PIN

#define Green2LEDon() GREEN2_LED_GPIO->BSRRL = 1 << GREEN2_LED_PIN
#define Green2LEDoff() GREEN2_LED_GPIO->BSRRH = 1 << GREEN2_LED_PIN

// =========================== UART ====================================
// Tryb pracy
#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable USART_CR1_UE
// Przesyłane słowo to dane łącznie z ewentualnym bitem parzystości
#define USART_WordLength_8b 0x0000
#define USART_WordLength_9b USART_CR1_M
// Bit parzystości
#define USART_Parity_No 0x0000
#define USART_Parity_Even USART_CR1_PCE
#define USART_Parity_Odd (USART_CR1_PCE | USART_CR1_PS)
// Bit(y) stopu
#define USART_StopBits_1 0x0000
#define USART_StopBits_0_5 0x1000
#define USART_StopBits_2 0x2000
#define USART_StopBits_1_5 0x3000

void confUSART() {
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	
	// Konfigurujemy linie TXD
	GPIOafConfigure(GPIOA,
		2,
		GPIO_OType_PP,
		GPIO_Fast_Speed,
		GPIO_PuPd_NOPULL,
		GPIO_AF_USART2
	);
	
	// Konfigurujemy linię RXD
	GPIOafConfigure(GPIOA,
		3,
		GPIO_OType_PP,
		GPIO_Fast_Speed,
		GPIO_PuPd_UP,
		GPIO_AF_USART2
	);
	
	USART2->CR1 = USART_Mode_Rx_Tx |
		USART_WordLength_8b |
		USART_Parity_No |
		USART Enable;
	
	
}

int main() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
	//RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	// todo remove ?
	
	

			
	__NOP();
	RedLEDoff();
	GreenLEDoff();
	BlueLEDoff();
	Green2LEDoff();
	
	// todo wypisz
	

	/*GPIOoutConfigure(RED_LED_GPIO,
		RED_LED_PIN,
		GPIO_OType_PP,
		GPIO_Low_Speed,
		GPIO_PuPd_NOPULL
	);
	
	GPIOoutConfigure(GREEN_LED_GPIO,
		GREEN_LED_PIN,
		GPIO_OType_PP,
		GPIO_Low_Speed,
		GPIO_PuPd_NOPULL
	);

	GPIOoutConfigure(BLUE_LED_GPIO,
		BLUE_LED_PIN,
		GPIO_OType_PP,
		GPIO_Low_Speed,
		GPIO_PuPd_NOPULL
	);
	
	GPIOoutConfigure(GREEN2_LED_GPIO,
		GREEN2_LED_PIN,
		GPIO_OType_PP,
		GPIO_Low_Speed,
		GPIO_PuPd_NOPULL
	);
	
	for (;;) {
		RedLEDon();
		Delay(4000000);
		RedLEDoff();
		GreenLEDon();
		Delay(4000000);
		GreenLEDoff();
		BlueLEDon();
		Delay(4000000);
		BlueLEDoff();
		Green2LEDon();
		Delay(4000000);
		Green2LEDoff();
	}*/
	
	return 0;
}


