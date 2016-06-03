/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 North Carolina State University
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
 * Author: Scott E. Carpenter <scarpen@ncsu.edu>
 *
 */

#include "ns3/bsm-application-node.h"
#include "ns3/seq-ts-header.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/wave-net-device.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/wave-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/mobility-helper.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("BsmApplicationNode");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BsmApplicationNode);

TypeId
BsmApplicationNode::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BsmApplicationNode")
    .SetParent<Application> ()
    .SetGroupName ("Wave")
    .AddConstructor<BsmApplicationNode> ()
    ;
  return tid;
}

BsmApplicationNode::BsmApplicationNode ()
  : m_waveBsmStats (0),
    m_txSafetyRangesSq (),
    m_TotalSimTime (Seconds (10)),
    m_wavePacketSize (200),
    m_numWavePackets (1),
    m_waveInterval (MilliSeconds (100)),
    m_gpsAccuracyNs (10000),
    m_nodes (0),
    m_nodesMoving (0),
    m_unirv (0),
    m_nodeId (0),
    m_chAccessMode (0),
    frameSendKindCount (0),
    m_txMaxDelay (MilliSeconds (10)),
    m_prevTxDelay (MilliSeconds (0))
{
  m_TotalSimTime = Seconds (10);
  NS_LOG_FUNCTION (this);
}

BsmApplicationNode::~BsmApplicationNode ()
{
  NS_LOG_FUNCTION (this);
}

void
BsmApplicationNode::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  // chain up
  Application::DoDispose ();
}

void
BsmApplicationNode::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoInitialize ();
}

// Application Methods
void BsmApplicationNode::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // setup generation of WAVE BSM messages
  Time waveInterPacketInterval = m_waveInterval;

  // BSMs are not transmitted for the first second
  Time startTime = Seconds (1.0);
  // total length of time transmitting WAVE packets
  Time totalTxTime = m_TotalSimTime - startTime;
  // total WAVE packets needing to be sent
  m_numWavePackets = (uint32_t) (totalTxTime.GetDouble () / m_waveInterval.GetDouble ());

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  Ptr<WaveNetDevice> device = DynamicCast<WaveNetDevice> (m_nodes->Get (m_nodeId)->GetDevice (0));
  device->SetReceiveCallback (MakeCallback (&BsmApplicationNode::ReceiveWavePacket, this));

  // Transmission start time for each BSM:
  // We assume that the start transmission time
  // for the first packet will be on a ns-3 time
  // "Second" boundary - e.g., 1.0 s.
  // However, the actual transmit time must reflect
  // additional effects of 1) clock drift and
  // 2) transmit delay requirements.
  // 1) Clock drift - clocks are not perfectly
  // synchronized across all nodes.  In a VANET
  // we assume all nodes sync to GPS time, which
  // itself is assumed  accurate to, say, 40-100 ns.
  // Thus, the start transmission time must be adjusted
  // by some value, t_drift.
  // 2) Transmit delay requirements - The US
  // minimum performance requirements for V2V
  // BSM transmission expect a random delay of
  // +/- 5 ms, to avoid simultanous transmissions
  // by all vehicles congesting the channel.  Thus,
  // we need to adjust the start trasmission time by
  // some value, t_tx_delay.
  // Therefore, the actual transmit time should be:
  // t_start = t_time + t_drift + t_tx_delay
  // t_drift is always added to t_time.
  // t_tx_delay is supposed to be +/- 5ms, but if we
  // allow negative numbers the time could drift to a value
  // BEFORE the interval start time (i.e., at 100 ms
  // boundaries, we do not want to drift into the
  // previous interval, such as at 95 ms.  Instead,
  // we always want to be at the 100 ms interval boundary,
  // plus [0..10] ms tx delay.
  // Thus, the average t_tx_delay will be
  // within the desired range of [0..10] ms of
  // (t_time + t_drift)

  // WAVE devices sync to GPS time
  // and all devices would like to begin broadcasting
  // their safety messages immediately at the start of
  // the CCH interval.  However, if all do so, then
  // significant collisions occur.  Thus, we assume there
  // is some GPS sync accuracy on GPS devices,
  // typically 40-100 ns.
  // Get a uniformly random number for GPS sync accuracy, in ns.
  Time tDrift = NanoSeconds (m_unirv->GetInteger (0, m_gpsAccuracyNs));

  // When transmitting at a default rate of 10 Hz,
  // the subsystem shall transmit every 100 ms +/-
  // a random value between 0 and 5 ms. [MPR-BSMTX-TXTIM-002]
  // Source: CAMP Vehicle Safety Communications 4 Consortium
  // On-board Minimum Performance Requirements
  // for V2V Safety Systems Version 1.0, December 17, 2014
  // max transmit delay (default 10ms)
  // get value for transmit delay, as number of ns
  uint32_t d_ns = static_cast<uint32_t> (m_txMaxDelay.GetInteger ());
  // convert random tx delay to ns-3 time
  // see note above regarding centering tx delay
  // offset by 5ms + a random value.
  Time txDelay = NanoSeconds (m_unirv->GetInteger (0, d_ns));
  m_prevTxDelay = txDelay;

  Time txTime = startTime + tDrift + txDelay;
  // schedule transmission of first packet
  Simulator::Schedule (txTime, &BsmApplicationNode::GenerateWaveTraffic, this,
                        m_wavePacketSize, m_numWavePackets, waveInterPacketInterval, m_nodeId);
}

