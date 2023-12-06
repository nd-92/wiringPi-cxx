#ifndef __wiringPi_H
#define __wiringPi_H

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>

// #include "softPwm.H"
// #include "softTone.H"

// C doesn't have true/false by default and I can never remember which
// way round they are, so ...
// (and yes, I know about stdbool.h but I like capitals for these and I'm old)

// Mask for the bottom 64 pins which belong to the Raspberry Pi
// The others are available for the other devices

#define PI_GPIO_MASK (0xFFFFFFC0)

// Handy defines

// Versions
enum version : int
{
    VERSION_MAJOR = 2,
    VERSION_MINOR = 70
};

// PWM
// Word offsets into the PWM control region

enum pwmOffsets : int
{
    PWM_CONTROL = 0,
    PWM_STATUS = 1,
    PWM0_RANGE = 4,
    PWM0_DATA = 5,
    PWM1_RANGE = 8,
    PWM1_DATA = 9
};

// wiringPi modes

enum wpiModes : int
{
    WPI_MODE_PINS = 0,
    WPI_MODE_GPIO = 1,
    WPI_MODE_GPIO_SYS = 2,
    WPI_MODE_PHYS = 3,
    WPI_MODE_PIFACE = 4,
    WPI_MODE_UNINITIALISED = -1
};

// Port function select bits

enum fselBits : int
{
    FSEL_INPT = 0b000,
    FSEL_OUTP = 0b001,
    FSEL_ALT0 = 0b100,
    FSEL_ALT1 = 0b101,
    FSEL_ALT2 = 0b110,
    FSEL_ALT3 = 0b111,
    FSEL_ALT4 = 0b011,
    FSEL_ALT5 = 0b010
};

// Pin modes

enum pinModes : int
{
    INPUT = 0,
    OUTPUT = 1,
    PWM_OUTPUT = 2,
    GPIO_CLOCK = 3,
    SOFT_PWM_OUTPUT = 4,
    SOFT_TONE_OUTPUT = 5,
    PWM_TONE_OUTPUT = 6
};

// Try to eliminate use of preprocessor for bool return vals
// #define LOW 0
// #define HIGH 1

// Pull up/down/none

enum pudModes : int
{
    PUD_OFF = 0,
    PUD_DOWN = 1,
    PUD_UP = 2
};

// PWM

enum pwmModes : int
{
    PWM_MODE_MS = 0,
    PWM_MODE_BAL = 1,
};

// Interrupt levels

enum interruptLevels : int
{
    INT_EDGE_SETUP = 0,
    INT_EDGE_FALLING = 1,
    INT_EDGE_RISING = 2,
    INT_EDGE_BOTH = 3,
};

// Pi model types

enum piModels : int
{
    PI_MODEL_A = 0,
    PI_MODEL_B = 1,
    PI_MODEL_AP = 2,
    PI_MODEL_BP = 3,
    PI_MODEL_2 = 4,
    PI_ALPHA = 5,
    PI_MODEL_CM = 6,
    PI_MODEL_07 = 7,
    PI_MODEL_3B = 8,
    PI_MODEL_ZERO = 9,
    PI_MODEL_CM3 = 10,
    PI_MODEL_ZERO_W = 12,
    PI_MODEL_3BP = 13,
    PI_MODEL_3AP = 14,
    PI_MODEL_CM3P = 16,
    PI_MODEL_4B = 17,
    PI_MODEL_ZERO_2W = 18,
    PI_MODEL_400 = 19,
    PI_MODEL_CM4 = 20,
};

// Pi version numbers

enum piVersions : int
{
    PI_VERSION_1 = 0,
    PI_VERSION_1_1 = 1,
    PI_VERSION_1_2 = 2,
    PI_VERSION_2 = 3,
};

// Pi manufacturers

enum piMakers : int
{
    PI_MAKER_SONY = 0,
    PI_MAKER_EGOMAN = 1,
    PI_MAKER_EMBEST = 2,
    PI_MAKER_UNKNOWN = 3
};

// piGpioBase
// Is this cast safe?
enum piGpioPeriBase : long int
{
    GPIO_PERI_BASE_OLD = 0x20000000,
    GPIO_PERI_BASE_2835 = 0x3F000000,
    GPIO_PERI_BASE_2711 = static_cast<long int>(0xFE000000)
};

enum gpPud : int
{
    GPPUD = 37,
    GPPUPPDN0 = 57,
    GPPUPPDN1 = 58,
    GPPUPPDN2 = 59,
    GPPUPPDN3 = 60
};

enum sizes : int
{
    PAGE_SIZE = 4096,
    BLOCK_SIZE = 4096
};

