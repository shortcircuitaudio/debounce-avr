#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define LED1 PB0
#define BUTTON PB3

typedef struct
{
  uint8_t acc;
} button_t;

inline void button_update(button_t *button, uint8_t pin_value)
{
  button->acc = (button->acc << 1) | pin_value;
}

volatile uint8_t tick = 0;

ISR(TIM0_COMPA_vect)
{
  tick = 1;
}

void timer0_init()
{
  TIMSK0 |= _BV(OCIE0A);
  OCR0A = 149;
  TCCR0A |= _BV(WGM01); // CTC mode
  TCCR0B |= _BV(CS01);  // prescaler DIV8
}

int main(void)
{
  button_t button = {0};

  // setting port direction and pull up resistors
  DDRB = _BV(LED1);
  PORTB = _BV(BUTTON);

  timer0_init();
  sei();

  while (1)
  {
    while (!tick)
      ;
    tick = 0;

    button_update(&button, (~PINB >> BUTTON) & 0x01);
    switch (button.acc)
    {
    case 0x00:
      PORTB &= ~_BV(LED1);
      break;
    case 0xFF:
      PORTB |= _BV(LED1);
      break;
    default:
      break;
    }
  }
}
