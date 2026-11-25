#pragma once

#include <QtGlobal>
#include <QString>

#include "mira/core/Log.h"

/**
 * @brief Resets global logging state so Qt Test slots stay independent.
 *
 * Qt exposes no getter for its message pattern or for Mira's log sink, so the
 * guard restores both to their documented defaults instead of to an arbitrary
 * previous value. Tests must not rely on another suite's logging settings.
 */
class LogStateGuard
{
public:
    LogStateGuard() :
        savedMiraPattern(Mira::Core::messagePattern()),
        previousHandler(nullptr)
    {
        reset();
        previousHandler = qInstallMessageHandler(nullptr);
        qInstallMessageHandler(previousHandler);
    }

    ~LogStateGuard()
    {
        reset();
        qInstallMessageHandler(previousHandler);
        Mira::Core::setMessagePattern(savedMiraPattern);
    }

    void reset()
    {
        Mira::Core::setLogSink(nullptr);
        Mira::Core::uninstallMessageHandler();
        Mira::Core::setMessagePattern(Mira::Core::defaultMessagePattern());
        qSetMessagePattern(Mira::Core::qtDefaultMessagePattern());
    }

private:
    Q_DISABLE_COPY(LogStateGuard)

    QString savedMiraPattern;
    QtMessageHandler previousHandler;
};