#define ENV_DEBUG "WIRINGPI_DEBUG"
#define ENV_CODES "WIRINGPI_CODES"
#define ENV_GPIOMEM "WIRINGPI_GPIOMEM"

enum wpiErrorCodes : int
{
    WPI_FATAL = true,
    WPI_ALMOST = false
};

// Timer
enum timerOffsets : int
{
    TIMER_LOAD = (0x400 >> 2),
    TIMER_VALUE = (0x404 >> 2),
    TIMER_CONTROL = (0x408 >> 2),
    TIMER_IRQ_CLR = (0x40C >> 2),
    TIMER_IRQ_RAW = (0x410 >> 2),
    TIMER_IRQ_MASK = (0x414 >> 2),
    TIMER_RELOAD = (0x418 >> 2),
    TIMER_PRE_DIV = (0x41C >> 2),
    TIMER_COUNTER = (0x420 >> 2)
};

class wiringPi
{
public:
    // Constructor
    inline wiringPi(const int mode)
    {
        std::cout << "wiringPi initialised on mode " << mode << std::endl;
    };

    // Destructor
    inline ~wiringPi()
    {
        std::cout << "wiringPi terminated." << std::endl;
    };

    // Inline methods methods
    inline void version(int *major, int *minor)
    {
        *major = VERSION_MAJOR;
        *minor = VERSION_MINOR;
    }

    // Non-inline methods
    int failure(int fatal, const char *message, ...);
    void gpioLayoutOops(const char *why);
    int gpioLayout();
    void boardID(int *model, int *rev, int *mem, int *maker, int *warranty);
    int setup();
    void initialiseEpoch();
    void delay(const time_t howLong);
    void delayMicroseconds(const time_t howLong);
    void delayMicrosecondsHard(const time_t howLong);
    time_t millis();
    time_t micros();
    int setupSys();
    int setupGpio();
    int setupPhys();

    // Data

    // Timers
    time_t epochMilli, epochMicro;

    // Debugging & Return codes
    int wiringPiDebug = 1;
    int wiringPiReturnCodes = 0;
    long int piGpioBase = 0;
    unsigned int piGpioPupOffset = 0;
    int wiringPiMode = WPI_MODE_UNINITIALISED;
    int *pinToGpio;
    int *physToGpio;
    unsigned int usingGpioMem = 0;

    long int GPIO_PADS;
    long int GPIO_CLOCK_BASE;
    long int GPIO_BASE;
    long int GPIO_TIMER;
    long int GPIO_PWM;
    int wiringPiSetuped = false;

    // Locals to hold pointers to the hardware
    uint32_t *gpio;
    uint32_t *pwm;
    uint32_t *clk;
    uint32_t *pads;
    uint32_t *timer;
    uint32_t *timerIrqRaw;

    // Export variables for the hardware pointers
    uint32_t *_wiringPiGpio;
    uint32_t *_wiringPiPwm;
    uint32_t *_wiringPiClk;
    uint32_t *_wiringPiPads;
    uint32_t *_wiringPiTimer;
    uint32_t *_wiringPiTimerIrqRaw;

    const char *piModelNames[21] = {
        "Model A",    // 00
        "Model B",    // 01
        "Model A+",   // 02
        "Model B+",   // 03
        "Pi 2",       // 04
        "Alpha",      // 05
        "CM",         // 06
        "Unknown07",  // 07
        "Pi 3",       // 08
        "Pi Zero",    // 09
        "CM3",        // 10
        "Unknown11",  // 11
        "Pi Zero-W",  // 12
        "Pi 3B+",     // 13
        "Pi 3A+",     // 14
        "Unknown15",  // 15
        "CM3+",       // 16
        "Pi 4B",      // 17
        "Pi Zero2-W", // 18
        "Pi 400",     // 19
        "CM4",        // 20
    };

    const char *piRevisionNames[16] = {
        "00",
        "01",
        "02",
        "03",
        "04",
        "05",
        "06",
        "07",
        "08",
        "09",
        "10",
        "11",
        "12",
        "13",
        "14",
        "15",
    };

    const char *piMakerNames[16] = {
        "Sony",      //	 0
        "Egoman",    //	 1
        "Embest",    //	 2
        "Unknown",   //	 3
        "Embest",    //	 4
        "Unknown05", //	 5
        "Unknown06", //	 6
        "Unknown07", //	 7
        "Unknown08", //	 8
        "Unknown09", //	 9
        "Unknown10", //	10
        "Unknown11", //	11
        "Unknown12", //	12
        "Unknown13", //	13
        "Unknown14", //	14
        "Unknown15", //	15
    };

