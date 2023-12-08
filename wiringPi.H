#ifndef __wiringPi_H
#define __wiringPi_H

#include "wiringPiIncludes.H"
#include "wiringPiEnums.H"

// For unused attributes
#define UNU __attribute__((unused))

// #include "softPwm.H"
// #include "softTone.H"

// C doesn't have true/false by default and I can never remember which
// way round they are, so ...
// (and yes, I know about stdbool.h but I like capitals for these and I'm old)

// Mask for the bottom 64 pins which belong to the Raspberry Pi
// The others are available for the other devices

#define PI_GPIO_MASK (0xFFFFFFC0)

#define BCM_PASSWORD 0x5A000000

#define ENV_DEBUG "WIRINGPI_DEBUG"
#define ENV_CODES "WIRINGPI_CODES"
#define ENV_GPIOMEM "WIRINGPI_GPIOMEM"

const uint32_t EMPTY_PIN = std::numeric_limits<uint32_t>::max();

#define GPIO_CLOCK_SOURCE 1

class wiringPi
{
public:
    // Constructor
    inline wiringPi(const int debugMode)
        : major(VERSION_MAJOR),
          minor(VERSION_MINOR)
    {
        std::cout << "wiringPi initialised, debug mode = " << debugMode << std::endl;
        wiringPiDebug = debugMode;
    };

    // Destructor
    inline ~wiringPi()
    {
        std::cout << "wiringPi terminated." << std::endl;
    };

    // Non-inline methods
    int failure(int fatal, const char *message, ...);
    void gpioLayoutOops(const char *why);
    int gpioLayout();
    void boardID(int *model, int *rev, int *mem, int *maker, int *warranty);
    int setup();
    int setupSys();
    int setupGpio();
    int setupPhys();
    void setupCheck(const char *fName);
    void pinModeAlt(uint32_t pin, const uint32_t mode);
    void setPadDrive(const uint32_t group, const uint32_t value);
    int getAlt(uint32_t pin);
    // void pwmToneWrite(int pin, int freq);
    void pwmSetMode(const int mode);
    void pwmSetRange(const uint32_t range);
    void pwmSetClock(uint32_t divisor);
    void gpioClockSet(uint32_t pin, const uint32_t freq);
    int waitForInterrupt(uint32_t pin, int mS);

    // Inline methods
    inline uint32_t wpiPinToGpio(const uint32_t wpiPin)
    {
        return pinToGpio[wpiPin & 63];
    }
    inline uint32_t physPinToGpio(const uint32_t physPin)
    {
        return physToGpio[physPin & 63];
    }
    inline void printVersion()
    {
        std::cout << "wiringPi version " << major << "." << minor << std::endl;
    }
    inline void initialiseEpoch()
    {
        struct timespec ts;

        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

        epochMilli = ts.tv_sec * 1000 + (ts.tv_nsec / 1000000L);
        epochMicro = ts.tv_sec * 1000000 + (ts.tv_nsec / 1000L);
    }
    inline void delay(const time_t howLong)
    {
        struct timespec sleeper, dummy;

        sleeper.tv_sec = (howLong / 1000);
        sleeper.tv_nsec = (howLong % 1000) * 1000000;

        nanosleep(&sleeper, &dummy);
    }
    inline void delayMicroseconds(const time_t howLong)
    {
        struct timespec sleeper;

        if (howLong == 0)
        {
            return;
        }
        else if (howLong < 100)
        {
            delayMicrosecondsHard(howLong);
        }
        else
        {
            sleeper.tv_sec = howLong / 1000000;
            sleeper.tv_nsec = (howLong % 1000000) * 1000L;
            nanosleep(&sleeper, NULL);
        }
    }
    inline void delayMicrosecondsHard(const time_t howLong)
    {
        struct timeval tNow, tLong, tEnd;

        gettimeofday(&tNow, NULL);

        tLong.tv_sec = howLong / 1000000;
        tLong.tv_usec = howLong % 1000000;

        timeradd(&tNow, &tLong, &tEnd);

        while (timercmp(&tNow, &tEnd, <))
        {
            gettimeofday(&tNow, NULL);
        }
    }
    inline time_t millis()
    {
        struct timespec ts;

        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

        return ((ts.tv_sec * 1000 + (ts.tv_nsec / 1000000)) - epochMilli);
    }
    inline time_t micros()
    {
        struct timespec ts;

        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

        return ((ts.tv_sec * 1000000 + (ts.tv_nsec / 1000)) - epochMicro);
    }

