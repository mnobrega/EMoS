cplusplus {{
	#include "ApplPkt_m.h"
	#include "Coord.h"

	typedef std::map<LAddress::L3Type,double> staticNodeSigsMap_t;
}}

class ApplPkt;
class noncobject staticNodeSigsMap_t;
class Coord extends void;
class LAddress::L3Type extends void;

message EMoSApplPkt extends ApplPkt
{
    //general data
    string payload;
    int EMoSMsgType;
    
    //mobile msg data
    LAddress::L3Type srcAppAddress; //equal to the netwAddr
	staticNodeSigsMap_t collectedRSSIs;
	
	//stat data
	Coord realPosition;
}