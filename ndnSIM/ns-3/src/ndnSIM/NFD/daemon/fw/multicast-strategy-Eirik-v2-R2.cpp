/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 * 
 **********************************************************************************
 * 
 * Edited to fit master thesis on AQM in NDN by Eirik Ketilsønn Kjevik.
 * University of Oslo, Spring 2019.
 */

#include "multicast-strategy-Eirik-v2-R2.hpp"
#include "algorithm.hpp"
#include "core/logger.hpp"

namespace nfd {
namespace fw {

NFD_REGISTER_STRATEGY(MulticastStrategyEirik2_R2);

NFD_LOG_INIT("MulticastStrategyEirik2_R2");

const time::milliseconds MulticastStrategyEirik2_R2::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds MulticastStrategyEirik2_R2::RETX_SUPPRESSION_MAX(250);

void
MulticastStrategyEirik2_R2::updateTime ()
{
  time::steady_clock::time_point timeNow = time::steady_clock::now();
  auto diff = timeNow - timeSinceLastPkt;
  timeSinceLastCalc += diff.count();
  timeSinceLastPkt = timeNow;
}

void
MulticastStrategyEirik2_R2::updateBandwidth ()
{
  unsigned int avgIntSize = totSizeInterestReceived / numberOfInterestReceived;
  unsigned int avgDataSize = totSizeDataReceived / numberOfDataReceived;
  unsigned int avgTotSize = avgIntSize + avgDataSize;
  numberOfInterestToSend = (BANDWIDTH / avgTotSize)*0.9;  // The number at the end defines the weighting between Data pkts and
  numberOfDataToSend = (BANDWIDTH / avgTotSize)*1.1;      // Interst pkts to be allowed to be sent for the next interval.

  // Reset
  numberOfInterestReceived = 1;
  totSizeInterestReceived = 1;
  numberOfDataReceived = 1;
  totSizeDataReceived = 1;
  timeSinceLastCalc = 0;
}

MulticastStrategyEirik2_R2::MulticastStrategyEirik2_R2(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , ProcessNackTraits(this)
  , m_retxSuppression(RETX_SUPPRESSION_INITIAL,
                      RetxSuppressionExponential::DEFAULT_MULTIPLIER,
                      RETX_SUPPRESSION_MAX)
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument("MulticastStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument(
      "MulticastStrategy does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

const Name&
MulticastStrategyEirik2_R2::getStrategyName()
{
  static Name strategyName("/localhost/nfd/strategy/multicast-Eirik-v2-R2/%FD%02");
  return strategyName;
}

void
MulticastStrategyEirik2_R2::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                        const shared_ptr<pit::Entry>& pitEntry)
{
  bool doNormalStuff = false;
  if(inFace.getId() == 257){
    this->updateTime();

    if(numberOfInterestToSend >= 0){
      numberOfInterestToSend--;
      numberOfInterestReceived++;
      totSizeInterestReceived += 28; // Could not find dynamic size, they are around 20 bytes.
      doNormalStuff = true;
    }

    if(timeSinceLastCalc > m_timeInterval){
      this->updateBandwidth();
    }
  }else{
    doNormalStuff = true;
  }

  if(doNormalStuff){
    const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
    const fib::NextHopList& nexthops = fibEntry.getNextHops();

    int nEligibleNextHops = 0;

    bool isSuppressed = false;

    for (const auto& nexthop : nexthops) {
      Face& outFace = nexthop.getFace();

      RetxSuppressionResult suppressResult = m_retxSuppression.decidePerUpstream(*pitEntry, outFace);

      if (suppressResult == RetxSuppressionResult::SUPPRESS) {
        NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                      << "to=" << outFace.getId() << " suppressed");
        isSuppressed = true;
        continue;
      }
      
      if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
          wouldViolateScope(inFace, interest, outFace)) {
        continue;
      }
      
      this->sendInterest(pitEntry, outFace, interest);
      NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                            << " pitEntry-to=" << outFace.getId());


      if (suppressResult == RetxSuppressionResult::FORWARD) {
        m_retxSuppression.incrementIntervalForOutRecord(*pitEntry->getOutRecord(outFace));
      }
      ++nEligibleNextHops;
    }

    if (nEligibleNextHops == 0 && !isSuppressed) {
      NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " noNextHop");

      lp::NackHeader nackHeader;
      nackHeader.setReason(lp::NackReason::NO_ROUTE);
      this->sendNack(pitEntry, inFace, nackHeader);

      this->rejectPendingInterest(pitEntry);
    }
  }else{
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " noNextHop");

    lp::NackHeader nackHeader;
    nackHeader.setReason(lp::NackReason::NO_ROUTE);
    this->sendNack(pitEntry, inFace, nackHeader);

    this->rejectPendingInterest(pitEntry);
  }
}

void
MulticastStrategyEirik2_R2::afterReceiveNack(const Face& inFace, const lp::Nack& nack,
                                    const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(inFace, nack, pitEntry);
}

void
MulticastStrategyEirik2_R2::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                           const Face& inFace, const Data& data)
{
  bool doNormalStuff = false;
  if(inFace.getId() == 258){

    this->updateTime();

    if(numberOfDataToSend >= 0){
      numberOfDataToSend--;
      numberOfDataReceived++;
      totSizeDataReceived += data.getContent().size();

      doNormalStuff = true;
    }

    if(timeSinceLastCalc > m_timeInterval){
      this->updateBandwidth();
    }
  }else{
    doNormalStuff = true;
  }

  if(doNormalStuff){
    NFD_LOG_DEBUG("afterReceiveData pitEntry=" << pitEntry->getName() <<
                  " inFace=" << inFace.getId() << " data=" << data.getName());
    this->beforeSatisfyInterest(pitEntry, inFace, data);
    this->sendDataToAll(pitEntry, inFace, data);
  }else{
    return;
  }
}

} // namespace fw
} // namespace nfd
