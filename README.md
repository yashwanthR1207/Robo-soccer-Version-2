# Robo-soccer-Version-2
using Arduino uno and flysky fs-i6 channel 2 and 4 
<div align="center">

<h1>SoccerBot-i6</h1>

<p><strong>Two-wheel differential drive soccer robot</strong><br/>
FlySky FS-i6 &nbsp;·&nbsp; BTS7960 x2 &nbsp;·&nbsp; Arduino Uno</p>

[![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Platform](https://img.shields.io/badge/Platform-AVR%20%2F%20Uno-00539C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://www.arduino.cc/en/software)
[![RC Protocol](https://img.shields.io/badge/RC-FlySky%20PWM-E8491D?style=for-the-badge)]()
[![Status](https://img.shields.io/badge/Status-Field%20Tested-2ea44f?style=for-the-badge)]()

<br/>

<table>
<tr>
<td align="center"><strong>Problem</strong></td>
<td align="center"><strong>Solution</strong></td>
</tr>
<tr>
<td><code>pulseIn()</code> reads one channel at a time.<br/>By the time CH4 is read, the pulse is gone.<br/>Steering never works.</td>
<td>A single polling loop captures both<br/>rising and falling edges simultaneously.<br/>No pulse is ever missed.</td>
</tr>
</table>

<br/>

```
  CH2 (Throttle)  ──┐
                     ├──► Tank mixing ──► Left motor  (BTS7960 #1)
  CH4 (Steering)  ──┘              └──► Right motor (BTS7960 #2)
```

<br/>

</div>

---

## Table of Contents

- [Why this code is different](#why-this-code-is-different)
- [Hardware required](#hardware-required)
- [Wiring](#wiring)
- [Channel mapping](#channel-mapping)
- [Tuning constants](#tuning-constants)
- [Motor direction logic](#motor-direction-logic)
- [Getting started](#getting-started)
- [Troubleshooting](#troubleshooting)
- [File structure](#file-structure)

---

## Why this code is different

Most RC robot code uses `pulseIn()` which blocks execution for up to 25ms per channel. Reading two channels sequentially means the second pulse has already passed before the function even starts listening.

```cpp
// BROKEN — sequential reading
int throttle = pulseIn(ch2, HIGH, 25000);   // blocks for up to 25ms
int steering  = pulseIn(ch4, HIGH, 25000);  // pulse has ALREADY GONE
```

This repo fixes that with a simultaneous polling loop — a single `while()` that watches both pins at the same time, catching every rising and falling edge on both channels before returning.

```cpp
// FIXED — both channels captured at the same time
void readChannels(int &ch2, int &ch4) {
    // tight polling loop — neither channel can miss the other's pulse
}
```

---

## Hardware required

| Qty | Component | Notes |
|:---:|-----------|-------|
| 1 | Arduino Uno or Nano | Any 5V AVR board |
| 1 | FlySky FS-i6 transmitter + FS-iA6B receiver | Any FlySky Rx with standard PWM output works |
| 2 | BTS7960 43A H-bridge motor driver | One per motor |
| 2 | DC gear motors | 6V–12V rated, matched to chassis size |
| 1 | 7.4V 2S LiPo or 11.1V 3S LiPo | Higher cell count = higher top speed |
| 1 | Buck converter (LM2596 or XL4016) | Steps battery voltage down to 5V for Arduino |
| — | Chassis, wheels, JST connectors, wiring | M3 nylon standoffs recommended for board mounting |

---

## Wiring

### Receiver to Arduino

| Receiver | Arduino | Notes |
|:---:|:---:|---|
| CH2 signal wire | `A0` | Forward / backward — right stick vertical |
| CH4 signal wire | `A1` | Left / right steering — VrA knob or reassigned stick |
| GND | `GND` | Must share common ground — critical |
| +5V | `5V` | Powers the receiver from Arduino |

> **Note:** CH4 maps to the VrA potentiometer knob by default. For proper stick control go to `Menu → Functions → Aux. channels → Channel 4` and reassign it to the left stick horizontal axis.

---

### Arduino to Left BTS7960

| Arduino | BTS7960 | Function |
|:---:|:---:|---|
| `D5` | `RPWM` | Forward PWM signal |
| `D6` | `LPWM` | Reverse PWM signal |
| `D7` | `R_EN` | Right half-bridge enable — driven HIGH by code |
| `D8` | `L_EN` | Left half-bridge enable — driven HIGH by code |
| `5V` | `VCC` | Logic power |
| `GND` | `GND` | Common ground |
| — | `M+` / `M−` | Left motor terminals |
| — | `B+` / `B−` | Battery power (direct, no regulation) |

---

### Arduino to Right BTS7960

| Arduino | BTS7960 | Function |
|:---:|:---:|---|
| `D9` | `RPWM` | Forward PWM signal |
| `D10` | `LPWM` | Reverse PWM signal |
| `D11` | `R_EN` | Right half-bridge enable — driven HIGH by code |
| `D12` | `L_EN` | Left half-bridge enable — driven HIGH by code |
| `5V` | `VCC` | Logic power |
| `GND` | `GND` | Common ground |
| — | `M+` / `M−` | Right motor terminals |
| — | `B+` / `B−` | Battery power (direct, no regulation) |

---

### Power

| From | To | Notes |
|---|---|---|
| Battery `+` | BTS7960 `B+` on both drivers | Motor power — direct, no buck needed here |
| Battery `−` | BTS7960 `B−` on both drivers | Motor ground |
| Battery `+` | Buck converter `IN+` | Input for Arduino regulation |
| Buck converter `OUT` (set to 5V) | Arduino `5V` pin | Logic power |
| Battery `−` / Buck `GND` | Arduino `GND` | All grounds must be tied together |

> **Warning:** Never power motors from Arduino's onboard regulator. Run a separate path through the buck converter and keep a single shared ground rail across Arduino, both BTS7960 boards, and the battery negative.

---

## Channel mapping

```
FlySky FS-i6 — default channel layout
───────────────────────────────────────────────────────
  CH1   Right stick   Left / Right      (not used)
  CH2   Right stick   Up / Down      →  THROTTLE  A0
  CH3   Left stick    Up / Down         (not used)
  CH4   VrA knob / reassigned axis  →  STEERING   A1
───────────────────────────────────────────────────────
```

To reassign CH4 to a stick axis on the transmitter:
```
Menu → Functions → Aux. channels → Channel 4 → select axis
```

---

## Tuning constants

All calibration values live at the top of `soccer_bot.ino`. No hunting through the code.

```cpp
// Pulse width calibration — measure these with Serial Monitor
const int CH2_MIN    =  997;   // us  stick fully back
const int CH2_CENTER = 1496;   // us  stick at rest centre
const int CH2_MAX    = 1988;   // us  stick fully forward

const int CH4_MIN    =  997;
const int CH4_CENTER = 1496;
const int CH4_MAX    = 1988;

const int DEADBAND  =  40;    // us either side of centre treated as zero
const int MAX_SPEED = 255;    // PWM ceiling 0–255
```

**To calibrate for your specific receiver:** uncomment the `Serial.begin` and `Serial.print` lines in `setup()` and `loop()`, open Serial Monitor at 115200 baud, and record `ch2Raw` / `ch4Raw` at stick minimum, centre, and maximum. Enter those values above.

---

## Motor direction logic

Differential (tank) drive mixing formula:

```
leftSpeed  = throttle + steering
rightSpeed = throttle − steering
```

| Stick position | throttle | steering | Left wheel | Right wheel | Movement |
|---|:---:|:---:|:---:|:---:|---|
| Full forward | `+` | `0` | Forward | Forward | Straight ahead |
| Full backward | `−` | `0` | Reverse | Reverse | Straight back |
| Steer right only | `0` | `+` | Forward | Reverse | Pivot right in place |
| Steer left only | `0` | `−` | Reverse | Forward | Pivot left in place |
| Forward + right | `+` | `+` | Faster | Slower | Curve right |
| Forward + left | `+` | `−` | Slower | Faster | Curve left |
| Centred | `0` | `0` | Stop | Stop | Stationary |

If a motor spins the wrong direction, swap `M+` and `M−` on that driver's output terminals. No code change required.

---

## Getting started

**1. Clone**
```bash
git clone https://github.com/YOUR_USERNAME/soccerbot-i6.git
cd soccerbot-i6
```

**2. Open sketch**
```
Arduino IDE  →  File  →  Open  →  soccer_bot/soccer_bot.ino
```

**3. Select target**
```
Tools  →  Board  →  Arduino Uno
Tools  →  Port   →  COMx  (Windows)  /  /dev/ttyUSBx  (Linux / Mac)
```

**4. Upload**
```
Ctrl + U
```

**5. Power-on order**
```
1. Transmitter ON first
2. Robot power ON
3. Receiver LED goes solid = link established
4. Test throttle slowly before applying full power
```

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Robot does not move at all | No RC signal or wrong pins | Verify CH2 → A0, CH4 → A1, receiver GND connected |
| Only forward / back, steering dead | Sequential `pulseIn()` still in code | Use `readChannels()` from this repo |
| Motors twitch at rest | Deadband too small | Increase `DEADBAND` to 50–70 |
| One motor spins wrong direction | Motor terminals reversed | Swap `M+` / `M−` on that BTS7960 |
| Robot creeps with sticks centred | Centre value mis-calibrated | Measure and update `CH2_CENTER` / `CH4_CENTER` |
| RC signal drops under load | Voltage sag on 5V rail | Add 470 uF capacitor across Arduino 5V and GND |
| BTS7960 becomes very hot | Motor stall or overcurrent | Fit heatsink; verify motor current draw vs 43A rating |
| Arduino resets when motors start | Shared power path | Separate motor power; use dedicated buck converter for logic |

---

## File structure

```
soccerbot-i6/
│
├── soccer_bot/
│   └── soccer_bot.ino       main sketch — all logic in one file
│
├── docs/
│   └── wiring_diagram.png   full connection diagram
│
└── README.md
```

---

<div align="center">

Built for the field. Go score some goals.

*If this helped your build, leave a star on the repo — it helps others find it.*

</div>