    // Data

    // Version data
    const int major;
    const int minor;

    // Timers
    time_t epochMilli, epochMicro;

    // Debugging & Return codes
    int wiringPiDebug = 0;
    int wiringPiReturnCodes = 0;
    long int piGpioBase = 0;
    int piGpioPupOffset = 0;
    int wiringPiMode = WPI_MODE_UNINITIALISED;
    uint32_t *pinToGpio;
    uint32_t *physToGpio;
    unsigned int usingGpioMem = 0;

    long int GPIO_PADS;
    long int GPIO_CLOCK_BASE;
    long int GPIO_BASE;
    long int GPIO_TIMER;
    long int GPIO_PWM;
    bool wiringPiSetuped = false;

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
        "Sony",      //	00
        "Egoman",    //	01
        "Embest",    //	02
        "Unknown",   //	03
        "Embest",    //	04
        "Unknown05", //	05
        "Unknown06", //	06
        "Unknown07", //	07
        "Unknown08", //	08
        "Unknown09", //	09
        "Unknown10", //	10
        "Unknown11", //	11
        "Unknown12", //	12
        "Unknown13", //	13
        "Unknown14", //	14
        "Unknown15", //	15
    };

    const int piMemorySize[8] = {
        256,  // 0
        512,  // 1
        1024, // 2
        2048, // 3
        4096, // 4
        8192, // 5
        0,    // 6
        0,    // 7
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

    const uint32_t pinToGpioR1[64] = {
        17, 18, 21, 22, 23, 24, 25, 4,                                                          //
        0, 1, 8, 7, 10, 9, 11, 14,                                                              //
        15, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN,        //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN  //
    };

    const uint32_t pinToGpioR2[64] = {
        17, 18, 27, 22, 23, 24, 25, 4,                                                          //
        2, 3, 8, 7, 10, 9, 11, 14,                                                              //
        15, 28, 29, 30, 31, 5, 6, 13,                                                           //
        19, 26, 12, 16, 20, 21, 0, 1,                                                           //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN  //
    };

    const uint32_t physToGpioR1[64] = {
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, 0, EMPTY_PIN, 1, EMPTY_PIN, 4,                         //
        14, EMPTY_PIN, 15, 17, 18, 21, EMPTY_PIN, 22,                                           //
        23, EMPTY_PIN, 24, 10, EMPTY_PIN, 9, 25, 11,                                            //
        8, EMPTY_PIN, 7, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN,                 //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN  //
    };

    const uint32_t physToGpioR2[64] = {
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, 2, EMPTY_PIN, 3, EMPTY_PIN, 4,                        //
        14, EMPTY_PIN, 15, 17, 18, 27, EMPTY_PIN, 22,                                          //
        23, EMPTY_PIN, 24, 10, EMPTY_PIN, 9, 25, 11,                                           //
        8, EMPTY_PIN, 7, 0, 1, 5, EMPTY_PIN, 6,                                                //
        12, 13, EMPTY_PIN, 19, 16, 26, 20, EMPTY_PIN,                                          //
        21, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN,       //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, 28, 29, 30, 31, EMPTY_PIN,                            //
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN //
    };

    const uint32_t gpioToGPFSEL[60] = {
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

    const uint32_t gpioToShift[60] = {
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

    const uint32_t gpioToClkDiv[64] = {
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, 29, 31, 33, EMPTY_PIN,                      //  0 ->  7
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, //  8 -> 15
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, 29, 31, EMPTY_PIN, EMPTY_PIN,               // 16 -> 23
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, // 24 -> 31
        29, EMPTY_PIN, 29, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN,               // 32 -> 39
        EMPTY_PIN, EMPTY_PIN, 29, 31, 29, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN,                      // 40 -> 47
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, // 48 -> 55
        EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, EMPTY_PIN, // 56 -> 63
    };
};

#endif