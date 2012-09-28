cplusplus {{
	#include "ApplPkt_m.h"
}}

class ApplPkt;
class LAddress::L3Type;

message HoHuTApplPkt extends ApplPkt
{
    double signalStrength = -1;
    string data;
}