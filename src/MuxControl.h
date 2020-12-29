#ifndef MUXCONTROL_h
#define MUXCONTROL_h

#include <stdio.h>

class MuxControl{
public:
    void enableMuxPort(int portNumber);
    void disableMuxPort(int portNumber);
private:
};

#endif
