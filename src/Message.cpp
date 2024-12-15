#include "Message.h"

String Message::typeToString() const
{
    switch (type)
    {
    case MsgType::POSITION:
        return "POSITION";
    case MsgType::GENERIC:
        return "GENERIC";
    case MsgType::ACKNOWLEDGEMENT:
        return "ACKNOWLEDGEMENT";
    case MsgType::TIMESTAMP:
        return "TIMESTAMP";
    default:
        return "UNKNOWN";
    }
}

String Message::format() const
{
    return "{type: \"" + typeToString() +
           "\", status: \"" + status +
           "\", payload: \"" + payload +
           "\", timestamp: \"" + getCurrentTimestamp() + "\"}";
}

// Utility to convert ErrorType to string
String errorTypeToString(ErrorType type)
{
    switch (type)
    {
    case ErrorType::NONE:
        return "success";
    case ErrorType::WARNING:
        return "warning";
    case ErrorType::ERROR:
        return "error";
    default:
        return "unknown";
    }
}

// Example print functions
void print_msg(const ErrorStatus &err, const MsgType &type)
{
    Message msg(type, err.msg, errorTypeToString(err.type));
    HWSerial.println(msg.format());
}

void print_info(const String &info, const MsgType &type)
{
    ErrorStatus status(ErrorType::NONE, info);
    print_msg(status, type);
}

void print_acknowledgement(const String &msg, const ErrorType &status)
{
    print_msg(ErrorStatus(status, msg), MsgType::ACKNOWLEDGEMENT);
}

void print_acknowledgement_error(const String &msg)
{
    print_acknowledgement(msg, ErrorType::ERROR);
}

void print_warning(const String &warning, const MsgType &type)
{
    ErrorStatus status(ErrorType::WARNING, warning);
    print_msg(status, type);
}

void print_error(const String &error, const MsgType &type)
{
    ErrorStatus status(ErrorType::ERROR, error);
    print_msg(status, type);
}

void print_position(float az, float elev, const ErrorStatus &err)
{
    String payload = (err.type != ErrorType::ERROR)
                         ? "{azimuth: " + String(az) + ", elevation: " + String(elev) + "}"
                         : err.msg;
    ErrorStatus status(err);
    if (err.type != ErrorType::ERROR)
    {
        status.msg = payload;
    }
    print_msg(status, MsgType::POSITION);
}

void print_timestamp(String timestamp)
{
    ErrorStatus status;
    status.msg = timestamp;
    print_msg(status, MsgType::TIMESTAMP);
}

void print_msg_filtered(const ErrorStatus &err, const MsgType &type)
/// Print warnings and errors
{
    if (err.type != ErrorType::NONE)
    {
        print_msg(err, type);
    }
}