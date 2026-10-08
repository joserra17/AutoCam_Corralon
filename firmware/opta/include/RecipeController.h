#pragma once
#include <stdint.h>

// Pure state machine: no GPIO/network. Integration MUST independently enforce
// relay interlocks, an emergency stop, and a physical watchdog.
class RecipeController {
public:
  enum class State : uint8_t { Idle, Filling, Settle, Dosing, Completed, Aborted };
  enum class Error : uint8_t { None, InvalidRequest, NotEmpty, SensorLost, FillTimeout, UnstableLevel, Interlock };

  static constexpr float CAPACITY_L = 20.0f;
  static constexpr float EMPTY_TOLERANCE_L = 0.10f; // provisional; verify sensor residuals
  static constexpr float TARGET_TOLERANCE_L = 0.20f; // provisional; adjust after real filling tests
  static constexpr uint32_t FRESH_MS = 2500;
  static constexpr uint32_t EMPTY_CONFIRM_MS = 3000;
  static constexpr uint32_t TARGET_CONFIRM_MS = 3000;
  static constexpr uint32_t SETTLE_LIMIT_MS = 30000;
  static constexpr uint32_t FILL_LIMIT_MS = 30UL * 60UL * 1000UL;
  static constexpr uint32_t DOSE_LIMIT_MS = 120000;
  static constexpr float CALIBRATED_ML_PER_SEC = 1.0f; // provisional water calibration

  bool start(float waterLitres, float additiveMl, uint32_t now,
             float levelLitres, bool sensorFresh) {
    if (state_ != State::Idle) return false;
    if (!(waterLitres > 0.0f && additiveMl > 0.0f &&
          waterLitres + additiveMl/1000.0f <= CAPACITY_L &&
          additiveMl * 1000.0f / CALIBRATED_ML_PER_SEC <= DOSE_LIMIT_MS)) {
      error_ = Error::InvalidRequest; return false;
    }
    if (!sensorFresh || !emptyConfirmed_ || levelLitres > EMPTY_TOLERANCE_L) {
      error_ = Error::NotEmpty; return false;
    }
    targetL_ = waterLitres;
    additiveMl_ = additiveMl;
    doseMs_ = static_cast<uint32_t>(additiveMl * 1000.0f / CALIBRATED_ML_PER_SEC + 0.5f);
    error_ = Error::None;
    state_ = State::Filling;
    phaseStarted_ = now;
    levelInRangeSince_ = 0;
    levelInRange_ = false;
    return true;
  }

  void update(uint32_t now, float levelL, bool sensorFresh, bool interlockOK) {
    // Stable empty confirmation while idle (no automatic new recipe).
    if (state_ == State::Idle) {
      if (sensorFresh && levelL >= 0 && levelL <= EMPTY_TOLERANCE_L) {
        if (!emptyTracking_) { emptyTracking_ = true; emptySince_ = now; }
        else if (now - emptySince_ >= EMPTY_CONFIRM_MS) emptyConfirmed_ = true;
      } else { emptyTracking_ = false; emptyConfirmed_ = false; }
      return;
    }
    if (state_ == State::Completed || state_ == State::Aborted) return;
    if (!interlockOK) { abort(Error::Interlock); return; }
    if (!sensorFresh || levelL < 0 || levelL > CAPACITY_L) { abort(Error::SensorLost); return; }
    if (state_ == State::Filling) {
      if (now - phaseStarted_ > FILL_LIMIT_MS) { abort(Error::FillTimeout); return; }
      if (levelL >= targetL_ - TARGET_TOLERANCE_L) {
        state_ = State::Settle;
        phaseStarted_ = now;
        levelInRange_ = false;
      }
    } else if (state_ == State::Settle) {
      // Brief outliers reset confirmation instead of aborting immediately.
      // Persistent instability still aborts; never dose unless level remains
      // within the target band for a full confirmation window.
      if (now - phaseStarted_ > SETTLE_LIMIT_MS) {
        abort(Error::UnstableLevel); return;
      }
      const bool inRange = levelL >= targetL_ - TARGET_TOLERANCE_L &&
                           levelL <= targetL_ + TARGET_TOLERANCE_L;
      if (!inRange) {
        levelInRange_ = false;
      } else if (!levelInRange_) {
        levelInRange_ = true;
        levelInRangeSince_ = now;
      } else if (now - levelInRangeSince_ >= TARGET_CONFIRM_MS) {
        state_ = State::Dosing;
        phaseStarted_ = now;
      }
    } else if (state_ == State::Dosing) {
      if (now - phaseStarted_ >= doseMs_) { state_ = State::Completed; }
    }
  }

  void abort(Error reason = Error::Interlock) { state_ = State::Aborted; error_ = reason; }
  // Requires manual acknowledgement and empty tank confirmation before a new job.
  bool acknowledge(float levelL, bool sensorFresh) {
    if (state_ != State::Completed && state_ != State::Aborted) return false;
    if (!sensorFresh || levelL < 0 || levelL > EMPTY_TOLERANCE_L) return false;
    state_ = State::Idle;
    error_ = Error::None;
    emptyConfirmed_ = false;
    emptyTracking_ = false;
    return true;
  }

  bool relay1() const { return state_ == State::Filling; }
  bool relay2() const { return state_ == State::Dosing; }
  State state() const { return state_; }
  Error error() const { return error_; }
  float waterTarget() const { return targetL_; }
  float additiveMl() const { return additiveMl_; }
  uint32_t doseDurationMs() const { return doseMs_; }
  bool emptyConfirmed() const { return emptyConfirmed_; }

private:
  State state_ = State::Idle;
  Error error_ = Error::None;
  float targetL_ = 0, additiveMl_ = 0;
  uint32_t phaseStarted_ = 0, doseMs_ = 0, emptySince_ = 0, levelInRangeSince_ = 0;
  bool emptyConfirmed_ = false, emptyTracking_ = false, levelInRange_ = false;
};
