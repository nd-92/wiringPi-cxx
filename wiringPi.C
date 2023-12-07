#include "wiringPi.H"

int wiringPi::failure(int fatal, const char *message, ...)
{
    if (!fatal && wiringPiReturnCodes)
    {
        return -1;
    }

    va_list argp;
    char buffer[1024];

    __builtin_va_start(argp, message);
    vsnprintf(buffer, 1023, message, argp);
    __builtin_va_end(argp);

    fprintf(stderr, "%s", buffer);
    exit(EXIT_FAILURE);

    return 0;
}

void wiringPi::gpioLayoutOops(const char *why)
{
    fprintf(stderr, "Oops: Unable to determine board revision from /proc/cpuinfo\n");
    fprintf(stderr, " -> %s\n", why);
    fprintf(stderr, " ->  You'd best google the error to find out why.\n");
    // fprintf (stderr, " ->  http://www.raspberrypi.org/phpBB3/viewtopic.php?p=184410#p184410\n") ;
    exit(EXIT_FAILURE);
}

int wiringPi::gpioLayout()
{
    FILE *cpuFd;
    char line[120];
    char *c;
    int gpioLayout = -1;

    if (gpioLayout != -1) // No point checking twice
    {
        return gpioLayout;
    }

    if ((cpuFd = fopen("/proc/cpuinfo", "r")) == NULL)
    {
        gpioLayoutOops("Unable to open /proc/cpuinfo");
    }

    // Start by looking for the Architecture to make sure we're really running
    // on a Pi. I'm getting fed-up with people whinging at me because
    // they can't get it to work on weirdFruitPi boards...

    while (fgets(line, 120, cpuFd) != NULL)
    {
        if (strncmp(line, "Hardware", 8) == 0)
        {
            break;
        }
    }

    if (strncmp(line, "Hardware", 8) != 0)
    {
        gpioLayoutOops("No \"Hardware\" line");
    }

    if (wiringPiDebug)
    {
        // printf("gpioLayout: Hardware: %s\n", line);
        std::cout << "gpioLayout: Hardware: " << line << std::endl;
    }

    // Isolate the Revision line

    rewind(cpuFd);
    while (fgets(line, 120, cpuFd) != NULL)
    {
        if (strncmp(line, "Revision", 8) == 0)
        {
            break;
        }
    }

    fclose(cpuFd);

    if (strncmp(line, "Revision", 8) != 0)
    {
        gpioLayoutOops("No \"Revision\" line");
    }

    // Chomp trailing CR/NL

    for (c = &line[strlen(line) - 1]; (*c == '\n') || (*c == '\r'); --c)
    {
        *c = 0;
    }

    if (wiringPiDebug)
    {
        // printf("gpioLayout: Revision string: %s\n", line);
        std::cout << "gpioLayout: Revision string: " << line << std::endl;
    }

    // Scan to the first character of the revision number

    for (c = line; *c; ++c)
    {
        if (*c == ':')
        {
            break;
        }
    }

    if (*c != ':')
    {
        gpioLayoutOops("Bogus \"Revision\" line (no colon)");
    }

    // Chomp spaces

    ++c;
    while (isspace(*c))
    {
        ++c;
    }

    if (!isxdigit(*c))
    {
        gpioLayoutOops("Bogus \"Revision\" line (no hex digit at start of revision)");
    }

    // Make sure its long enough

    if (strlen(c) < 4)
    {
        gpioLayoutOops("Bogus revision line (too small)");
    }

    // Isolate  last 4 characters: (in-case of overvolting or new encoding scheme)

    c = c + strlen(c) - 4;

    if (wiringPiDebug)
    {
        // printf("gpioLayout: last4Chars are: \"%s\"\n", c);
        std::cout << "gpioLayout: last4Chars are: " << c << std::endl;
    }

    if ((strcmp(c, "0002") == 0) || (strcmp(c, "0003") == 0))
    {
        gpioLayout = 1;
    }
    else
    {
        gpioLayout = 2;
    } // Covers everything else from the B revision 2 to the B+, the Pi v2, v3, zero and CM's.

    if (wiringPiDebug)
    {
        // printf("gpioLayoutOops: Returning revision: %d\n", gpioLayout);
        std::cout << "gpioLayoutOops: Returning revision: " << gpioLayout << std::endl;
    }

    return gpioLayout;
}

