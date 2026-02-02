#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "coap.h"

#define GPIO0_BASE  0x44E07000
#define GPIO1_BASE  0x4804C000
#define GPIO2_BASE  0x481AC000
#define GPIO3_BASE  0x481AE000
#define GPIO_SIZE   0x1000

#define GPIO_OE     0x134
#define GPIO_DATAIN 0x138
#define GPIO_DATAOUT 0x13C
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

#define GPIO_BANK(gpio) ((gpio) / 32)
#define GPIO_BIT(gpio)  ((gpio) % 32)

volatile unsigned *gpio_base;
int gpio_pin, gpio_fd;

// Function prototypes
void setup_gpio(int pin);
void set_gpio_output(int pin);
void set_gpio_input(int pin);
void gpio_write(int pin, int value);
int gpio_read(int pin);
unsigned long micros();
bool readDHT11(int pin, uint8_t *humidity_int, uint8_t *humidity_dec, uint8_t *temp_int, uint8_t *temp_dec);

// Function to initialize GPIO
void setup_gpio(int pin) 
{
    gpio_pin = pin;
    int bank = GPIO_BANK(pin);
    unsigned int gpio_addr;

    switch (bank) 
{
        case 0: gpio_addr = GPIO0_BASE; break;
        case 1: gpio_addr = GPIO1_BASE; break;
        case 2: gpio_addr = GPIO2_BASE; break;
        case 3: gpio_addr = GPIO3_BASE; break;
	default:
            fprintf(stderr, "Invalid GPIO pin\n");
            exit(EXIT_FAILURE);
    }

    gpio_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (gpio_fd < 0) 
	{
        perror("open");
        exit(EXIT_FAILURE);
   	 }

    gpio_base = (volatile unsigned *)mmap(NULL, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gpio_fd, gpio_addr);
    if (gpio_base == MAP_FAILED) 
	{
        perror("mmap");
        exit(EXIT_FAILURE);
    }
}

// Set GPIO as output
void set_gpio_output(int pin) 
{
    *(gpio_base + (GPIO_OE / 4)) &= ~(1 << GPIO_BIT(pin));
}

// Set GPIO as input
void set_gpio_input(int pin) 
{
    *(gpio_base + (GPIO_OE / 4)) |= (1 << GPIO_BIT(pin));
}

// Write value to GPIO
void gpio_write(int pin, int value) 
{
    if (value)
        *(gpio_base + (GPIO_SETDATAOUT / 4)) = (1 << GPIO_BIT(pin));
    else
        *(gpio_base + (GPIO_CLEARDATAOUT / 4)) = (1 << GPIO_BIT(pin));
}

// Read value from GPIO
int gpio_read(int pin) 
{
    return (*(gpio_base + (GPIO_DATAIN / 4)) & (1 << GPIO_BIT(pin))) ? 1 : 0;

}

// Get time in microseconds
unsigned long micros() 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000UL + ts.tv_nsec / 1000;
}

// Function to read DHT11 sensor
bool readDHT11(int pin, uint8_t *humidity_int, uint8_t *humidity_dec, uint8_t *temp_int, uint8_t *temp_dec) 
{
    uint8_t data[5] = {0, 0, 0, 0, 0};
    unsigned long t;
	int i,j;
    // Step 1: Send start signal to DHT11
    // TODO: Set GPIO as output and pull it low for 18ms
 	set_gpio_output(pin);
	gpio_write(pin,0);
       	usleep(18000);
	gpio_write(pin,1);
 	usleep(40);	
    // Step 2: Set GPIO as input and wait for DHT response
    // TODO: Implement logic to detect DHT11 response signal
	set_gpio_input(pin);

    // Step 3: Read 40 bits (5 bytes) from the DHT sensor
    // TODO: Implement bit reading logic based on timing constraints
 	 t = micros();
    while (!(gpio_read(pin)));
        if ((micros() - t) > 110) 
	return false;

    t = micros();
    while (gpio_read(pin));
        if ((micros() - t) > 110) 
	return false;

    // Step 4: Validate checksum
    // TODO: Implement checksum validation
     for (i = 0; i < 5; i++) 
     {
        for (j = 7; j >= 0; j--) 
	{
		t=micros();
            while (!(gpio_read(pin)));   // wait HIGH
            t = micros()-t;

            if (t > 60)
		return false;
		t=micros();
		while(gpio_read(pin));
		t=micros()-t;
		if(t > 40)
                data[i] |= (1 << j);
		else
		data[i] &= ~(1<<j);
        }
    }

    // Step 5: Store temperature and humidity values
    // TODO: Assign values to humidity_int, humidity_dec, temp_int, temp_dec
    if ((uint8_t)(data[0] + data[1] +data[2] + data[3]) != data[4])
        return false;

    *humidity_int = data[0];
    *humidity_dec = data[1];
    *temp_int     = data[2];
    *temp_dec     = data[3];
    return true;  // Return true if data is successfully read
}

int fun() 
{
    int dht_pin = 45; // Change to actual BBB GPIO pin
    printf("DHT11 Sensor Reading on BeagleBone Black\n");
    setup_gpio(dht_pin);

    while (1) {
//       uint8_t hum_int, hum_dec, temp_int, temp_dec;
        if (readDHT11(dht_pin, &hum_int, &hum_dec, &temp_int, &temp_dec)) 
	{
            printf("Humidity: %d.%d %%\tTemperature: %d.%d \u00B0C\n", hum_int, hum_dec, temp_int, temp_dec);
        } 
	else {
           printf("Failed to read from DHT11 sensor.\n");
        }
        sleep(2);
	break;
    }
    return 0;
}
