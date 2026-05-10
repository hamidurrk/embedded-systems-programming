# Elevator Project — Implementation Report

Summary
- Project implements a master/slave elevator control system using Arduino MEGA (ATmega2560) as master and Arduino UNO (ATmega328P) as slave over I2C.
- Master responsibilities: keypad input, main elevator state machine (IDLE, GOING UP, GOING DOWN, DOOR OPENING, DOOR CLOSING, OBSTACLE DETECTION, FAULT), LCD output, sonar sampling, request queuing, power/sleep management, and sending runtime commands over I2C.
- Slave responsibilities (UNO): handle runtime commands from master to drive LEDs, buzzer, fan, dot-matrix, perform local tests, and manage melodies/obstacle patterns.

Implemented Functionalities (mapped to project requirements)
- Keypad control (floor 0–99)
  - Non-blocking keypad input, accumulator with digits and confirm (`#`). See `input/keypad_input.c`.
  - Supports special keys: `*` clears input, a configurable `ELEVATOR_OBSTACLE_KEY` triggers obstacle when allowed.

- State machine (master on MEGA)
  - States implemented: `STATE_IDLE`, `STATE_GOING_UP`, `STATE_GOING_DOWN`, `STATE_DOOR_OPENING`, `STATE_DOOR_CLOSING`, `STATE_OBSTACLE_DETECTION`, `STATE_FAULT` in `elevator/elevator.c`.
  - Behavior per state (with entry actions and timeouts):
    - IDLE
      - Entry: sends `CMD_IDLE` to UNO, resets keypad accumulator, clears the live input buffer, and shows "Choose the floor" + current input on the LCD.
      - UNO outputs: all LEDs off, dot‑matrix cleared, fan off.
      - During: collects digit-by-digit input (0–99). `*`/`#` clear the live input line; `#` confirms a valid floor. Confirmed floors are enqueued.
      - If queue has a request, transitions immediately to `GOING_UP` or `GOING_DOWN` based on target vs current floor.
      - Sleep trigger: while staying in IDLE, holding `ELEVATOR_SLEEP_TRIGGER_KEY` (`C`) for `ELEVATOR_SLEEP_TRIGGER_HOLD_MS` (2000ms) enters sleep mode and sends `CMD_SLEEP` to UNO.

    - GOING_UP
      - Entry: sends `CMD_MOVING_UP` and shows current floor on LCD.
      - UNO outputs: green LED on, dot‑matrix shows up arrow, fan on, moving melody selected.
      - During: every `ELEVATOR_MOVE_ONE_FLOOR_MS` (1000ms), increments `s_current_floor`, updates LCD, and checks if target reached.
      - On arrival: clears target flag and transitions to `DOOR_OPENING`.

    - GOING_DOWN
      - Entry: sends `CMD_MOVING_DOWN` and shows current floor on LCD.
      - UNO outputs: green LED on, dot‑matrix shows down arrow, fan on, moving melody selected.
      - During: every `ELEVATOR_MOVE_ONE_FLOOR_MS` (1000ms), decrements `s_current_floor`, updates LCD, and checks if target reached.
      - On arrival: clears target flag and transitions to `DOOR_OPENING`.

    - DOOR_OPENING
      - Entry: sends `CMD_DOOR_OPEN` and shows "Door open" on LCD.
      - UNO outputs: blue LED on, dot‑matrix shows full square, fan off, door melody selected.
      - During: waits `ELEVATOR_DOOR_OPEN_MS` (3000ms) then transitions to `DOOR_CLOSING`.

    - OBSTACLE_DETECTION
      - Entry: sends `CMD_OBSTACLE` and `CMD_PLAY_MELODY`, shows "Obstacle detected" on LCD.
      - UNO outputs: red LED blinks (6 toggles at 800ms intervals), dot‑matrix shows red circle, fan off, obstacle melody selected.
      - During: any key press stops the melody (`CMD_STOP_MELODY`) and transitions to `DOOR_CLOSING`.

    - DOOR_CLOSING
      - Entry: sends `CMD_DOOR_CLOSE`, shows "Door closing" on LCD, and resets sonar tracking so the first sonar sample happens immediately.
      - UNO outputs: yellow LED on, dot‑matrix cleared, fan off.
      - During:
        - If `ELEVATOR_OBSTACLE_FALLBACK_ENABLED` is enabled and the obstacle key is pressed, transitions to `OBSTACLE_DETECTION`.
        - Otherwise, samples sonar every `ELEVATOR_SONAR_SAMPLE_MS` (100ms). If distance stays below `ELEVATOR_OBSTACLE_THRESHOLD_CM` (10cm) for `ELEVATOR_OBSTACLE_STREAK_REQUIRED` (3) samples, transitions to `OBSTACLE_DETECTION`.
        - If no obstacle and `ELEVATOR_DOOR_CLOSE_MS` (2000ms) elapses, transitions to next queued request or `IDLE`.

    - FAULT
      - Entry: sends `CMD_FAULT` and shows "Same floor" on LCD.
      - UNO outputs: red LED on (steady), dot‑matrix shows checker pattern, fan off.
      - During: after `ELEVATOR_FAULT_MS` (1000ms), returns to `IDLE`.

