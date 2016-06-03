/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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
 * Contributions: Timo Bingmann <timo.bingmann@student.kit.edu>
 * Contributions: Tom Hewer <tomhewer@mac.com> for Two Ray Ground Model
 *                Pavel Boyko <boyko@iitp.ru> for matrix
 */

#include "propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PropagationLossModel");

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PropagationLossModel);

TypeId 
PropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PropagationLossModel")
    .SetParent<Object> ()
  ;
  return tid;
}

PropagationLossModel::PropagationLossModel ()
  : m_next (0)
{
}

PropagationLossModel::~PropagationLossModel ()
{
}

void
PropagationLossModel::SetNext (Ptr<PropagationLossModel> next)
{
  m_next = next;
}

Ptr<PropagationLossModel>
PropagationLossModel::GetNext ()
{
  return m_next;
}

double
PropagationLossModel::CalcRxPower (double txPowerDbm,
                                   Ptr<MobilityModel> a,
                                   Ptr<MobilityModel> b) const
{
  double self = DoCalcRxPower (txPowerDbm, a, b);
  if (m_next != 0)
    {
      self = m_next->CalcRxPower (self, a, b);
    }
  return self;
}
    


int64_t
PropagationLossModel::AssignStreams (int64_t stream)
{
  int64_t currentStream = stream;
  currentStream += DoAssignStreams (stream);
  if (m_next != 0)
    {
      currentStream += m_next->AssignStreams (currentStream);
    }
  return (currentStream - stream);
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (RandomPropagationLossModel);

TypeId 
RandomPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<RandomPropagationLossModel> ()
    .AddAttribute ("Variable", "The random variable used to pick a loss everytime CalcRxPower is invoked.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&RandomPropagationLossModel::m_variable),
                   MakePointerChecker<RandomVariableStream> ())
  ;
  return tid;
}
RandomPropagationLossModel::RandomPropagationLossModel ()
  : PropagationLossModel ()
{
}

RandomPropagationLossModel::~RandomPropagationLossModel ()
{
}

double
RandomPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                           Ptr<MobilityModel> a,
                                           Ptr<MobilityModel> b) const
{
  double rxc = -m_variable->GetValue ();
  NS_LOG_DEBUG ("attenuation coefficient="<<rxc<<"Db");
  return txPowerDbm + rxc;
}

