#ifndef __wiringPiEnums_H
#define __wiringPiEnums_H

// Version
enum version : int
{
    VERSION_MAJOR = 2,
    VERSION_MINOR = 70
};

// PWM
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

// Memory sizes
enum sizes : int
{
    PAGE_SIZE = 4096,
    BLOCK_SIZE = 4096
};

// Integer error codes
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

// Clock register offsets
enum pwmClockRegisterOffsets : int
{
    PWMCLK_CNTL = 40,
    PWMCLK_DIV = 41,
    PWM0_MS_MODE = 0x0080,
    PWM0_USEFIFO = 0x0020,
    PWM0_REVPOLAR = 0x0010,
    PWM0_OFFSTATE = 0x0008,
    PWM0_REPEATFF = 0x0004,
    PWM0_SERIAL = 0x0002,
    PWM0_ENABLE = 0x0001,
    PWM1_MS_MODE = 0x8000,
    PWM1_USEFIFO = 0x2000,
    PWM1_REVPOLAR = 0x1000,
    PWM1_OFFSTATE = 0x0800,
    PWM1_REPEATFF = 0x0400,
    PWM1_SERIAL = 0x0200,
    PWM1_ENABLE = 0x0100
};

#endif