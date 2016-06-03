/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Junling Bu <linlinjavaer@gmail.com>
 */
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/seq-ts-header.h"
#include "ns3/wave-net-device.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/wave-helper.h"
#include "ns3/wave-bsm-node-helper.h"
#include "ns3/string.h"
#include "ns3/application-control.h"

using namespace ns3;
/**
 * This simulation is to show the routing service of WaveNetDevice described in IEEE 09.4.
 *
 * note: although txPowerLevel is supported now, the "TxPowerLevels"
 * attribute of YansWifiPhy is 1 which means phy devices only support 1
 * levels. Thus, if users want to control txPowerLevel, they should set
 * these attributes of YansWifiPhy by themselves..
 */

class WaveNetDeviceExample
{
public:
  void HighwaySendWsmpExample (void);

  void CitySendWsmpExample (void);

  int VehicleNumberOfLine;   // the number of vehicle each line

  int LineNumber;    // the number of line 

  double LineWidth;

private:
  void CreateHighwayWaveNodes (void);

  void CreateCityWaveNodes (void);

  NodeContainer total;

  NetDeviceContainer devices;

  Ptr<ApplicationControl> control;
};

void
WaveNetDeviceExample::CreateHighwayWaveNodes (void)
{
  VehicleNumberOfLine = 1;
  LineNumber = 4;
  LineWidth = 2.5;
  
  MobilityHelper mobility;

  for(int k1 = 0; k1 < LineNumber; k1 ++)
  {
    NodeContainer temp = NodeContainer ();
    temp.Create (VehicleNumberOfLine);
    // PostionAllcation init
    Ptr<LinePositionAllocator> positionAlloc = CreateObject<LinePositionAllocator> ();
  
    if(k1 < LineNumber / 2) {
      positionAlloc->SetDirection (0, LineWidth * k1);
      positionAlloc->SetMaxX (6000);
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::HighwayMobilityModel","Speed",VectorValue(Vector(30.0,0.0,0.0)));
    } else {
      positionAlloc->SetDirection (1, LineWidth * k1);
      positionAlloc->SetMaxX (6000);
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::HighwayMobilityModel","Speed",VectorValue(Vector(-30.0,0.0,0.0)));
    }

    mobility.Install (temp);

    if(k1 == 0) {
      total = temp;
    } else {
      total = NodeContainer (temp,total);
    }

  }

  WaveBsmNodeHelper wavebsmhelper;
  wavebsmhelper.Install(total,Seconds(5),300,MilliSeconds (100),0,0,MilliSeconds (10));

  YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Mobility (0);

  YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
  wavePhy.SetChannel (waveChannel.Create ());
  wavePhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();
  devices = waveHelper.Install (wavePhy, waveMac, total);

  // Tracing
  //wavePhy.EnablePcap ("wave-simple-device", devices);
}

void
WaveNetDeviceExample::CreateCityWaveNodes (void)
{
  VehicleNumberOfLine = 1;
  LineNumber = 16;
  LineWidth = 2.5;
  double spaceX = LineWidth * 2 + 230;
  double spaceY = LineWidth * 2 + 413;

  MobilityHelper mobility;

  control =  CreateObject<ApplicationControl> ();
  control->CreateTxtNote (11);

  for (int k2 = 0;k2 < LineNumber;k2 ++) 
  {
    NodeContainer temp = NodeContainer ();
    temp.Create (VehicleNumberOfLine);
    Ptr<LinePositionAllocator> positionAlloc = CreateObject<LinePositionAllocator> ();
    if (k2 < LineNumber / 2) {
      if (k2 % 2 == 0) {
        positionAlloc->SetDirection (2, spaceX * (k2 / 2));
        positionAlloc->SetMaxY (1299);
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::CityMobilityModel","Speed",VectorValue(Vector(0.0,-30.0,0.0)));
      } else {
        positionAlloc->SetDirection (3, spaceX * (k2-1) / 2 + LineWidth);
        positionAlloc->SetMaxY (1299);
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::CityMobilityModel","Speed",VectorValue(Vector(0.0,30.0,0.0)));
      }
    } else {
      if (k2 % 2 == 0) {
        positionAlloc->SetDirection (0, spaceY * (k2 - LineNumber/2) / 2);
        positionAlloc->SetMaxX (750);
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::CityMobilityModel","Speed",VectorValue(Vector(30.0,0.0,0.0)));
      } else {
        positionAlloc->SetDirection (1, spaceY * (k2 - LineNumber/2 - 1) / 2 + LineWidth);
        positionAlloc->SetMaxX (750);
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::CityMobilityModel","Speed",VectorValue(Vector(-30.0,0.0,0.0)));
      }
    }

    mobility.DoSetAppControl(control);

    mobility.Install (temp);

    if (k2 == 0) {
      total = temp;
    } else {
      total = NodeContainer (temp, total);
    }
  }

  WaveBsmNodeHelper wavebsmhelper;
  wavebsmhelper.Install(total,Seconds(5),300,MilliSeconds (100),0,0,MilliSeconds (10));

  YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Mobility (1);
  //waveChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //waveChannel.AddPropagationLoss ("ns3::CityPropagationLossModel");

  YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
  wavePhy.SetChannel (waveChannel.Create ());
  wavePhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();
  devices = waveHelper.Install (wavePhy, waveMac, total);
}

void
WaveNetDeviceExample::HighwaySendWsmpExample ()
{
  CreateHighwayWaveNodes ();
  Simulator::Run ();
  Simulator::Destroy ();
}

void
WaveNetDeviceExample::CitySendWsmpExample ()
{
  CreateCityWaveNodes ();
  Simulator::Run ();
  Simulator::Destroy ();
}

int
main (int argc, char *argv[])
{
  WaveNetDeviceExample example;
  // control = ApplicationControl ();
  // control.CreateTxtNote();

  std::string s;

  std::cout << "Choose your Mobility Model (highway or city):" << std::endl;
  std::cin >> s;

  std::cout << "run WAVE WSMP routing service case:" << std::endl;
  if(s == "highway")
    example.HighwaySendWsmpExample ();
  else if (s == "city")
    example.CitySendWsmpExample ();

  return 0;
}
