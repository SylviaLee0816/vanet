#ifndef HIGHWAY_MOBILITY_MODEL_H
#define HIGHWAY_MOBILITY_MODEL_H

#include <stdint.h>
#include "ns3/nstime.h"
#include "mobility-model.h"
#include "highway-helper.h"

namespace ns3 {

class HighwayMobilityModel : public MobilityModel
{
public:
	static TypeId GetTypeId(void);

	HighwayMobilityModel ();
	virtual ~HighwayMobilityModel ();
    
    Vector DoGetPrePosition (void) const;

	void SetVelocity(const Vector &speed);

private:
	virtual Vector DoGetPosition (void) const;
	virtual void DoSetPosition (const Vector &position);
	virtual Vector DoGetVelocity (void) const;
	HighwayHelper m_helper;
};

}

#endif