void BsmApplicationNode::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
}

void
BsmApplicationNode::Setup (NodeContainer & nodes,
                       int nodeId,
                       Time totalTime,
                       uint32_t wavePacketSize, // bytes
                       Time waveInterval,
                       double gpsAccuracyNs,
                       // std::vector <double> rangesSq,           // m ^2
                       Ptr<WaveBsmStats> waveBsmStats,
                       std::vector<int> * nodesMoving,
                       int chAccessMode,
                       Time txMaxDelay)
{
  NS_LOG_FUNCTION (this);
  m_unirv = CreateObject<UniformRandomVariable> ();
  m_TotalSimTime = totalTime;
  m_wavePacketSize = wavePacketSize;
  m_waveInterval = waveInterval;
  m_gpsAccuracyNs = gpsAccuracyNs;
  int size = 1000;
  m_waveBsmStats = waveBsmStats;
  m_nodesMoving = nodesMoving;
  m_chAccessMode = chAccessMode;
  m_txSafetyRangesSq.clear ();
  m_txSafetyRangesSq.resize (size, 0);

  for (int index = 0; index < size; index++)
    {
      // stored as square of value, for optimization
      m_txSafetyRangesSq[index] = 1000;
    }

  m_nodes = &nodes;
  m_nodeId = nodeId;
  m_txMaxDelay = txMaxDelay;
}

