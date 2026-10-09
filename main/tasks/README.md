# Tasks

Overview of every FreeRTOS task in the OBC flight software thus far.

## Task Summary

| Task | Name in FreeRTOS | Source | Priority | Stack (words) | Core |
|------|------------------|--------|----------|---------------|------|
| USB Serial | `USB` | `usb_serial/` | `configMAX_PRIORITIES - 2` | 256 | 0 |
| Scheduler (STEVE) | `SCHED` | `steve/` | `tskIDLE_PRIORITY` | 2048 | Any |
| GSE / Debug | `DEBUG` | `gse/` | `tskIDLE_PRIORITY` | 2048 | Any |
| Logging | `LOGGING` | `utilities/src/log.c` | `tskIDLE_PRIORITY` | 2048 | 1 |
| CAN | TODO | `can/` | TODO | TODO | TODO |
| Misc | TODO | `misc/` | TODO | TODO | TODO |

### STEVE Scheduler (`steve/`)

#### Purpose

STEVE is a cooperative job scheduler that runs inside a single FreeRTOS task (`SCHED`). Instead of creating a FreeRTOS task for every periodic or one-off action, modules register lightweight **jobs** (a function pointer plus timing info), and STEVE runs each one when its scheduled tick arrives.

#### How It Works

- `scheduler_task()` is the entry point created in `main.c`. It calls `scheduler_init()` (resets the job count and creates `job_mutex`), runs any job setup (`sensor_setup()`), and then enters `run_scheduler()`, which never returns.
- Jobs are stored in `global_job_context.jobs[]`, an array of up to `MAX_JOBS` (10) **pointers** to `jobs_t` structs. The structs themselves are owned by the module that defines them and must be `static` or global.
- Each loop, `run_scheduler()`:
  1. Reads the current tick with `xTaskGetTickCount()`.
  2. Calls `find_ready_job()`, which returns the **first** job in array order whose `execute_time <= current_time`.
  3. If no job is ready, sleeps for 10 ms (`vTaskDelay`) and checks again.
  4. Otherwise it runs the job's `func(args)` inline, to completion, in the `SCHED` task.
  5. If the job is recurring (`recurr_time != 0`), it reschedules it for `current_time + recurr_time`. If it is one-shot (`recurr_time == 0`), it removes it with `delete_job()`.
- Deleting shifts every later job one slot left, so array order (insertion order) is preserved. Array position acts as the tie-break priority when several jobs are due on the same tick.
- All times are in FreeRTOS ticks. With `configTICK_RATE_HZ = 1000`, one tick is 1 ms.

#### The `jobs_t` Struct

| Field | Type | Description |
|-------|------|-------------|
| `recurr_time` | `TickType_t` | Period in ticks for a recurring job; `0` = run once, then delete |
| `execute_time` | `TickType_t` | Absolute tick at which the job next becomes due |
| `name` | `char[20]` | Human-readable name (used in log messages) |
| `func` | `job_function` | `void (*)(void *args)` called when the job runs |
| `args` | `void *` | Argument passed to `func` (may be `NULL`) |

#### Public API

| Function | Description |
|----------|-------------|
| `scheduler_task(void *)` | FreeRTOS task entry point; initializes and runs the scheduler |
| `add_job(jobs_t *job)` | Appends a job to the end of the job array (mutex-protected) |
| `delete_job(jobs_t *job)` | Removes a job and shifts the remaining jobs left (mutex-protected) |
| `run_scheduler(void)` | Main scheduling loop (called by `scheduler_task`, does not return) |

#### Adding a Job

1. Write the job function with the signature `void my_job(void *args)`. Keep it short and non-blocking, because it runs inside the scheduler task.
2. Define a **static or global** `jobs_t` for it (never on the stack):

   ```c
   jobs_t my_job_def = {
       .func         = my_job,
       .recurr_time  = 1000,  // every 1000 ticks; 0 = one-shot
       .execute_time = 0,     // first run: as soon as possible
       .name         = "My Job",
       .args         = NULL,
   };
   ```

3. Register it with `add_job(&my_job_def);`, either in `scheduler_task()` before `run_scheduler()`, or from another task after the scheduler has started.

#### Jobs (`steve/jobs/`)

