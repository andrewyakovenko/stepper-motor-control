#define ENCODER_INTERRUPT_PIN 2
#define BUTTON_PIN 3
#define ENCODER_DIRECTION_PIN 4
#define STEP_PIN 6
#define DIR_PIN 7


const int MAX_VALUE = 15;
volatile int value = 0;

const double ACCELERATION = 0.42;
const int RPM_LUT[] = {0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275, 300, 325, 350, 375};
double currentRPM = 0;

bool motorCW = true;

void setup() {
  Serial.begin(9600);

  // setup rotary encoder pins and initial state
  pinMode(ENCODER_INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DIRECTION_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_INTERRUPT_PIN), changeSpeed, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), stop, FALLING);

  // setup stepper motor pins
  pinMode(STEP_PIN, OUTPUT); 
  pinMode(DIR_PIN, OUTPUT);
}

void changeSpeed() {
  bool cw = (digitalRead(ENCODER_INTERRUPT_PIN) != digitalRead(ENCODER_DIRECTION_PIN));
  if (cw) {
    value = min(value + 1, MAX_VALUE);
    Serial.print("+ ");
    Serial.println(value);
  } else {
    value = max(value - 1, -MAX_VALUE);
    Serial.print("- ");
    Serial.println(value);
  }
}

void stop() {
  value = 0;
  Serial.print("0: ");
  Serial.println(value);
}

void checkMotorDirection(double speed) {
  if (motorCW && speed < 0) {
    digitalWrite(DIR_PIN, LOW);
    motorCW = false;
  }
  if (!motorCW && speed > 0) {
    digitalWrite(DIR_PIN, HIGH);
    motorCW = true;
  }
}

int getTargetRPM() {
  int targetRpm = RPM_LUT[abs(value)];
  if (value < 0) {
    return -targetRpm;
  } else {
    return targetRpm;
  }
}

void updateSpeed() {
  int targetRPM = getTargetRPM();
  if (currentRPM == targetRPM) {
    return;
  }
  if (currentRPM < targetRPM) {
    currentRPM = min(currentRPM + ACCELERATION, targetRPM);
  } else {
    currentRPM = max(currentRPM - ACCELERATION, targetRPM);
  }
}

int delayForRPM(int rpm) {
  double rotationDurationMS = 60000 / abs(rpm);
  double stepDurationMS = rotationDurationMS / 200;
  return round(1000 * stepDurationMS / 2);
}

void loop() {
  updateSpeed();
  if (currentRPM == 0) {
    delay(1);
  } else {
    checkMotorDirection(currentRPM);
    int d = delayForRPM(currentRPM);
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(d);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(d);
  }
}