    const int piMemorySize[8] = {
        256,  //	 0
        512,  //	 1
        1024, //	 2
        2048, //	 3
        4096, //	 4
        8192, //	 5
        0,    //	 6
        0,    //	 7
    };

    int sysFds[64] = {
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1  //
    };

    const int pinToGpioR1[64] = {
        17, 18, 21, 22, 23, 24, 25, 4,  //
        0, 1, 8, 7, 10, 9, 11, 14,      //
        15, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1  //
    };

    const int pinToGpioR2[64] = {
        17, 18, 27, 22, 23, 24, 25, 4,  //
        2, 3, 8, 7, 10, 9, 11, 14,      //
        15, 28, 29, 30, 31, 5, 6, 13,   //
        19, 26, 12, 16, 20, 21, 0, 1,   //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1  //
    };

    const int physToGpioR1[64] = {
        -1, -1, -1, 0, -1, 1, -1, 4,    //
        14, -1, 15, 17, 18, 21, -1, 22, //
        23, -1, 24, 10, -1, 9, 25, 11,  //
        8, -1, 7, -1, -1, -1, -1, -1,   //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1  //
    };

    const int physToGpioR2[64] = {
        -1, -1, -1, 2, -1, 3, -1, 4,    //
        14, -1, 15, 17, 18, 27, -1, 22, //
        23, -1, 24, 10, -1, 9, 25, 11,  //
        8, -1, 7, 0, 1, 5, -1, 6,       //
        12, 13, -1, 19, 16, 26, 20, -1, //
        21, -1, -1, -1, -1, -1, -1, -1, //
        -1, -1, -1, 28, 29, 30, 31, -1, //
        -1, -1, -1, -1, -1, -1, -1, -1  //
    };

    const int gpioToGPFSEL[60] = {
        0, 0, 0, 0, //
        0, 0, 0, 0, //
        0, 0, 1, 1, //
        1, 1, 1, 1, //
        1, 1, 1, 1, //
        2, 2, 2, 2, //
        2, 2, 2, 2, //
        2, 2, 3, 3, //
        3, 3, 3, 3, //
        3, 3, 3, 3, //
        4, 4, 4, 4, //
        4, 4, 4, 4, //
        4, 4, 5, 5, //
        5, 5, 5, 5, //
        5, 5, 5, 5  //
    };

    const int gpioToShift[60] = {
        0, 3, 6, 9,     //
        12, 15, 18, 21, //
        24, 27, 0, 3,   //
        6, 9, 12, 15,   //
        18, 21, 24, 27, //
        0, 3, 6, 9,     //
        12, 15, 18, 21, //
        24, 27, 0, 3,   //
        6, 9, 12, 15,   //
        18, 21, 24, 27, //
        0, 3, 6, 9,     //
        12, 15, 18, 21, //
        24, 27, 0, 3,   //
        6, 9, 12, 15,   //
        18, 21, 24, 27  //
    };

    const int gpioToGPSET[64] = {
        7, 7, 7, 7, 7, 7, 7, 7, //
        7, 7, 7, 7, 7, 7, 7, 7, //
        7, 7, 7, 7, 7, 7, 7, 7, //
        7, 7, 7, 7, 7, 7, 7, 7, //
        8, 8, 8, 8, 8, 8, 8, 8, //
        8, 8, 8, 8, 8, 8, 8, 8, //
        8, 8, 8, 8, 8, 8, 8, 8, //
        8, 8, 8, 8, 8, 8, 8, 8  //
    };

    const int gpioToGPCLR[64] = {
        10, 10, 10, 10, 10, 10, 10, 10, //
        10, 10, 10, 10, 10, 10, 10, 10, //
        10, 10, 10, 10, 10, 10, 10, 10, //
        10, 10, 10, 10, 10, 10, 10, 10, //
        11, 11, 11, 11, 11, 11, 11, 11, //
        11, 11, 11, 11, 11, 11, 11, 11, //
        11, 11, 11, 11, 11, 11, 11, 11, //
        11, 11, 11, 11, 11, 11, 11, 11  //
    };

    const int gpioToGPLEV[64] = {
        13, 13, 13, 13, 13, 13, 13, 13, //
        13, 13, 13, 13, 13, 13, 13, 13, //
        13, 13, 13, 13, 13, 13, 13, 13, //
        13, 13, 13, 13, 13, 13, 13, 13, //
        14, 14, 14, 14, 14, 14, 14, 14, //
        14, 14, 14, 14, 14, 14, 14, 14, //
        14, 14, 14, 14, 14, 14, 14, 14, //
        14, 14, 14, 14, 14, 14, 14, 14  //
    };

