# RoboMAZ — Real-Time OS: Deep Learning Guide & Implementation Plan

> **Target hardware:** ESP32-S3 (dual-core Xtensa LX7, 240 MHz)  
> **RTOS:** FreeRTOS (already bundled in ESP-IDF — you are already using it, just not fully yet)  
> **Goal:** Transform the current single-loop `app_main` into a proper multi-task real-time system

---

## Part 1 — What Is "Real-Time"?

### 1.1 The Core Idea

"Real-time" does **not** mean "fast". It means **predictable** and **deadline-guaranteed**.

A system is real-time if it can guarantee that a specific action happens within a specific time window — every single time, no matter what else is running.

| Type | Meaning | Example |
|---|---|---|
| **Hard real-time** | Missing the deadline is a system failure | Airbag sensor: 5 ms or the bag doesn't deploy |
| **Soft real-time** | Missing occasionally degrades quality but isn't fatal | Video streaming: late frame = stutter |
| **Firm real-time** | Missing deadline makes the result useless, but system survives | Sensor reading: too late = discard it |

For RoboMAZ, most tasks are **soft/firm real-time**. The IMU must be read every ~10 ms. The motor command must be updated every ~20 ms. Missing one cycle is fine; missing many causes the robot to jerk or drift.

### 1.2 The Problem With Your Current Code

Your current `app_main` looks like this internally:

```
app_main()
│
├── init everything
│
└── while(true)
    ├── read compass        ← might take 2 ms
    ├── read IMU            ← might take 3 ms
    ├── mpuDriveRobot()     ← instant
    ├── update LCD          ← might take 5 ms (I2C slow)
    └── delay_ms(100)       ← blocks everything
```

**Problems:**
- If the LCD takes 8 ms instead of 5 ms, the IMU read is late. No one compensates.
- While `delay_ms(100)` is running, **nothing** else can happen — no Bluetooth, no LED update, nothing.
- Adding Bluetooth later means jamming it into this same loop. The loop gets longer, everything gets slower.
- There is no priority: a blinking LED has the same urgency as a motor command.

### 1.3 The RTOS Solution

An RTOS gives you **tasks** (threads). Each task has its own stack, its own loop, and a **priority**. The RTOS **scheduler** decides who runs when.

```
Task: IMU_Task        (Priority 5 — HIGH)   runs every 10 ms
Task: Motor_Task      (Priority 5 — HIGH)   runs every 20 ms
Task: Compass_Task    (Priority 3 — MEDIUM) runs every 100 ms
Task: LCD_Task        (Priority 2 — LOW)    runs every 200 ms
Task: Bluetooth_Task  (Priority 4)          runs when data arrives
Task: LED_Task        (Priority 1 — IDLE)   runs whenever nothing else runs
```

The scheduler runs the highest-priority task that is ready. When a task calls `vTaskDelayUntil()`, it blocks and the scheduler immediately runs the next ready task. No CPU time is wasted.

---

## Part 2 — FreeRTOS Core Concepts (with ESP-IDF examples)

### 2.1 Tasks

A **task** is just a C function that never returns — it has an infinite loop inside.

```c
void imu_task(void *pvParameters) {
    while (true) {
        // read IMU
        // send data to queue
        vTaskDelay(pdMS_TO_TICKS(10));  // release CPU for 10 ms
    }
}
```

You create it with:
```c
xTaskCreate(
    imu_task,       // function
    "IMU",          // name (for debugging)
    4096,           // stack size in bytes
    NULL,           // parameter passed to function
    5,              // priority (higher = more important)
    NULL            // optional handle to control the task later
);
```

On ESP32-S3 (dual-core), you can pin a task to a core:
```c
xTaskCreatePinnedToCore(imu_task, "IMU", 4096, NULL, 5, NULL, 1); // core 1
```

### 2.2 The Scheduler & Priorities

FreeRTOS uses **preemptive scheduling**: if a higher-priority task becomes ready, it **immediately interrupts** the lower-priority task.

