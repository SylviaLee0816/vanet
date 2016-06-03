#ifndef CITY_HELPER_H
#define CITY_HELPER_H

#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {

class CityHelper
{
public:
	CityHelper ();
	CityHelper (const Vector &position);
	CityHelper (const Vector &position, const Vector &vel);

	void SetPosition (const Vector &position);
	Vector GetCurrentPosition (void) const;
    Vector GetPrePosition (void) const;
	Vector GetVelocity (void) const;
	void SetVelocity (const Vector &vel);

	void Update (void) const;
	//void td_adhoc_ME_city_coordinate_discrete (CENTRAL_SV &lvo_central, double lvf_vehicle_x, double lvf_vehicle_y, int& lvc_grid_x, int& lvc_grid_y);
	//ADHOC_LOS_TYPE td_adhoc_ME_city_los_status_judge(int lvc_vehicle1_grid_x, int lvc_vehicle1_grid_y, int lvc_vehicle2_grid_x, int lvc_vehicle2_grid_y);
private:
	mutable Time m_lastUpdate;
	mutable Vector m_position;
    mutable Vector m_prePosition;
	Vector m_velocity;
};

}

#endif