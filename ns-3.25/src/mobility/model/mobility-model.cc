/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include <cmath>

#include "mobility-model.h"
#include "ns3/trace-source-accessor.h"
 #include "ns3/application-control.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MobilityModel);

TypeId 
MobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MobilityModel")
    .SetParent<Object> ()
    .SetGroupName ("Mobility")
    .AddAttribute ("Position", "The current position of the mobility model.",
                   TypeId::ATTR_SET | TypeId::ATTR_GET,
                   VectorValue (Vector (0.0, 0.0, 0.0)),
                   MakeVectorAccessor (&MobilityModel::SetPosition,
                                       &MobilityModel::GetPosition),
                   MakeVectorChecker ())
    .AddAttribute ("Velocity", "The current velocity of the mobility model.",
                   TypeId::ATTR_GET,
                   VectorValue (Vector (0.0, 0.0, 0.0)), // ignored initial value.
                   MakeVectorAccessor (&MobilityModel::GetVelocity),
                   MakeVectorChecker ())
    .AddTraceSource ("CourseChange", 
                     "The value of the position and/or velocity vector changed",
                     MakeTraceSourceAccessor (&MobilityModel::m_courseChangeTrace),
                     "ns3::MobilityModel::TracedCallback")
  ;
  return tid;
}

MobilityModel::MobilityModel ()
{
    m_shadowValue.clear ();
    m_shadowValue.resize (1000, 0);
    for (int i = 0;i < 1000;i++) {
        m_shadowValue[i] = -100;
    }

    m_appControl = NULL;
}

MobilityModel::~MobilityModel ()
{
}

void MobilityModel::SetSharedId(int id){
  m_sharedId = id;
 }

 int MobilityModel::GetSharedId(void)
 {
  return m_sharedId;
 }

Vector
MobilityModel::GetPosition (void) const
{
  return DoGetPosition ();
}
    
Vector
MobilityModel::GetPrePosition (void) const
{
    return DoGetPrePosition ();
}
    
Vector
MobilityModel::DoGetPrePosition (void) const
{

    return Vector(0,0,0);
}
    
Vector
MobilityModel::GetVelocity (void) const
{
  return DoGetVelocity ();
}

void
MobilityModel::SetShadowValue (const int index, const double shadowValue)
{
  m_shadowValue[index] = shadowValue;

}

double
MobilityModel::GetShadowValue (const int index)
{

  return m_shadowValue[index];

}

void 
MobilityModel::SetPosition (const Vector &position)
{
  DoSetPosition (position);
}

double 
MobilityModel::GetDistanceFrom (Ptr<const MobilityModel> other) const
{
  Vector oPosition = other->DoGetPosition ();
  Vector position = DoGetPosition ();
  return CalculateDistance (position, oPosition);
}

double
MobilityModel::GetPreDistanceFrom (Ptr<const MobilityModel> other) const
{
    Vector oPosition = other->DoGetPrePosition ();
    Vector position = DoGetPrePosition ();
    return CalculateDistance (position, oPosition);
}

double
MobilityModel::GetRelativeSpeed (Ptr<const MobilityModel> other) const
{
  double x = GetVelocity().x - other->GetVelocity().x;
  double y = GetVelocity().y - other->GetVelocity().y;
  double z = GetVelocity().z - other->GetVelocity().z;
  return sqrt( (x*x) + (y*y) + (z*z) );
}

void
MobilityModel::NotifyCourseChange (void) const
{
  m_courseChangeTrace (this);
}

int64_t
MobilityModel::AssignStreams (int64_t start)
{
  return DoAssignStreams (start);
}

// Default implementation does nothing
int64_t
MobilityModel::DoAssignStreams (int64_t start)
{
  return 0;
}

void
MobilityModel::SetAppControl (Ptr<ApplicationControl> control)
{
  // m_appControl->CreateTxtNote(12);
  m_appControl = control;
}

Ptr<ApplicationControl>
MobilityModel::GetAppControl (void) const
{
   //m_appControl->CreateTxtNote(12);
  return m_appControl;
}