void wiringPi::boardID(int *model, int *rev, int *mem, int *maker, int *warranty)
{
    FILE *cpuFd;
    char line[120];
    char *c;
    unsigned int revision;
    int bRev, bType, bProc, bMfg, bMem, bWarranty;

    // Call this first to make sure all's OK. Don't care about the result.
    static_cast<void>(gpioLayout());

    if ((cpuFd = fopen("/proc/cpuinfo", "r")) == NULL)
    {
        gpioLayoutOops("Unable to open /proc/cpuinfo");
    }

    while (fgets(line, 120, cpuFd) != NULL)
    {
        if (strncmp(line, "Revision", 8) == 0)
        {
            break;
        }
    }

    fclose(cpuFd);

    if (strncmp(line, "Revision", 8) != 0)
    {
        gpioLayoutOops("No \"Revision\" line");
    }

    // Chomp trailing CR/NL

    for (c = &line[strlen(line) - 1]; (*c == '\n') || (*c == '\r'); --c)
    {
        *c = 0;
    }

    if (wiringPiDebug)
    {
        printf("piBoardId: Revision string: %s\n", line);
    }

    // Need to work out if it's using the new or old encoding scheme:

    // Scan to the first character of the revision number

    for (c = line; *c; ++c)
    {
        if (*c == ':')
        {
            break;
        }
    }

    if (*c != ':')
    {
        gpioLayoutOops("Bogus \"Revision\" line (no colon)");
    }

    // Chomp spaces

    ++c;
    while (isspace(*c))
    {
        ++c;
    }

    if (!isxdigit(*c))
    {
        gpioLayoutOops("Bogus \"Revision\" line (no hex digit at start of revision)");
    }

    revision = (unsigned int)strtol(c, NULL, 16); // Hex number with no leading 0x

    // Check for new way:

    if ((revision & (1 << 23)) != 0) // New way
    {
        if (wiringPiDebug)
        {
            printf("piBoardId: New Way: revision is: %08X\n", revision);
        }

        bRev = (revision & (0x0F << 0)) >> 0;
        bType = (revision & (0xFF << 4)) >> 4;
        bProc = (revision & (0x0F << 12)) >> 12; // Not used for now.
        bMfg = (revision & (0x0F << 16)) >> 16;
        bMem = (revision & (0x07 << 20)) >> 20;
        bWarranty = (revision & (0x03 << 24)) != 0;

        *model = bType;
        *rev = bRev;
        *mem = bMem;
        *maker = bMfg;
        *warranty = bWarranty;

        if (wiringPiDebug)
        {
            printf(
                "piBoardId: rev: %d, type: %d, proc: %d, mfg: %d, mem: %d, warranty: %d\n",
                bRev, bType, bProc, bMfg, bMem, bWarranty);
        }
    }
    else // Old way
    {
        if (wiringPiDebug)
        {
            printf("piBoardId: Old Way: revision is: %s\n", c);
        }

        if (!isdigit(*c))
        {
            gpioLayoutOops("Bogus \"Revision\" line (no digit at start of revision)");
        }

        // Make sure its long enough

        if (strlen(c) < 4)
        {
            gpioLayoutOops("Bogus \"Revision\" line (not long enough)");
        }

        // If longer than 4, we'll assume it's been overvolted

        *warranty = strlen(c) > 4;

        // Extract last 4 characters:

        c = c + strlen(c) - 4;

        // Fill out the replys as appropriate

        /**/ if (strcmp(c, "0002") == 0)
        {
            *model = PI_MODEL_B;
            *rev = PI_VERSION_1;
            *mem = 0;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "0003") == 0)
        {
            *model = PI_MODEL_B;
            *rev = PI_VERSION_1_1;
            *mem = 0;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "0004") == 0)
        {
            *model = PI_MODEL_B;
            *rev = PI_VERSION_1_2;
            *mem = 0;
            *maker = PI_MAKER_SONY;
        }
        else if (strcmp(c, "0005") == 0)
        {
            *model = PI_MODEL_B;
            *rev = PI_VERSION_1_2;
            *mem = 0;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "0006") == 0)
        {
            *model = PI_MODEL_B;
            *rev = PI_VERSION_1_2;
            *mem = 0;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "0007") == 0)
        {
            *model = PI_MODEL_A;
            *rev = PI_VERSION_1_2;
            *mem = 0;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "0008") == 0)
        {
            *model = PI_MODEL_A;
            *rev = PI_VERSION_1_2;
            *mem = 0;
            *maker = PI_MAKER_SONY;
            ;
        }
        else if (strcmp(c, "0009") == 0)
        {
            *model = PI_MODEL_A;
            *rev = PI_VERSION_1_2;
            *mem = 0;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "000d") == 0)
        {
            *model = PI_MODEL_B;
            *rev = PI_VERSION_1_2;
            *mem = 1;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "000e") == 0)
        {
            *model = PI_MODEL_B;
            *rev = PI_VERSION_1_2;
            *mem = 1;
            *maker = PI_MAKER_SONY;
        }
        else if (strcmp(c, "000f") == 0)
        {
            *model = PI_MODEL_B;
            *rev = PI_VERSION_1_2;
            *mem = 1;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "0010") == 0)
        {
            *model = PI_MODEL_BP;
            *rev = PI_VERSION_1_2;
            *mem = 1;
            *maker = PI_MAKER_SONY;
        }
        else if (strcmp(c, "0013") == 0)
        {
            *model = PI_MODEL_BP;
            *rev = PI_VERSION_1_2;
            *mem = 1;
            *maker = PI_MAKER_EMBEST;
        }
        else if (strcmp(c, "0016") == 0)
        {
            *model = PI_MODEL_BP;
            *rev = PI_VERSION_1_2;
            *mem = 1;
            *maker = PI_MAKER_SONY;
        }
        else if (strcmp(c, "0019") == 0)
        {
            *model = PI_MODEL_BP;
            *rev = PI_VERSION_1_2;
            *mem = 1;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "0011") == 0)
        {
            *model = PI_MODEL_CM;
            *rev = PI_VERSION_1_1;
            *mem = 1;
            *maker = PI_MAKER_SONY;
        }
        else if (strcmp(c, "0014") == 0)
        {
            *model = PI_MODEL_CM;
            *rev = PI_VERSION_1_1;
            *mem = 1;
            *maker = PI_MAKER_EMBEST;
        }
        else if (strcmp(c, "0017") == 0)
        {
            *model = PI_MODEL_CM;
            *rev = PI_VERSION_1_1;
            *mem = 1;
            *maker = PI_MAKER_SONY;
        }
        else if (strcmp(c, "001a") == 0)
        {
            *model = PI_MODEL_CM;
            *rev = PI_VERSION_1_1;
            *mem = 1;
            *maker = PI_MAKER_EGOMAN;
        }
        else if (strcmp(c, "0012") == 0)
        {
            *model = PI_MODEL_AP;
            *rev = PI_VERSION_1_1;
            *mem = 0;
            *maker = PI_MAKER_SONY;
        }
        else if (strcmp(c, "0015") == 0)
        {
            *model = PI_MODEL_AP;
            *rev = PI_VERSION_1_1;
            *mem = 1;
            *maker = PI_MAKER_EMBEST;
        }
        else if (strcmp(c, "0018") == 0)
        {
            *model = PI_MODEL_AP;
            *rev = PI_VERSION_1_1;
            *mem = 0;
            *maker = PI_MAKER_SONY;
        }
        else if (strcmp(c, "001b") == 0)
        {
            *model = PI_MODEL_AP;
            *rev = PI_VERSION_1_1;
            *mem = 0;
            *maker = PI_MAKER_EGOMAN;
        }
        else
        {
            *model = 0;
            *rev = 0;
            *mem = 0;
            *maker = 0;
        }
    }
}