```
Time →
Core0: [LCD_Task running] → IMU timer fires → [IMU_Task preempts] → [IMU done] → [LCD resumes]
```

Priority numbers: higher number = higher priority. In ESP-IDF, `configMAX_PRIORITIES` is typically 25. Use values 1–10 for your application tasks. Leave 0 for idle.

**Rule of thumb for RoboMAZ:**
```
Priority 5 → IMU reading (must be fresh)
Priority 5 → Motor command (must be responsive)
Priority 4 → Bluetooth receive (user command in)
Priority 3 → Compass reading
Priority 2 → LCD display
Priority 1 → LED animation
```

### 2.3 Queues — How Tasks Talk to Each Other

Tasks run concurrently. You **never** share a raw global variable between tasks without protection — it causes race conditions (one task reads while another is mid-write).

A **queue** is a thread-safe FIFO buffer:

```c
// Define a data structure
typedef struct {
    float pitch;
    float roll;
} ImuData_t;

// Create a queue that holds 5 ImuData_t items
QueueHandle_t imu_queue = xQueueCreate(5, sizeof(ImuData_t));

// Producer (IMU task): send data
ImuData_t data = { .pitch = pitch, .roll = roll };
xQueueSend(imu_queue, &data, 0);  // 0 = don't wait if full

// Consumer (Motor task): receive data
ImuData_t received;
if (xQueueReceive(imu_queue, &received, pdMS_TO_TICKS(10))) {
    // use received.pitch, received.roll
}
```

No locks needed. FreeRTOS handles all the synchronization internally.

### 2.4 Semaphores & Mutexes — Protecting Shared Resources

The I2C bus is **shared** between the IMU, compass, and LCD. If two tasks try to use it simultaneously, the data gets corrupted.

A **mutex** (mutual exclusion) ensures only one task uses the bus at a time:

```c
SemaphoreHandle_t i2c_mutex = xSemaphoreCreateMutex();

// In any I2C task:
if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    // safe to use I2C bus
    compass.read(heading);
    xSemaphoreGive(i2c_mutex);  // MUST release
}
```

A **binary semaphore** is used for signaling ("event happened"):

```c
SemaphoreHandle_t bt_data_ready = xSemaphoreCreateBinary();

// Bluetooth ISR or callback: signal that data arrived
xSemaphoreGiveFromISR(bt_data_ready, NULL);

// Motor task: wait for signal
xSemaphoreTake(bt_data_ready, portMAX_DELAY);
// now process Bluetooth command
```

### 2.5 Timers

FreeRTOS software timers call a function periodically without needing a dedicated task:

```c
TimerHandle_t led_timer = xTimerCreate(
    "LED",
    pdMS_TO_TICKS(500),   // period
    pdTRUE,               // auto-reload (repeating)
    NULL,
    led_blink_callback    // function called each period
);
xTimerStart(led_timer, 0);
```

Use timers for lightweight periodic work (toggling an LED). Use tasks for heavier periodic work (reading sensors).

### 2.6 vTaskDelayUntil — Precise Timing

`vTaskDelay(100ms)` means "sleep for 100 ms after I finish my work". If the work takes 5 ms, the actual period is 105 ms — it drifts.

`vTaskDelayUntil()` means "wake me up at the next 100 ms boundary, regardless of how long I took":

```c
void imu_task(void *pvParameters) {
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        // do the work (even if it takes variable time)
        read_imu();
        send_to_queue();

        // sleep until exactly 10 ms after last_wake, then update last_wake
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}
```

This is what makes a loop genuinely **periodic** — essential for control systems.

### 2.7 The Two Cores of ESP32-S3

ESP32-S3 has **two cores**: Core 0 (Protocol CPU) and Core 1 (Application CPU).

By default, ESP-IDF runs Wi-Fi/BT on Core 0 and your `app_main` on Core 1.

