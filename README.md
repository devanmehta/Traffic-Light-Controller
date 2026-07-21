# 🚦 Traffic Light Controller — TI MSPM0G3507

A finite state machine-based traffic light controller for a 2-street intersection running on Texas Instruments MSPM0G3507 microcontroller in bare metal embedded C. Implements real-time traffic signal sequencing with pedestrian crossing support and hardware safety features.

---
 
## Features
 
- **13-State Moore FSM**: Fully data-driven design — a single `struct State` array holds every state's outputs, wait time, and all 8 next-state transitions. No conditional branches (`if`, `switch`, loops) in the control logic; the next state is purely a table lookup.
- **Input-Dependent Transitions**: Every state has 8 next-state pointers (one per possible South/West/Walk sensor combination), so the FSM reacts correctly to any of 1, 2, or 3 simultaneous requests.
- **Safety-First Sequencing**: Mandatory all-red buffer states between every yellow→green and Walk→car transition to prevent collisions. Exactly one South LED and one West LED is ever on at a time.
- **Flashing Pedestrian Warning**: 5-state "don't walk" flash sequence (white → red → off → red → off → red) before returning traffic to the roads, satisfying the minimum double-flash requirement.
- **No-Starvation Round Robin**: When all three sensors are held active, the FSM cycles South → Walk → West → South ... indefinitely, servicing every request in order.

---

## Hardware
 
| Component | Pin / Channel | Purpose |
|---|---|---|
| MSPM0G3507 | — | Main CPU (80 MHz) |
| South Red LED | PB2 | South road red light |
| South Yellow LED | PB1 | South road yellow light |
| South Green LED | PB0 | South road green light |
| West Red LED | PB8 | West road red light |
| West Yellow LED | PB7 | West road yellow light |
| West Green LED | PB6 | West road green light |
| Walk Signal (RGB) | PB22 (Blue) / PB26 (Red) / PB27 (Green) | Onboard tri-color LED — white = Walk, red = Don't Walk |
| South Sensor | PB16 | Simulated car sensor (button), South road |
| West Sensor | PB15 | Simulated car sensor (button), West road |
| Pedestrian Sensor | PB17 | Simulated pedestrian call button (button) |
 
Each of the 6 traffic LEDs is driven through a ULN2003B Darlington array, since each LED draws ~10mA which is more than the MSPM0's native 6mA GPIO drive capability. The walk signal reuses the LaunchPad's onboard RGB LED directly. All three buttons "sensors" use external 10kΩ pull-down resistors for clean positive-logic switching.
 
---

## The Finite State Machine
 
The intersection is modeled as a 13-state Moore machine. Each state defines its South/West/Walk outputs, a wait time (units of 10ms), and 8 next-state transitions — one for every combination of the 3 binary sensor inputs (`Walk, South, West`).
 
| State | South | West | Walk | Wait | 000 | 001 (W) | 010 (S) | 011 (S,W) | 100 (Wk) | 101 (Wk,W) | 110 (Wk,S) | 111 (All) |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| **goS** | Green | Red | Red | 100 | goS | waitS | goS | waitS | waitS | waitS | waitS | waitS |
| **waitS** | Yellow | Red | Red | 50 | allRedS | allRedS | allRedS | allRedS | allRedS | allRedS | allRedS | allRedS |
| **allRedS** | Red | Red | Red | 20 | goS | goW | goS | goW | goWalk | goWalk | goWalk | goWalk |
| **goWalk** | Red | Red | White | 100 | goWalk | flash1 | flash1 | flash1 | goWalk | flash1 | flash1 | flash1 |
| **flash1** | Red | Red | Red | 20 | flash2 | flash2 | flash2 | flash2 | flash2 | flash2 | flash2 | flash2 |
| **flash2** | Red | Red | Off | 20 | flash3 | flash3 | flash3 | flash3 | flash3 | flash3 | flash3 | flash3 |
| **flash3** | Red | Red | Red | 20 | flash4 | flash4 | flash4 | flash4 | flash4 | flash4 | flash4 | flash4 |
| **flash4** | Red | Red | Off | 20 | flash5 | flash5 | flash5 | flash5 | flash5 | flash5 | flash5 | flash5 |
| **flash5** | Red | Red | Red | 20 | allRedWk | allRedWk | allRedWk | allRedWk | allRedWk | allRedWk | allRedWk | allRedWk |
| **allRedWk** | Red | Red | Red | 20 | goWalk | goW | goS | goW | goWalk | goW | goS | goW |
| **goW** | Red | Green | Red | 100 | goW | goW | waitW | waitW | waitW | waitW | waitW | waitW |
| **waitW** | Red | Yellow | Red | 50 | allRedW | allRedW | allRedW | allRedW | allRedW | allRedW | allRedW | allRedW |
| **allRedW** | Red | Red | Red | 20 | goW | goW | goS | goS | goWalk | goWalk | goS | goS |
 