- LCD feedback
  - Implemented via `display/` module. Functions include `display_show_idle`, `display_show_floor`, `display_show_door_open`, `display_show_door_closing`, `display_show_obstacle`, `display_show_same_floor_fault`.
  - Master logs LCD writes to UART (useful for debugging).

- LED indicators (UNO)
  - LED control API in `led/led.c` with helper patterns: idle, moving (green), door open (blue), door close (yellow), obstacle (red blinking), fault (red steady). UNO `elevator.c` applies LED patterns on received runtime commands.

- Buzzer and melodies (UNO)
  - `buzzer/` implements tone playback and non-blocking melody playback via `buzzer_task()`.
  - Multiple melodies implemented: moving melody, door melody (Nokia), obstacle melody (arranged snippet), selectable via `buzzer_select_melody()`.
  - Obstacle detection triggers a melody of many notes; melody stops when any keypad button is pressed (master detects key press and sends `CMD_STOP_MELODY`).

- Obstacle detection
  - Two mechanisms: manual obstacle key via keypad and sonar sensor sampling on master during door closing.
  - Sonar threshold and streak tracking implemented in master (`elevator.c`) to trigger obstacle detection during DOOR_CLOSING.

- Queueing (additional functionality)
  - Master maintains a floor request queue (`s_floor_queue`) with enqueue/dequeue logic and processes requests FIFO.

- Energy saving (additional functionality)
  - Master supports a sleep mode: configurable `ELEVATOR_SLEEP_MODE_ENABLED` and key-hold to enter sleep. While sleeping, the MEGA reduces peripherals (PRRx), sends `CMD_SLEEP` to UNO, and wakes on keypad/TWI.
  - Slave UNO handles `CMD_SLEEP`/`CMD_WAKE` — it can power down and be woken by I2C address match.

- Robust I2C
  - Master includes bus recovery logic (`i2c_master_bus_recover()`) to clock-stretch/free stuck I2C.
  - Master sends one-byte runtime commands to UNO (`i2c_master_send_command()`), using defined `elevator_commands.h` constants.
  - Slave handles commands in ISR-driven approach and processes them in `i2c_slave_task()`.

- Non-blocking and test modes
  - Both firmwares support a TEST MODE: hold a boot key at startup to enter test routines (`test/` modules). See `app/app.c` in both projects.
  - UNO supports test commands (LED, BUZZER, FAN, DOTMATRIX) sent by master for validation.

- Additional output devices and features
  - Dot-matrix display driver (`dotmatrix/`) on UNO for visual indicators (arrows, patterns).
  - Fan control on UNO used as an occupant/ambient indicator during movement.
  - UART debug logging across both boards for development and troubleshooting.

