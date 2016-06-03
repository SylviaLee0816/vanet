#include "ns3/simulator.h"
#include "ns3/log.h"
#include "city-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CityHelper");

CityHelper::CityHelper ()
{
	NS_LOG_FUNCTION (this);
}

CityHelper::CityHelper (const Vector &position)
	: m_position (position)
{
	NS_LOG_FUNCTION (this << position);
}

CityHelper::CityHelper (const Vector &position, const Vector &vel)
	: m_position (position),
	  m_velocity (vel)
{
	NS_LOG_FUNCTION (this << position << vel);
}

void
CityHelper::SetPosition (const Vector &position)
{
	NS_LOG_FUNCTION (this << position);
	m_position = position;
	m_lastUpdate = Simulator::Now ();
}

Vector
CityHelper::GetCurrentPosition (void) const
{
	NS_LOG_FUNCTION (this);
	return m_position;
}
Vector
CityHelper::GetPrePosition (void) const
{
    NS_LOG_FUNCTION (this);
    return m_prePosition;
}

Vector
CityHelper::GetVelocity (void) const
{
	NS_LOG_FUNCTION (this);
	return m_velocity;
}

void
CityHelper::SetVelocity (const Vector &vel)
{
	NS_LOG_FUNCTION (this << vel);
	m_velocity = vel;
	m_lastUpdate = Simulator::Now ();
}

void
CityHelper::Update (void) const
{
	NS_LOG_FUNCTION (this);
	Time now = Simulator::Now ();
	NS_ASSERT (m_lastUpdate <= now);
	Time deltaTime = now - m_lastUpdate;
	m_lastUpdate = now;
	double deltaS = deltaTime.GetSeconds ();
    
    m_prePosition.x = m_position.x;
    m_prePosition.z = m_position.z;

	int temp_x = m_position.x + m_velocity.x * deltaS;
	int temp_y = m_position.y + m_velocity.y * deltaS;
	if(temp_x > 750) {
		m_position.x = 0;
	} else if(temp_x < 0) {
		m_position.x = 750;
	} else {
		m_position.x = temp_x;
	}
	if(temp_y > 1299) {
		m_position.y = 0;
	} else if(temp_y < 0) {
		m_position.y = 1299;
	} else {
		m_position.y = temp_y;
	}
}

}