#ifndef ERROR_H
#define ERROR_H

#include <Arduino.h>
#include <queue>
#include <string>

enum class ErrorType
{
    NONE,
    WARNING,
    ERROR
};

/*
// Overload comparison operators for ErrorType
inline bool operator>(const ErrorType &lhs, const ErrorType &rhs)
{
    return static_cast<int>(lhs) > static_cast<int>(rhs);
}

inline bool operator<(const ErrorType &lhs, const ErrorType &rhs)
{
    return static_cast<int>(lhs) < static_cast<int>(rhs);
}

inline bool operator==(const ErrorType &lhs, const ErrorType &rhs)
{
    return static_cast<int>(lhs) == static_cast<int>(rhs);
}
*/
struct ErrorStatus
{

    ErrorType type;
    String msg;

    ErrorStatus()
    {
        type = ErrorType::NONE;
        msg = "";
    }

    ErrorStatus(ErrorType type, String msg) : type(type),
                                              msg(msg)
    {
    }
};

ErrorStatus hierarchize_status(const ErrorStatus &s1, const ErrorStatus &s2);
#endif

// Pas sûr de l'utilité
/*
class ErrorMonitor
{
private:
    std::queue<ErrorStatus> errorQueue;
    SemaphoreHandle_t mutex;

public:
    ErrorMonitor()
    {
        mutex = xSemaphoreCreateMutex();
    }

    // Safely add a new error to the queue
    void reportError(MsgStatus status, const String &msg)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY))
        {
            errorQueue.push(ErrorStatus(status, msg));
            xSemaphoreGive(mutex);
        }
    }

    // Broadcast all pending errors
    void broadcastErrors()
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY))
        {
            while (!errorQueue.empty())
            {
                ErrorStatus currentError = errorQueue.front();
                print_generic(currentError.msg, currentError.type);
                errorQueue.pop();
            }
            xSemaphoreGive(mutex);
        }
    }
};
*/