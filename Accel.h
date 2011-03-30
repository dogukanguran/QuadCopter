/*
  Accel.h - Library for interfacing with the ADXL345 accelerometer (specifically, via the Sparkfun IMU Digital Combo Board: SEN-10121)
  Created by Myles Grant <myles@mylesgrant.com>
  See also: https://github.com/grantmd/QuadCopter
  
  This program is free software: you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version. 

  This program is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details. 

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>. 
*/
#ifndef Accel_h
#define Accel_h

#include "WProgram.h"
#include "Definitions.h"
#include "I2C.h"

class Accel : public I2C
{
  public:
    Accel();
    void init();
    void autoZero();
    
    void updateAll();
    
    // The raw values from the sensor
    int getRawRoll();
    int getRawPitch();
    int getRawYaw();
    
    // Smoothed/compensated values
    int getRoll();
    int getPitch();
    int getYaw();
    
  private:
    I2C _i2c;
    
    int dataRaw[3]; // Raw and unfiltered accel data
    float dataSmoothed[3]; // Smoothed accel data
    int zero[3]; // Zero points for the accel axes
    
    float _scaleFactor; // How to convert raw sensor data to SI units
    float _smoothFactor; // 1.0 to not smooth, otherwise adjust as necessary

    long int _lastMeasureTime;
};

#endif