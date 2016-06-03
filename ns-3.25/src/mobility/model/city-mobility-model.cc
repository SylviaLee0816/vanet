#include "city-mobility-model.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CityMobilityModel);

TypeId CityMobilityModel::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::CityMobilityModel")
		.SetParent<MobilityModel> ()
		.SetGroupName ("Mobility")
		.AddConstructor<CityMobilityModel> ()
		.AddAttribute("Speed",
			"The speed of the vanet",
			VectorValue(Vector (0,0,0)),
	  		MakeVectorAccessor(&CityMobilityModel::SetVelocity),
			MakeVectorChecker());
	return tid;
}

CityMobilityModel::CityMobilityModel ()
{
}

CityMobilityModel::~CityMobilityModel ()
{
}

void
CityMobilityModel::SetVelocity (const Vector &speed)
{
	m_helper.Update ();
	m_helper.SetVelocity (speed);
	NotifyCourseChange ();
}

Vector
CityMobilityModel::DoGetPosition (void) const
{
	m_helper.Update ();
	return m_helper.GetCurrentPosition ();
}
    
Vector
CityMobilityModel::DoGetPrePosition (void) const
{
    return m_helper.GetPrePosition ();
}


void
CityMobilityModel::DoSetPosition (const Vector &position)
{
	m_helper.SetPosition (position);
	NotifyCourseChange ();
}

Vector
CityMobilityModel::DoGetVelocity (void) const
{
	return m_helper.GetVelocity ();
}

}