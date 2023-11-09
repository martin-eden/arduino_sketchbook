// Firmware for my Rover v4.

/*
  Status: stable
  Version: 5
  Last mod.: 2023-11-09
*/

/*
  This code is for Arduino Uno with stacked Deek motor shield.
*/

/*
  Deek motor shield is for Arduino Uno, it is not electrically
  compatible with Wemos D1. So Arduino Uno hosts Deek motor shield.
  It is connected to Wemos D1 over UART.
*/

/*
  Hardware setup

    Board: Arduino Uno (ATmega328P)
    Peripherials:
      * Deek motor board (stacked)
*/

#include <me_DeekMotor.h>
#include <me_Parser_MotorBoard.h>
#include <me_Install_StandardStreams.h>

const uint32_t
  SerialBaud = 9600; // 115200;

const uint16_t
  // Delay between main loop iterations:
  TickTime_Ms = 30,
  // Timeout to stop motors when no command received:
  AutoStopTimeout_Ms = 1000;

// Deek motor board pin mapping:
const uint8_t
  Deek_DirA_Pin = 12,
  Deek_PwmA_Pin = 3,
  Deek_BrakeA_Pin = 9,

  Deek_DirB_Pin = 13,
  Deek_PwmB_Pin = 11,
  Deek_BrakeB_Pin = 8;

const TDeekMotorPins
  LeftMotorPins =
    {
      Direction_Pin: Deek_DirA_Pin,
      Pwm_Pin: Deek_PwmA_Pin,
      Brake_Pin: Deek_BrakeA_Pin
    },
  RightMotorPins =
    {
      Direction_Pin: Deek_DirB_Pin,
      Pwm_Pin: Deek_PwmB_Pin,
      Brake_Pin: Deek_BrakeB_Pin
    };

DeekMotor LeftMotor(LeftMotorPins);
DeekMotor RightMotor(RightMotorPins);

/*
  Timeout for Serial.parseInt() function.

  That function will wait in empty stream for this time if next character
  could be a digit.

  Actually this value should be barely larger than delay between two characters.
*/

const uint16_t SerialParseIntTimeout_Ms = 100;

using namespace MotorboardCommandsParser;

// ---

void setup()
{
  Serial.begin(SerialBaud);
  Serial.setTimeout(SerialParseIntTimeout_Ms);

  Install_StandardStreams();

  PrintProtocolHelp();
  PrintSetupGreeting();
}

void loop()
{
  static uint32_t LastCommandTime_Ms = 0;
  static bool MotorsAreStopped = false;
  static bool AcknowledgePrinted = false;
  uint32_t TimePassed_Ms;

  TMotorboardCommand Command;

  while (Serial.available())
  {
    if (ParseCommand(&Command))
    {
      DisplayCommand(Command);

      ExecuteCommand(Command);

      MotorsAreStopped = false;

      LastCommandTime_Ms = millis();
    }

    AcknowledgePrinted = false;
  }

  if (!Serial.available())
  {
    if (!AcknowledgePrinted)
    {
      printf("G\n");
      AcknowledgePrinted = true;
    }
  }

  TimePassed_Ms = millis() - LastCommandTime_Ms;

  // Motors auto-stop:
  if ((TimePassed_Ms >= AutoStopTimeout_Ms) && !MotorsAreStopped)
  {
    StopMotors();
    // printf("(Motors are stopped.)\n");
    MotorsAreStopped = true;
  }

  delay(TickTime_Ms);
}

// ---