int wiringPi::setup()
{
    int fd;
    int model, rev, mem, maker, overVolted;

    if (wiringPiSetuped)
    {
        return 0;
    }

    wiringPiSetuped = 1;

    if (getenv(ENV_DEBUG) != NULL)
    {
        wiringPiDebug = 1;
    }

    if (getenv(ENV_CODES) != NULL)
    {
        wiringPiReturnCodes = 1;
    }

    if (wiringPiDebug)
    {
        // printf("wiringPi: wiringPiSetup called\n");
        std::cout << "wiringPi: wiringPiSetup called" << std::endl;
    }

    // Get the board ID information. We're not really using the information here,
    // but it will give us information like the GPIO layout scheme (2 variants
    // on the older 26-pin Pi's) and the GPIO peripheral base address.
    // and if we're running on a compute module, then wiringPi pin numbers
    // don't really mean anything, so force native BCM mode anyway.

    boardID(&model, &rev, &mem, &maker, &overVolted);

    if ((model == PI_MODEL_CM) || (model == PI_MODEL_CM3) || (model == PI_MODEL_CM3P))
    {
        wiringPiMode = WPI_MODE_GPIO;
    }
    else
    {
        wiringPiMode = WPI_MODE_PINS;
    }

    /**/ if (gpioLayout() == 1) // A, B, Rev 1, 1.1
    {
        pinToGpio = const_cast<uint32_t *>(pinToGpioR1);
        physToGpio = const_cast<uint32_t *>(physToGpioR1);
    }
    else // A2, B2, A+, B+, CM, Pi2, Pi3, Zero, Zero W, Zero 2 W
    {
        pinToGpio = const_cast<uint32_t *>(pinToGpioR2);
        physToGpio = const_cast<uint32_t *>(physToGpioR2);
    }

    // ...

    switch (model)
    {
    case PI_MODEL_A:
    case PI_MODEL_B:
    case PI_MODEL_AP:
    case PI_MODEL_BP:
    case PI_ALPHA:
    case PI_MODEL_CM:
    case PI_MODEL_ZERO:
    case PI_MODEL_ZERO_W:
        piGpioBase = GPIO_PERI_BASE_OLD;
        piGpioPupOffset = GPPUD;
        break;
    case PI_MODEL_4B:
    case PI_MODEL_400:
    case PI_MODEL_CM4:
        piGpioBase = GPIO_PERI_BASE_2711;
        piGpioPupOffset = GPPUPPDN0;
        break;
    default:
        piGpioBase = GPIO_PERI_BASE_2835;
        piGpioPupOffset = GPPUD;
        break;
    }

    // Open the master /dev/ memory control device
    // Device strategy: December 2016:
    // Try /dev/mem. If that fails, then
    // try /dev/gpiomem. If that fails then game over.

    if ((fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0)
    {
        if ((fd = open("/dev/gpiomem", O_RDWR | O_SYNC | O_CLOEXEC)) >= 0) // We're using gpiomem
        {
            piGpioBase = 0;
            usingGpioMem = 1;
        }
        else
        {
            return failure(
                WPI_ALMOST,
                "wiringPiSetup: Unable to open /dev/mem or /dev/gpiomem: %s.\n"
                "  Aborting your program because if it can not access the GPIO\n"
                "  hardware then it most certianly won't work\n"
                "  Try running with sudo?\n",
                strerror(errno));
        }
    }

    // Set the offsets into the memory interface.

    GPIO_PADS = piGpioBase + 0x00100000;
    GPIO_CLOCK_BASE = piGpioBase + 0x00101000;
    GPIO_BASE = piGpioBase + 0x00200000;
    GPIO_TIMER = piGpioBase + 0x0000B000;
    GPIO_PWM = piGpioBase + 0x0020C000;

    // Map the individual hardware components

    //	GPIO:

    gpio = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);
    if (gpio == MAP_FAILED)
    {
        return failure(WPI_ALMOST, "wiringPiSetup: mmap (GPIO) failed: %s\n", strerror(errno));
    }

    //	PWM

    pwm = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_PWM);
    if (pwm == MAP_FAILED)
    {
        return failure(WPI_ALMOST, "wiringPiSetup: mmap (PWM) failed: %s\n", strerror(errno));
    }

    //	Clock control (needed for PWM)

    clk = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_CLOCK_BASE);
    if (clk == MAP_FAILED)
    {
        return failure(WPI_ALMOST, "wiringPiSetup: mmap (CLOCK) failed: %s\n", strerror(errno));
    }

    //	The drive pads

    pads = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_PADS);
    if (pads == MAP_FAILED)
    {
        return failure(WPI_ALMOST, "wiringPiSetup: mmap (PADS) failed: %s\n", strerror(errno));
    }

    //	The system timer

    timer = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_TIMER);
    if (timer == MAP_FAILED)
    {
        return failure(WPI_ALMOST, "wiringPiSetup: mmap (TIMER) failed: %s\n", strerror(errno));
    }

    // Set the timer to free-running, 1MHz.
    //	0xF9 is 249, the timer divide is base clock / (divide+1)
    //	so base clock is 250MHz / 250 = 1MHz.

    *(timer + TIMER_CONTROL) = 0x0000280;
    *(timer + TIMER_PRE_DIV) = 0x00000F9;
    timerIrqRaw = timer + TIMER_IRQ_RAW;

    // Export the base addresses for any external software that might need them

    _wiringPiGpio = gpio;
    _wiringPiPwm = pwm;
    _wiringPiClk = clk;
    _wiringPiPads = pads;
    _wiringPiTimer = timer;

    initialiseEpoch();

    return 0;
}

