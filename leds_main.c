#include <delay.h>
#include <gpio.h>
#include <stm32.h>
// TODO remove! added only to support functions and macros recognition in IDE
#include <stm32f411xe.h>


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

void confLED() {
	// Włącz taktowanie
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

	__NOP();
	RedLEDoff();
	GreenLEDoff();
	BlueLEDoff();
	Green2LEDoff();

	GPIOoutConfigure(RED_LED_GPIO,
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
}


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
// Sterowanie przepływem
#define USART_FlowControl_None 0x0000
#define USART_FlowControl_RTS USART_CR3_RTSE
#define USART_FlowControl_CTS USART_CR3_CTSE
// Po włączeniu mikrokontroler STM32F411 jest taktowany
// wewnętrznym generatorem RC HSI (ang. High Speed
// Internal) o częstotliwości 16 MHz
#define HSI_HZ 16000000U
// Układ UART2 jest taktowany zegarem PCLK1, który po
// włączeniu mikrokontrolera jest zegarem HSI
#define PCLK1_HZ HSI_HZ


void confUSART() {
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	//RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	// Wlacz taktowanie
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

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
		USART_Parity_No;

	USART2->CR2 = USART_StopBits_1;

	USART2->CR3 = USART_FlowControl_None;

	// I Przykładowa konfiguracja
	uint32_t const baudrate = 9600U;
	USART2->BRR = (PCLK1_HZ + (baudrate / 2U)) / baudrate;

	USART2->CR1 |= USART_Enable;
}


// ============================= BUTTONS ============================

// czytanka: https://www.mimuw.edu.pl/~marpe/mikrokontrolery/w2_gpio.pdf

// stan aktywny niski – logiczne 0
#define USER_BUTTON_GPIO GPIOC
#define USER_BUTTON_PIN 13
// stan aktywny niski – logiczne 0
#define JOYSTICK_GPIO GPIOB
#define JOYSTICK_UP_PIN 5
#define JOYSTICK_DOWN_PIN 6
#define JOYSTICK_LEFT_PIN 3
#define JOYSTICK_RIGHT_PIN 4
#define JOYSTICK_FIRE_PIN 10
// stan aktywny wysoki – logiczna 1
#define MODE_BUTTON_GPIO GPIOA
#define MODE_BUTTON_PIN 0

#define UserButtonPressed() (!(USER_BUTTON_GPIO->IDR & (1 << USER_BUTTON_PIN)))
#define JoystickPressed(pin) (!(JOYSTICK_GPIO->IDR & (1 << pin)))
#define LeftPressed()  JoystickPressed(JOYSTICK_LEFT_PIN)
#define RightPressed() JoystickPressed(JOYSTgetcBockingICK_RIGHT_PIN)
#define UpPressed()    JoystickPressed(JOYSTICK_UP_PIN)
#define DownPressed()  JoystickPressed(JOYSTICK_DOWN_PIN)
#define FirePressed()  JoystickPressed(JOYSTICK_FIRE_PIN)
#define ModeButtonPressed() (MODE_BUTTON_GPIO->IDR & (1 << MODE_BUTTON_PIN))


void confInput(GPIO_TypeDef * const gpio, uint32_t pin) {
    GPIOainConfigure(gpio, pin);
//                    ?,   // GPIOPuPd_TypeDef pull,
//                    ?,   // EXTIMode_TypeDef mode,
//                    ?    // EXTITrigger_TypeDef trigger);
}

void confButtons() {
    // Włącz taktowanie
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN;

    confInput(USER_BUTTON_GPIO, USER_BUTTON_PIN);

    confInput(JOYSTICK_GPIO,    JOYSTICK_UP_PIN);
    confInput(JOYSTICK_GPIO,    JOYSTICK_DOWN_PIN);
    confInput(JOYSTICK_GPIO,    JOYSTICK_LEFT_PIN);
    confInput(JOYSTICK_GPIO,    JOYSTICK_RIGHT_PIN);
    confInput(JOYSTICK_GPIO,    JOYSTICK_FIRE_PIN);

    confInput(MODE_BUTTON_GPIO, MODE_BUTTON_PIN);
}

// todo spytac czy to mamy obslugiwac, czy moze ma w kolko wypisywac ze wcisniety tak dlugo jak wcisniety
// i'th bit says what was previous pressed/released state of a button with pin i,
// we can rely on it as long as pins  for buttons are unique
int BUTTON_STATE = 0;
#define prevButtonState(pin) (BUTTON_STATE & (1<<pin))
#define hasButtonStateChanged(pin, is_active) ((prevButtonState(pin) == 0) != (is_active == 0))


// ============================= UTILS ===============================

#define CAN_WRITE (USART2->SR & USART_SR_TXE)
#define HAS_NEXT_CHAR (USART2->SR & USART_SR_RXNE)


char getcActiveWait() {
	for (;!HAS_NEXT_CHAR;) {
		__NOP();
	}
	// todo
	return 'a';
}

void putcActiveWait(char c) {
	for (;!CAN_WRITE;) {
		__NOP();
	}
	USART2->DR = c;
}



int main() {
	confUSART();
	confLED();
    confButtons();


	for (;;) {
		RedLEDon();
		Delay(4000000);

        putcActiveWait('a');

		RedLEDoff();

		GreenLEDon();
		Delay(4000000);
		GreenLEDoff();
		BlueLEDon();
		Delay(4000000);
		BlueLEDoff();
		//Green2LEDon();
		//Delay(4000000);
		//Green2LEDoff();
	}

	return 0;
}