int64_t
RandomPropagationLossModel::DoAssignStreams (int64_t stream)
{
  m_variable->SetStream (stream);
  return 1;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (FriisPropagationLossModel);

TypeId 
FriisPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FriisPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Propagation")
    .AddConstructor<FriisPropagationLossModel> ()
    .AddAttribute ("Frequency", 
                   "The carrier frequency (in Hz) at which propagation occurs  (default is 5.15 GHz).",
                   DoubleValue (5.150e9),
                   MakeDoubleAccessor (&FriisPropagationLossModel::SetFrequency,
                                       &FriisPropagationLossModel::GetFrequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SystemLoss", "The system loss",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&FriisPropagationLossModel::m_systemLoss),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinLoss", 
                   "The minimum value (dB) of the total loss, used at short ranges. Note: ",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&FriisPropagationLossModel::SetMinLoss,
                                       &FriisPropagationLossModel::GetMinLoss),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

FriisPropagationLossModel::FriisPropagationLossModel ()
{
}
void
FriisPropagationLossModel::SetSystemLoss (double systemLoss)
{
  m_systemLoss = systemLoss;
}
double
FriisPropagationLossModel::GetSystemLoss (void) const
{
  return m_systemLoss;
}
void
FriisPropagationLossModel::SetMinLoss (double minLoss)
{
  m_minLoss = minLoss;
}
double
FriisPropagationLossModel::GetMinLoss (void) const
{
  return m_minLoss;
}

void
FriisPropagationLossModel::SetFrequency (double frequency)
{
  m_frequency = frequency;
  static const double C = 299792458.0; // speed of light in vacuum
  m_lambda = C / frequency;
}

double
FriisPropagationLossModel::GetFrequency (void) const
{
  return m_frequency;
}

double
FriisPropagationLossModel::DbmToW (double dbm) const
{
  double mw = std::pow (10.0,dbm/10.0);
  return mw / 1000.0;
}

double
FriisPropagationLossModel::DbmFromW (double w) const
{
  double dbm = std::log10 (w * 1000.0) * 10.0;
  return dbm;
}

double 
FriisPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                          Ptr<MobilityModel> a,
                                          Ptr<MobilityModel> b) const
{
  /*
   * Friis free space equation:
   * where Pt, Gr, Gr and P are in Watt units
   * L is in meter units.
   *
   *    P     Gt * Gr * (lambda^2)
   *   --- = ---------------------
   *    Pt     (4 * pi * d)^2 * L
   *
   * Gt: tx gain (unit-less)
   * Gr: rx gain (unit-less)
   * Pt: tx power (W)
   * d: distance (m)
   * L: system loss
   * lambda: wavelength (m)
   *
   * Here, we ignore tx and rx gain and the input and output values 
   * are in dB or dBm:
   *
   *                           lambda^2
   * rx = tx +  10 log10 (-------------------)
   *                       (4 * pi * d)^2 * L
   *
   * rx: rx power (dB)
   * tx: tx power (dB)
   * d: distance (m)
   * L: system loss (unit-less)
   * lambda: wavelength (m)
   */
  double distance = a->GetDistanceFrom (b);
  if (distance < 3*m_lambda)
    {
      NS_LOG_WARN ("distance not within the far field region => inaccurate propagation loss value");
    }
  if (distance <= 0)
    {
      return txPowerDbm - m_minLoss;
    }
  double numerator = m_lambda * m_lambda;
  double denominator = 16 * M_PI * M_PI * distance * distance * m_systemLoss;
  double lossDb = -10 * log10 (numerator / denominator);
  NS_LOG_DEBUG ("distance=" << distance<< "m, loss=" << lossDb <<"dB");
  return txPowerDbm - std::max (lossDb, m_minLoss);
}

int64_t
FriisPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

// ------------------------------------------------------------------------- //
// -- Two-Ray Ground Model ported from NS-2 -- tomhewer@mac.com -- Nov09 //

NS_OBJECT_ENSURE_REGISTERED (TwoRayGroundPropagationLossModel);

TypeId 
TwoRayGroundPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TwoRayGroundPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<TwoRayGroundPropagationLossModel> ()
    .AddAttribute ("Frequency", 
                   "The carrier frequency (in Hz) at which propagation occurs  (default is 5.15 GHz).",
                   DoubleValue (5.150e9),
                   MakeDoubleAccessor (&TwoRayGroundPropagationLossModel::SetFrequency,
                                       &TwoRayGroundPropagationLossModel::GetFrequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SystemLoss", "The system loss",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&TwoRayGroundPropagationLossModel::m_systemLoss),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinDistance",
                   "The distance under which the propagation model refuses to give results (m)",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&TwoRayGroundPropagationLossModel::SetMinDistance,
                                       &TwoRayGroundPropagationLossModel::GetMinDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("HeightAboveZ",
                   "The height of the antenna (m) above the node's Z coordinate",
                   DoubleValue (0),
                   MakeDoubleAccessor (&TwoRayGroundPropagationLossModel::m_heightAboveZ),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TwoRayGroundPropagationLossModel::TwoRayGroundPropagationLossModel ()
{
}
void
TwoRayGroundPropagationLossModel::SetSystemLoss (double systemLoss)
{
  m_systemLoss = systemLoss;
}
double
TwoRayGroundPropagationLossModel::GetSystemLoss (void) const
{
  return m_systemLoss;
}
void
TwoRayGroundPropagationLossModel::SetMinDistance (double minDistance)
{
  m_minDistance = minDistance;
}
double
TwoRayGroundPropagationLossModel::GetMinDistance (void) const
{
  return m_minDistance;
}
void
TwoRayGroundPropagationLossModel::SetHeightAboveZ (double heightAboveZ)
{
  m_heightAboveZ = heightAboveZ;
}

void
TwoRayGroundPropagationLossModel::SetFrequency (double frequency)
{
  m_frequency = frequency;
  static const double C = 299792458.0; // speed of light in vacuum
  m_lambda = C / frequency;
}

double
TwoRayGroundPropagationLossModel::GetFrequency (void) const
{
  return m_frequency;
}

double 
TwoRayGroundPropagationLossModel::DbmToW (double dbm) const
{
  double mw = std::pow (10.0,dbm / 10.0);
  return mw / 1000.0;
}

double
TwoRayGroundPropagationLossModel::DbmFromW (double w) const
{
  double dbm = std::log10 (w * 1000.0) * 10.0;
  return dbm;
}

double 
TwoRayGroundPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                                 Ptr<MobilityModel> a,
                                                 Ptr<MobilityModel> b) const
{
  /*
   * Two-Ray Ground equation:
   *
   * where Pt, Gt and Gr are in dBm units
   * L, Ht and Hr are in meter units.
   *
   *   Pr      Gt * Gr * (Ht^2 * Hr^2)
   *   -- =  (-------------------------)
   *   Pt            d^4 * L
   *
   * Gt: tx gain (unit-less)
   * Gr: rx gain (unit-less)
   * Pt: tx power (dBm)
   * d: distance (m)
   * L: system loss
   * Ht: Tx antenna height (m)
   * Hr: Rx antenna height (m)
   * lambda: wavelength (m)
   *
   * As with the Friis model we ignore tx and rx gain and output values
   * are in dB or dBm
   *
   *                      (Ht * Ht) * (Hr * Hr)
   * rx = tx + 10 log10 (-----------------------)
   *                      (d * d * d * d) * L
   */
  double distance = a->GetDistanceFrom (b);
  if (distance <= m_minDistance)
    {
      return txPowerDbm;
    }

  // Set the height of the Tx and Rx antennae
  double txAntHeight = a->GetPosition ().z + m_heightAboveZ;
  double rxAntHeight = b->GetPosition ().z + m_heightAboveZ;

  // Calculate a crossover distance, under which we use Friis
  /*
   * 
   * dCross = (4 * pi * Ht * Hr) / lambda
   *
   */

  double dCross = (4 * M_PI * txAntHeight * rxAntHeight) / m_lambda;
  double tmp = 0;
  if (distance <= dCross)
    {
      // We use Friis
      double numerator = m_lambda * m_lambda;
      tmp = M_PI * distance;
      double denominator = 16 * tmp * tmp * m_systemLoss;
      double pr = 10 * std::log10 (numerator / denominator);
      NS_LOG_DEBUG ("Receiver within crossover (" << dCross << "m) for Two_ray path; using Friis");
      NS_LOG_DEBUG ("distance=" << distance << "m, attenuation coefficient=" << pr << "dB");
      return txPowerDbm + pr;
    }
  else   // Use Two-Ray Pathloss
    {
      tmp = txAntHeight * rxAntHeight;
      double rayNumerator = tmp * tmp;
      tmp = distance * distance;
      double rayDenominator = tmp * tmp * m_systemLoss;
      double rayPr = 10 * std::log10 (rayNumerator / rayDenominator);
      NS_LOG_DEBUG ("distance=" << distance << "m, attenuation coefficient=" << rayPr << "dB");
      return txPowerDbm + rayPr;

    }
}

int64_t
TwoRayGroundPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (LogDistancePropagationLossModel);

TypeId
LogDistancePropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LogDistancePropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Propagation")
    .AddConstructor<LogDistancePropagationLossModel> ()
    .AddAttribute ("Exponent",
                   "The exponent of the Path Loss propagation model",
                   DoubleValue (3.0),
                   MakeDoubleAccessor (&LogDistancePropagationLossModel::m_exponent),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReferenceDistance",
                   "The distance at which the reference loss is calculated (m)",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&LogDistancePropagationLossModel::m_referenceDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReferenceLoss",
                   "The reference loss at reference distance (dB). (Default is Friis at 1m with 5.15 GHz)",
                   DoubleValue (46.6777),
                   MakeDoubleAccessor (&LogDistancePropagationLossModel::m_referenceLoss),
                   MakeDoubleChecker<double> ())
  ;
  return tid;

}

LogDistancePropagationLossModel::LogDistancePropagationLossModel ()
{
}

void
LogDistancePropagationLossModel::SetPathLossExponent (double n)
{
  m_exponent = n;
}
void
LogDistancePropagationLossModel::SetReference (double referenceDistance, double referenceLoss)
{
  m_referenceDistance = referenceDistance;
  m_referenceLoss = referenceLoss;
}
double
LogDistancePropagationLossModel::GetPathLossExponent (void) const
{
  return m_exponent;
}

double
LogDistancePropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                                Ptr<MobilityModel> a,
                                                Ptr<MobilityModel> b) const
{
  double distance = a->GetDistanceFrom (b);
  if (distance <= m_referenceDistance)
    {
      return txPowerDbm;
    }
  /**
   * The formula is:
   * rx = 10 * log (Pr0(tx)) - n * 10 * log (d/d0)
   *
   * Pr0: rx power at reference distance d0 (W)
   * d0: reference distance: 1.0 (m)
   * d: distance (m)
   * tx: tx power (dB)
   * rx: dB
   *
   * Which, in our case is:
   *
   * rx = rx0(tx) - 10 * n * log (d/d0)
   */
  double pathLossDb = 10 * m_exponent * std::log10 (distance / m_referenceDistance);
  double rxc = -m_referenceLoss - pathLossDb;
  NS_LOG_DEBUG ("distance="<<distance<<"m, reference-attenuation="<< -m_referenceLoss<<"dB, "<<
                "attenuation coefficient="<<rxc<<"db");
  return txPowerDbm + rxc;
}

int64_t
LogDistancePropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (ThreeLogDistancePropagationLossModel);

TypeId
ThreeLogDistancePropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ThreeLogDistancePropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<ThreeLogDistancePropagationLossModel> ()
    .AddAttribute ("Distance0",
                   "Beginning of the first (near) distance field",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&ThreeLogDistancePropagationLossModel::m_distance0),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Distance1",
                   "Beginning of the second (middle) distance field.",
                   DoubleValue (200.0),
                   MakeDoubleAccessor (&ThreeLogDistancePropagationLossModel::m_distance1),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Distance2",
                   "Beginning of the third (far) distance field.",
                   DoubleValue (500.0),
                   MakeDoubleAccessor (&ThreeLogDistancePropagationLossModel::m_distance2),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Exponent0",
                   "The exponent for the first field.",
                   DoubleValue (1.9),
                   MakeDoubleAccessor (&ThreeLogDistancePropagationLossModel::m_exponent0),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Exponent1",
                   "The exponent for the second field.",
                   DoubleValue (3.8),
                   MakeDoubleAccessor (&ThreeLogDistancePropagationLossModel::m_exponent1),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Exponent2",
                   "The exponent for the third field.",
                   DoubleValue (3.8),
                   MakeDoubleAccessor (&ThreeLogDistancePropagationLossModel::m_exponent2),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReferenceLoss",
                   "The reference loss at distance d0 (dB). (Default is Friis at 1m with 5.15 GHz)",
                   DoubleValue (46.6777),
                   MakeDoubleAccessor (&ThreeLogDistancePropagationLossModel::m_referenceLoss),
                   MakeDoubleChecker<double> ())
  ;
  return tid;

}