void wiringPi::initialiseEpoch()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    epochMilli = ts.tv_sec * 1000 + (ts.tv_nsec / 1000000L);
    epochMicro = ts.tv_sec * 1000000 + (ts.tv_nsec / 1000L);
}

void wiringPi::delay(const time_t howLong)
{
    struct timespec sleeper, dummy;

    sleeper.tv_sec = (howLong / 1000);
    sleeper.tv_nsec = (howLong % 1000) * 1000000;

    nanosleep(&sleeper, &dummy);
}

void wiringPi::delayMicroseconds(const time_t howLong)
{
    struct timespec sleeper;
    const time_t uSecs = howLong % 1000000;
    const time_t wSecs = howLong / 1000000;

    /**/ if (howLong == 0)
    {
        return;
    }
    else if (howLong < 100)
    {
        delayMicrosecondsHard(howLong);
    }
    else
    {
        sleeper.tv_sec = wSecs;
        sleeper.tv_nsec = uSecs * 1000L;
        nanosleep(&sleeper, NULL);
    }
}

void wiringPi::delayMicrosecondsHard(const time_t howLong)
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

time_t wiringPi::millis()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return ((ts.tv_sec * 1000 + (ts.tv_nsec / 1000000)) - epochMilli);
}

time_t wiringPi::micros()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return ((ts.tv_sec * 1000000 + (ts.tv_nsec / 1000)) - epochMicro);
}

