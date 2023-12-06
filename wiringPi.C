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
        pinToGpio = const_cast<int *>(pinToGpioR1);
        physToGpio = const_cast<int *>(physToGpioR1);
    }
    else // A2, B2, A+, B+, CM, Pi2, Pi3, Zero, Zero W, Zero 2 W
    {
        pinToGpio = const_cast<int *>(pinToGpioR2);
        physToGpio = const_cast<int *>(physToGpioR2);
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
        pinToGpio = const_cast<int *>(pinToGpioR1);
        physToGpio = const_cast<int *>(physToGpioR1);
    }
    else
    {
        pinToGpio = const_cast<int *>(pinToGpioR2);
        physToGpio = const_cast<int *>(physToGpioR2);
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

int main()
{
    wiringPi wiringObject(0);

    // const int i = wiringObject.gpioLayout();

    const int i = wiringObject.setup();

    // std::cout << "Model = " << wiringObject.piModelNames[wiringObject.model] << std::endl;
    // std::cout << "Rev = " << wiringObject.piRevisionNames[rev] << std::endl;
    // std::cout << "Mem = " << wiringObject.piMemorySize[mem] << std::endl;
    // std::cout << "Maker = " << wiringObject.piMakerNames[maker] << std::endl;
    // std::cout << "Overvolted = " << overVolted << std::endl;

    std::cout << "i = " << i << std::endl;

    wiringObject.delay(1000);

    return 0;
}