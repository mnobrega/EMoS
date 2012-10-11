cplusplus {{
	#include "ApplPkt_m.h"
	#include "Coord.h"
	typedef std::map<LAddress::L3Type,double> addressRSSIMap_t;
}}

class ApplPkt;
class noncobject addressRSSIMap_t;
class Coord extends void;

message HoHuTApplPkt extends ApplPkt
{
    string payload;
	addressRSSIMap_t collectedRSSIs;
	int HoHuTMsgType;
	
	Coord realPosition;
}