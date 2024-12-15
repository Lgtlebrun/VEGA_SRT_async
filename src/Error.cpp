#include "Error.h"

ErrorStatus hierarchize_status(const ErrorStatus &s1, const ErrorStatus &s2)
{
    // Determine the most severe status
    if (s1.type > s2.type)
        return s1;
    if (s2.type > s1.type)
        return s2;

    // If types are equal, merge messages
    String mergedMsg = s1.msg;
    if (!s2.msg.isEmpty())
    {
        if (!mergedMsg.isEmpty())
            mergedMsg += "; "; // Separator if both messages are non-empty
        mergedMsg += s2.msg;
    }

    return ErrorStatus(s1.type, mergedMsg);
}
