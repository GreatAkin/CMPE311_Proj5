# Project-5: PROJECT-USERTOS

An embedded control system built on **FreeRTOS 202604.01 LTS** for the ELEGOO Arduino Uno R3. It migrates the bare-metal `PROJECT-ADDMOTOR` design to an RTOS task-management framework, concurrently handling asynchronous Serial Monitor I/O, independent blinking of two LEDs, and a four-state PWM DC motor cooling fan controlled by a momentary push button.

## Hardware Setup
* **Board:** ELEGOO Arduino Uno R3 (or compatible)
* **LED 1:** Digital Pin 2 (with 200 Ω current-limiting resistor)
* **LED 2:** Digital Pin 3 (with 200 Ω current-limiting resistor)
* **Push Button:** Digital Pin 4 (momentary, wired to GND using `INPUT_PULLUP`)
* **DC Motor Fan (PWM):** Digital Pin 5 (via motor driver circuit)

## Compile
1. Open the `.ino` sketch in **Arduino IDE 2.3.3** or later.
2. Ensure **FreeRTOS 202604.01 LTS** (`Arduino_FreeRTOS.h`) is installed via **Library Manager** or **Sketch > Include Library > Add .ZIP Library...**.
3. Select **Tools > Board > Arduino AVR Boards > Arduino Uno**.
4. Click **Verify** (`Ctrl + R`) to cross-compile the firmware for the AVR ATmega328p.

## Run
1. Connect the ELEGOO Arduino Uno R3 via USB and select the active port under **Tools > Port**.
2. Click **Upload** (`Ctrl + U`) to flash the firmware to the board.
3. Open the **Serial Monitor** (`Ctrl + Shift + M`) and set the baud rate to **9600** with **Newline** line endings.
4. Follow the prompts to set independent blink intervals (in milliseconds) for LED 1 and LED 2:
   * `What LED? (1 or 2)` → enter `1` or `2`
   * `What interval (in msec)?` → enter a non-negative integer (e.g., `600`, `1600`, `0` for OFF)
5. Press the momentary push button on Pin 4 at any time to cycle the PWM fan through **OFF → LOW → MEDIUM → HIGH → OFF** without interrupting LED timing or serial input.
