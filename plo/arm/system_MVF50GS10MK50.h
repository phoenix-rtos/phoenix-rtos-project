/*
** ###################################################################
**     Processor:           MVF50GS10MK50
**     Compilers:           ARM Compiler
**                          Freescale C/C++ for Embedded ARM
**                          GNU C Compiler
**                          IAR ANSI C/C++ Compiler for ARM
**
**     Reference manual:    Faraday RM Rev. 1, Draft B Feb 2012
**     Version:             rev. 0.1, 2012-05-30
**
**     Abstract:
**         Provides a system configuration function and a global variable that
**         contains the system frequency. It configures the device and initializes
**         the oscillator (PLL) that is part of the microcontroller device.
**
**     Copyright: 2012 Freescale Semiconductor, Inc. All Rights Reserved.
**
**     http:                 www.freescale.com
**     mail:                 support@freescale.com
**
**     Revisions:
**     - rev. 0.1 (2012-05-30)
**         Initial version
**
** ###################################################################
*/

/**
 * @file system_MVF50GS10MK50.h
 * @version 0.1
 * @date 2012-05-30
 * @brief Device specific configuration file for MVF50GS10MK50 (header file)
 *
 * Provides a system configuration function and a global variable that contains
 * the system frequency. It configures the device and initializes the oscillator
 * (PLL) that is part of the microcontroller device.
 */

#ifndef SYSTEM_MVF50GS10MK50_H_
#define SYSTEM_MVF50GS10MK50_H_                  /**< Symbol preventing repeated inclusion */

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
typedef u64 uint64_t;
typedef s64 int64_t;
typedef u32 uint32_t;
typedef s32 int32_t;
typedef u16 uint16_t;
typedef s16 int16_t;
typedef u8  uint8_t;

/* Define Clock Setup for system */
/* The BootROM by default will modify registers to set the system clock to 264MHz. 
   Clock Setup #1 and #2 assume BootROM has ran and now want to go to higher clock speed
   Clock Setup #3 is a no_init case which leaves the default BootROM values
   Clock Setup #4 is a special debug case if the core was halted immediatly after reset so that the BootROM never ran.
      The setup code for #4 must then do the clock init the BootROM normally does
*/

#ifndef CLOCK_SETUP //If not set in project settings, use the value set here

  //#define CLOCK_SETUP 	  0	/* Use 396MHz CA5, 396 MHz DDR, 132MHz CM4, and 66MHz bus, 24MHz XOSC clock source */
  //#define CLOCK_SETUP     1	/* Use 500MHz CA5, 396 MHz DDR, 167MHz CM4, and 83.5MHz bus, 24MHz XOSC clock source */
  #define CLOCK_SETUP     2	/* Leave BootROM defaults - Uses 264MHz CA5 (only changes bus clock to 66.0MHz), 24MHz XOSC clock source */
  //#define CLOCK_SETUP     3	/* Assumes BootROM did not run. Use 396MHz CA5, 396 MHz DDR, 167MHz CM4, and 83.5MHz bus, 24MHz XOSC clock source */
#endif

/* Define CPU version on the board */

#ifdef NO_PLL_INIT
  #define A5_CORE_CLK_KHZ 24000
  #define M4_CORE_CLK_KHZ 12000
  #define BUS_CLK_KHZ     12000
#elif (CLOCK_SETUP == 0)
  #define A5_CORE_CLK_KHZ 396000
  #define M4_CORE_CLK_KHZ 132000
  #define BUS_CLK_KHZ      66000
#elif (CLOCK_SETUP == 1)
  #define A5_CORE_CLK_KHZ 500000
  #define M4_CORE_CLK_KHZ 167000
  #define BUS_CLK_KHZ      83500
#elif (CLOCK_SETUP == 2)
  #define ARM_CLOCK_DIV  2
  #define BUS_CLOCK_DIV  2
  #define IPG_CLOCK_DIV  2
  #define PLL_OUTPUT_CLK_KHZ  528000
  #define A5_CORE_CLK_KHZ (PLL_OUTPUT_CLK_KHZ / ARM_CLOCK_DIV) // 264000
  #define M4_CORE_CLK_KHZ (A5_CORE_CLK_KHZ    / BUS_CLOCK_DIV) // 132000
  #define BUS_CLK_KHZ     (M4_CORE_CLK_KHZ    / IPG_CLOCK_DIV) //  66000
#elif (CLOCK_SETUP == 3)
  #define A5_CORE_CLK_KHZ 396000
  #define M4_CORE_CLK_KHZ 132000
  #define BUS_CLK_KHZ      66000
#else
  #error "CLOCK_SETUP not supported"
#endif

	
/**
 * @brief System clock frequency (core clock)
 *
 * The system clock frequency supplied to the SysTick timer and the processor
 * core clock. This variable can be used by the user application to setup the
 * SysTick timer or configure other parameters. It may also be used by debugger to
 * query the frequency of the debug timer or configure the trace clock speed
 * SystemCoreClock is initialized with a correct predefined value.
 */
extern uint32_t SystemCoreClock;

/**
 * @brief Setup the microcontroller system.
 *
 * Typically this function configures the oscillator (PLL) that is part of the
 * microcontroller device. For systems with variable clock speed it also updates
 * the variable SystemCoreClock. SystemInit is called from startup_device file.
 */
void SystemInit (void);

/**
 * @brief Updates the SystemCoreClock variable.
 *
 * It must be called whenever the core clock is changed during program
 * execution. SystemCoreClockUpdate() evaluates the clock register settings and calculates
 * the current core clock.
 * @note 	SystemCoreClock is in units of KHz
 */
void SystemCoreClockUpdate (void);

/**
 * @brief Calculate the bus clock from the SystemCoreClock and the chip's configuration
 *
 * @note 	Units of KHz
 */
extern u32 get_bus_clk(void);


extern void hal_reset(void);

#ifdef __cplusplus
}
#endif

#endif  /* #if !defined(SYSTEM_MVF50GS10MK50_H_) */
