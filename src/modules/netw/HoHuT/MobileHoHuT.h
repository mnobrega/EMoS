/***************************************************************************
 * file:        MobileRoute.h
 *
 * author:      Marcio Nobrega
 *
 * description: Placeholder module that simply "translates" netwControlInfo to macControlInfo
 *
 **************************************************************************/
#include <omnetpp.h>
#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"

class WiseRoutePkt;
class HoHuTApplPkt;

class MIXIM_API MobileHoHuT : public BaseNetwLayer
{
public:
    virtual void initialize(int);
    virtual void finish();

protected:
    enum messagesTypes {
        UNKNOWN=0,
        DATA,
        ROUTE_FLOOD
    };
    bool stats, trace;

    virtual void handleUpperMsg(cMessage* msg);
    virtual void handleLowerMsg(cMessage* msg);
    virtual void handleSelfMsg(cMessage* msg) { };
    virtual void handleLowerControl(cMessage* msg);
    virtual void handleUpperControl(cMessage* msg) { delete msg; }
    WiseRoutePkt* encapsMsg(HoHuTApplPkt *pkt);
    HoHuTApplPkt* decapsMsg(WiseRoutePkt *pkt);
};