    const int gpioToPUDCLK[64] = {
        38, 38, 38, 38, 38, 38, 38, 38, //
        38, 38, 38, 38, 38, 38, 38, 38, //
        38, 38, 38, 38, 38, 38, 38, 38, //
        38, 38, 38, 38, 38, 38, 38, 38, //
        39, 39, 39, 39, 39, 39, 39, 39, //
        39, 39, 39, 39, 39, 39, 39, 39, //
        39, 39, 39, 39, 39, 39, 39, 39, //
        39, 39, 39, 39, 39, 39, 39, 39  //
    };

    const int gpioToPwmALT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,                         //  0 ->  7
        0, 0, 0, 0, FSEL_ALT0, FSEL_ALT0, 0, 0,         //  8 -> 15
        0, 0, FSEL_ALT5, FSEL_ALT5, 0, 0, 0, 0,         // 16 -> 23
        0, 0, 0, 0, 0, 0, 0, 0,                         // 24 -> 31
        0, 0, 0, 0, 0, 0, 0, 0,                         // 32 -> 39
        FSEL_ALT0, FSEL_ALT0, 0, 0, 0, FSEL_ALT0, 0, 0, // 40 -> 47
        0, 0, 0, 0, 0, 0, 0, 0,                         // 48 -> 55
        0, 0, 0, 0, 0, 0, 0, 0,                         // 56 -> 63
    };

    const int gpioToPwmPort[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,                         //  0 ->  7
        0, 0, 0, 0, PWM0_DATA, PWM1_DATA, 0, 0,         //  8 -> 15
        0, 0, PWM0_DATA, PWM1_DATA, 0, 0, 0, 0,         // 16 -> 23
        0, 0, 0, 0, 0, 0, 0, 0,                         // 24 -> 31
        0, 0, 0, 0, 0, 0, 0, 0,                         // 32 -> 39
        PWM0_DATA, PWM1_DATA, 0, 0, 0, PWM1_DATA, 0, 0, // 40 -> 47
        0, 0, 0, 0, 0, 0, 0, 0,                         // 48 -> 55
        0, 0, 0, 0, 0, 0, 0, 0,                         // 56 -> 63
    };

    const int gpioToGpClkALT0[64] = {
        0, 0, 0, 0, FSEL_ALT0, FSEL_ALT0, FSEL_ALT0, 0, //  0 ->  7
        0, 0, 0, 0, 0, 0, 0, 0,                         //  8 -> 15
        0, 0, 0, 0, FSEL_ALT5, FSEL_ALT5, 0, 0,         // 16 -> 23
        0, 0, 0, 0, 0, 0, 0, 0,                         // 24 -> 31
        FSEL_ALT0, 0, FSEL_ALT0, 0, 0, 0, 0, 0,         // 32 -> 39
        0, 0, FSEL_ALT0, FSEL_ALT0, FSEL_ALT0, 0, 0, 0, // 40 -> 47
        0, 0, 0, 0, 0, 0, 0, 0,                         // 48 -> 55
        0, 0, 0, 0, 0, 0, 0, 0,                         // 56 -> 63
    };

    const int gpioToClkCon[64] = {
        -1, -1, -1, -1, 28, 30, 32, -1, //  0 ->  7
        -1, -1, -1, -1, -1, -1, -1, -1, //  8 -> 15
        -1, -1, -1, -1, 28, 30, -1, -1, // 16 -> 23
        -1, -1, -1, -1, -1, -1, -1, -1, // 24 -> 31
        28, -1, 28, -1, -1, -1, -1, -1, // 32 -> 39
        -1, -1, 28, 30, 28, -1, -1, -1, // 40 -> 47
        -1, -1, -1, -1, -1, -1, -1, -1, // 48 -> 55
        -1, -1, -1, -1, -1, -1, -1, -1, // 56 -> 63
    };

    const int gpioToClkDiv[64] = {
        -1, -1, -1, -1, 29, 31, 33, -1, //  0 ->  7
        -1, -1, -1, -1, -1, -1, -1, -1, //  8 -> 15
        -1, -1, -1, -1, 29, 31, -1, -1, // 16 -> 23
        -1, -1, -1, -1, -1, -1, -1, -1, // 24 -> 31
        29, -1, 29, -1, -1, -1, -1, -1, // 32 -> 39
        -1, -1, 29, 31, 29, -1, -1, -1, // 40 -> 47
        -1, -1, -1, -1, -1, -1, -1, -1, // 48 -> 55
        -1, -1, -1, -1, -1, -1, -1, -1, // 56 -> 63
    };
};

#endif