Architecture & Data Flow
- Master (MEGA) — primary controller
  - Reads keypad, sonar, and manages the main elevator state machine and floor queue.
  - Updates LCD and logs to UART.
  - Sends simple run-time commands via I2C to UNO using `CMD_*` codes defined in `comm/elevator_commands.h`.
  - Commands include: `CMD_MOVING_UP`, `CMD_MOVING_DOWN`, `CMD_DOOR_OPEN`, `CMD_DOOR_CLOSE`, `CMD_OBSTACLE`, `CMD_PLAY_MELODY`, `CMD_STOP_MELODY`, `CMD_FAULT`, `CMD_SLEEP`, `CMD_WAKE`, and test commands.

- Slave (UNO) — actuator & local UI controller
  - Receives runtime commands via I2C ISR and runs `i2c_slave_task()` to apply outputs.
  - Drives LEDs, buzzer (melodies), fan, dot-matrix and provides local behavior such as obstacle LED blink patterns and melody playback.

Workflow / Sequence Example
1. System boots: both `app_init()` run on MEGA and UNO; display shows idle.
2. User types digits on keypad (MEGA) and confirms with `#`.
3. MEGA enqueues request; if idle, it enters GOING_UP or GOING_DOWN and sends `CMD_MOVING_UP`/`CMD_MOVING_DOWN` to UNO.
4. UNO turns on movement outputs (green LED, fan, moving melody) and displays arrow on dot-matrix.
5. MEGA updates LCD periodically with `display_show_floor()`.
6. On arrival MEGA enters DOOR_OPENING: sends `CMD_DOOR_OPEN` and after open time either closes or enters OBSTACLE_DETECTION (on sonar or key).
7. OBSTACLE_DETECTION: MEGA sends `CMD_OBSTACLE` and `CMD_PLAY_MELODY`; UNO blinks LED pattern and plays obstacle melody. Melody stops when master detects a key press and sends `CMD_STOP_MELODY`.
8. On DOOR_CLOSING, UNO sets yellow LED; sonar checks may re-trigger obstacle; otherwise, MEGA proceeds to next queued request or IDLE.
9. Sleep: supported via long key-hold; MEGA sends `CMD_SLEEP` and disables many peripherals; UNO can be awoken by I2C address match.

How to Use (build & upload)
- Open the project in Microchip Studio:
  - Master: [Mega_Elevator.cproj](Mega_Elevator/Mega_Elevator.cproj)
  - Slave: [Uno_Elevator.cproj](Uno_Elevator/Uno_Elevator.cproj)
- Build and flash each board using Microchip Studio's build/upload commands. Ensure correct programmer and COM port.
- Normal run:
  - Power both boards.
  - On MEGA LCD, follow prompt "Choose the floor".
  - Enter floor digits (0–99), press `#` to confirm.
  - To test obstacle behavior, either press configured obstacle key (special char) while door open or let sonar detect obstacle during closing.
  - To enter TEST mode: hold the configured boot/test key at startup (as in `app/app.c`).

Code Layout (high-level)
- [Mega_Elevator](Mega_Elevator/)
  - `main.c` — initialization entry point.
  - `app/` — `app.c`, `app.h`: app-level init and task scheduling (normal/test).
  - `comm/` — `i2c_master.c`, `i2c_master.h`, `elevator_protocol.h`, `elevator_commands.h`: I2C master code and command definitions.
  - `display/` — `display.c`, `display.h`, LCD wrappers.
  - `elevator/` — `elevator.c`, `elevator.h`: master state machine, request queue, sonar handling, sleep.
  - `input/` — `keypad_input.c`, `keypad_input.h`: keypad parsing and non-blocking API.
  - `sonar/` — sonar sensor handling.
  - `config/` — pins, timing, compile-time feature flags.
  - `test/` — test harnesses used in test mode.

