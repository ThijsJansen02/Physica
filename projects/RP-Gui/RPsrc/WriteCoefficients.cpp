#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <time.h>

#include "rp.h"

#define GPIO_BASE  0x41200000

#define GPIO_SIZE  0x1000

#define GPIO_DATA1  0x00
#define GPIO_DATA2 0x08

#define GPIO_TRI1   0x04
#define GPIO_TRI2  0x0C

#define WRITE_PIN_OFFSET 1
#define ADRESS_PIN_OFFSET 8

#define WRITE_PIN_MASK  (1 << WRITE_PIN_OFFSET)
#define ADRESS_PINS_MASK (255 << ADRESS_PIN_OFFSET)

void print_binary(unsigned int n) {
    // Determine the number of bits in the integer
    int bits = sizeof(n) * 8;
    int started = 0;

    for (int i = bits - 1; i >= 0; i--) {
        // Shift bit to the 1st position and mask it
        int bit = (n >> i) & 1;
        
        if (bit) started = 1; // Skip leading zeros
        
        if (started) {
            printf("%d", bit);
        }
    }
    if (!started) printf("0"); // If the number is 0
    printf("\n");
}

void writebit(uint32_t position, uint32_t value, volatile uint32_t* target) {
    uint32_t mask = 1 << position;

    //clamp value to 1
    if(value > 1) {
        value = 1;
    }
    uint32_t insert = value << position;

    //clearvalue at position
    *target = *target & (~mask);

    *target = *target | insert;
}

void writebits(uint32_t offset, uint32_t value, uint32_t nbits, volatile uint32_t* target) {

    uint32_t max_val = ((1 << nbits) - 1);
    uint32_t mask = max_val << offset;
    
    if(value > max_val) {
        value = max_val;
    }

    uint32_t insert = value << offset; 

    *target = *target & (~mask);

    *target = *target | insert;
}

#define DEBUG_BUILD

void writeCoefficient(volatile uint32_t* gpio, uint32_t value, uint32_t adress, uint32_t timing_ns = 1000) {

    //write the needed data in the data bus
    gpio[GPIO_DATA2 / 4] = value;

    
    //write in the adress
    writebits(ADRESS_PIN_OFFSET, adress, 8, &gpio[GPIO_DATA1 / 4]);
    
    
#ifdef DEBUG_BUILD
    printf("data: ");
    printf("%u", gpio[GPIO_DATA2 / 4]);
    printf("write address: ");
    printf("%u", gpio[GPIO_DATA1 / 4]);
#endif

    //create timing struct
    timespec ts{};
    ts.tv_sec = 0;
    ts.tv_nsec = timing_ns;

    //timing result TODO: figure out if the wait actually happened
    timespec rem{};

    //sleep to make sure the FPGA has read the values in the GPIO buffer
    nanosleep(&ts, &rem);

    //set write pin
    writebit(WRITE_PIN_OFFSET, 1, &gpio[GPIO_DATA1 / 4]);

#ifdef DEBUG_BUILD
    printf("write enabled:");
    print_binary(gpio[GPIO_DATA1 / 4]);
#endif
    
    //sleep inbetween writes
    nanosleep(&ts, &rem);

    printf("hello world!");

    //reset the write pin
    writebit(WRITE_PIN_OFFSET, 0, &gpio[GPIO_DATA1 / 4]);

#ifdef DEBUG_BUILD
    printf("write disabled:");
    print_binary(gpio[GPIO_DATA1 / 4]);
#endif

    nanosleep(&ts, &rem);
}

struct BiquadCoefficients {
    uint32_t b0;
    uint32_t b1;
    uint32_t b2;
    uint32_t a1;
    uint32_t a2;
};

struct FilterFileLayout {
    BiquadCoefficients filters[4];
};



int main (int argc, char **argv) {

    //the adresses in memory where the filters are located
    int32_t fpga_filtercoeff_base_adress[4] = {0, 5, 10, 15};

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open /dev/mem");
        return 1;
    }

    FILE* filterfile = fopen("loadedcoeffs.rpbin", "rb");

    if(filterfile == NULL) {
        printf("failed to open coefficient file");
        return -1;
    }

    FilterFileLayout filters{};
    size_t nelements_read = fread(&filters, sizeof(FilterFileLayout), 1, filterfile);
    if(nelements_read < 1) {
        printf("coeff file is comprimised!");
        return -1;
    }

#ifdef DEBUG_BUILD
    printf("coeffs: %u, %u, %u, %u, %u", filters.filters[0].b0, filters.filters[0].b1, filters.filters[0].b2, filters.filters[0].a1, filters.filters[0].a2);
#endif

    unsigned int filter = 0;


    volatile uint32_t *gpio1 = (volatile uint32_t*)mmap(
        NULL, GPIO_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, GPIO_BASE
    );

     // Configure all GPIO bits as outputs
    gpio1[GPIO_TRI1 / 4] =  0x00000000;
    gpio1[GPIO_TRI2 / 4] = 0x00000000;
    

    //filter 1
    uint32_t adress = fpga_filtercoeff_base_adress[filter];
    writeCoefficient(gpio1, filters.filters[filter].b0, adress++);
    writeCoefficient(gpio1, filters.filters[filter].b1, adress++);
    writeCoefficient(gpio1, filters.filters[filter].b2, adress++);
    writeCoefficient(gpio1, filters.filters[filter].a1, adress++);
    writeCoefficient(gpio1, filters.filters[filter].a2, adress++);

    //filter 2
    filter++;
    adress = fpga_filtercoeff_base_adress[filter];
    writeCoefficient(gpio1, filters.filters[filter].b0, adress++);
    writeCoefficient(gpio1, filters.filters[filter].b1, adress++);
    writeCoefficient(gpio1, filters.filters[filter].b2, adress++);
    writeCoefficient(gpio1, filters.filters[filter].a1, adress++);
    writeCoefficient(gpio1, filters.filters[filter].a2, adress++);

    //filter 3
    filter++;
    adress = fpga_filtercoeff_base_adress[filter];
    writeCoefficient(gpio1, filters.filters[filter].b0, adress++);
    writeCoefficient(gpio1, filters.filters[filter].b1, adress++);
    writeCoefficient(gpio1, filters.filters[filter].b2, adress++);
    writeCoefficient(gpio1, filters.filters[filter].a1, adress++);
    writeCoefficient(gpio1, filters.filters[filter].a2, adress++);

    //filter 4
    filter++;
    adress = fpga_filtercoeff_base_adress[filter];
    writeCoefficient(gpio1, filters.filters[filter].b0, adress++);
    writeCoefficient(gpio1, filters.filters[filter].b1, adress++);
    writeCoefficient(gpio1, filters.filters[filter].b2, adress++);
    writeCoefficient(gpio1, filters.filters[filter].a1, adress++);
    writeCoefficient(gpio1, filters.filters[filter].a2, adress++);



    //toggle shadow pulse
    timespec ts{};
    ts.tv_sec = 0;
    ts.tv_nsec = 1000;

    writebit(2, 1, &gpio1[GPIO_DATA1 / 4]);
    nanosleep(ts);
    writebit(2, 0, &gpio1[GPIO_DATA1 / 4]);
    nanosleep(ts);
    return 0;
}