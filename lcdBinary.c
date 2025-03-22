/* ***************************************************************************** */
/* You can use this file to define the low-level hardware control fcts for       */
/* LED, button and LCD devices.                                                  */ 
/* Note that these need to be implemented in Assembler.                          */
/* You can use inline Assembler code, or use a stand-alone Assembler file.       */
/* Alternatively, you can implement all fcts directly in master-mind.c,          */  
/* using inline Assembler code there.                                            */
/* The Makefile assumes you define the functions here.                           */
/* ***************************************************************************** */


#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif

#define	PAGE_SIZE		(4*1024)
#define	BLOCK_SIZE		(4*1024)

#define	INPUT			 0
#define	OUTPUT			 1

#define	LOW			 0
#define	HIGH			 1


// APP constants   ---------------------------------

// Wiring (see call to lcdInit in main, using BCM numbering)
// NB: this needs to match the wiring as defined in master-mind.c

#define STRB_PIN 24
#define RS_PIN   25
#define DATA0_PIN 23
#define DATA1_PIN 10
#define DATA2_PIN 27
#define DATA3_PIN 22

// -----------------------------------------------------------------------------
// includes 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

// -----------------------------------------------------------------------------
// prototypes

int failure (int fatal, const char *message, ...);

// -----------------------------------------------------------------------------
// Functions to implement here (or directly in master-mind.c)

/* this version needs gpio as argument, because it is in a separate file */
void digitalWrite (uint32_t *gpio, int pin, int value) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
  int reg = (value) ? (7): (10);
  int shift = pin % 32; 

  asm volatile(
    "mov r1, %1\n"        // Move the shift amount into r1
    "mov r2, #1\n"        // Move 1 into r2
    "lsl r2, r2, r1\n"    // Shift r2 left by the shift amount
    "str r2, [%0]\n"      // Store the value into the GPIO register
    :
    : "r" (gpio + reg), "r" (shift)
    : "r1", "r2", "memory"
  );
}

// adapted from setPinMode
// void pinMode(uint32_t *gpio, int pin, int mode /*, int fSel, int shift */) {
//   int fSel = pin / 10;
//   int shift = (pin % 10) * 3;
  
//   if (mode == INPUT) {
//       asm volatile (
//           "ldr r0, [%0, %1]\n\t"
          
//           "bic r0, r0, #(7 << %2)\n\t"
//           "str r0, [%0, %1]\n\t"
//           : 
//           : "r" (gpio), "r" (fSel * 4), "r" (shift)
//           : "r0"
//       );
//   } else if (mode == OUTPUT) {
//       asm volatile (
//           "ldr r0, [%0, %1]\n\t"
//           "bic r0, r0, #(7 << %2)\n\t"
//           "orr r0, r0, #(1 << %2)\n\t"
//           "str r0, [%0, %1]\n\t"
//           : 
//           : "r" (gpio), "r" (fSel * 4), "r" (shift)
//           : "r0"
//       );
//   }
// }

// adapted from setPinMode
void pinMode(uint32_t *gpio, int pin, int mode) {
  int reg = pin / 10;
  int shift = (pin % 10) * 3;

  if (mode == INPUT) {
    asm volatile(
      "ldr r1, [%0]\n"        // Load the current value of the GPIO register into r1
      "bic r1, r1, %1\n"      // Clear the 3 bits corresponding to the pin
      "str r1, [%0]\n"        // Store the modified value back into the GPIO register
      :
      : "r" (&gpio[reg]), "r" (7 << shift)
      : "r1", "memory"
    );
  } else if (mode == OUTPUT) {
    asm volatile(
      "ldr r1, [%0]\n"        // Load the current value of the GPIO register into r1
      "bic r1, r1, %1\n"      // Clear the 3 bits corresponding to the pin
      "orr r1, r1, %2\n"      // Set the bit corresponding to OUTPUT mode
      "str r1, [%0]\n"        // Store the modified value back into the GPIO register
      :
      : "r" (&gpio[reg]), "r" (7 << shift), "r" (1 << shift)
      : "r1", "memory"
    );
  }
}
//sets or clears the LED using GPSET and GPCLR
void writeLED(uint32_t *gpio, int led, int value) {
  if (gpio == NULL) {
    fprintf(stderr, "Error: GPIO not initialized\n");
    exit(EXIT_FAILURE);
}
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
  int reg = (value) ? (7) : (10); // 7 for setting high, 10 for setting low
  int shift = led % 32; // Calculate the shift amount

  asm volatile(
    "mov r1, %1\n"        // Move the shift amount into r1
    "mov r2, #1\n"        // Move 1 into r2
    "lsl r2, r2, r1\n"    // Shift r2 left by the shift amount
    "str r2, [%0]\n"      // Store the value into the GPIO register
    :
    : "r" (gpio + reg), "r" (shift)
    : "r1", "r2", "memory"
  );
}

int readButton(uint32_t *gpio, int button) {
  /* This function reads the state of a button by checking the corresponding bit in the GPIO register.
     It returns 1 if the button is pressed (high) and 0 if the button is not pressed (low). */
  int shift = button % 32; // Calculate the shift amount
  uint32_t *gplev = gpio +13; //13 is the offset for the GPLEV0 register
  int value;
  asm volatile(
    "ldr r0, [%1]\n"      // Load the value from the GPIO register into r0
    "mov r1, #1\n"        // Move 1 into r1
    "lsl r1, r1, %2\n"    // Shift r1 left by the shift amount
    "and %0, r0, r1\n"    // Perform bitwise AND to isolate the button state
    "lsr %0, %0, %2\n"    // Shift right to get the button state (0 or 1)
    : "=r" (value)
    : "r" (gplev), "r" (shift) // 13 is the offset for GPLEV0 register
    : "r0", "r1", "memory"
  );

  return value;
}

void waitForButton(uint32_t *gpio, int button) {
  /* ***  COMPLETE the code here, just C no Assembler; you can use readButton ***  */
  printf("Waiting for button press...\n");
  while (1){ //infinite loop 
    if (readButton(gpio, button)== 1){  //if readbutton is high, no longer need to wait for it to be clicked
      printf("Button pressed!\n");
      break; //breaks out of loop 
    }
  }
}