Vector
MobilityModel::td_adhoc_ME_city_coordinate_discrete (const double lvf_vehicle_x, const double lvf_vehicle_y) const
{
  int city_block_size_x = 250;  //包含道路的一个block的x长度
  int city_block_size_y = 433;  //包含道路的一个block的y长度
  int city_block_number_x = 3;  //仿真区域中x方向包含的block数
  int city_block_number_y = 3;  //仿真区域中y方向包含的block数
  int city_building_size_losnlos_x = 230;  //不包含道路的一个block的x长度
  int city_building_size_losnlos_y = 413;  //不包含道路的一个block的y长度
  
  double lvd_lane_total_width;
  long  a1,a2,b1,b2;
  int lvc_grid_x, lvc_grid_y;

  double  lvf_coordinate_shift_x = lvf_vehicle_x + city_block_size_x * city_block_number_x *1.5 + (city_block_size_x - city_building_size_losnlos_x)/2;
  double  lvf_coordinate_shift_y = lvf_vehicle_y + city_block_size_y * city_block_number_y *1.5 + (city_block_size_y - city_building_size_losnlos_y)/2;

  a1 =(long)(lvf_coordinate_shift_x/city_block_size_x);
  a2 =(long)lvf_coordinate_shift_x %(long)city_block_size_x;

  lvd_lane_total_width = city_block_size_x - city_building_size_losnlos_x;

  if(a2 >= lvd_lane_total_width)
  {
    lvc_grid_x = (int)(2* a1 +1 );
  }
  else 
  {
    lvc_grid_x=(int)(2*a1) ;
  }

  b1 =(long)(lvf_coordinate_shift_y/city_block_size_y);
  b2 =(long)lvf_coordinate_shift_y % (long)city_block_size_y;

  if(b2 >= lvd_lane_total_width)
  {

    lvc_grid_y = (int)(2* b1 +1 );
  }
  else 
  {
    lvc_grid_y=(int)(2*b1) ;
  }

  if(lvc_grid_x <0 ||lvc_grid_x>24 ||lvc_grid_y <0 ||lvc_grid_y>24)  // 24为总的车道数（包括x方向和y方向）
  {
    //printf("坐标映射出错X映射为%ldY映射为%ldX坐标为%lfY坐标为%lf\n",lvc_grid_x,lvc_grid_y, lvf_vehicle_x, lvf_vehicle_y );
    std::cout << "坐标映射出错,X映射为" << lvc_grid_x << "Y映射为" << lvc_grid_y << "X坐标为" << lvf_vehicle_x << "Y坐标为" << lvf_vehicle_y << std::endl;
  }
  NS_ASSERT (lvc_grid_x >=0 &&lvc_grid_x<=24 &&lvc_grid_y >=0 &&lvc_grid_y<=24);

  Vector grid;
  grid.x = lvc_grid_x;
  grid.y = lvc_grid_y;
  grid.z = 0;

  return grid;
}

int 
MobilityModel::td_adhoc_ME_city_los_status_judge (int lvc_vehicle1_grid_x, int lvc_vehicle1_grid_y, int lvc_vehicle2_grid_x, int lvc_vehicle2_grid_y) const
{
  int lvo_los_status;
  if( (lvc_vehicle1_grid_x == lvc_vehicle2_grid_x) && (lvc_vehicle1_grid_y == lvc_vehicle2_grid_y))
  {
    lvo_los_status = 0;//ADHOC_Type_LOS;
  }
  else if ( lvc_vehicle1_grid_x == lvc_vehicle2_grid_x && lvc_vehicle1_grid_y != lvc_vehicle2_grid_y && lvc_vehicle1_grid_x % 2 == 0 )
  {
    lvo_los_status = 0;//ADHOC_Type_LOS;  
  }
  else if ( lvc_vehicle1_grid_y == lvc_vehicle2_grid_y && lvc_vehicle1_grid_x != lvc_vehicle2_grid_x && lvc_vehicle1_grid_y % 2 == 0)
  {
    lvo_los_status = 0;//ADHOC_Type_LOS; 
  }
  // 两拐的场景
  else if ( lvc_vehicle1_grid_x == lvc_vehicle2_grid_x && lvc_vehicle1_grid_y !=lvc_vehicle2_grid_y &&lvc_vehicle1_grid_x % 2 == 1)
  {
    lvo_los_status = 2;//ADHOC_Type_NNLOS;   
  }  
  else if ( lvc_vehicle1_grid_y == lvc_vehicle2_grid_y && lvc_vehicle1_grid_x != lvc_vehicle2_grid_x && lvc_vehicle1_grid_y % 2 == 1)
  {
    lvo_los_status = 2;//ADHOC_Type_NNLOS;
  }
  else if(lvc_vehicle1_grid_x != lvc_vehicle2_grid_x &&lvc_vehicle1_grid_y != lvc_vehicle2_grid_y &&  lvc_vehicle1_grid_x % 2 == 0 &&  lvc_vehicle2_grid_x % 2 == 0 &&  lvc_vehicle1_grid_y % 2 == 1 &&  lvc_vehicle2_grid_y % 2 == 1)
  {
    lvo_los_status = 2;//ADHOC_Type_NNLOS;   
  }
  else if(lvc_vehicle1_grid_x != lvc_vehicle2_grid_x && lvc_vehicle1_grid_y != lvc_vehicle2_grid_y&&  lvc_vehicle1_grid_x % 2  == 1 &&  lvc_vehicle2_grid_x % 2 == 1 &&  lvc_vehicle1_grid_y % 2 == 0 && lvc_vehicle2_grid_y % 2 == 0)
  {
    lvo_los_status = 2;    
  }
  else 
  {             
    lvo_los_status = 1;     
  }
  return (int)lvo_los_status;
}


} // namespace ns3
