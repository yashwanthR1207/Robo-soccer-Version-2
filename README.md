<div align="center">

# Robo-Soccer — Version 2

**Two-wheel RC soccer robot · Arduino Uno · FlySky FS-i6 · BTS7960**

[![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=flat-square&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)]()
[![RC](https://img.shields.io/badge/RC-FlySky%20FS--i6-E8491D?style=flat-square)]()
[![Status](https://img.shields.io/badge/Status-Field%20Tested-2ea44f?style=flat-square)]()

</div>

---

### What this is

A two-wheel differential drive robot controlled over FlySky PWM radio.
The main problem with most beginner code is `pulseIn()` — it reads one channel, blocks for 25 ms, then tries to read the second. By then that pulse is long gone, so steering never responds.

This code reads **both channels at the same time** using a tight polling loop. No blocking. No missed pulses.

```
CH2  (throttle) ──┬──► left  motor  BTS7960 #1
CH4  (steering) ──┘──► right motor  BTS7960 #2
```

---

### Hardware

| Qty | Part |
|:---:|------|
| 1 | Arduino Uno |
| 1 | FlySky FS-i6 transmitter + FS-iA6B receiver |
| 2 | BTS7960 43 A H-bridge motor driver |
| 2 | DC gear motors (6 V – 12 V) |
| 1 | 7.4 V 2S or 11.1 V 3S LiPo battery |
| 1 | Buck converter — output set to 5 V (for Arduino logic) |

---

### Wiring

**Receiver → Arduino**

| Receiver | Arduino | What |
|:---:|:---:|---|
| CH2 | `A0` | Throttle — forward / back |
| CH4 | `A1` | Steering — left / right |
| GND | `GND` | Common ground (required) |
| +5V | `5V` | Receiver power |

> CH4 is the VrA knob by default. To use a stick instead:
> `Menu → Functions → Aux. channels → Channel 4 → assign axis`

---

**Arduino → Left BTS7960**

| Arduino | BTS7960 |
|:---:|:---:|
| `D5` | `RPWM` |
| `D6` | `LPWM` |
| `D7` | `R_EN` |
| `D8` | `L_EN` |
| `5V` | `VCC` |
| `GND` | `GND` |

---

**Arduino → Right BTS7960**

| Arduino | BTS7960 |
|:---:|:---:|
| `D9` | `RPWM` |
| `D10` | `LPWM` |
| `D11` | `R_EN` |
| `D12` | `L_EN` |
| `5V` | `VCC` |
| `GND` | `GND` |

---

**Power**

| From | To |
|---|---|
| Battery `+` | `B+` on both BTS7960 boards |
| Battery `−` | `B−` on both BTS7960 boards |
| Battery `+` | Buck converter input |
| Buck converter output (5 V) | Arduino `5V` |
| Battery `−` | Arduino `GND` — tie all grounds together |
| BTS7960 `M+` / `M−` | Motor terminals (swap to reverse direction) |

> Never power motors from the Arduino 5 V pin. Always use the buck converter for logic and connect battery power directly to the BTS7960 B+ / B−.

---

### Channel map

```
CH1   right stick  left/right    — not used
CH2   right stick  up/down       — THROTTLE → A0
CH3   left stick   up/down       — not used
CH4   VrA / axis   left/right    — STEERING → A1
```

---

### Calibration

Open `soccer_bot.ino` and update these six values at the top to match your receiver:

```cpp
const int CH2_MIN    =  997;
const int CH2_CENTER = 1496;
const int CH2_MAX    = 1988;

const int CH4_MIN    =  997;
const int CH4_CENTER = 1496;
const int CH4_MAX    = 1988;
```

To find the real values: enable `Serial.begin(115200)`, print `ch2Raw` and `ch4Raw` in `loop()`, open Serial Monitor, and move each stick to its extremes and centre. Write down what you see.

---

### How the drive mixing works

```
left  speed = throttle + steering
right speed = throttle − steering
```

| Stick | Left wheel | Right wheel |
|---|:---:|:---:|
| Forward | fwd | fwd |
| Backward | rev | rev |
| Steer right | fwd | rev |
| Steer left | rev | fwd |
| Forward + right | faster | slower |
| Forward + left | slower | faster |

If one motor spins backwards, swap its `M+` and `M−` wires on the BTS7960. No code change needed.

---

### Upload

```bash
# 1. Clone
git clone https://github.com/YOUR_USERNAME/robo-soccer-v2.git

# 2. Open in Arduino IDE
File → Open → soccer_bot/soccer_bot.ino

# 3. Board + port
Tools → Board → Arduino Uno
Tools → Port  → your port

# 4. Upload
Ctrl + U
```

Power-on order: **transmitter first**, then robot. Wait for receiver LED to go solid before moving sticks.

---

### Troubleshooting

| Problem | Fix |
|---|---|
| No movement at all | Check CH2 → A0, CH4 → A1, receiver GND |
| Steering does nothing | Make sure you are using `readChannels()`, not `pulseIn()` |
| Motors twitch at rest | Raise `DEADBAND` to 50–70 |
| One motor reversed | Swap `M+` / `M−` on that BTS7960 |
| Creeps with sticks centred | Re-calibrate `CH2_CENTER` / `CH4_CENTER` |
| Signal drops under load | Add 470 µF cap across Arduino 5 V and GND |
| BTS7960 overheats | Add heatsink; check motor stall current |
| Arduino resets on motor start | Use separate power paths; shared buck converter for logic only |

---


---

<div align="center">
Built for the field.<br/>
If this helped, leave a star. (try to copy this if you can >__<)
</div>
