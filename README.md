# i2c-servo-driver

C++ library to control servo motors over i2c. Specificly developer to work on Jetson Nano

### Requires i2c-tools to set up:

```shell
    sudo apt-get install i2c-tools
```

The to verify that the I2C device is connected `i2cdetect -y -r <bus>`. Here the I2C bus 1 on the device

```shell
i2cdetect -y -r 1
```

Which results in, and it can be seen that it is at hex address `40`:
```shell
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: 40 -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: 70 -- -- -- -- -- -- --   
```



```c++
void SetServoPosition(int fd, int channel, float pulseWidthMs) {
  float pulseLength = 1000000;   // 1,000,000 us per second
  pulseLength /= 50;             // 50 Hz
  pulseLength /= 4096;           // 12 bits of resolution
  int pulse = int(pulseWidthMs * 1000 / pulseLength); // Convert to pulse width in units of ticks
  SetPWM(fd, channel, 0, pulse);
}
```



### Just in case

```c++
//
// Created by JS-Robotics on 24.03.24.
//

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cmath>


// PCA9685 registers
#define MODE1 0x00
#define PRESCALE 0xFE
#define LED0_ON_L 0x06 // Base address for servo 0

// Function to initialize PCA9685
bool InitPCA9685(int fd) {
  unsigned char reset[] = {MODE1, 0x00};
  if (write(fd, reset, sizeof(reset)) != sizeof(reset)) {
    perror("Failed to reset PCA9685");
    return false;
  }

  // Enter sleep mode before setting prescaler
  unsigned char sleep[] = {MODE1, 0x10};
  if (write(fd, sleep, sizeof(sleep)) != sizeof(sleep)) {
    perror("Failed to enter sleep mode");
    return false;
  }

  float prescale_val = 25000000.0f; // 25MHz
  prescale_val /= 4096.0; // 12-bit
  prescale_val /= 50.0; // frequency
  prescale_val -= 1.0;
  unsigned char prescale_calc = static_cast<unsigned char>(floor(prescale_val + 0.5));
  unsigned char prescale[] = {PRESCALE, prescale_calc};
  if (write(fd, prescale, sizeof(prescale)) != sizeof(prescale)) {
    perror("Failed to set prescaler");
    return false;
  }

  // Restart
  unsigned char restart[] = {MODE1, 0x80};
  if (write(fd, restart, sizeof(restart)) != sizeof(restart)) {
    perror("Failed to restart");
    return false;
  }

  // Wait for oscillator
  usleep(10000);

  // Set to auto increment mode
  unsigned char auto_inc[] = {MODE1, 0xa1};
  if (write(fd, auto_inc, sizeof(auto_inc)) != sizeof(auto_inc)) {
    perror("Failed to set auto increment mode");
    return false;
  }

  return true;
}

// Function to set PWM for a specific channel
void SetPWM(int fd, int channel, int on, int off) {
  int reg = LED0_ON_L + 4 * channel;
  unsigned char data[5];
  data[0] = reg;
  data[1] = on & 0xFF;
  data[2] = (on >> 8) & 0xFF;
  data[3] = off & 0xFF;
  data[4] = (off >> 8) & 0xFF;
  if (write(fd, data, sizeof(data)) != sizeof(data)) {
    perror("Failed to set PWM");
  }
}


int main() {
  const char *device = "/dev/i2c-1";
  int addr = 0x40; // The I2C address of the PCA9685

  std::cout << "Connecting to " << device << std::endl;
  int fd = open(device, O_RDWR);
  if (fd < 0) {
    perror("Failed to open device");
    return EXIT_FAILURE;
  }

  if (ioctl(fd, I2C_SLAVE, addr) < 0) {
    perror("Failed to select device");
    close(fd);
    return EXIT_FAILURE;
  }

  if (!InitPCA9685(fd)) {
    close(fd);
    return EXIT_FAILURE;
  }

  SetPWM(fd, 3, 0, 600);

  close(fd);
  return EXIT_SUCCESS;
}
```