void
BsmApplicationNode::GenerateWaveTraffic (uint32_t pktSize,
                                     uint32_t pktCount, Time pktInterval,
                                     uint32_t sendingNodeId)
{
  NS_LOG_FUNCTION (this);
   
  // more packets to send?
  if (pktCount > 0)
    {
      if(frameSendKindCount == 3){
        pktSize = 300;
      }else{
        pktSize =100;
      }
      // for now, we cannot tell if each node has
      // started mobility.  so, as an optimization
      // only send if  this node is moving
      // if not, then skip
      int txNodeId = sendingNodeId;
      Ptr<Node> txNode = m_nodes->Get (txNodeId);
      Ptr<WaveNetDevice>  sender = DynamicCast<WaveNetDevice> (txNode->GetDevice (0));
      Ptr<MobilityModel> txPosition = txNode->GetObject<MobilityModel> ();
      const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
      Mac48Address bssWildcard = Mac48Address::GetBroadcast ();

      const TxInfo txInfo = TxInfo (CCH);
      Ptr<Packet> p  = Create<Packet> (pktSize);
      //SeqTsHeader seqTs;
     // seqTs.SetSeq (txNodeId);
      //p->AddHeader (seqTs);

      NS_ASSERT (txPosition != 0);
    
      // send it!
      sender->SendX (p, bssWildcard, WSMP_PROT_NUMBER, txInfo);
      // count it
      m_waveBsmStats->IncTxPktCount ();
      m_waveBsmStats->IncTxByteCount (pktSize);
      int wavePktsSent = m_waveBsmStats->GetTxPktCount ();
      if ((m_waveBsmStats->GetLogging () != 0) && ((wavePktsSent % 1000) == 0))
      {
        NS_LOG_UNCOND ("Sending WAVE pkt # " << wavePktsSent );
      }
 
      frameSendKindCount ++;
      frameSendKindCount = frameSendKindCount % 4;

      // every BSM must be scheduled with a tx time delay
      // of +/- (5) ms.  See comments in StartApplication().
      // we handle this as a tx delay of [0..10] ms
      // from the start of the pktInterval boundary
      uint32_t d_ns = static_cast<uint32_t> (m_txMaxDelay.GetInteger ());
      Time txDelay = NanoSeconds (m_unirv->GetInteger (0, d_ns));

      // do not want the tx delay to be cumulative, so
      // deduct the previous delay value.  thus we adjust
      // to schedule the next event at the next pktInterval,
      // plus some new [0..10] ms tx delay
      Time txTime = pktInterval - m_prevTxDelay + txDelay;
      m_prevTxDelay = txDelay;

      Simulator::Schedule (txTime, &BsmApplicationNode::GenerateWaveTraffic, this,
                                pktSize, pktCount - 1, pktInterval, m_nodeId);
    }
  else
    {
      
    }
}

bool BsmApplicationNode::ReceiveWavePacket (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender)
{
  NS_LOG_FUNCTION (this);
  //SeqTsHeader seqTs;
  //pkt->PeekHeader (seqTs);
  std::cout << "receive a packet: " << std::endl
          //  << "  sequence = " << seqTs.GetSeq () << "," << std::endl
            //<< "  sendTime = " << seqTs.GetTs ().GetSeconds () << "s," << std::endl
            << "  Uid = " << pkt->GetUid ()<< std::endl
            << "  recvTime = " << Now ().GetSeconds () << "s," << std::endl;
          //  << "  protocol = 0x" << std::hex << mode << std::dec  << std::endl;
   return true;
  }

/*void BsmApplicationNode::HandleReceivedBsmPacket (Ptr<Node> txNode,
                                              Ptr<Node> rxNode)
{
  NS_LOG_FUNCTION (this);

  m_waveBsmStats->IncRxPktCount ();

  Ptr<MobilityModel> rxPosition = rxNode->GetObject<MobilityModel> ();
  NS_ASSERT (rxPosition != 0);
  // confirm that the receiving node
  // has also started moving in the scenario
  // if it has not started moving, then
  // it is not a candidate to receive a packet
  int rxNodeId = rxNode->GetId ();
  int receiverMoving = m_nodesMoving->at (rxNodeId);
  if (receiverMoving == 1)
    {
      double rxDistSq = MobilityHelper::GetDistanceSquaredBetween (rxNode, txNode);
      if (rxDistSq > 0.0)
        {
          int rangeCount = m_txSafetyRangesSq.size ();
          for (int index = 1; index <= rangeCount; index++)
            {
              if (rxDistSq <= m_txSafetyRangesSq[index - 1])
                {
                  m_waveBsmStats->IncRxPktInRangeCount (index);
                }
            }
        }
    }
}*/

int64_t
BsmApplicationNode::AssignStreams (int64_t streamIndex)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_unirv);  // should be set by Setup() prevoiusly
  m_unirv->SetStream (streamIndex);

  return 1;
}

} // namespace ns3
