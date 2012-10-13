cplusplus {{
	#include "ApplPkt_m.h"
	#include "Coord.h"
	typedef std::map<LAddress::L3Type,double> addressRSSIMap_t;
}}

class ApplPkt;
class noncobject addressRSSIMap_t;
class Coord extends void;
class LAddress::L3Type extends void;

message HoHuTApplPkt extends ApplPkt
{
    //general data
    string payload;
    int HoHuTMsgType;
    
    //mobile msg data
    LAddress::L3Type srcAppAddress; //equal to the netwAddr
	addressRSSIMap_t collectedRSSIs;
	
	//debug data
	Coord realPosition;
}