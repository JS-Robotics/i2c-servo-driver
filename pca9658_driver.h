//
// Created by sondre on 24.03.24.
//

#ifndef I2CSERVODRIVER__PCA9658_DRIVER_H_
#define I2CSERVODRIVER__PCA9658_DRIVER_H_

/*!
 * PCA9685 Datasheet: https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
 * */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cmath>

class PCA9658Driver {
 public:
  PCA9658Driver(std::string device, int address) : device_(device), address_(address), i2c_(-1) {

    if (!Connect()) {
      std::cerr << "Unable to Connect to device: " << device_ << std::endl;
    } else {
      if (!Init()) {
        std::cerr << "Unable to set up PCA9685 device" << std::endl;
      }
    }

  }

  ~PCA9658Driver() {
    close(i2c_);

  }

  void SetPWM(int channel, int on, int off) const {
    int reg = kLED0_ON_L + 4 * channel;
    unsigned char data[5];
    data[0] = reg;
    data[1] = on & 0xFF;
    data[2] = (on >> 8) & 0xFF;
    data[3] = off & 0xFF;
    data[4] = (off >> 8) & 0xFF;
    if (write(i2c_, data, sizeof(data)) != sizeof(data)) {
      perror("Failed to set PWM");
    }
  }

  void SetMS(int channel, float ms) const {
    auto period_ms = 1000.0/frequency_;  //Ticks per milliseconds
    auto ticks_per_ms = 4096 / period_ms;
    auto ticks = ms * ticks_per_ms;
    std::cout << "Setting motor on for :" << ticks << std::endl;
    SetPWM(channel, 0, static_cast<int>(ticks));
  }

 private:
  bool Connect() {
    std::cout << "Connecting to " << device_ << std::endl;
    i2c_ = open(device_.c_str(), O_RDWR);
    if (i2c_ < 0) {
      perror("Failed to open device");
      return false;
      return EXIT_FAILURE;
    }

    if (ioctl(i2c_, I2C_SLAVE, address_) < 0) {
      perror("Failed to select device");
//      close(fd);
      return false;
      return EXIT_FAILURE;
    }

    return true;
  }

  bool Init() {
    unsigned char reset[] = {kMODE1, 0x00};
    if (write(i2c_, reset, sizeof(reset)) != sizeof(reset)) {
      perror("Failed to reset PCA9685");
      return false;
    }

    // Enter sleep mode before setting prescaler
    unsigned char sleep[] = {kMODE1, 0x10};
    if (write(i2c_, sleep, sizeof(sleep)) != sizeof(sleep)) {
      perror("Failed to enter sleep mode");
      return false;
    }

    float prescale_val = 25000000.0f; // 25MHz
    prescale_val /= 4096.0; // 12-bit
    prescale_val /= frequency_; // frequency
    prescale_val -= 1.0;
    unsigned char prescale_calc = static_cast<unsigned char>(floor(prescale_val + 0.5));
    unsigned char prescale[] = {kPRE_SCALE, prescale_calc};
    if (write(i2c_, prescale, sizeof(prescale)) != sizeof(prescale)) {
      perror("Failed to set prescaler");
      return false;
    }

    // Restart
    unsigned char restart[] = {kMODE1, 0x80};
    if (write(i2c_, restart, sizeof(restart)) != sizeof(restart)) {
      perror("Failed to restart");
      return false;
    }

    // Wait for oscillator
    usleep(10000);

    // Set to auto increment mode
    unsigned char auto_inc[] = {kMODE1, 0xa1};
    if (write(i2c_, auto_inc, sizeof(auto_inc)) != sizeof(auto_inc)) {
      perror("Failed to set auto increment mode");
      return false;
    }

    return true;
  }

  int i2c_;
  const std::string device_;
  const int address_; // The I2C address of the PCA9685
  const float frequency_ = 50.0f;

  static constexpr int kMODE1 = 0x00;
  static constexpr int kPRE_SCALE = 0xFE;
  static constexpr int kLED0_ON_L = 0x06; // Base address for servo 0

};

#endif //I2CSERVODRIVER__PCA9658_DRIVER_H_
