// ATtiny85 pinout:
// PB0 - Step 2 Output (OC0A)
// PB1 - Step 1 Output (OC0B)
// PB2 - ASR Trigger (INT0 falling edge mode)
// PB3 - Input signal (ADC3)
// PB4 - Step 3 Output (OC1B)
#include <avr/io.h>                       // Port operations
#include <avr/interrupt.h>                // Interrupts
volatile uint8_t sample = 127;            // Sample value for input signal
ISR(INT0_vect) {                          // Propagate values on trigger (starting with last step)
  OCR1B = 0xff - OCR0A;                     // 2 -> 3
  OCR0A = OCR0B;                            // 1 -> 2
  OCR0B = sample;                           // sample -> 1
}
ISR(ADC_vect, ISR_NOBLOCK) {              // Sample Input signal, non-blocking
  sample = ADCH;                            // Since PWM is 8-bit, grab value shifted to left from ADCH
}
int main() {
                                          // Prepare basic ports operation mode
  DDRB = 0x13;                              // Only ASR Trigger (PB2) and Input signal (PB3) as inputs
  PORTB = 0x04;                             // Configure pull-up resistor for trigger input
                                          // Prepare PWM
  OCR0A = OCR0B = OCR1B = sample;           // Initialize all outputs to default sample value
  OCR1C = 0xff;                             // Timer/Counter 1 TOP is 255
  TCCR0A = 0xf3;                            // Fast PWM (mode 3) on OC0A & OC0B (inverting, PW 100% glitch)
  TCCR0B = 0x01;                            // Fast PWM, full speed (no prescaling)
  TCCR1 = 0x01;                             // Fast PWM, full speed (no prescaling) (for Timer/Counter 1)
  GTCCR = 0x60;                             // PWM on OC1B (non-inverting because of a hardware bug)
                                          // Prepare ADC
  DIDR0 = 0x08;                             // Disable digital buffer on ADC3 (PB3)
  ADCSRB = 0x00;                            // Free-running ADC conversion, unipolar
  ADMUX = 0x23;                             // Left aligned, Vcc as reference, ADC3
  ADCSRA = 0xef;                            // ADC conversion, 125kHz (16MHz / 128)
                                          // Prepare trigger interrupts
  MCUCR = 0x02;                             // Interrupt on falling edge on INT0
  GIMSK = 0x40;                             // Enable ISR for INT0 interrupt
  sei();                                  // Enable interrupts
  while (true) {                          // Main dummy loop to keep ATTiny85 running
    uint8_t dummy = 0;
    for (uint8_t i = 1; i < 255; ++i) dummy = (dummy * 0) + i - 1;
  }
}
