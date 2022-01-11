// ATtiny85 pinout:
// PB0 - Random Byte output (OC0A)
// PB1 - S&H Value (OC0B)
// PB2 - S&H Trigger (INT0 falling edge mode)
// PB3 - Noise Output (Digital Output)
// PB4 - Noise Output (Digital Output)
#include <avr/io.h>                       // Port operations
#include <avr/interrupt.h>                // Interrupts
ISR(INT0_vect) { OCR0B = OCR0A; }         // Copy Random Byte as S&H value
int main() {
  uint32_t lfsr = 0xCAFEBABE;             // LFSR variable
  uint8_t tap_1 = (lfsr >> 31) & 0x1;     // LFSR tap 1 optimization (rightshifts reduction)
  uint8_t tap_2 = (lfsr >> 30) & 0x1;     // LFSR tap 2 optimization (rightshifts reduction)
  uint8_t future = 0;                     // Next bit to be inserted to the LFSR
  sei();                                  // Enable interrupts
                                          // Prepare basic ports operation mode
  DDRB = 0x1B;                              // Only PB2 (S&H Trigger) and PB5 as inputs
  PORTB = 0x04;                             // Configure pull-up resistor for S&H Trigger
                                          // Prepare PWM
  OCR0B = 0x7F;                             // Initial value for S&H
  TCCR0A = 0xA3;                            // Fast PWM (mode 3) on OC0A & OC0B (non-inverting, PW 0% glitch)
  TCCR0B = 0x01;                            // Fast PWM, full speed (no prescaling)
                                          // Prepare trigger interrupts
  MCUCR = 0x02;                             // Interrupt on falling edge on INT0
  GIMSK = 0x40;                             // Enable ISR for INT0 interrupt
  while (true) {                          // Main loop
                                            // Calculate new bit
    future = lfsr;                            // Tap 32
    future ^= (lfsr >> 10);                   // Tap 22
    future ^= tap_2;                          // Tap 2
    future ^= tap_1;                          // Tap 1
                                            // Modify LFSR
    lfsr >>= 1;                               // Shift LFSR to next position
    lfsr |= (uint32_t)(future) << 31;         // Insert future value to LFSR
                                            // Update new taps
    tap_2 = tap_1;                            // Tap 2 becomes Tap 1
    tap_1 = future;                           // Tap 1 becomes current bit
                                            // Handle outputs
    PORTB &= 0xE7;                            // Clear Noise Outputs
    PORTB |= (lfsr & 0x18);                   // Set new Noise Output values
    OCR0A = lfsr;                             // Set Random Byte
  }
}