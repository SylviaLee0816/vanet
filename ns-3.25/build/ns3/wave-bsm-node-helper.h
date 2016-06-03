#ifndef WAVE_BSM_NODE_HELPER_H
#define WAVE_BSM_NODE_HELPER_H

#include <vector>
#include "ns3/wave-bsm-stats.h"
#include "ns3/bsm-application-node.h"
#include "ns3/object-factory.h"
#include "ns3/application-container.h"
#include "ns3/nstime.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/mobility-model.h"

namespace ns3 {

class WaveBsmNodeHelper
{
public:

	WaveBsmNodeHelper ();

	void SetAttribute (std::string name, const AttributeValue &value);

	ApplicationContainer Install (NodeContainer nodes) const;

	void Install (NodeContainer & nodes,
                        Time totalTime,          // seconds
                        uint32_t wavePacketSize, // bytes
                        Time waveInterval,       // seconds
                        double gpsAccuracyNs,    // clock drift range in number of ns
                        // std::vector <double> ranges,           // m
                        int chAccessMode,        // channel access mode
                        Time txMaxDelay);

	Ptr<WaveBsmStats> GetWaveBsmStats ();

	int64_t AssignStreams (NodeContainer c, int64_t stream);

	static std::vector<int>& GetNodesMoving ();

private:
	Ptr<Application> InstallPriv (Ptr<Node> node) const;

	ObjectFactory m_factory; //!< Object factory.
  	WaveBsmStats m_waveBsmStats;
  	// tx safety range squared, for optimization
  	std::vector <double> m_txSafetyRangesSq;
  	static std::vector<int> nodesMoving;

};

}

#endif