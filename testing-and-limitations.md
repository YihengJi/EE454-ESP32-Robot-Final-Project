# Testing and Implementation Limitations

## Available evidence

This page separates original project observations from source-code behavior. No new physical trials or quantitative experiments were performed for this documentation update.

| Item | Evidence | Limit |
| --- | --- | --- |
| Manual driving and boost | Original project account and source implementation | No repeatability or connection-reliability dataset. |
| Three-sensor integration | Individual/combined debugging reported; initialization/read code included | No calibration or coverage dataset. |
| Fixed-distance stop | Light contact at 0.5 m reported; threshold changed to 0.7 m | No measured stopping-distance or collision-rate study. |
| Manual override | Explicit stick and L2 logic | No measured override latency. |
| 300 ms check | Timeout branch in main sketch | Timestamp semantics do not establish stale-report detection. |
| Fault injection | No harness or results in this repository | Validation cannot be claimed as completed. |
| Speed-adaptive braking | Future investigation | No speed estimate or adaptive threshold in the source. |

## Controller freshness

`bt_hid_update()` calls `BP32.update()`, copies connected gamepad fields and sets `g_last_report_ms = millis()` on every connected loop. It does not confirm that a fresh report arrived. The main sketch checks elapsed time immediately afterward, so a connected controller with stale data may continually appear fresh.

The earlier Pico W project documented a report-arrival timestamp modification. Those notes are not proof of identical ESP32 behavior. The original continued-turning incident was not reliably reproduced, and early testing did not fully validate the new protection.

## Distance and motor behavior

- Timeout, 65535, and values above 8000 mm are treated as invalid and represented as `-1.0f`.
- Valid distances are averaged; larger readings can mask one close obstacle.
- With no valid readings, the function returns while both motors remain commanded at full boost PWM.
- Distance-based stopping is active only in boost, not manual driving.
- The stop is not latched. A later average above the threshold can resume forward output while boost is still requested.
- Sequential sensor reads and timeouts can delay controller processing; 300 ms is not a measured total response-time bound.
- Zero motor output does not prove immediate physical stopping. Behavior depends on speed, hardware, load and traction.

## Proposed validation

These tasks remain future work:

1. Track actual input-report arrival and deliberately pause reports while connection state remains active; compare report age with motor-command timestamps.
2. Define and test desired responses to partial/all sensor failure and controller disconnection in a secured setup.
3. Measure sensing-to-command delay and physical stopping distance over repeated trials at different speeds and on different surfaces.
4. Compare averaging with minimum-distance or directional rules and assess a latched-stop requirement.
5. Compare fixed and speed-aware thresholds using minimum clearance, collisions and unnecessary stops.

Retain configuration, trial counts, raw measurements and failures before making quantitative claims.
