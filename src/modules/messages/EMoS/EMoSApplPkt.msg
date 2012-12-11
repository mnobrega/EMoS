//**************************************************************************
// * file:        EMoSApplPkt.msg
// *
// * author:      Márcio Nóbrega
// *
// * copyright:   (C) 2012 IST - Instituto Superior Técnico, Lisbon, Portugal
// *
// *           	  Message used to send the application layer data to the other
// *		      layers above. In EMoS a data message can be used to carry
// *			  the static nodes collected signatures or a message generated
// * 			  by the application layer.
// **************************************************************************
// * part of:     EMoS - Elder Monitoring System
// **************************************************************************/

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