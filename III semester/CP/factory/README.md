## Multi-threaded factory task scheduler

A multi-threaded task scheduling engine that manages asynchronous computation tasks across stations with 
varying capacity constrains and workers with dynamic shift schedules.

> **Note:** My code is located in `factory/solution/plant/solution.c`.
All other files were provided by the course coordinator as a base code and documentation.

- - -

### My contribution

The core goal of this project was to implement concurrent, deadlock-free factory engine capable of processing
numbers of tasks submitted by multiple threads simultaneously.

**Functions::**
* `init_plant`: initializes the factory with station capacities and expected workers count,
* `destroy_plant`: cleans up global resources after waiting for active tasks executions to finish,
* `add_worker`: registers a new worker into the system,
* `add_task`: schedules a processing task for execution,
* `collect_task`: blocks calling threads until a specific task completes and retrieves its status.


**Key features**
* **Thread safety:** functions (`add_task`, `collect_task` and `add_worker`) are fully thread-self and can be called concurrently from multiply threads.
* **Immediate failure detection:** designed an early rejection mechanism in `add_task`: if task can never be fulfilled due to
  a lack of staff or structural limitations, it terminates immediately with `-1`.
* **Lifecycle managements & cleanup:** `destroy_plant` blocks until all active tasks are finish executing and registered worker shifts end.
  Returning `-1` for any tasks/workers arriving after destruction.


- - -

### Repository structure
```
plant/                 # Primary deliverable (My implementation of the scheduler)
  * CMakeLists.txt     # Sub-project CMake configuration
  * solution.c         # Thread-safe factory, worker, and task management
common/                # Shared C headers and data structure definitions
  * CMakeLists.txt
  * err.c
  * err.h
  * plant.h            # Public API contracts (task_t, worker_t, plant interface)
demo/                  # Demo client application and usage example
  * main.c
  * CMakeLists.txt
CMakeLists.txt
```
- - -
### Compilation & run

``` bash
cmake -S solution/ -B build/ -DCMAKE_BUILD_TYPE=Release
cd build/
make ./demo/demo 
