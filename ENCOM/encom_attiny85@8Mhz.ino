// ATtiny85 pinout:
// PB0 - Stairs output (fast PWM)
// PB1 - Pulse output (digital)
// PB2 - Not used
// PB3 - Trigger input (interrupt based)
// PB4 - Division analog input (ADC2)
#include <avr/io.h>                       // Port operations
#include <avr/interrupt.h>                // Interrupts
static const uint8_t stairs[32] = {       // PWM stairs output values
  0, 9, 17, 26, 34, 43, 51, 60,           // (somewhat reflect NCOM full tone steps)
  68, 77, 85, 94, 102, 111, 119, 128,
  137, 145, 154, 162, 171, 179, 188, 196,
  205, 213, 222, 230, 239, 247, 255, 255
};
volatile uint8_t step = 0;                // Current step
volatile uint8_t clk = 0;                 // Current clock phase
volatile uint8_t division = 0;            // Pulse division
ISR(PCINT0_vect) {                        // This interrupt is executed every time the trigger pin is changed
  clk = ((~PINB) & 0x08);                 // Save current clock phase (inverting logic)
  if (clk && ++step > division) step = 0; // If clock is present and preincremented value is greater than division, set it to zero
}
ISR(ADC_vect, ISR_NOBLOCK) {              // This interrupt is executed every time the ADC finished reading analog input. It will not block trigger interrupt routine
  division = (ADCH >> 3);                 // Since ADC value is left adjusted in ADCH & ADCL, and we need only 5 bits (0-31), we can fetch it from ADCH and shift it right by 3 places
}

int main() {
  sei();                                  // Enable interrupts
                                          // Prepare basic ports operation mode
  DDRB = 0x03;                              // Only Pulse and Stairs configured as outputs
  PORTB = 0x08;                             // Configure pull-up resistor for trigger input
                                          // Prepare PWM
  OCR0A = 0x01;                             // Initial PWM state
  TCCR0A = 0x83;                            // COM0A non-inverting mode, COM0B disconnected, fast PWM (mode 3)
  TCCR0B = 0x01;                            // fast PWM (mode 3), no prescaling
                                          // Prepare ADC
  DIDR0 = 0x14;                             // Disable digital buffer on ADC2 (PB4) and ADC1 (PB2 unused)
  ADCSRB = 0x00;                            // Free-running ADC conversion, unipolar
  ADMUX = 0x22;                             // Left aligned, Vcc as reference, ADC2
  ADCSRA = 0xee;                            // ADC conversion, 125kHz (8MHz / 64)
                                          // Prepare trigger interrupts
  PCMSK = 0x08;                             // Trigger on PB3 (PCINT3)
  GIMSK = 0x20;                             // Enable PCINT interrupt
  while (true) {                          // Main loop
    (clk && !step) ? PORTB |= 0x02 : PORTB &= 0xfd; // In step 0, send pulse when clock present
    OCR0A = stairs[step];                           // Constantly update current Stairs PWM based on step value
  }
}