- [Uno_Elevator](Uno_Elevator/)
  - `main.c` — initialization entry point.
  - `app/` — `app.c`: app init, runs `i2c_slave_task()`.
  - `comm/` — `i2c_slave.c`, `i2c_slave.h`, `elevator_commands.h`: ISR-based I2C slave and command handler.
  - `buzzer/` — `buzzer.c`, `buzzer.h`: tone generation, melody sequences, non-blocking melody engine.
  - `led/` — LED helpers: on/off, blink patterns.
  - `dotmatrix/` — dot matrix display driver and patterns.
  - `fan/` — fan control for movement indicator.
  - `elevator/` — local actuator-state handling reacting to commands (applies outputs and patterns).
  - `test/` — unit test helpers used when master triggers test commands.

Notable Implementation Details
- Non-blocking design: keypad, buzzer melody, and I2C slave processing avoid long blocking delays — `buzzer_task()` is tick-driven so melodies can run without halting other tasks.
- Sonar sampling occurs while DOOR_CLOSING to detect obstacles dynamically.
- The queue supports multiple stored floor requests and processes FIFO.
- Sleep mode reduces peripherals via PRR registers on MEGA; I2C re-initialization after wake.
- I2C bus recovery implemented in master in case SDA gets stuck by slave.
- Test mode and individual device test commands exist for system validation.
- Rich debug logging via UART across both firmwares.

Files of Interest (quick links)
- Master (MEGA):
  - [Mega_Elevator/app/app.c](app/app.c)
  - [Mega_Elevator/elevator/elevator.c](elevator/elevator.c)
  - [Mega_Elevator/input/keypad_input.c](input/keypad_input.c)
  - [Mega_Elevator/display/display.c](display/display.c)
  - [Mega_Elevator/comm/i2c_master.c](comm/i2c_master.c)
  - [Mega_Elevator/comm/elevator_commands.h](comm/elevator_commands.h)

- Slave (UNO):
  - [Uno_Elevator/app/app.c](../Uno_Elevator/Uno_Elevator/app/app.c)
  - [Uno_Elevator/comm/i2c_slave.c](../Uno_Elevator/Uno_Elevator/comm/i2c_slave.c)
  - [Uno_Elevator/buzzer/buzzer.c](../Uno_Elevator/Uno_Elevator/buzzer/buzzer.c)
  - [Uno_Elevator/elevator/elevator.c](../Uno_Elevator/Uno_Elevator/elevator/elevator.c)
  - [Uno_Elevator/led/led.c](../Uno_Elevator/Uno_Elevator/led/led.c)

Hardware Pinouts
- Arduino MEGA (master):
  - Sonar trigger: `SONAR_TRIG` = D33 (mapped to PC4 in code)
  - Sonar echo: `SONAR_ECHO` = D35 (mapped to PC2 in code)
  - Keypad: uses PORTK (higher nibble = rows, lower nibble = cols) via the keypad library (`lib/keypad/keypad.h`).
  - I2C / TWI: uses the hardware TWI peripheral (TWBR/TWCR). The master includes a software bus-recovery path that toggles the SCL/SDA pins directly (see `comm/i2c_master.c` comments).

- Arduino UNO (slave):
  - Dot-matrix (MAX7219 interface): `DOTMATRIX_DIN` = D11, `DOTMATRIX_CS` = D12, `DOTMATRIX_CLK` = D13
  - Buzzer: `BUZZER` = D6
  - Fan: `FAN` = D8
  - Status LEDs: `LED_RED` = D2, `LED_BLUE` = D3, `LED_YELLOW` = D4, `LED_GREEN` = D5

Dot‑Matrix (how it works)
- The UNO drives a MAX7219-based 8x8 dot-matrix (`dotmatrix/`): pre-defined 8-byte patterns represent symbols (up/down arrows, circle, checker, full square).
- `dotmatrix_show_rows()` converts the 8 bytes into an 8x8 boolean buffer and calls `max7219_show_rotated_clockwise()` to write the buffer to the display.
- `dotmatrix_print_message()` composes characters into a virtual buffer using `max7219_draw_char()` and scrolls the display by repeatedly shifting the draw offset and calling `max7219_show_rotated_clockwise()` with a short delay — this scrolling loop is blocking by design and used for explicit message display/test modes.