Strategy for RoboMAZ:
```
Core 0 (Protocol CPU):
  - Bluetooth stack (handled automatically by ESP-IDF)
  - Bluetooth_Task (pinned here, close to the stack)

Core 1 (Application CPU):
  - IMU_Task       (time-critical)
  - Motor_Task     (time-critical)
  - Compass_Task
  - LCD_Task
  - LED_Task
```

---

## Part 3 — Architecture Design for RoboMAZ

### 3.1 The Data Flow

```
┌─────────────┐     Queue: imu_queue      ┌──────────────┐
│  IMU_Task   │ ─────────────────────────▶│  Motor_Task  │
│ (10 ms)     │                           │  (20 ms)     │──▶ robot.move()
└─────────────┘                           └──────────────┘
                                                 ▲
┌─────────────┐     Queue: bt_cmd_queue          │
│  BT_Task    │ ─────────────────────────────────┘
│ (event-driv)│
└─────────────┘

┌─────────────┐     Queue: compass_queue  ┌──────────────┐
│Compass_Task │ ─────────────────────────▶│  LCD_Task    │
│ (100 ms)    │                           │  (200 ms)    │──▶ lcd.print()
└─────────────┘                 ┌─────────┘
                                │ also reads imu_queue (for display)
┌─────────────┐
│  LED_Task   │──▶ blink/color animation
│ (500 ms)    │
└─────────────┘

       All I2C tasks share: i2c_mutex
```

### 3.2 File Structure After Refactor

```
main/
├── main.cpp                  ← only: create queues/mutexes, spawn tasks, done
├── rtos/
│   ├── rtos_shared.hpp       ← queue handles, mutex handles (extern declarations)
│   ├── rtos_shared.cpp       ← actual definitions
│   ├── task_imu.cpp          ← IMU_Task
│   ├── task_motor.cpp        ← Motor_Task
│   ├── task_compass.cpp      ← Compass_Task
│   ├── task_lcd.cpp          ← LCD_Task
│   ├── task_led.cpp          ← LED_Task
│   └── task_bluetooth.cpp    ← BT_Task (added in Phase 3)
├── Motor/   (unchanged)
├── PWM/     (unchanged)
├── LCD/     (unchanged)
├── Compass/ (unchanged)
└── IMU/     (unchanged)
```

---

## Part 4 — Step-by-Step Implementation Plan

### Phase 1 — Understand the current baseline (1–2 days)

**Goal:** Read and annotate the existing code before touching anything.

1. Read `main.cpp` and draw on paper: what happens in what order, how long each step approximately takes.
2. In ESP-IDF monitor (`idf.py monitor`), add `ESP_LOGI` timestamps around each operation to measure real durations.
3. Answer these questions for yourself:
   - Which operations share the I2C bus?
   - What is the current loop period? (100 ms delay + execution time)
   - What would break if IMU and LCD ran truly concurrently?

**No code changes in Phase 1.**

---

### Phase 2 — Create shared infrastructure (Day 2–3)

Create `rtos/rtos_shared.hpp` and `rtos/rtos_shared.cpp`.

**`rtos_shared.hpp`:**
```cpp
#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// ── Shared data types ─────────────────────────────────────────
struct ImuData_t  { float pitch; float roll; };
struct CompassData_t { float heading; };

// Commands from Bluetooth (Phase 3 placeholder)
enum class BtCmd : uint8_t { STOP, FORWARD, BACKWARD, LEFT, RIGHT, ROTATE_CW, ROTATE_CCW };
struct BtCommand_t { BtCmd cmd; float speed; };

// ── Queue handles (defined in rtos_shared.cpp) ────────────────
extern QueueHandle_t imu_queue;      // ImuData_t,    depth 3
extern QueueHandle_t compass_queue;  // CompassData_t, depth 3
extern QueueHandle_t bt_cmd_queue;   // BtCommand_t,  depth 5

// ── Mutex handles ─────────────────────────────────────────────
extern SemaphoreHandle_t i2c_mutex;  // guards shared I2C bus

// ── Init: call once from app_main before spawning tasks ───────
void rtos_shared_init();
```