void PrintSetupGreeting()
{
  printf_P(
    PSTR(
      "\n"
      "-------------------------------------------------------------\n"
      "                    Rover-4 motor board\n"
      "-------------------------------------------------------------\n"
      "\n"
      "  Pins assignment\n"
      "\n"
      "    Left motor : Direction = %2d, PWM = %2d, Brake = %2d\n"
      "    Right motor: Direction = %2d, PWM = %2d, Brake = %2d\n"
      "\n"
      "  Tick duration (ms): %d\n"
      "  Auto-stop timeout (ms): %d\n"
      "\n"
      "-------------------------------------------------------------\n"
      "\n"
    ),
    LeftMotorPins.Direction_Pin,
    LeftMotorPins.Pwm_Pin,
    LeftMotorPins.Brake_Pin,
    RightMotorPins.Direction_Pin,
    RightMotorPins.Pwm_Pin,
    RightMotorPins.Brake_Pin,
    TickTime_Ms,
    AutoStopTimeout_Ms
  );
}

void PrintProtocolHelp()
{
  printf_P(
    PSTR(
      "\n"
      "                          Protocol\n"
      "\n"
      "Motor board command format.\n"
      "\n"
      "  Command is two tokens. Command type and command value.\n"
      "\n"
      "  Commands\n"
      "\n"
      "    L [-100, 100]\n"
      "      Left motor. Set specified power and direction.\n"
      "    R [-100, 100]\n"
      "      Right motor. Set specified power and direction.\n"
      "    D [0, 5000]\n"
      "      Delay for given interval in milliseconds.\n"
      "      Motors will be running.\n"
      "\n"
      "  Whitespaces are stripped: \"L 50 R -50 D 1000\" == \"L50R-50D1000\".\n"
      "\n"
      "Feedback\n"
      "\n"
      "  If we input channel is empty now, we emit \"G\\n\" (go, got it,\n"
      "  gimme more) as a sign that we are ready for next commands.\n"
      "\n"
      "  This behavior is needed for throttling commands-provider board.\n"
      "  Without it can overflow our Serial input buffer while we are\n"
      "  executing commands.\n"
      "\n"
      "  Command-provider board MAY send any garbage but\n"
      "\n"
      "    * MUST assure that data chunk size is no more than\n"
      "      our input buffer\n"
      "    * MUST wait for \"G\\n\" before sending chunk\n"
      "\n"
      "Behavior\n"
      "\n"
      "  Motors are stopped when no command received in <AutoStop>\n"
      "  interval of time.\n"
      "\n"
      "\n"
    )
  );
}

// Print parsed command.
void DisplayCommand(TMotorboardCommand Command)
{
  char
    EmptyName[] = "",
    LeftMotorName[] = "LeftMotor",
    RightMotorName[] = "RightMotor",
    DurationStr[] = "Duration";

  char * CommandName;
  int32_t CommandData;

  switch (Command.CommandType)
  {
    case CommandType_LeftMotor:
      CommandName = LeftMotorName;
      CommandData = Command.MotorSpeed_Pc;
      break;

    case CommandType_RightMotor:
      CommandName = RightMotorName;
      CommandData = Command.MotorSpeed_Pc;
      break;

    case CommandType_Duration:
      CommandName = DurationStr;
      CommandData = Command.Duration_Ms;
      break;

    default:
      CommandName = EmptyName;
      CommandData = 0;
  }

  if (CommandName)
    printf("(%s: %ld)\n", CommandName, CommandData);
  else
    printf("Unsupported command.\n");
}

void ExecuteCommand(TMotorboardCommand Command)
{
  switch (Command.CommandType)
  {
    case CommandType_LeftMotor:
      LeftMotor.SetSpeed(Command.MotorSpeed_Pc);
      break;

    case CommandType_RightMotor:
      RightMotor.SetSpeed(Command.MotorSpeed_Pc);
      break;

    case CommandType_Duration:
      delay(Command.Duration_Ms);
      break;
  }
}

void StopMotors()
{
  ExecuteCommand({CommandType_LeftMotor, 0});
  ExecuteCommand({CommandType_RightMotor, 0});
}

// ---

/*
  2023-10-07
  2023-10-08
  2023-10-11
  2023-10-14
  2023-11-02
  2023-11-05
  2023-11-09
*/