Non‑blocking buzzer implementation
- The UNO `buzzer/` module provides both blocking and non-blocking APIs:
  - Blocking: `buzzer_tone(hz)` and `buzzer_start_melody()` perform synchronous playback (used in quick tests).
  - Non-blocking (engine): `buzzer_set_melody_loop(enabled)` enables a looping melody, and `buzzer_task()` advances the melody on each millisecond tick.
- Runtime state for non-blocking melody is stored in module-level variables:
  - `g_melody_loop_enabled` — whether the melody engine is active.
  - `g_melody_phase_is_gap`, `g_melody_note_index`, `g_melody_phase_remaining_ms` — current phase and timing.
  - `g_current_melody_hz` / `g_current_melody_dur` / `g_current_melody_count` — pointers/size for the selected melody.
- Integration: the UNO `elevator_task()` sets `buzzer_set_melody_loop()` based on the active command and calls `buzzer_task()` periodically (via a 1ms tick when melody is enabled). This keeps melody playback interleaved with other tasks (LED updates, dot‑matrix, fan) without long blocking delays.

Queueing implementation (master)
- The MEGA maintains a simple circular FIFO queue for floor requests in `elevator/elevator.c`:
  - Storage: `s_floor_queue[ELEVATOR_QUEUE_CAPACITY]`, capacity defined in `config/config.h` (`ELEVATOR_QUEUE_CAPACITY` = 32).
  - Indices/counters: `s_queue_head`, `s_queue_tail`, `s_queue_count`.
  - Operations:
    - `elevator_queue_enqueue(floor)` places `floor` at `s_queue_tail`, advances `s_queue_tail = (s_queue_tail+1) % CAP`, increments `s_queue_count` (returns 0 on full).
    - `elevator_queue_dequeue(&floor_out)` reads from `s_queue_head`, advances the head and decrements count (returns 0 if empty).
    - `elevator_queue_contains(floor)` checks for duplicates by iterating `s_queue_count` entries (prevents duplicate requests).
- This FIFO ensures multiple floor requests can be stored while the elevator completes prior requests; the master calls `elevator_start_next_request_or_idle()` to dequeue the next target when ready.

Sonar operation (master)
- The sonar module (`sonar/sonar.c`) implements an HC-SR04‑style measurement using a trigger and echo pin:
  - Pins: `SONAR_TRIG` and `SONAR_ECHO` are defined in `config/pins.h` (D33 -> PC4, D35 -> PC2 on the MEGA board mapping used here).
  - Procedure: `sonar_measure_distance_cm()` pulses the trigger pin for 10µs, waits for echo high, starts Timer1 (prescaler=8, tick=0.5µs), waits while echo is high, records `TCNT1` ticks, stops timer.
  - Conversion: ticks are converted to centimeters using `SONAR_TICKS_PER_CM` (116 ticks ≈ 1 cm round‑trip with tick=0.5µs), with an echo timeout to avoid blocking forever.
  - Integration: the elevator master samples sonar periodically while in `STATE_DOOR_CLOSING` (configured by `ELEVATOR_SONAR_SAMPLE_MS`) and maintains a short streak counter (`s_obstacle_below_threshold_streak`) before declaring an obstacle to reduce false positives.


Limitations & Notes
- Safety: This is a simulation/educational project. Physical deployment requires hardware safety checks (limit switches, failsafe relays, certified sensors).
- Timing: movement timing and sonar thresholds set via `config/timing.h` and `config/config.h` — tune for your hardware.
- I2C commands are one-byte; expand protocol if you need richer telemetry (ACKs, statuses).

Suggestions / Next steps
- Add telemetry from UNO back to MEGA (current floor confirmation, door sensor) to close feedback loop.
- Add persistent storage of queued requests if power-loss recovery is needed.
- Add unit tests and CI-targeted host mocks for keypad/I2C to enable automated testing of logic.

---
Report generated from code inspection of the workspace on May 10, 2026. If you want, I can:
- Add diagrams (mermaid) for architecture.
- Produce a short README with build/upload steps for Microchip Studio.
- Run a sweep to extract all config constants (`config/` headers) into a one-page tuning guide.

