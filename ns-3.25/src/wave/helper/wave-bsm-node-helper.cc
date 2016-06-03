#include "ns3/wave-bsm-node-helper.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("WaveBsmNodeHelper");

namespace ns3 {

std::vector<int> WaveBsmNodeHelper::nodesMoving;

WaveBsmNodeHelper::WaveBsmNodeHelper ()
  : m_waveBsmStats ()
{
  m_txSafetyRangesSq.resize (10, 0);
  m_txSafetyRangesSq[0] = 50.0 * 50.0;
  m_txSafetyRangesSq[1] = 100.0 * 100.0;
  m_txSafetyRangesSq[2] = 200.0 * 200.0;
  m_txSafetyRangesSq[3] = 300.0 * 300.0;
  m_txSafetyRangesSq[4] = 400.0 * 400.0;
  m_txSafetyRangesSq[5] = 500.0 * 500.0;
  m_txSafetyRangesSq[6] = 600.0 * 600.0;
  m_txSafetyRangesSq[7] = 800.0 * 800.0;
  m_txSafetyRangesSq[8] = 1000.0 * 1000.0;
  m_txSafetyRangesSq[9] = 1500.0 * 1500.0;

  m_factory.SetTypeId ("ns3::BsmApplicationNode");
}

void
WaveBsmNodeHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
WaveBsmNodeHelper::Install(NodeContainer nodes) const
{
  ApplicationContainer apps;
  for(uint32_t i = 0; i != nodes.GetN(); ++i){
    apps.Add(InstallPriv(nodes.Get(i)));
  }

  return apps;
}

Ptr<Application>
WaveBsmNodeHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

void
WaveBsmNodeHelper::Install (NodeContainer & nodes,
                        Time totalTime,          // seconds
                        uint32_t wavePacketSize, // bytes
                        Time waveInterval,       // seconds
                        double gpsAccuracyNs,    // clock drift range in number of ns
                        // std::vector <double> ranges,           // m
                        int chAccessMode,        // channel access mode
                        Time txMaxDelay)         // max delay prior to transmit
{

  int size = 1000;
  m_txSafetyRangesSq.clear ();
  m_txSafetyRangesSq.resize (size, 0);
  for (int index = 0; index < size; index++)
  {
      // stored as square of value, for optimization
      m_txSafetyRangesSq[index] = 1000*1000;
  }
  // install a BsmApplication on each node
  ApplicationContainer bsmApps = Install (nodes);
  // start BSM app immediately (BsmApplication will
  // delay transmission of first BSM by 1.0 seconds)
  bsmApps.Start (Seconds (0));
  bsmApps.Stop (totalTime);

  // for each app, setup the app parameters
  ApplicationContainer::Iterator aci;
  int nodeId = 0;
  for (aci = bsmApps.Begin (); aci != bsmApps.End (); ++aci)
    {
      Ptr<BsmApplicationNode> bsmApp = DynamicCast<BsmApplicationNode> (*aci);
      bsmApp->Setup (nodes,
                     nodeId,
                     totalTime,
                     wavePacketSize,
                     waveInterval,
                     gpsAccuracyNs,
                     // m_txSafetyRangesSq,
                     GetWaveBsmStats (),
                     &nodesMoving,
                     chAccessMode,
                     txMaxDelay);
      nodeId++;
    }

}

Ptr<WaveBsmStats>
WaveBsmNodeHelper::GetWaveBsmStats ()
{
  return &m_waveBsmStats;
}

int64_t
WaveBsmNodeHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      for (uint32_t j = 0; j < node->GetNApplications (); j++)
        {
          Ptr<BsmApplicationNode> bsmApp = DynamicCast<BsmApplicationNode> (node->GetApplication (j));
          if (bsmApp)
            {
              currentStream += bsmApp->AssignStreams (currentStream);
            }
        }
    }
  return (currentStream - stream);
}

std::vector<int>&
WaveBsmNodeHelper::GetNodesMoving ()
{
  return nodesMoving;
}

}