int wiringPi::setupGpio()
{
    static_cast<void>(setup());

    if (wiringPiDebug)
    {
        std::cout << "wiringPi: wiringPiSetupGpio called" << std::endl;
    }

    wiringPiMode = WPI_MODE_GPIO;

    return 0;
}

int wiringPi::setupPhys()
{
    static_cast<void>(setup());

    if (wiringPiDebug)
    {
        std::cout << "wiringPi: wiringPiSetupPhys called" << std::endl;
    }

    wiringPiMode = WPI_MODE_PHYS;

    return 0;
}

int wiringPi::setupSys()
{
    int pin;
    char fName[128];

    if (wiringPiSetuped)
    {
        return 0;
    }

    wiringPiSetuped = true;

    if (getenv(ENV_DEBUG) != NULL)
    {
        wiringPiDebug = true;
    }

    if (getenv(ENV_CODES) != NULL)
    {
        wiringPiReturnCodes = true;
    }

    if (wiringPiDebug)
    {
        printf("wiringPi: wiringPiSetupSys called\n");
    }

    if (gpioLayout() == 1)
    {
        pinToGpio = const_cast<uint32_t *>(pinToGpioR1);
        physToGpio = const_cast<uint32_t *>(physToGpioR1);
    }
    else
    {
        pinToGpio = const_cast<uint32_t *>(pinToGpioR2);
        physToGpio = const_cast<uint32_t *>(physToGpioR2);
    }

    // Open and scan the directory, looking for exported GPIOs, and pre-open
    // the 'value' interface to speed things up for later

    for (pin = 0; pin < 64; ++pin)
    {
        sprintf(fName, "/sys/class/gpio/gpio%d/value", pin);
        sysFds[pin] = open(fName, O_RDWR);
    }

    initialiseEpoch();

    wiringPiMode = WPI_MODE_GPIO_SYS;

    return 0;
}

