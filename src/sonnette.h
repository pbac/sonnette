#define VERSION "1.3.1"

#define DEBUG 

#ifdef DEBUG
#define isDebug     1
#define debug(x)    Serial.print(x)
#define debugln(x)  Serial.println(x)
#else
#define isDebug     0
#define debug(x)    {}
#define debugln(x)  {}
#endif


#define LED_BUILTIN       16
#define SONNETTE_PIN      4
#define POWERLED_PIN      14      //WS2812 led
#define EVENTLED_PIN      5



/*
#define PIN_WIRE_SDA (4)
#define PIN_WIRE_SCL (5)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

static const uint8_t LED_BUILTIN = 16;
static const uint8_t BUILTIN_LED = 16;

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
*/