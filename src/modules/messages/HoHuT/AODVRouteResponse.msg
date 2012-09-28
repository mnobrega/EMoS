cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;

message AODVRouteResponse extends NetwPkt
{
	int 				destSeqNo; 	//destination sequence number
	int					lifetime;	//time in milliseconds
	int					hopCount;	//hop count
}