void wiringPi::setupCheck(const char *fName)
{
    if (!wiringPiSetuped)
    {
        fprintf(stderr, "%s: You have not called one of the wiringPiSetup\n"
                        "  functions, so I'm aborting your program before it crashes anyway.\n",
                fName);
        exit(EXIT_FAILURE);
    }
}

void wiringPi::pinModeAlt(uint32_t pin, const uint32_t mode)
{
    setupCheck("pinModeAlt");

    // Determine if 0 <= pin <= 63
    // This cast is believed safe
    if ((pin & static_cast<uint32_t>(PI_GPIO_MASK)) == 0) // On-board pin
    {
        /**/ if (wiringPiMode == WPI_MODE_PINS)
        {
            pin = pinToGpio[pin];
        }
        else if (wiringPiMode == WPI_MODE_PHYS)
        {
            pin = physToGpio[pin];
        }
        else if (wiringPiMode != WPI_MODE_GPIO)
        {
            return;
        }

        // These casts are safe since all values are <= 0
        const uint32_t fSel = static_cast<uint32_t>(gpioToGPFSEL[pin]);
        const uint32_t shift = static_cast<uint32_t>(gpioToShift[pin]);
        *(gpio + fSel) = (*(gpio + fSel) & ~(7U << shift)) | ((mode & 0x7) << shift);
    }
}

void wiringPi::setPadDrive(const uint32_t group, const uint32_t value)
{
    if ((wiringPiMode == WPI_MODE_PINS) || (wiringPiMode == WPI_MODE_PHYS) || (wiringPiMode == WPI_MODE_GPIO))
    {
        if (group > 2)
        {
            return;
        }

        const uint32_t wrVal = BCM_PASSWORD | 0x18 | (value & 7);
        *(pads + group + 11) = wrVal;

        if (wiringPiDebug)
        {
            printf("setPadDrive: Group: %d, value: %d (%08X)\n", group, value, wrVal);
            printf("Read : %08X\n", *(pads + group + 11));
        }
    }
}

int wiringPi::getAlt(uint32_t pin)
{
    pin &= 63;

    if (wiringPiMode == WPI_MODE_PINS)
    {
        pin = pinToGpio[pin];
    }
    else if (wiringPiMode == WPI_MODE_PHYS)
    {
        pin = physToGpio[pin];
    }
    else if (wiringPiMode != WPI_MODE_GPIO)
    {
        return 0;
    }

    return (*(gpio + gpioToGPFSEL[pin]) >> gpioToShift[pin]) & 7;
}

// void wiringPi::pwmToneWrite(int pin, int freq)
// {
//     int range;

//     setupCheck("pwmToneWrite");

//     if (freq == 0)
//         pwmWrite(pin, 0); // Off
//     else
//     {
//         range = 600000 / freq;
//         pwmSetRange(range);
//         pwmWrite(pin, freq / 2);
//     }
// }

void wiringPi::pwmSetMode(const int mode)
{
    if ((wiringPiMode == WPI_MODE_PINS) || (wiringPiMode == WPI_MODE_PHYS) || (wiringPiMode == WPI_MODE_GPIO))
    {
        if (mode == PWM_MODE_MS)
        {
            *(pwm + PWM_CONTROL) = PWM0_ENABLE | PWM1_ENABLE | PWM0_MS_MODE | PWM1_MS_MODE;
        }
        else
        {
            *(pwm + PWM_CONTROL) = PWM0_ENABLE | PWM1_ENABLE;
        }
    }
}

void wiringPi::pwmSetRange(const uint32_t range)
{
    if ((wiringPiMode == WPI_MODE_PINS) || (wiringPiMode == WPI_MODE_PHYS) || (wiringPiMode == WPI_MODE_GPIO))
    {
        *(pwm + PWM0_RANGE) = range;
        delayMicroseconds(10);
        *(pwm + PWM1_RANGE) = range;
        delayMicroseconds(10);
    }
}

