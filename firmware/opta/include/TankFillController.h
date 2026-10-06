#pragma once

#include <stdint.h>

class TankFillController
{
public:
    enum class Fault : uint8_t
    {
        None = 0,
        NoSetpoint,
        SensorTimeout,
        InvalidLevel,
        MaximumRunTime
    };

    static constexpr uint32_t SENSOR_TIMEOUT_MS = 2500;
    static constexpr uint32_t MAXIMUM_RUN_TIME_MS = 30UL * 60UL * 1000UL;
    static constexpr uint8_t HYSTERESIS_PERCENT = 5;
    static constexpr uint8_t HARD_STOP_MARGIN_PERCENT = 3;
    static constexpr uint32_t START_CONFIRMATION_MS = 10000;
    static constexpr uint32_t STOP_CONFIRMATION_MS = 2000;
    static constexpr uint32_t MINIMUM_OFF_TIME_MS = 30000;

    void setSetpoint(uint8_t percent)
    {
        if (percent < 1 || percent > 100)
        {
            setpointValid_ = false;
            pumpOn_ = false;
            cancelPendingTransitions();
            fault_ = Fault::NoSetpoint;
            return;
        }
        setpointPercent_ = percent;
        setpointValid_ = true;
        if (fault_ == Fault::NoSetpoint) fault_ = Fault::None;
    }

    void updateLevel(uint8_t percent, uint32_t nowMs)
    {
        if (percent > 100)
        {
            invalidateLevel();
            return;
        }
        levelPercent_ = percent;
        lastLevelMs_ = nowMs;
        levelValid_ = true;
        hasReceivedLevel_ = true;
        if (fault_ == Fault::SensorTimeout || fault_ == Fault::InvalidLevel)
            fault_ = Fault::None;
    }

    void invalidateLevel()
    {
        levelValid_ = false;
        pumpOn_ = false;
        cancelPendingTransitions();
        fault_ = Fault::InvalidLevel;
    }

    void resetMaximumRunTimeFault()
    {
        if (fault_ == Fault::MaximumRunTime) fault_ = Fault::None;
    }

    void update(uint32_t nowMs)
    {
        if (!setpointValid_)
        {
            stopPump();
            cancelPendingTransitions();
            fault_ = Fault::NoSetpoint;
            return;
        }
        if (!hasReceivedLevel_ || (uint32_t)(nowMs - lastLevelMs_) > SENSOR_TIMEOUT_MS)
        {
            levelValid_ = false;
            stopPump();
            cancelPendingTransitions();
            fault_ = Fault::SensorTimeout;
            return;
        }
        if (!levelValid_ || fault_ == Fault::MaximumRunTime)
        {
            stopPump();
            cancelPendingTransitions();
            return;
        }
        if (pumpOn_ && (uint32_t)(nowMs - pumpStartedMs_) > MAXIMUM_RUN_TIME_MS)
        {
            stopPump();
            cancelPendingTransitions();
            fault_ = Fault::MaximumRunTime;
            return;
        }
        if (pumpOn_)
        {
            startConditionActive_ = false;

            const uint8_t hardStopLevel = setpointPercent_ > 100 - HARD_STOP_MARGIN_PERCENT
                ? 100 : setpointPercent_ + HARD_STOP_MARGIN_PERCENT;
            if (levelPercent_ >= hardStopLevel)
            {
                stopPumpAtTarget(nowMs);
                return;
            }

            if (levelPercent_ >= setpointPercent_)
            {
                if (!stopConditionActive_)
                {
                    stopConditionActive_ = true;
                    stopConditionSinceMs_ = nowMs;
                }
                else if ((uint32_t)(nowMs - stopConditionSinceMs_) >= STOP_CONFIRMATION_MS)
                {
                    stopPumpAtTarget(nowMs);
                }
            }
            else
            {
                stopConditionActive_ = false;
            }
        }
        else
        {
            stopConditionActive_ = false;
            const uint8_t startThreshold = setpointPercent_ > HYSTERESIS_PERCENT
                ? setpointPercent_ - HYSTERESIS_PERCENT : 0;

            const bool minimumOffTimeElapsed = !hasStoppedAtTarget_ ||
                (uint32_t)(nowMs - lastStoppedMs_) >= MINIMUM_OFF_TIME_MS;

            if (levelPercent_ <= startThreshold && minimumOffTimeElapsed)
            {
                if (!startConditionActive_)
                {
                    startConditionActive_ = true;
                    startConditionSinceMs_ = nowMs;
                }
                else if ((uint32_t)(nowMs - startConditionSinceMs_) >= START_CONFIRMATION_MS)
                {
                    pumpOn_ = true;
                    pumpStartedMs_ = nowMs;
                    startConditionActive_ = false;
                }
            }
            else
            {
                startConditionActive_ = false;
            }
        }
    }

    bool pumpOn() const { return pumpOn_; }
    bool levelValid() const { return levelValid_; }
    uint8_t levelPercent() const { return levelPercent_; }
    uint8_t setpointPercent() const { return setpointPercent_; }
    Fault fault() const { return fault_; }

private:
    void stopPump() { pumpOn_ = false; }

    void stopPumpAtTarget(uint32_t nowMs)
    {
        pumpOn_ = false;
        lastStoppedMs_ = nowMs;
        hasStoppedAtTarget_ = true;
        stopConditionActive_ = false;
    }

    void cancelPendingTransitions()
    {
        startConditionActive_ = false;
        stopConditionActive_ = false;
    }

    bool pumpOn_ = false;
    bool levelValid_ = false;
    bool hasReceivedLevel_ = false;
    bool setpointValid_ = false;
    uint8_t levelPercent_ = 0;
    uint8_t setpointPercent_ = 0;
    uint32_t lastLevelMs_ = 0;
    uint32_t pumpStartedMs_ = 0;
    uint32_t lastStoppedMs_ = 0;
    uint32_t startConditionSinceMs_ = 0;
    uint32_t stopConditionSinceMs_ = 0;
    bool hasStoppedAtTarget_ = false;
    bool startConditionActive_ = false;
    bool stopConditionActive_ = false;
    Fault fault_ = Fault::NoSetpoint;
};
