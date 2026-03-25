// ============================================================
//  Robo-Soccer Version 2
//  2-Wheel Soccer Bot  |  FlySky FS-i6  |  BTS7960 x2
//
//  Both RC channels are read simultaneously using a polling
//  loop — pulseIn() is NOT used because it blocks for 25ms
//  per channel, causing the second pulse to be missed entirely.
//
//  Wiring — Receiver
//    CH2 signal  →  A0   (forward / backward)
//    CH4 signal  →  A1   (left / right)
//    GND         →  Arduino GND
//    5V          →  Arduino 5V
//
//  Wiring — Left motor BTS7960
//    RPWM → D5    LPWM → D6
//    R_EN → D7    L_EN → D8
//    B+   → battery +   (direct)
//    M+/M- → left motor terminals
//
//  Wiring — Right motor BTS7960
//    RPWM → D9    LPWM → D10
//    R_EN → D11   L_EN → D12
//    B+   → battery +   (direct)
//    M+/M- → right motor terminals
//
//  Wiring — Power
//    Battery +  →  Buck converter IN+
//    Buck OUT (5V)  →  Arduino 5V
//    All GNDs joined: Arduino + both BTS7960 + battery −
// ============================================================

// ── Receiver input pins ──────────────────────────────────────
#define CH2_PIN  A0       // throttle — forward / backward
#define CH4_PIN  A1       // steering — left / right

// ── Left motor (BTS7960 #1) ──────────────────────────────────
#define L_RPWM   5
#define L_LPWM   6
#define L_R_EN   7
#define L_L_EN   8

// ── Right motor (BTS7960 #2) ─────────────────────────────────
#define R_RPWM   9
#define R_LPWM   10
#define R_R_EN   11
#define R_L_EN   12

// ── Calibration — measure with Serial Monitor for your Rx ────
const int CH2_MIN    =  997;
const int CH2_CENTER = 1496;
const int CH2_MAX    = 1988;

const int CH4_MIN    =  997;
const int CH4_CENTER = 1496;
const int CH4_MAX    = 1988;

// ── Tuning ───────────────────────────────────────────────────
const int          DEADBAND  =  40;     // us dead zone around centre
const int          MAX_SPEED = 255;     // PWM ceiling 0–255
const unsigned long TIMEOUT  = 25000;  // 25 ms per channel timeout (us)


// ── readChannels ─────────────────────────────────────────────
// Polls both pins inside a single loop so neither channel can
// miss the other's pulse. Falls back to centre on timeout.
// ─────────────────────────────────────────────────────────────
void readChannels(int &ch2, int &ch4) {
  unsigned long ch2Start = 0, ch4Start = 0;
  unsigned long ch2Pulse = 0, ch4Pulse = 0;
  bool ch2Done = false, ch4Done = false;
  bool ch2High = false, ch4High = false;

  unsigned long startWait = micros();

  while ((!ch2Done || !ch4Done) && (micros() - startWait < TIMEOUT * 2)) {

    bool ch2State = digitalRead(CH2_PIN);
    bool ch4State = digitalRead(CH4_PIN);

    // CH2 rising edge
    if (ch2State && !ch2High) {
      ch2Start = micros();
      ch2High  = true;
    }
    // CH2 falling edge
    if (!ch2State && ch2High && !ch2Done) {
      ch2Pulse = micros() - ch2Start;
      ch2Done  = true;
    }

    // CH4 rising edge
    if (ch4State && !ch4High) {
      ch4Start = micros();
      ch4High  = true;
    }
    // CH4 falling edge
    if (!ch4State && ch4High && !ch4Done) {
      ch4Pulse = micros() - ch4Start;
      ch4Done  = true;
    }
  }

  // Failsafe: return centre if a channel timed out
  ch2 = ch2Done ? constrain((int)ch2Pulse, CH2_MIN, CH2_MAX) : CH2_CENTER;
  ch4 = ch4Done ? constrain((int)ch4Pulse, CH4_MIN, CH4_MAX) : CH4_CENTER;
}


// ── pwmToSpeed ───────────────────────────────────────────────
// Maps a raw pulse width to a signed speed value -255 … +255.
// Values within DEADBAND of centre return 0.
// ─────────────────────────────────────────────────────────────
int pwmToSpeed(int raw, int minVal, int centre, int maxVal) {
  int offset = raw - centre;
  if (abs(offset) < DEADBAND) return 0;
  if (offset > 0)
    return map(offset, DEADBAND, maxVal - centre, 0, MAX_SPEED);
  else
    return map(offset, -(centre - minVal), -DEADBAND, -MAX_SPEED, 0);
}


// ── driveMotor ───────────────────────────────────────────────
// Drives one BTS7960 H-bridge.
//   speed > 0  →  forward  (RPWM active, LPWM = 0)
//   speed < 0  →  reverse  (LPWM active, RPWM = 0)
//   speed = 0  →  coast    (both = 0)
// ─────────────────────────────────────────────────────────────
void driveMotor(int rpwm, int lpwm, int speed) {
  speed = constrain(speed, -MAX_SPEED, MAX_SPEED);
  if (speed > 0) {
    analogWrite(rpwm, speed);
    analogWrite(lpwm, 0);
  } else if (speed < 0) {
    analogWrite(rpwm, 0);
    analogWrite(lpwm, -speed);
  } else {
    analogWrite(rpwm, 0);
    analogWrite(lpwm, 0);
  }
}


// ════════════════════════════════════════════════════════════
void setup() {
  // Receiver inputs
  pinMode(CH2_PIN, INPUT);
  pinMode(CH4_PIN, INPUT);

  // Left motor driver
  pinMode(L_RPWM, OUTPUT);  pinMode(L_LPWM, OUTPUT);
  pinMode(L_R_EN, OUTPUT);  pinMode(L_L_EN, OUTPUT);
  digitalWrite(L_R_EN, HIGH);
  digitalWrite(L_L_EN, HIGH);

  // Right motor driver
  pinMode(R_RPWM, OUTPUT);  pinMode(R_LPWM, OUTPUT);
  pinMode(R_R_EN, OUTPUT);  pinMode(R_L_EN, OUTPUT);
  digitalWrite(R_R_EN, HIGH);
  digitalWrite(R_L_EN, HIGH);

  // Uncomment to enable calibration output:
  // Serial.begin(115200);
}


// ════════════════════════════════════════════════════════════
void loop() {
  int ch2Raw, ch4Raw;
  readChannels(ch2Raw, ch4Raw);

  int throttle = pwmToSpeed(ch2Raw, CH2_MIN, CH2_CENTER, CH2_MAX);
  int steering = pwmToSpeed(ch4Raw, CH4_MIN, CH4_CENTER, CH4_MAX);

  // Tank mixing: left = throttle + steering, right = throttle − steering
  int leftSpeed  = constrain(throttle + steering, -MAX_SPEED, MAX_SPEED);
  int rightSpeed = constrain(throttle - steering, -MAX_SPEED, MAX_SPEED);

  driveMotor(L_RPWM, L_LPWM, leftSpeed);
  driveMotor(R_RPWM, R_LPWM, rightSpeed);

  // Uncomment to calibrate — open Serial Monitor at 115200 baud:
  // Serial.print("CH2: "); Serial.print(ch2Raw);
  // Serial.print("  CH4: "); Serial.println(ch4Raw);
}