**`rtos_shared.cpp`:**
```cpp
#include "rtos_shared.hpp"

QueueHandle_t imu_queue     = nullptr;
QueueHandle_t compass_queue = nullptr;
QueueHandle_t bt_cmd_queue  = nullptr;
SemaphoreHandle_t i2c_mutex = nullptr;

void rtos_shared_init() {
    imu_queue     = xQueueCreate(3, sizeof(ImuData_t));
    compass_queue = xQueueCreate(3, sizeof(CompassData_t));
    bt_cmd_queue  = xQueueCreate(5, sizeof(BtCommand_t));
    i2c_mutex     = xSemaphoreCreateMutex();

    assert(imu_queue && compass_queue && bt_cmd_queue && i2c_mutex);
}
```

---

### Phase 3 — Extract IMU task (Day 3–4)

**`rtos/task_imu.cpp`:**
```cpp
#include "rtos_shared.hpp"
#include "../IMU/MAZMPU6050.hpp"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "IMU_Task";

extern MAZMPU6050 imu;  // defined in main.cpp

void task_imu(void* pv) {
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        ImuData_t data = {};

        if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            imu.readAngles(data.pitch, data.roll);
            xSemaphoreGive(i2c_mutex);
        } else {
            ESP_LOGW(TAG, "I2C mutex timeout");
        }

        // Overwrite old value if consumer is slow (use xQueueOverwrite for single-item queues)
        xQueueOverwrite(imu_queue, &data);

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));  // exactly 10 ms period
    }
}
```

**Key point:** `xQueueOverwrite` always writes the latest value, discarding the old one if unread. This is the right choice for sensor data — you always want the newest reading, not a stale one.

> **For single-item "latest value" queues, create them with depth=1 and use `xQueueOverwrite`.**

---

### Phase 4 — Extract Motor task (Day 4–5)

```cpp
void task_motor(void* pv) {
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        ImuData_t imu_data = {};
        BtCommand_t bt_cmd = {};

        // Check for a Bluetooth command first (higher-level intent)
        if (xQueueReceive(bt_cmd_queue, &bt_cmd, 0) == pdTRUE) {
            // Execute Bluetooth command (Phase 3)
            apply_bt_command(bt_cmd);
        } else {
            // Fall back to IMU tilt control
            if (xQueuePeek(imu_queue, &imu_data, 0) == pdTRUE) {
                mpuDriveRobot(imu_data.pitch, imu_data.roll);
            }
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(20));  // 50 Hz motor update
    }
}
```

**Note:** `xQueuePeek` reads without removing — Motor_Task can read IMU data while LCD_Task also needs it.

---

### Phase 5 — Extract Compass and LCD tasks (Day 5–6)

```cpp
// task_compass.cpp
void task_compass(void* pv) {
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        CompassData_t data = {};
        if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            compass.read(data.heading);
            xSemaphoreGive(i2c_mutex);
        }
        xQueueOverwrite(compass_queue, &data);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));  // 10 Hz
    }
}

// task_lcd.cpp
void task_lcd(void* pv) {
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        ImuData_t imu_data = {};
        CompassData_t compass_data = {};
        xQueuePeek(imu_queue, &imu_data, 0);
        xQueuePeek(compass_queue, &compass_data, 0);

        if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            // update LCD with latest data
            xSemaphoreGive(i2c_mutex);
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(200));  // 5 Hz display
    }
}
```

---

### Phase 6 — New app_main (Day 6)

After extracting all tasks, `main.cpp` becomes clean:

```cpp
extern "C" void app_main(void) {
    // 1. Hardware init
    init_led();
    lcd.init(GPIO_NUM_5, GPIO_NUM_6, 0x27);
    compass.init(lcd.busHandle());
    imu.init(lcd.busHandle());
    robot.begin();

    // 2. Create RTOS shared resources
    rtos_shared_init();

    // 3. Spawn tasks
    xTaskCreatePinnedToCore(task_imu,     "IMU",     4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(task_motor,   "Motor",   4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(task_compass, "Compass", 4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(task_lcd,     "LCD",     4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(task_led,     "LED",     2048, NULL, 1, NULL, 1);

    // 4. app_main is done — tasks run forever
    // DO NOT return — deleting app_main task causes a reset
    vTaskDelete(NULL);
}
```

**`vTaskDelete(NULL)` means "delete myself" — this is the standard pattern.** The tasks you spawned keep running forever.

---

### Phase 7 — Add Bluetooth (separate branch, Day 7+)

Once the RTOS structure is in place, adding Bluetooth is clean:

1. Enable Bluetooth in `sdkconfig` (via `idf.py menuconfig`)
2. Create `task_bluetooth.cpp` that initializes BT Classic SPP or BLE
3. When a command arrives, push a `BtCommand_t` onto `bt_cmd_queue`
4. `task_motor` already reads from `bt_cmd_queue` — it will respond immediately

No changes to any other task needed. This is the power of the queue-based architecture.

---

## Part 5 — Common Pitfalls & How to Avoid Them

### Stack overflow
Each task has its own stack. If the stack is too small, you get a corrupt crash with no obvious error.
- Start with 4096 bytes per task. If you use `sprintf`, heavy math, or C++ objects, use 8192.
- Enable stack overflow detection in `menuconfig → FreeRTOS → Check for stack overflow` (method 2).
- Call `uxTaskGetStackHighWaterMark(NULL)` to see how close to the edge you got.

### Priority inversion
If a low-priority task holds a mutex that a high-priority task needs, the high-priority task is blocked. FreeRTOS mutexes support **priority inheritance** automatically — `xSemaphoreCreateMutex()` already handles this.

### Forgetting to release the mutex
Always `xSemaphoreGive()` in every code path, including error paths. A mutex left taken will deadlock every other task that needs it.

### Using `vTaskDelay` instead of `vTaskDelayUntil`
`vTaskDelay` drifts. `vTaskDelayUntil` is periodic. Use `vTaskDelayUntil` for all sensor/control tasks.

### Calling FreeRTOS API from an ISR
Most FreeRTOS functions cannot be called from an interrupt handler. Use the `...FromISR` variants:
- `xQueueSendFromISR` instead of `xQueueSend`
- `xSemaphoreGiveFromISR` instead of `xSemaphoreGive`

---

## Part 6 — Debugging Tools

### FreeRTOS Task Inspector
```c
// Print all task states, stack usage, and CPU usage
char buf[1024];
vTaskList(buf);
printf("%s", buf);
```

### ESP-IDF SystemView
A real-time tracing tool that shows you exactly when each task runs, for how long, and what it's waiting for. Invaluable for finding timing problems. Enable in `menuconfig → Component config → SEGGER SystemView`.

### Log timestamps
```c
ESP_LOGI(TAG, "[%lu ms] IMU read done", xTaskGetTickCount() * portTICK_PERIOD_MS);
```

---

## Summary — Learning Sequence

| Day | Topic | Activity |
|---|---|---|
| 1 | What is RTOS, why it matters | Read Part 1 & 2, draw the current data flow |
| 2 | FreeRTOS tasks & scheduler | Create `rtos_shared.hpp`, study Phase 2 |
| 3 | Queues | Implement `task_imu.cpp`, verify it compiles |
| 4 | vTaskDelayUntil, priority | Implement `task_motor.cpp`, measure timing |
| 5 | Mutexes | Implement `task_compass.cpp` with i2c_mutex |
| 6 | Full refactor | Implement `task_lcd.cpp`, new `app_main` |
| 7 | Debugging | Run `vTaskList`, measure stack high-water marks |
| 8+ | Bluetooth | Add `task_bluetooth.cpp` on the clean foundation |

---

*Document created for the RoboMAZ project — ESP32-S3 RTOS refactor plan.*
