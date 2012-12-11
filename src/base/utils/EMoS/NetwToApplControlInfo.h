/* -*- mode:c++ -*- *******************************************************
 * file:        NetwToApplControlInfo.h
 *
 * author:      Márcio Nóbrega
 *
 * copyright:   (C) 2012 IST - Instituto Superior Técnico, Lisbon, Portugal
 *
 *              This module implements the controlinfo the flows from the
 *              Network layer to the Application layer.
 ***************************************************************************
 * part of:     EMoS - Elder Monitoring System
 ***************************************************************************/
#ifndef NETWTOAPPLCONTROLINFO_H_
#define NETWTOAPPLCONTROLINFO_H_

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "SimpleAddress.h"

class MIXIM_API NetwToApplControlInfo : public cObject {

protected:
    double rssi;
    LAddress::L3Type srcNetwAddr;

public:
	NetwToApplControlInfo(const double& rssi = 0, const LAddress::L3Type& srcAddr = LAddress::L3NULL):rssi(rssi),srcNetwAddr(srcAddr){}
	virtual ~NetwToApplControlInfo() {}

	virtual const double getRSSI()
	{
	    return rssi;
	}
	virtual const LAddress::L3Type& getSrcNetwAddr()
	{
	    return srcNetwAddr;
	}
	virtual void setRSSI(double _rssi)
	{
	    rssi = _rssi;
	}
	virtual void setSrcNetwAddr(const LAddress::L3Type& addr)
	{
	    srcNetwAddr = addr;
	}

    static cObject *const setControlInfo(cMessage *const pMsg, double rssi, LAddress::L3Type srcAddr)
    {
    	NetwToApplControlInfo *const cCtrlInfo = new NetwToApplControlInfo(rssi,srcAddr);
    	pMsg->setControlInfo(cCtrlInfo);
    	return cCtrlInfo;
    }
};

#endif /* NETWTOAPPLCONTROLINFO_H_ */
