#include "me_Math.h"

float RoundToUnit(float Value, float UnitSize)
{
  return trunc(Value / UnitSize) * UnitSize;
}

float RoundToDigits(float Value, int8_t LastSignificantDigitPosition)
{
  float Result = 0;

  bool IsNegative = (Value < 0);
  if (IsNegative)
  {
    Value = -Value;
  }

  float IntegerPart = trunc(Value);

  float FractionalPart = Value - IntegerPart;

  float ScaledFractionalPart;

  float Scale = 1.0;

  if (LastSignificantDigitPosition < 0)
  {
    for (int8_t i = 0; i > LastSignificantDigitPosition; --i)
    {
      Scale = Scale * 10;
    }
  }

  ScaledFractionalPart = trunc(FractionalPart * Scale);

  Result = IntegerPart + (ScaledFractionalPart / Scale);
  if (IsNegative)
  {
    Result = -Result;
  }

  return Result;
}

float RadToDeg(float Radians)
{
  const float RadsInCircle = 2 * M_PI;
  const float DegsInCircle = 360.0;
  return (Radians / RadsInCircle * DegsInCircle);
}

float DegToRad(float Degrees)
{
  const float RadsInCircle = 2 * M_PI;
  const float DegsInCircle = 360.0;
  return (Degrees / DegsInCircle * RadsInCircle);
}

/*
  2023-11-03
  2023-11-04
  2024-03-23
*/
