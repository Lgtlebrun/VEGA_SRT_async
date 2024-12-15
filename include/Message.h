#ifndef MESSAGE_H
#define MESSAGE_H
#include <sstream>
#include <iomanip>
#include <string>
#include "Error.h"
#include "define.h"
#include "utils.h"

enum class MsgType
{
    POSITION,
    GENERIC,
    ACKNOWLEDGEMENT,
    TIMESTAMP
};

class Message
{
private:
    MsgType type;
    String payload;
    String status;

public:
    Message(MsgType _type, String _payload, String _status)
        : type(_type), payload(_payload), status(_status) {}

    String typeToString() const;

    String format() const;
};

// Utility to convert ErrorType to string
String errorTypeToString(ErrorType type);

// print functions
void print_msg(const ErrorStatus &err, const MsgType &type = MsgType::GENERIC);

void print_info(const String &info, const MsgType &type = MsgType::GENERIC);

void print_acknowledgement(const String &msg, const ErrorType &status = ErrorType::NONE);

void print_acknowledgement_error(const String &msg);

void print_warning(const String &warning, const MsgType &type = MsgType::GENERIC);

void print_error(const String &error, const MsgType &type = MsgType::GENERIC);

void print_position(float az, float elev, const ErrorStatus &err = ErrorStatus());

void print_msg_filtered(const ErrorStatus &err, const MsgType &type = MsgType::GENERIC);

void print_timestamp(String timestamp = getCurrentTimestamp());

#endif