#ifndef AppSerial_h
#define AppSerial_h

#include <Arduino.h>

const static char frameDelimiter = '|';
const static char frameEndMarker = '\n';

struct SerialFrame {
    const char *command;
    const char *param;
    const char delimiter;
    const char endMarker;

    SerialFrame(
            const char *_command,
            const char *_param,
            const char _delimiter = frameDelimiter,
            const char _endMarker = frameEndMarker
    ) : command(_command),
        param(_param),
        delimiter(_delimiter),
        endMarker(_endMarker) {}
};

class AppSerial {
public:
    AppSerial();

    ~AppSerial();

    static SerialFrame getFrame();

    static void sendFrame(SerialFrame *serialFrame);
};

#endif /* AppSerial_h */