void wiringPi::pwmSetClock(uint32_t divisor)
{
    if (piGpioBase == GPIO_PERI_BASE_2711)
    {
        divisor = 540 * divisor / 192;
    }
    divisor &= 4095;

    if ((wiringPiMode == WPI_MODE_PINS) || (wiringPiMode == WPI_MODE_PHYS) || (wiringPiMode == WPI_MODE_GPIO))
    {
        if (wiringPiDebug)
        {
            printf("Setting to: %d. Current: 0x%08X\n", divisor, *(clk + PWMCLK_DIV));
        }

        const uint32_t pwm_control = *(pwm + PWM_CONTROL); // preserve PWM_CONTROL

        // We need to stop PWM prior to stopping PWM clock in MS mode otherwise BUSY
        // stays high.

        *(pwm + PWM_CONTROL) = 0; // Stop PWM

        // Stop PWM clock before changing divisor. The delay after this does need to
        // this big (95uS occasionally fails, 100uS OK), it's almost as though the BUSY
        // flag is not working properly in balanced mode. Without the delay when DIV is
        // adjusted the clock sometimes switches to very slow, once slow further DIV
        // adjustments do nothing and it's difficult to get out of this mode.

        *(clk + PWMCLK_CNTL) = BCM_PASSWORD | 0x01; // Stop PWM Clock
        delayMicroseconds(110);                     // prevents clock going sloooow

        while ((*(clk + PWMCLK_CNTL) & 0x80) != 0) // Wait for clock to be !BUSY
        {
            delayMicroseconds(1);
        }

        *(clk + PWMCLK_DIV) = BCM_PASSWORD | (divisor << 12);

        *(clk + PWMCLK_CNTL) = BCM_PASSWORD | 0x11; // Start PWM clock
        *(pwm + PWM_CONTROL) = pwm_control;         // restore PWM_CONTROL

        if (wiringPiDebug)
        {
            printf("Set     to: %d. Now    : 0x%08X\n", divisor, *(clk + PWMCLK_DIV));
        }
    }
}

void wiringPi::gpioClockSet(uint32_t pin, const uint32_t freq)
{

    pin &= 63;

    /**/ if (wiringPiMode == WPI_MODE_PINS)
    {
        pin = pinToGpio[pin];
    }
    else if (wiringPiMode == WPI_MODE_PHYS)
    {
        pin = physToGpio[pin];
    }
    else if (wiringPiMode != WPI_MODE_GPIO)
    {
        return;
    }

    uint32_t divi = 19200000 / freq;
    const uint32_t divr = 19200000 % freq;
    const uint32_t divf = static_cast<uint32_t>(static_cast<double>(divr) * 4096.0 / 19200000.0);

    if (divi > 4095)
    {
        divi = 4095;
    }

    *(clk + gpioToClkCon[pin]) = BCM_PASSWORD | GPIO_CLOCK_SOURCE; // Stop GPIO Clock
    while ((*(clk + gpioToClkCon[pin]) & 0x80) != 0)               // ... and wait
    {
        ;
    }

    *(clk + gpioToClkDiv[pin]) = BCM_PASSWORD | (divi << 12) | divf;      // Set dividers
    *(clk + gpioToClkCon[pin]) = BCM_PASSWORD | 0x10 | GPIO_CLOCK_SOURCE; // Start Clock
}

int wiringPi::waitForInterrupt(uint32_t pin, int mS)
{
    /**/ if (wiringPiMode == WPI_MODE_PINS)
    {
        pin = pinToGpio[pin];
    }
    else if (wiringPiMode == WPI_MODE_PHYS)
    {
        pin = physToGpio[pin];
    }

    const int fd = sysFds[pin];
    if (fd == -1)
    {
        return -2;
    }

    // Setup poll structure

    struct pollfd polls;
    polls.fd = fd;
    polls.events = POLLPRI | POLLERR;

    // Wait for it ...

    const int x = poll(&polls, 1, mS);

    // If no error, do a dummy read to clear the interrupt
    //	A one character read appars to be enough.

    if (x > 0)
    {
        uint8_t c;
        static_cast<void>(lseek(fd, 0, SEEK_SET)); // Rewind
        static_cast<void>(read(fd, &c, 1));        // Read & clear
    }

    return x;
}

int main()
{
    wiringPi wiringObject(1);

    // const int i = wiringObject.gpioLayout();

    const int i = wiringObject.setup();
    std::cout << "i = " << i << std::endl;

    wiringObject.pinModeAlt(20, 6);

    return 0;
}