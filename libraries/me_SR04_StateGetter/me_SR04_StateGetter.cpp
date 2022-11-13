// Higher-level wrapper for <me_SR04> ultrasonic sensor.

/*
  Status: stable
  Version: 1.0
  Last mod.: 2022-11-01
*/

#include "me_SR04_StateGetter.h"

#include <me_SR04.h>
#include <me_Physics_Sound.h>

using namespace me_SR04_StateGetter;

State me_SR04_StateGetter::GetState(me_SR04::SR04* sensor)
{
  State result;

  if (sensor->RequestStatus == me_SR04::Status::Success)
  {
    result.IsSensorWorking = true;
    result.HasDistance = true;
    result.DistanceCm = me_Physics_Sound::GetDistanceFromEchoCm(sensor->EchoDelayMcr);
  }
  else
  {
    if (sensor->RequestStatus == me_SR04::Status::NoSignalStart)
      result.IsSensorWorking = false;
    else if (sensor->RequestStatus == me_SR04::Status::NoSignalEnd)
      result.IsSensorWorking = true;
    else
      exit(1);
    result.HasDistance = false;
    result.DistanceCm = 0.0;
  }

  return result;
}
