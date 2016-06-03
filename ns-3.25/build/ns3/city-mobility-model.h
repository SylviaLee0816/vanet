#ifndef CITY_MOBILITY_MODEL_H
#define CITY_MOBILITY_MODEL_H

#include <stdint.h>
#include "ns3/nstime.h"
#include "mobility-model.h"
#include "city-helper.h"

namespace ns3 {

class CityMobilityModel : public MobilityModel
{
public:
	static TypeId GetTypeId (void);

	CityMobilityModel ();
	virtual ~CityMobilityModel ();
    Vector DoGetPrePosition (void) const;

	void SetVelocity (const Vector &speed);
private:
	virtual Vector DoGetPosition (void) const;
	virtual void DoSetPosition (const Vector &position);
	virtual Vector DoGetVelocity (void) const;
	CityHelper m_helper;
};

}

#endif