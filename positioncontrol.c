#include "positioncontrol.h"
#include "currentcontrol.h"


volatile float Kpp = 0, Kip = 0, Kdp = 0; // position control gains
