/*
  Gyro.cpp - Library for interfacing with the ITG-3200 gyro (specifically, via the Sparkfun IMU Digital Combo Board: SEN-10121)
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

#include "WProgram.h"
#include "Definitions.h"
#include "Gyro.h"
#include "I2C.h"
#include "Utils.h"

Gyro::Gyro() : I2C(){
  _scaleFactor = radians(1.0 / 14.375); // ITG3200 14.375 LSBs per °/sec
  _smoothFactor = 0.75;
  _sleeping = false;
}

// Init the gyro by checking if it's connected and then resetting it and applying our settings
void Gyro::init(){
  Serial.println("Initing Gyro");
  setAddress(GYRO_ADDR);
  
  if (!getAddressFromDevice()){
    Serial.println("GYRO NOT CONNECTED!");
  }
  else{
    writeSetting(0x3E, 0x80); // Reset it
    delay(50); // Give it some time to startup
    writeSetting(0x16, 0x1D); // 10Hz low pass filter
    writeSetting(0x3E, 0x01); // use X gyro oscillator
    
    Serial.print("Current temp: ");
    Serial.print(getTemp());
    Serial.println("F");
    
    // TODO: We should calculate these once under known conditions and store them in eeprom
    autoZero();
  }
}

// Calculate zero for all 3 axis, storing it for later measurements
// This assumes your platform is truly level!
void Gyro::autoZero(){
  // Take 50 measurements of all 3 axis, find the median, that's our zero-point
  // Why 50? Because that's what the aeroquad project does
  // (note, I could not get to 50 -- my arduino locks up. so I'm leaving it at 10)
  byte loopCount = 10;
  
  Serial.print("Starting gyro autoZero with ");
  Serial.print(loopCount, DEC);
  Serial.println(" iterations.");
  int findZero[loopCount];
  for (byte axis = PITCH; axis <= YAW; axis++){
    for (byte i=0; i<loopCount; i++){
      sendReadRequest(0x1D + (axis * 2));
      findZero[i] = readWord();
      delay(10);
    }
    
    zero[axis] = findMedian(findZero, loopCount);
    Serial.print("Zero of gyro axis ");
    Serial.print(axis, DEC);
    Serial.print(" is: ");
    Serial.println(zero[axis]);
  }
}

// Updates all raw measurements from the gyro (except temp)
void Gyro::updateAll(){
  //Serial.println("Updating all gyro data");
  sendReadRequest(0x1D);
  requestBytes(6);

  for (byte axis = PITCH; axis <= YAW; axis++) {
     dataRaw[axis] = zero[axis] - readNextWord();
     dataSmoothed[axis] = filterSmooth((float)dataRaw[axis] * _scaleFactor, dataSmoothed[axis], _smoothFactor);
  }
  
  _lastMeasureTime = micros();
}

///////////

int Gyro::getRawPitch(){
  return dataRaw[PITCH];
}

int Gyro::getRawRoll(){
  return dataRaw[ROLL];
}

int Gyro::getRawYaw(){
  return dataRaw[YAW];
}

///////////

int Gyro::getTemp(){
  sendReadRequest(0x1B);
  temp = readWord();
  temp = 35.0 + ((temp + 13200) / 280.0); // -13200 == 35C, 280 == Each degree
  temp = 32 + (temp * 1.8); // Convert to F
  
  return temp;
}

int Gyro::getPitch(){
  return dataSmoothed[PITCH];
}

int Gyro::getRoll(){
  return dataSmoothed[ROLL];
}

int Gyro::getYaw(){
  return dataSmoothed[YAW];
}

/////////////

void Gyro::sleep(){
  if (_sleeping) return;
  writeSetting(0x3E, 0x40);
  _sleeping = true;
}

void Gyro::unsleep(){
  if (!_sleeping) return;
  writeSetting(0x3E, 0x00);
  _sleeping = false;
}