`sensor_job.c` currently holds LED test jobs on `PICO_DEFAULT_LED_PIN`:

| Job | Type | First Run (ticks) | Behavior |
|-----|------|-------------------|----------|
| `led_blinking_once` | One-shot | 1000 | Turns LED off |
| `led_blinking_onoff` | One-shot | 2000 | LED on, hold 7 s, off |
| `led_blinking_fast_blink` | One-shot | 3000 | Two fast blinks, 1 s hold, two fast blinks |
| `led_blinking_recurr` | Recurring (500) | 4000 | Toggles LED every 500 ticks |

These are currently commented out in `scheduler_task()`. `sensor_setup()` configures the LED GPIO.

A GSE test job, `test_steve` in `gse/gse.c`, logs a message every 1000 ticks. It can be added and removed over serial with `send_steve` and `remove_steve_task`.

#### Thread / Core Safety

- `add_job()`, `delete_job()` and the search in `find_ready_job()` take `job_mutex`, so jobs can be added or removed from other tasks on either core (for example, the GSE task).
- `reorganize_job()` does **not** take the mutex. It must only be called from inside `delete_job()`, which already holds it. `job_mutex` is non-recursive, so taking it again there would deadlock.
- `scheduler_init()` must run before any call to `add_job()` or `delete_job()`, or `job_mutex` is `NULL`.
- Job functions run with the mutex **released**, so a job may call `add_job()` or `delete_job()` itself.

### GSE / Debug (`gse/`)

#### Purpose

The GSE (Ground Support Equipment) task is the text command interface over USB serial. It reads lines typed into a serial monitor, parses them into commands, and acts on them. It also owns the global **debug mode** flag, which gates whether the logging system records and prints anything.

#### Serial Commands

Commands end with Enter (`\n` or `\r`), and matching is exact and case-sensitive.

| Command | Description |
|---------|-------------|
| `debug` | Turns debug mode on; log messages start streaming live over serial |
| `no_debug` | Turns debug mode off; new log messages are dropped and the log queue is cleared |
| `pull_log` | Recognized by the parser but **not implemented** (falls through to `default`) |
| `send_steve` | Adds the `test_steve` job to STEVE (logs a message every 1000 ticks) |
| `remove_steve_task` | Removes the `test_steve` job from STEVE |

Unknown commands are silently ignored.

#### How It Works

- **Startup:** `gse_init()` is called in `main()` before the scheduler starts. It creates the debug mode mutex (`debug_mode_init()`) and initializes logging (`log_init()`).
- **Task loop (`vDebugTask`):**
  1. If no serial host is connected (`safe_tud_cdc_connected()` is false), it sleeps `GSE_TASK_DELAY_MS` (10 ms) and checks again.
  2. Otherwise it reads one character without blocking (`getchar_timeout_us(0)`).
  3. Characters are appended to a 256-byte line buffer (`GSE_BUFFER_SIZE`). Input past 255 characters is dropped.
  4. On `\n` or `\r`, the buffer is null-terminated, passed to `parse_command()`, dispatched through a `switch`, and then reset.
  5. It sleeps 10 ms between characters.
- **Debug mode:**
  - Stored in `debug_mode` and protected by `debug_mode_mutex`.
  - Defaults to **on** in Debug builds (`DEBUG_BUILD`, set by CMake for the Debug config) and **off** otherwise.
  - Read by other modules through `get_debug_mode()`. The logger checks it in `log_to_queue()` and `log_task()`.

#### Public API

| Function | Description |
|----------|-------------|
| `gse_init(void)` | Creates the debug mutex and initializes logging; call once from `main()` |
| `vDebugTask(void *)` | FreeRTOS task entry point (created as `DEBUG` in `main.c`) |
| `debug_mode_init(void)` | Creates `debug_mode_mutex` if needed; safe to call repeatedly |
| `get_debug_mode(void)` | Thread-safe read of the debug mode flag |

#### Adding a Command

1. Add a value to the `Command` enum (before `CMD_UNKNOWN`).
2. Add a `strcmp` line in `parse_command()` mapping the typed string to it.
3. Add a `case` in the `switch` in `vDebugTask`, ending with `break;`.
4. Add a row to the command table above.
