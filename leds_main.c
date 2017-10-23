#include <delay.h>
#include <gpio.h>
#include <stm32.h>
// TODO remove! added only to support functions and macros recognition in IDE
#include <stm32f411xe.h>
#include <stdbool.h>


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
#define RedLEDtoggle() RED_LED_GPIO->ODR ^= 1 << RED_LED_PIN

#define GreenLEDon() GREEN_LED_GPIO->BSRRH = 1 << GREEN_LED_PIN
#define GreenLEDoff() GREEN_LED_GPIO->BSRRL = 1 << GREEN_LED_PIN
#define GreenLEDtoggle() GREEN_LED_GPIO->ODR ^= 1 << RED_LED_PIN

#define BlueLEDon() BLUE_LED_GPIO->BSRRH = 1 << BLUE_LED_PIN
#define BlueLEDoff() BLUE_LED_GPIO->BSRRL = 1 << BLUE_LED_PIN
#define BlueLEDtoggle() BLUE_LED_GPIO->ODR ^= 1 << RED_LED_PIN

#define Green2LEDon() GREEN2_LED_GPIO->BSRRL = 1 << GREEN2_LED_PIN
#define Green2LEDoff() GREEN2_LED_GPIO->BSRRH = 1 << GREEN2_LED_PIN
#define Green2LEDtoggle() GREEN2_LED_GPIO->ODR ^= 1 << RED_LED_PIN


void GPIOconfLED(GPIO_TypeDef * const gpio, uint32_t pin) {
    GPIOoutConfigure(gpio,
                     pin,
                     GPIO_OType_PP,
                     GPIO_Low_Speed,
                     GPIO_PuPd_NOPULL
    );
}

