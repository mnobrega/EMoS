cplusplus {{
	#include "ApplPkt_m.h"
}}

class ApplPkt;
class LAddress::L3Type extends void;

message HoHuTApplPkt extends ApplPkt
{
    string payload;
    LAddress::L3Type netwDestAddr;
}