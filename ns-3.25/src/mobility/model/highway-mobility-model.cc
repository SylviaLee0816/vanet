#include "highway-mobility-model.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HighwayMobilityModel);

TypeId HighwayMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HighwayMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<HighwayMobilityModel> ()
    .AddAttribute("Speed",
		"The speed of the vanet",
		VectorValue(Vector (0,0,0)),
	  	MakeVectorAccessor(&HighwayMobilityModel::SetVelocity),
		MakeVectorChecker());
  return tid;
}

HighwayMobilityModel::HighwayMobilityModel ()
{
}

HighwayMobilityModel::~HighwayMobilityModel ()
{
}

void
HighwayMobilityModel::SetVelocity (const Vector &speed)
{
  m_helper.Update ();
  m_helper.SetVelocity (speed);
  NotifyCourseChange ();
}

Vector
HighwayMobilityModel::DoGetPosition (void) const
{
  m_helper.Update ();
  return m_helper.GetCurrentPosition ();
}
    
Vector
HighwayMobilityModel::DoGetPrePosition (void) const
{
    return m_helper.GetPrePosition ();
}

void 
HighwayMobilityModel::DoSetPosition (const Vector &position)
{
  m_helper.SetPosition (position);
  NotifyCourseChange ();
}

Vector
HighwayMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}

}