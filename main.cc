//
// Created by JS-Robotics on 24.03.24.
//

#include "pca9658_driver.h"

int main() {
  PCA9658Driver driver("/dev/i2c-1", 0x40);
//  driver.SetPWM(3, 10, 120);
int channel = 0;
float max = 2.0;
float mid = 1.5;
float min = 1.0;

float angle_max = 1.57079633;
float angle_mid = angle_max /2;
float angle_min = 0;

//  driver.SetMS(channel, mid);
//  sleep(1);
//  driver.SetMS(channel, max);
//  sleep(1);
//  driver.SetMS(channel, mid);
//  sleep(1);
//  driver.SetMS(channel, min);
//  sleep(1);
//  driver.SetMS(channel, mid);
//  sleep(1);
//  driver.SetMS(channel, max);
//  sleep(1);
//  driver.SetMS(channel, mid);

  driver.SetAngle(channel, angle_mid);
  sleep(1);
  driver.SetAngle(channel, angle_max);
  sleep(1);
  driver.SetAngle(channel, angle_mid);
  sleep(1);
  driver.SetAngle(channel, angle_min);
  sleep(1);
  driver.SetAngle(channel, angle_mid);
  sleep(1);
  driver.SetAngle(channel, angle_max);
  sleep(1);
  driver.SetAngle(channel, angle_mid);

}