ThreeLogDistancePropagationLossModel::ThreeLogDistancePropagationLossModel ()
{
}

double 
ThreeLogDistancePropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                                     Ptr<MobilityModel> a,
                                                     Ptr<MobilityModel> b) const
{
  double distance = a->GetDistanceFrom (b);
  NS_ASSERT (distance >= 0);

  // See doxygen comments for the formula and explanation

  double pathLossDb;

  if (distance < m_distance0)
    {
      pathLossDb = 0;
    }
  else if (distance < m_distance1)
    {
      pathLossDb = m_referenceLoss
        + 10 * m_exponent0 * std::log10 (distance / m_distance0);
    }
  else if (distance < m_distance2)
    {
      pathLossDb = m_referenceLoss
        + 10 * m_exponent0 * std::log10 (m_distance1 / m_distance0)
        + 10 * m_exponent1 * std::log10 (distance / m_distance1);
    }
  else
    {
      pathLossDb = m_referenceLoss
        + 10 * m_exponent0 * std::log10 (m_distance1 / m_distance0)
        + 10 * m_exponent1 * std::log10 (m_distance2 / m_distance1)
        + 10 * m_exponent2 * std::log10 (distance / m_distance2);
    }

  NS_LOG_DEBUG ("ThreeLogDistance distance=" << distance << "m, " <<
                "attenuation=" << pathLossDb << "dB");

  return txPowerDbm - pathLossDb;
}

