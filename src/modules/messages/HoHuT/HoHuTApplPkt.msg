cplusplus {{
	#include "ApplPkt_m.h"
	#include "Coord.h"
	typedef struct staticNodeSigsSample
    {
        LAddress::L3Type addr;
        double mean;
    }staticNodeSigsSample_t;
    struct staticNodeSigsSampleCompare
    {
        bool operator() (staticNodeSigsSample_t* lhs, staticNodeSigsSample_t* rhs)
        {
            return lhs->mean > rhs->mean; //mean DESC - static nodes samples are order by desc mean
        }
    };
    typedef std::set<staticNodeSigsSample_t*,staticNodeSigsSampleCompare> staticNodeSigsSamplesSet_t;
}}

class ApplPkt;
class noncobject staticNodeSigsSamplesSet_t;
class Coord extends void;
class LAddress::L3Type extends void;

message HoHuTApplPkt extends ApplPkt
{
    //general data
    string payload;
    int HoHuTMsgType;
    
    //mobile msg data
    LAddress::L3Type srcAppAddress; //equal to the netwAddr
	staticNodeSigsSamplesSet_t collectedRSSIs;
	
	//stat data
	Coord realPosition;
}