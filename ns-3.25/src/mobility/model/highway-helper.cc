#include "ns3/simulator.h"
#include "ns3/log.h"
#include "highway-helper.h"
#include "ns3/application-control.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HighwayHelper");

HighwayHelper::HighwayHelper ()
{
  NS_LOG_FUNCTION (this);
}
HighwayHelper::HighwayHelper (const Vector &position)
  : m_position (position)
{
  NS_LOG_FUNCTION (this << position);
}
HighwayHelper::HighwayHelper (const Vector &position, const Vector &vel)
  : m_position (position),
    m_velocity (vel)
{
  NS_LOG_FUNCTION (this << position << vel);
}

void
HighwayHelper::SetPosition (const Vector &position) {
	NS_LOG_FUNCTION (this << position);
  	m_position = position;
  	if(m_position.x > 6000)
  		m_position.x = 6000;
  	if(m_position.x < 0)
  		m_position.x = 0;
  	//NS_ASSERT (m_position.y == 0 || m_position.y == 5 || m_position == 10 || m_position == 15);
  	m_lastUpdate = Simulator::Now ();
}

Vector
HighwayHelper::GetCurrentPosition (void) const
{
	NS_LOG_FUNCTION (this);
	return m_position;
}
    
Vector
HighwayHelper::GetPrePosition (void) const
{
    NS_LOG_FUNCTION (this);
    return m_prePosition;
}

Vector
HighwayHelper::GetVelocity (void) const
{
	NS_LOG_FUNCTION (this);
	return m_velocity;
}

void
HighwayHelper::SetVelocity (const Vector &vel)
{
	NS_LOG_FUNCTION (this << vel);
	m_velocity = vel;
	m_lastUpdate = Simulator::Now ();
}

void
HighwayHelper::Update (void) const
{
	NS_LOG_FUNCTION (this);
	Time now = Simulator::Now ();
	NS_ASSERT (m_lastUpdate <= now);
	Time deltaTime = now - m_lastUpdate;
	m_lastUpdate = now;
	double deltaS = deltaTime.GetSeconds ();
	
	/*if (m_position.y == 0 || m_position.y == 5) {
		int temp1 = m_position.x + m_velocity.x * deltaS;
		if (temp1 >= 6000) {
			m_position.x = 0;
		} else {
			m_position.x = temp1;
		}
	}
	else if(m_position.y == 10 || m_position.y == 15) {
		int temp2 = m_position.x - m_velocity.x * deltaS;
		if(temp2 <= 0) {
			m_position.x = 6000;
		} else {
			m_position.x = temp2;
		}
	}*/
    
    m_prePosition.x = m_position.x;
    m_prePosition.z = m_position.z;
    
	double temp = m_position.x + m_velocity.x * deltaS;
	if (temp > 6000) {
		m_position.x = 0;
	} else if (temp < 0) {
		m_position.x = 6000;
	} else {
		m_position.x = temp;
	}

	if (m_position.x >= 2000 && m_position.x <= 4000) {
		m_position.z = 1;
	} else {
		m_position.z = 0;
	}

	//std::cout << m_position.x << "  haha  " << m_position.y   << std::endl;
	
}

}
