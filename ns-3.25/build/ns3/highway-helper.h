#ifndef HIGHWAY_HELPER_H
#define HIGHWAY_HELPER_H

#include "ns3/nstime.h"
#include "ns3/vector.h"
#include "ns3/application-control.h"

namespace ns3 {

class HighwayHelper
{
public:
	HighwayHelper ();
	HighwayHelper (const Vector &position);
	HighwayHelper (const Vector &position, const Vector &vel);

	void SetPosition (const Vector &position);
	Vector GetCurrentPosition (void) const;
    Vector GetPrePosition (void) const;
	Vector GetVelocity (void) const;
	void SetVelocity (const Vector &vel);
	void Update (void) const;

private:
	mutable Time m_lastUpdate;
	mutable Vector m_position;
    mutable Vector m_prePosition;
	Vector m_velocity;
};

}

#endif