int64_t
ThreeLogDistancePropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (NakagamiPropagationLossModel);

TypeId
NakagamiPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NakagamiPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Propagation")
    .AddConstructor<NakagamiPropagationLossModel> ()
    .AddAttribute ("Distance1",
                   "Beginning of the second distance field. Default is 80m.",
                   DoubleValue (80.0),
                   MakeDoubleAccessor (&NakagamiPropagationLossModel::m_distance1),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Distance2",
                   "Beginning of the third distance field. Default is 200m.",
                   DoubleValue (200.0),
                   MakeDoubleAccessor (&NakagamiPropagationLossModel::m_distance2),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("m0",
                   "m0 for distances smaller than Distance1. Default is 1.5.",
                   DoubleValue (1.5),
                   MakeDoubleAccessor (&NakagamiPropagationLossModel::m_m0),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("m1",
                   "m1 for distances smaller than Distance2. Default is 0.75.",
                   DoubleValue (0.75),
                   MakeDoubleAccessor (&NakagamiPropagationLossModel::m_m1),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("m2",
                   "m2 for distances greater than Distance2. Default is 0.75.",
                   DoubleValue (0.75),
                   MakeDoubleAccessor (&NakagamiPropagationLossModel::m_m2),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ErlangRv",
                   "Access to the underlying ErlangRandomVariable",
                   StringValue ("ns3::ErlangRandomVariable"),
                   MakePointerAccessor (&NakagamiPropagationLossModel::m_erlangRandomVariable),
                   MakePointerChecker<ErlangRandomVariable> ())
    .AddAttribute ("GammaRv",
                   "Access to the underlying GammaRandomVariable",
                   StringValue ("ns3::GammaRandomVariable"),
                   MakePointerAccessor (&NakagamiPropagationLossModel::m_gammaRandomVariable),
                   MakePointerChecker<GammaRandomVariable> ());
  ;
  return tid;

}

NakagamiPropagationLossModel::NakagamiPropagationLossModel ()
{
}

double
NakagamiPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                             Ptr<MobilityModel> a,
                                             Ptr<MobilityModel> b) const
{
  // select m parameter

  double distance = a->GetDistanceFrom (b);
  NS_ASSERT (distance >= 0);

  double m;
  if (distance < m_distance1)
    {
      m = m_m0;
    }
  else if (distance < m_distance2)
    {
      m = m_m1;
    }
  else
    {
      m = m_m2;
    }

  // the current power unit is dBm, but Watt is put into the Nakagami /
  // Rayleigh distribution.
  double powerW = std::pow (10, (txPowerDbm - 30) / 10);

  double resultPowerW;

  // switch between Erlang- and Gamma distributions: this is only for
  // speed. (Gamma is equal to Erlang for any positive integer m.)
  unsigned int int_m = static_cast<unsigned int>(std::floor (m));

  if (int_m == m)
    {
      resultPowerW = m_erlangRandomVariable->GetValue (int_m, powerW / m);
    }
  else
    {
      resultPowerW = m_gammaRandomVariable->GetValue (m, powerW / m);
    }

  double resultPowerDbm = 10 * std::log10 (resultPowerW) + 30;

  NS_LOG_DEBUG ("Nakagami distance=" << distance << "m, " <<
                "power=" << powerW <<"W, " <<
                "resultPower=" << resultPowerW << "W=" << resultPowerDbm << "dBm");

  return resultPowerDbm;
}

int64_t
NakagamiPropagationLossModel::DoAssignStreams (int64_t stream)
{
  m_erlangRandomVariable->SetStream (stream);
  m_gammaRandomVariable->SetStream (stream + 1);
  return 2;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (FixedRssLossModel);

TypeId 
FixedRssLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FixedRssLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Propagation")
    .AddConstructor<FixedRssLossModel> ()
    .AddAttribute ("Rss", "The fixed receiver Rss.",
                   DoubleValue (-150.0),
                   MakeDoubleAccessor (&FixedRssLossModel::m_rss),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}
FixedRssLossModel::FixedRssLossModel ()
  : PropagationLossModel ()
{
}

FixedRssLossModel::~FixedRssLossModel ()
{
}

void
FixedRssLossModel::SetRss (double rss)
{
  m_rss = rss;
}

double
FixedRssLossModel::DoCalcRxPower (double txPowerDbm,
                                  Ptr<MobilityModel> a,
                                  Ptr<MobilityModel> b) const
{
  return m_rss;
}

int64_t
FixedRssLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (MatrixPropagationLossModel);

TypeId 
MatrixPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MatrixPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Propagation")
    .AddConstructor<MatrixPropagationLossModel> ()
    .AddAttribute ("DefaultLoss", "The default value for propagation loss, dB.",
                   DoubleValue (std::numeric_limits<double>::max ()),
                   MakeDoubleAccessor (&MatrixPropagationLossModel::m_default),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

MatrixPropagationLossModel::MatrixPropagationLossModel ()
  : PropagationLossModel (), m_default (std::numeric_limits<double>::max ())
{
}

MatrixPropagationLossModel::~MatrixPropagationLossModel ()
{
}

void 
MatrixPropagationLossModel::SetDefaultLoss (double loss)
{
  m_default = loss;
}

void
MatrixPropagationLossModel::SetLoss (Ptr<MobilityModel> ma, Ptr<MobilityModel> mb, double loss, bool symmetric)
{
  NS_ASSERT (ma != 0 && mb != 0);

  MobilityPair p = std::make_pair (ma, mb);
  std::map<MobilityPair, double>::iterator i = m_loss.find (p);

  if (i == m_loss.end ())
    {
      m_loss.insert (std::make_pair (p, loss));
    }
  else
    {
      i->second = loss;
    }

  if (symmetric)
    {
      SetLoss (mb, ma, loss, false);
    }
}

double 
MatrixPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                           Ptr<MobilityModel> a,
                                           Ptr<MobilityModel> b) const
{
  std::map<MobilityPair, double>::const_iterator i = m_loss.find (std::make_pair (a, b));

  if (i != m_loss.end ())
    {
      return txPowerDbm - i->second;
    }
  else
    {
      return txPowerDbm - m_default;
    }
}

int64_t
MatrixPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (RangePropagationLossModel);

TypeId
RangePropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RangePropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<RangePropagationLossModel> ()
    .AddAttribute ("MaxRange",
                   "Maximum Transmission Range (meters)",
                   DoubleValue (250),
                   MakeDoubleAccessor (&RangePropagationLossModel::m_range),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

RangePropagationLossModel::RangePropagationLossModel ()
{
}

double
RangePropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                          Ptr<MobilityModel> a,
                                          Ptr<MobilityModel> b) const
{
  double distance = a->GetDistanceFrom (b);
  if (distance <= m_range)
    {
      return txPowerDbm;
    }
  else
    {
      return -1000;
    }
}

int64_t
RangePropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}
    

NS_OBJECT_ENSURE_REGISTERED (HighwayPropagationLossModel);
    
TypeId
HighwayPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HighwayPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<HighwayPropagationLossModel> ()
    .AddAttribute ("AntennaEffectiveHeight",
                   "Antenna Effective Height (meters), default is 0.5m",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&HighwayPropagationLossModel::m_antennaEffectiveHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AntennaHeight",
                   "Antenna Height (meters), default is 1.5m",
                   DoubleValue (1.5),
                   MakeDoubleAccessor (&HighwayPropagationLossModel::m_antennaHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinDistance",
                   "MinDistance (meters), default is 3m",
                   DoubleValue (3.0),
                   MakeDoubleAccessor (&HighwayPropagationLossModel::m_minDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxDistance",
                   "MaxDistance (meters), default is 5000m",
                   DoubleValue (5000.0),
                   MakeDoubleAccessor (&HighwayPropagationLossModel::m_maxDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Frequency",
                   "The carrier frequency (in GHz) at which propagation occurs  (default is 5.9 GHz).",
                   DoubleValue (5.9),
                   MakeDoubleAccessor (&HighwayPropagationLossModel::SetFrequency,
                                       &HighwayPropagationLossModel::GetFrequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ShadingDelta",
                   "ShadingDelta, default is 3.0",
                   DoubleValue (3.0),
                   MakeDoubleAccessor (&HighwayPropagationLossModel::m_slow_shading_delta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ShadingCorDis",
                   "ShadingCorDis, default is 10.0",
                   DoubleValue (10.0),
                   MakeDoubleAccessor (&HighwayPropagationLossModel::m_slow_shading_cor_dis),
                   MakeDoubleChecker<double> ())
    ;
    return tid;
}
    
    
HighwayPropagationLossModel::HighwayPropagationLossModel ()
{
}
    
HighwayPropagationLossModel::~HighwayPropagationLossModel ()
{
}
    
void
HighwayPropagationLossModel::SetFrequency (double frequency)
{
    m_frequency = frequency;
}
    
double
HighwayPropagationLossModel::GetFrequency (void) const
{
    return m_frequency;
}
    
double
HighwayPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                           Ptr<MobilityModel> a,
                                           Ptr<MobilityModel> b) const
{
   // std::cout << txPowerDbm <<"   dddddddddddddddddddddddd   " << std::endl;
    double  lvf_freespace_pathloss;
    double  lvf_d_bp;
    double  lvf_pathloss = -1000;	//初始化为无效值
    double  lvf_pathloss_db;
    
    double  rxPowerDbm;
    
    /*
     * 计算说明
     */
    
    double distance = a->GetDistanceFrom (b);
    lvf_freespace_pathloss = 20.0f * log10(distance) + 46.4 + 20.0 * log10(m_frequency/5.0);
    lvf_d_bp = 4 * m_antennaEffectiveHeight * m_antennaEffectiveHeight * m_frequency* (1.0e9) / (3*pow(10,8) + 0.0);
    
    if(distance >= m_maxDistance)
    {
        std::cout << "高速场景下LOS衰减计算程序错误，节点之间的距离为" << distance << std::endl
                  << " 超过了可能的最大距离" << m_maxDistance<< "。" << std::endl;

        return -1000.0;
    }
    //NS_ASSERT(distance < m_maxDistance);
    
    if (distance < m_minDistance)
    {
        distance = m_minDistance;
    }
    
    if( distance <= lvf_d_bp && distance >= m_minDistance)
    {
        lvf_pathloss = 22.7f * log10(distance) + 27.0 + 20.0 * log10(m_frequency);
    
    }else if(distance > lvf_d_bp)
    {
        lvf_pathloss = 40.0f * log10(distance) + 7.56 - 34.6 * log10(m_antennaEffectiveHeight)
        + 2.7 * log10(m_frequency);
    }
    
    lvf_pathloss_db = lvf_freespace_pathloss;
    if(lvf_pathloss_db < lvf_pathloss)
    {
        lvf_pathloss_db =lvf_pathloss;
    }
    
    
    
    //以下 ShadowLoss
    
    double  random_value;
    
    Ptr<NormalRandomVariable> nrv = CreateObject<NormalRandomVariable> ();
    nrv->SetAttribute ("Mean", DoubleValue (0.0));
    nrv->SetAttribute ("Variance", DoubleValue (m_slow_shading_delta));
    nrv->SetAttribute ("Bound", DoubleValue (1.0));
    random_value=nrv->GetValue();
    while (random_value<0) {
        random_value=nrv->GetValue();
    }
    
    
    double preShadow =a->GetShadowValue (b->GetSharedId ());
  

    if(preShadow==-100){
        rxPowerDbm = txPowerDbm - lvf_pathloss_db - random_value;
        a->SetShadowValue(b->GetSharedId (),random_value);
        return rxPowerDbm;
        
    }
    
    double preDistance = a->GetPreDistanceFrom (b);

    double change_distance = fabs(distance - preDistance);
    //std::cout << "xixiixixshshhahf     "<<change_distance<<std::endl;
    double self_correlation =exp(-change_distance/m_slow_shading_cor_dis);
    
    
    double current_shadowing_value=self_correlation *preShadow + sqrt(1-self_correlation*self_correlation) *random_value;
    
    a->SetShadowValue(b->GetSharedId (),current_shadowing_value);

    rxPowerDbm =txPowerDbm - lvf_pathloss_db - current_shadowing_value;
    return rxPowerDbm;

 
}
    
int64_t
HighwayPropagationLossModel::DoAssignStreams (int64_t stream)
{
    return 0;
}
    
    
                       
NS_OBJECT_ENSURE_REGISTERED (CityPropagationLossModel);
                       
TypeId
CityPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CityPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<CityPropagationLossModel> ()
    .AddAttribute ("AntennaEffectiveHeight",
                   "Antenna Effective Height (meters), default is 0.5m",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&CityPropagationLossModel::m_antennaEffectiveHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AntennaHeight",
                   "Antenna Height (meters), default is 1.5m",
                   DoubleValue (1.5),
                   MakeDoubleAccessor (&CityPropagationLossModel::m_antennaHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinDistance",
                   "MinDistance (meters), default is 3m",
                   DoubleValue (3.0),
                   MakeDoubleAccessor (&CityPropagationLossModel::m_minDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxDistance",
                   "MaxDistance (meters), default is 5000m",
                   DoubleValue (5000.0),
                   MakeDoubleAccessor (&CityPropagationLossModel::m_maxDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Frequency",
                   "The carrier frequency (in GHz) at which propagation occurs  (default is 5.9 GHz).",
                   DoubleValue (5.9),
                   MakeDoubleAccessor (&CityPropagationLossModel::SetFrequency,
                                       &CityPropagationLossModel::GetFrequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ShadingDeltaLos",
                   "ShadingDeltaLos, default is 3.0",
                   DoubleValue (3.0),
                   MakeDoubleAccessor (&CityPropagationLossModel::m_los_slow_shading_delta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ShadingDeltaNLos",
                   "ShadingDeltaNLos, default is 4.0",
                   DoubleValue (4.0),
                   MakeDoubleAccessor (&CityPropagationLossModel::m_nlos_slow_shading_delta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ShadingCorDis",
                   "ShadingCorDis, default is 10.0",
                   DoubleValue (10.0),
                   MakeDoubleAccessor (&CityPropagationLossModel::m_slow_shading_cor_dis),
                   MakeDoubleChecker<double> ())
    ;
    return tid;
}
                                      
                                      
CityPropagationLossModel::CityPropagationLossModel ()
{
}

CityPropagationLossModel::~CityPropagationLossModel ()
{
}
                                      
void
CityPropagationLossModel::SetFrequency (double frequency)
{
    m_frequency = frequency;
}
                                      
double
CityPropagationLossModel::GetFrequency (void) const
{
    return m_frequency;
}
    

double
CityPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                   Ptr<MobilityModel> a,
                                   Ptr<MobilityModel> b) const
{

    Vector pos_a = a->GetPosition () ;
    Vector pos_b = b->GetPosition () ;
    /*
    Vector grid_a = a->td_adhoc_ME_city_coordinate_discrete (pos_a.x, pos_a.y);
    Vector grid_b = b->td_adhoc_ME_city_coordinate_discrete (pos_b.x, pos_b.y);
    int lvc_grid_a_x = (int) grid_a.x;
    int lvc_grid_a_y = (int) grid_a.y;
    int lvc_grid_b_x = (int) grid_b.x;
    int lvc_grid_b_y = (int) grid_b.y;
    int result = a->td_adhoc_ME_city_los_status_judge(lvc_grid_a_x, lvc_grid_a_y, lvc_grid_b_x, lvc_grid_b_y);

    if(result==0){
      */
  
        return DoCalcLosRxPower(txPowerDbm,a,b);  /*
    }else if(result==2){
        return -1000;
    }
 */
 
    // passloss
    
    float   lvf_d1;
    float   lvf_d2;
    
    double  lvf_n1;
    double  lvf_n2;
    
    double  lvf_pathloss_db = -1000;	//初始化为无效值
    double  lvf_pathloss2;
    double  lvf_pathloss1;
    double  rxPowerDbm;
    

    lvf_d1 = fabs( pos_a.x - pos_b.x );
    lvf_d2 = fabs( pos_a.y - pos_b.y );
    
    lvf_n1 = 2.8 - 0.0024 * lvf_d2;
    if (lvf_n1 < 1.84)
    {
        lvf_n1 = 1.84;
    }
    lvf_n2 = 2.8 - 0.0024 * lvf_d1;
    if (lvf_n2 < 1.84)
    {
        lvf_n2 = 1.84;
    }
    
    double pl_d1_d2= CityPropagationLossModel::city_nlos_pathloss_calc(lvf_d1);
    double pl_d2_d1= CityPropagationLossModel::city_nlos_pathloss_calc(lvf_d2);
    
    lvf_pathloss1 = pl_d1_d2 + 17.3 - 12.5*lvf_n2+ 10*lvf_n2*log10(lvf_d2) + 3*log10(m_frequency);
    lvf_pathloss2 = pl_d2_d1 + 17.3 - 12.5*lvf_n1 + 10*lvf_n1*log10(lvf_d1) + 3*log10(m_frequency);
    lvf_pathloss_db  = lvf_pathloss1;
    if (lvf_pathloss_db > lvf_pathloss2)
    {
        lvf_pathloss_db = lvf_pathloss2;
    }
    
    // shadow
    
    double  random_value;
    
    
    Ptr<NormalRandomVariable> nrv = CreateObject<NormalRandomVariable> ();
    nrv->SetAttribute ("Mean", DoubleValue (0.0));
    nrv->SetAttribute ("Variance", DoubleValue (m_nlos_slow_shading_delta));
    nrv->SetAttribute ("Bound", DoubleValue (1.0));
    random_value=nrv->GetValue();
    while (random_value<0) {
        random_value=nrv->GetValue();
    }
    
    double preShadow = a->GetShadowValue (b->GetSharedId ());


    if(preShadow==-100){
        rxPowerDbm = txPowerDbm - lvf_pathloss_db - random_value;
        a->SetShadowValue(b->GetSharedId (),random_value);
        return rxPowerDbm;
        
    }
    
    double distance = a->GetDistanceFrom (b);
    double preDistance = a->GetPreDistanceFrom (b);
    
    //std::cout << "高速场景下距离" << preDistance << std::endl;
    
    double change_distance = fabs(distance -preDistance);
    double self_correlation =exp(-change_distance/m_slow_shading_cor_dis);
    
    double current_shadowing_value=self_correlation *preShadow + sqrt(1-self_correlation*self_correlation) *random_value;
    rxPowerDbm = txPowerDbm - lvf_pathloss_db - current_shadowing_value;
    a->SetShadowValue(b->GetSharedId (),current_shadowing_value);

    return rxPowerDbm;

}
    
    
int64_t
CityPropagationLossModel::DoAssignStreams (int64_t stream)
{
    return 0;
}
    
double
CityPropagationLossModel::city_nlos_pathloss_calc(float lvf_distance)const
{
    double  lvf_freespace_pathloss;
    double  lvf_pathloss_db;
    double  lvf_pathloss = -1000;	//初始化为无效值
    float   lvf_d_bp;
    
    NS_ASSERT(lvf_distance < m_maxDistance);
    lvf_freespace_pathloss = 20.0f * log10(lvf_distance) + 46.4 + 20.0 * log10(m_frequency/5.0);
    lvf_d_bp = 4 * m_antennaEffectiveHeight * m_antennaEffectiveHeight * m_frequency* (1.0e9) / (3*pow(10,8) + 0.0);
    if (lvf_distance < m_minDistance)
    {
        lvf_distance = m_minDistance;
    }
    if( lvf_distance <= lvf_d_bp && lvf_distance >= m_minDistance)
    {
        lvf_pathloss = 22.7f * log10(lvf_distance) + 27.0 + 20.0 * log10(m_frequency);
    
    }else if(lvf_distance > lvf_d_bp)
    {
        lvf_pathloss = 40.0f * log10(lvf_distance) + 7.56 - 34.6 * log10(m_antennaEffectiveHeight)
        + 2.7 * log10(m_frequency);
    }
    lvf_pathloss_db = lvf_freespace_pathloss;
    if(lvf_pathloss_db < lvf_pathloss)
    {
        lvf_pathloss_db =lvf_pathloss;
    }
    
    return lvf_pathloss_db;
}

double
CityPropagationLossModel::DoCalcLosRxPower (double txPowerDbm,
                                    Ptr<MobilityModel> a,
                                    Ptr<MobilityModel> b) const
{
    double  lvf_freespace_pathloss;
    double  lvf_d_bp;
    double  lvf_pathloss = -1000;	//初始化为无效值
    double  lvf_pathloss_db;
    
    double  rxPowerDbm;
    
    /*
     * 计算说明
     */
    double distance = a->GetDistanceFrom (b);
    lvf_freespace_pathloss = 20.0f * log10(distance) + 46.4 + 20.0 * log10(m_frequency/5.0);
    lvf_d_bp = 4 * m_antennaEffectiveHeight * m_antennaEffectiveHeight * m_frequency* (1.0e9) / (3*pow(10,8) + 0.0);
    
    if(distance >= m_maxDistance)
    {
        std::cout << "城市场景下LOS衰减计算程序错误，节点之间的距离为" << distance << std::endl
        << " 超过了可能的最大距离" << m_maxDistance<< "。" << std::endl;
        
    }
    NS_ASSERT(distance < m_maxDistance);
    if (distance < m_minDistance)
    {
        distance = m_minDistance;
    }
    if( distance <= lvf_d_bp && distance >= m_minDistance)
    {
        lvf_pathloss = 22.7f * log10(distance) + 27.0 + 20.0 * log10(m_frequency);
    }
    else if(distance > lvf_d_bp)
    {
        lvf_pathloss = 40.0f * log10(distance) + 7.56 - 34.6 * log10(m_antennaEffectiveHeight)
        + 2.7 * log10(m_frequency);
    }
    
    lvf_pathloss_db = lvf_freespace_pathloss;
    if(lvf_pathloss_db < lvf_pathloss)
    {
        lvf_pathloss_db =lvf_pathloss;
    }
    
    
    
    double  random_value;
    
    
    Ptr<NormalRandomVariable> nrv = CreateObject<NormalRandomVariable> ();
    nrv->SetAttribute ("Mean", DoubleValue (0.0));
    nrv->SetAttribute ("Variance", DoubleValue (m_los_slow_shading_delta));
    nrv->SetAttribute ("Bound", DoubleValue (1.0));
    random_value=nrv->GetValue();
    while (random_value<0) {
        random_value=nrv->GetValue();
    }
    
    double preShadow = a->GetShadowValue (b->GetSharedId ());
    
    if(preShadow==0.0){
        rxPowerDbm = txPowerDbm - lvf_pathloss_db - random_value;
        a->SetShadowValue(b->GetSharedId (),random_value);
        return rxPowerDbm;
        
    }
    
    double preDistance = a->GetPreDistanceFrom (b);
    double change_distance = fabs(distance -preDistance);
    double self_correlation =exp(-change_distance/m_slow_shading_cor_dis);
    
    double current_shadowing_value=self_correlation *preShadow + sqrt(1-self_correlation*self_correlation) *random_value;
    rxPowerDbm = txPowerDbm - lvf_pathloss_db - current_shadowing_value;
    a->SetShadowValue(b->GetSharedId (),current_shadowing_value);
    
    return rxPowerDbm;

}


// ------------------------------------------------------------------------- //

} // namespace ns3