---
 
## Debugging

Uses a **non-intrusive debug dump**: each test logs a 32-bit snapshot to a RAM buffer that can be inspected afterword in CCS's memory view.
 
Bring-up was staged and isolated before full integration:
1. **LED test** — cycle each LED, dump one-hot codes, verify with a voltmeter
2. **Switch test** — poll sensors, dump values on change
3. **FSM sequencing test** — force all sensors active, dump `{state, West, South, Walk}` each cycle to confirm no starvation
4. **Full integration** — live switches on the physical board
 
---

## Build & Flash
 
### Prerequisites
- Code Composer Studio (CCS) v12+
- MSPM0 SDK installed
- LaunchPad connected via USB
### Build Steps
1. Open Code Composer Studio
2. File → Open Projects from File System → Browse to the project folder → Click **Finish**
3. Project → Build (or Ctrl+B) → Check Console tab for errors
4. Run → Debug (or F11) → CCS flashes and starts debugger
5. Run → Resume (or F8) to start
6. Hold switches on PB15/PB16/PB17 to simulate West/South/pedestrian traffic
### Common Build Errors & Fixes
 
| Error | Cause | Fix |
|---|---|---|
| Cannot find include file `ti/devices/msp/...` | Missing SDK path | Project → Properties → CCS Build → Include Options → verify MSPM0 SDK path |
| Unresolved symbol `_c_int00` | Missing startup file | Ensure `startup_MSPM0G3507_ccs.c` is in your project |
| Port not found / Device not detected | USB not connected | Check LaunchPad USB cable; try a different port |
| Build successful but no output | Debugger issue | Try Run → Terminate, then Run → Debug again |

---
 
## File Structure
 
```
Traffic-Light-Controller/
├── .gitignore                   
├── ECE319K_Lab4main.c           # Main FSM + Tester functions
├── ECE319K_Lab4Grader.c         # automated validation
├── mspm0g3507.cmd               # Linker command file for MSPM0G3507
│
└── inc/                         
    ├── LaunchPad.c / LaunchPad.h # Board initialization and on-board LED primitives
    ├── Clock.c / Clock.h        # 80 MHz PLL initialization
    └── Timer.c / Timer.h        # SysTick fixed-time delay Timer
    └── Dump.c / Dump.h          # Non-intrusive RAM debug dump
    └── UART.c / UART.h          # Serial communication for terminal output
```

---
 
## Code Architecture
 
**Data structure (1-to-1 mapped to the state graph above):**
```c
struct State {
  uint32_t outSouth;   // South LED pattern (G/Y/R)
  uint32_t outWest;    // West LED pattern (G/Y/R)
  uint32_t outWalk;    // Walk LED pattern (white/red/off)
  uint32_t delay;      // Wait time, units of 10ms
  uint32_t Next[8];    // Next-state index for each of the 8 sensor combinations
};
```
 
**Main FSM loop** (no conditional branches other than inside the wait helper):
1. Drive outputs for the current state via `Traffic_Out()`
2. Wait `fsm[S].delay` × 10ms via `SysTick_Wait10ms()`
3. Read the 3 sensors via `Traffic_In()`
4. Look up the next state: `S = fsm[S].Next[input]`
Because the next-state logic lives entirely in the `fsm[]` table rather than in branching code, the sequencing, timing, or number of states can be changed by editing the data structure alone — no changes to the executable control flow are required.



 