void confLED() {
	// Włącz taktowanie
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

	__NOP();
	RedLEDoff();
	GreenLEDoff();
	BlueLEDoff();
    Green2LEDoff();

    GPIOconfLED(RED_LED_GPIO,    RED_LED_PIN);
    GPIOconfLED(GREEN_LED_GPIO,  GREEN_LED_PIN);
    GPIOconfLED(BLUE_LED_GPIO,   BLUE_LED_PIN);
    GPIOconfLED(GREEN2_LED_GPIO, GREEN2_LED_PIN);
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

#define USER_BUTTON_GPIO GPIOC
#define USER_BUTTON_PIN 13
#define USER_BUTTON_ACTIVE 0

#define MODE_BUTTON_GPIO GPIOA
#define MODE_BUTTON_PIN 0
#define MODE_BUTTON_ACTIVE 1

#define JOYSTICK_GPIO GPIOB
#define JOYSTICK_UP_PIN 5
#define JOYSTICK_DOWN_PIN 6
#define JOYSTICK_LEFT_PIN 3
#define JOYSTICK_RIGHT_PIN 4
#define JOYSTICK_FIRE_PIN 10
#define JOYSTICK_ACTIVE 0

// i'th bit says what was previous pressed/released state of a button with pin i,
// we can rely on it as long as pins  for buttons are unique
int BUTTON_STATE = 0;

#define PrevButtonState(pin) ((BUTTON_STATE & (1<<pin)) != 0)
#define IsButtonPressed(gpio, pin, active) (((gpio->IDR & (1 << pin)) != 0) == active)
#define HasButtonChanged(gpio, pin, active) (IsButtonPressed(gpio, pin, active) != PrevButtonState(pin))
#define RevertButtonState(pin) BUTTON_STATE ^= (1 << pin)



//#define UserButtonPressed() (!(USER_BUTTON_GPIO->IDR & (1 << USER_BUTTON_PIN)))
//#define ModeButtonPressed() (MODE_BUTTON_GPIO->IDR & (1 << MODE_BUTTON_PIN))
//#define JoystickPressed(pin) (!(JOYSTICK_GPIO->IDR & (1 << pin)))
//#define LeftPressed()  JoystickPressed(JOYSTICK_LEFT_PIN)
//#define RightPressed() JoystickPressed(JOYSTICK_RIGHT_PIN)
//#define UpPressed()    JoystickPressed(JOYSTICK_UP_PIN)
//#define DownPressed()  JoystickPressed(JOYSTICK_DOWN_PIN)
//#define FirePressed()  JoystickPressed(JOYSTICK_FIRE_PIN)


// experimental :
//const uint32_t buttonPIN[7] = {USER_BUTTON_PIN, MODE_BUTTON_PIN, JOYSTICK_UP_PIN, JOYSTICK_DOWN_PIN, JOYSTICK_LEFT_PIN, JOYSTICK_RIGHT_PIN, JOYSTICK_FIRE_PIN};
//const GPIO_TypeDef * buttonGPIO[7] = {GPIOC, GPIOA, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB};
//const char* consoleText[2][7] = {
//        {"USER PRESSED", "MODE PRESSED", "UP PRESSED", "DOWN PRESSED", "LEFT PRESSED", "RIGHT PRESSED", "FIRE PRESSED"},
//        {"USER RELEASED", "MODE RELEASED", "UP RELEASED", "DOWN RELEASED", "LEFT RELEASED", "RIGHT RELEASED", "FIRE RELEASED"}
//};
//const char buttonActive[7] = {0, 1, 0, 0, 0, 0, 0};
//char previousState[7] = {0, 0, 0, 0, 0, 0, 0};
// -----------------



void GPIOconfButton(GPIO_TypeDef *const gpio, uint32_t pin) {
    // TODO czym sie to rozni od GPIOinConfigure i ktorego uzwac
//    GPIOainConfigure(gpio, pin); // to nie dziala
    GPIOinConfigure(gpio, pin,
                    GPIO_PuPd_NOPULL,   // GPIOPuPd_TypeDef pull, // todo co to oznacza
                    EXTI_Mode_Event,   // EXTIMode_TypeDef mode,
                    EXTI_Trigger_Irrelevant    // EXTITrigger_TypeDef trigger);
    );
}

void confButtons() {
    // Włącz taktowanie
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN;

    // todo spytac czy musze to robic tak jak resetowalem LEDy
    USER_BUTTON_GPIO->BSRRH = 1 << USER_BUTTON_PIN;
    JOYSTICK_GPIO->BSRRL = (1 << JOYSTICK_UP_PIN) | (1 << JOYSTICK_DOWN_PIN) | (1 << JOYSTICK_LEFT_PIN) |
            (1 << JOYSTICK_RIGHT_PIN) | (1 << JOYSTICK_FIRE_PIN);
    MODE_BUTTON_GPIO->BSRRH = 1 << MODE_BUTTON_PIN;

    GPIOconfButton(USER_BUTTON_GPIO, USER_BUTTON_PIN);

    GPIOconfButton(JOYSTICK_GPIO, JOYSTICK_UP_PIN);
    GPIOconfButton(JOYSTICK_GPIO, JOYSTICK_DOWN_PIN);
    GPIOconfButton(JOYSTICK_GPIO, JOYSTICK_LEFT_PIN);
    GPIOconfButton(JOYSTICK_GPIO, JOYSTICK_RIGHT_PIN);
    GPIOconfButton(JOYSTICK_GPIO, JOYSTICK_FIRE_PIN);

    GPIOconfButton(MODE_BUTTON_GPIO, MODE_BUTTON_PIN);
}

// ============================= UTILS ===============================

#define CAN_WRITE (USART2->SR & USART_SR_TXE)
#define HAS_NEXT_CHAR (USART2->SR & USART_SR_RXNE)


char getcActiveWait() {
	for (;!HAS_NEXT_CHAR;) {
		__NOP();
	}
    char c;
    c = USART2->DR;
    return c;
}

char uartGetc() {
    char c;
    c = USART2->DR;
    return c;
}

void putcActiveWait(char c) {
	for (;!CAN_WRITE;) {
		__NOP();
	}
	USART2->DR = c;
}

void uartPutc(char c) {
    USART2->DR = c;
}


// ====================== SIMPLE QUE IMPLEMENTATION =========================

#define Q_SIZE 50

#include <stdio.h>
#include <string.h>

typedef struct {
    char buf[Q_SIZE];
    int begin, end;
} Que;

int clear(Que *q) {
    q->begin = q->end = 0;
}

int available(const Que *q) {

}

int size(const Que *q) {

}

bool isEmpty(const Que *q) {
    return q->begin == q->end;
}

bool isFull(const Que *q) {
//    return
}

bool startsWith(const Que *q, char* str) {
    int len = strlen(str);
    return 0;
}

void pushChar(Que *q, char c) {

}

void pushStr(Que *q, char* command) {
    // len x pushChar
}


char popFront(Que *q) {
    return ' ';
}

// returns true if found
bool popFrontIfEqual(const Que *q, char *str) {
    // combine startsWith and len x popFront
}

void buttonChange(GPIO_TypeDef * gpio, uint32_t pin, int active, char* name, Que *q) {
    if (HasButtonChanged(gpio, pin, active)) {
        pushStr(q, name);
        if (IsButtonPressed(gpio, pin, active)) {
            pushStr(q, " PRESSED\n");
            putcActiveWait('P');
        } else {
            pushStr(q, " RELEASED\n");
            putcActiveWait('R');
        }
        RevertButtonState(pin);
    }
}

int main() {
	confUSART();
	confLED();
    confButtons();

    Que out_q;
    Que in_q;
    clear(&out_q);
    clear(&in_q);

    // todo implement for
    // implement buttons, write whatever to console, keep leds blinkig
    // implement out
    // implement in
    // implement leds

	for (;;) {

        if (HAS_NEXT_CHAR) {
            char c = uartGetc();
            pushChar(&in_q, c);
        }

        if (!isEmpty(&in_q)) {
            if (popFrontIfEqual(&in_q, "LED 1 ON")) {
                RedLEDon();
            } else if (popFrontIfEqual(&in_q, "LED 1 OFF")) {
                RedLEDoff();
            } else if (popFrontIfEqual(&in_q, "LED 1 TOGGLE")) {
                RedLEDtoggle();
            } else if (popFrontIfEqual(&in_q, "LED 2 ON")) {
                GreenLEDon();
            } else if (popFrontIfEqual(&in_q, "LED 2 OFF")) {
                GreenLEDoff();
            } else if (popFrontIfEqual(&in_q, "LED 2 TOGGLE")) {
                GreenLEDtoggle();
            } else if (popFrontIfEqual(&in_q, "LED 3 ON")) {
                BlueLEDon();
            } else if (popFrontIfEqual(&in_q, "LED 3 OFF")) {
                BlueLEDoff();
            } else if (popFrontIfEqual(&in_q, "LED 3 TOGGLE")) {
                BlueLEDtoggle();
            } else if (popFrontIfEqual(&in_q, "LED 4 ON")) {
                Green2LEDon();
            } else if (popFrontIfEqual(&in_q, "LED 4 OFF")) {
                Green2LEDoff();
            } else if (popFrontIfEqual(&in_q, "LED 4 TOGGLE")) {
                Green2LEDtoggle();
            }
        }

        buttonChange(MODE_BUTTON_GPIO, MODE_BUTTON_PIN, MODE_BUTTON_ACTIVE, "MODE", &out_q);
        buttonChange(USER_BUTTON_GPIO, USER_BUTTON_PIN, USER_BUTTON_ACTIVE, "USER", &out_q);
        buttonChange(JOYSTICK_GPIO, JOYSTICK_UP_PIN, JOYSTICK_ACTIVE, "UP", &out_q);
        buttonChange(JOYSTICK_GPIO, JOYSTICK_DOWN_PIN, JOYSTICK_ACTIVE, "DOWN", &out_q);
        buttonChange(JOYSTICK_GPIO, JOYSTICK_LEFT_PIN, JOYSTICK_ACTIVE, "LEFT", &out_q);
        buttonChange(JOYSTICK_GPIO, JOYSTICK_RIGHT_PIN, JOYSTICK_ACTIVE, "RIGHT", &out_q);
        buttonChange(JOYSTICK_GPIO, JOYSTICK_FIRE_PIN, JOYSTICK_ACTIVE, "FIRE", &out_q);

        if (!isEmpty(&out_q)) {
            uartPutc(popFront(&out_q));
        }
	}

	return 0;
}


