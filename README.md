# Temperature Control

A room temperature controller in C, tested in closed loop against a model of the room in a [CppModel](https://www.cppmodel.com) simulation, with a verification rule for its step response.

This is the code of the tutorial [Temperature Control](https://www.cppmodel.com/docs/TemperatureControlTutorial). The tutorial walks through it step by step and shows the results.

## The loop

- **The controller** (`tempControl.c`) compares the room temperature with the set point and sets the heater power, 0–100 %. It runs as a P controller, or as a PI controller with or without anti-windup.
- **The plant model** (in `SimulateTempControl.c`) is the room: at full power the heater warms it by `HeatingRate` °C per second, and the walls lose heat to the outside at `LossRate` per second times the temperature difference.

Every 1 s step, the simulation hands the room temperature to the controller, runs it, and applies the heater power to the room. The run covers 150 minutes: a night setback of 16 °C, then 21 °C from 30 minutes on.

## Requirement and verification

| Requirement | Verification rule |
| ----------- | ----------------- |
| After a set point change, the room is within ±0.5 °C of it within 30 min, stays there for 10 min, and never overshoots it by more than 1 °C. | `settles at set point` (Settle) |

## Three controllers, one build

The controller variant is chosen by parameters, so all three run from the same build. The default is P control; the other two are given in files with `--inputs`:

```bash
./build/SimulateTempControl                               # P control
./build/SimulateTempControl --inputs pi.json              # PI control
./build/SimulateTempControl --inputs pi-antiwindup.json   # PI control with anti-windup
```

| Run | Result |
| --- | ------ |
| P control | **Fails**: the room settles at about 18.7 °C, more than 2 °C short of the set point (steady-state error). |
| PI control | **Fails**: the integral term winds up while the heater is at 100 %, and the room overshoots by 1.3 °C (integrator windup). |
| PI control with anti-windup | **Passes**: settled in 856 s, without overshoot. |

```
CppModel.Verify.settles at set point: FAIL change at t=1800 s: not settled within 7200 s (limit 1800 s), overshoot 0 (limit 1)
CppModel.Verify.settles at set point: FAIL change at t=1800 s: settled in 1989 s (limit 1800 s), overshoot 1.287 (limit 1), steady-state error 0.298
CppModel.Verify.settles at set point: PASS slowest change at t=1800 s: settled in 856 s (limit 1800 s), overshoot 0 (limit 1), steady-state error -0.2578
```

Every run is recorded in the same simulation in your workspace, so the three can be compared on one graph there.

## Files

| File | What it is |
| ---- | ---------- |
| `tempControl.h`, `tempControl.c` | The controller under test: plain C, no dependency on CppModel |
| `SimulateTempControl.c` | The simulation: room model, set point schedule, parameters and the verification rule |
| `pi.json`, `pi-antiwindup.json` | Parameters for the PI variants, for `--inputs` |
| `CMakeLists.txt` | Builds the simulation and registers it as a CTest test |

## Build and run

You need a C compiler, CMake 3.21 or later, and the CppModel SDK 0.18.0 or later.

1. Log in to [CppModel Workspace](https://workspace.cppmodel.com) (a free account works), open the `Download` menu and download the SDK for your platform.
2. Extract it into a `dependencies` folder here. On Linux and macOS:

   ```bash
   mkdir -p dependencies
   tar -xzf CppModel-x.x.x-Linux-x86_64.tar.gz -C dependencies --strip-components=1
   ```

   On Windows, extract the `.zip` and move the contents of its top-level folder into `dependencies`. See [Installation](https://www.cppmodel.com/docs/Installation).
3. Build and run:

   ```bash
   cmake -S . -B build
   cmake --build build
   ./build/SimulateTempControl
   ```

On the first run, the program prints a login link; open it to connect to your workspace. When the run finishes, it prints the rule's outcome and a link to the results. The exit code is 0 only when the rule passed, so `ctest --test-dir build` runs it as a test (with the default parameters, P control, it fails).

In CI, set `CPPMODEL_USERNAME` and `CPPMODEL_PASSWORD` instead of the browser login (see [Command Line and Environment](https://www.cppmodel.com/docs/CommandLineTool)).

## Parameters

| Parameter | Default |
| --------- | ------- |
| `ProportionalGain [%/C]` | 30 |
| `IntegralTime [s]` | 0 (P control) |
| `AntiWindup` | 0 |
| `AmbientTemperature [C]` | 5 |
| `HeatingRate [C/s]` | 0.02 |
| `LossRate [1/s]` | 0.001 |
| `InitialTemperature [C]` | 16 |

The set point is an input, `Setpoint [C]`, so another schedule can be given in an `--inputs` file too.

## License

[MIT](LICENSE)
