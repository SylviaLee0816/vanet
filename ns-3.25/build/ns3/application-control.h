#ifndef APPLICATION_CONTROL_H
#define APPLICATION_CONTROL_H

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/node-container.h"

namespace ns3 {

class ApplicationControl : public Object
{

public:
	ApplicationControl ();
	static TypeId GetTypeId (void);

    // calculate the vehicle number in the circle 
	void CreateTxtNote (int packetSeq, double x, double y);

	void WriteMessage (int packetSeq, double x, double y);

	void CloseTxtNote (int packetSeq);

	void SetNodeContainer(NodeContainer temp);

	NodeContainer total;

private:
	mutable int Count1;
	mutable int Count2;
	mutable int Count3;

};

}

#endif