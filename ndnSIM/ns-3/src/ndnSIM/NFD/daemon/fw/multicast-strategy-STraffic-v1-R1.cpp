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
 * Edited to fit master thesis on AQM in NDN by Eirik Ketilsoenn Kjevik.
 * University of Oslo, Spring 2019.
 */

#include "multicast-strategy-STraffic-v1-R1.hpp"
#include "algorithm.hpp"
#include "core/logger.hpp"

//KJEVIK
#include <string>
#include <typeinfo>

namespace nfd {
namespace fw {

NFD_REGISTER_STRATEGY(MulticastStrategySTraffic1_R1);

NFD_LOG_INIT("MulticastStrategySTraffic1_R1");

const time::milliseconds MulticastStrategySTraffic1_R1::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds MulticastStrategySTraffic1_R1::RETX_SUPPRESSION_MAX(250);



void
MulticastStrategySTraffic1_R1::updateTime ()
{
  time::steady_clock::time_point timeNow = time::steady_clock::now();
  auto diff = timeNow - timeSinceLastPkt;
  timeSinceLastCalc += diff.count();
  timeSinceLastPkt = timeNow;
}

void
MulticastStrategySTraffic1_R1::updateShaper ()
{
  float avgDataSize_A = 1024;
  float avgDataSize_B = 1024;

  // If received Data packets rom upstream
  if(upStreamInData_A_Quantity > 1){
    avgDataSize_A = upStreamInData_A_Size / upStreamInData_A_Quantity;
  }else if(downStreamInData_A_Quantity > 1){
    avgDataSize_A = downStreamInData_A_Size / downStreamInData_A_Quantity;
  }

  if(upStreamInData_B_Quantity > 1){
    avgDataSize_B = upStreamInData_B_Size / upStreamInData_B_Quantity;
  }else if(downStreamInData_B_Quantity > 1){
    avgDataSize_B = downStreamInData_B_Size / downStreamInData_B_Quantity;
  }

  // If not received Data packets from upstream
  // Received Interest packets from downstream
  if(downStreamInInterest_A_Quantity > 1 && downStreamInInterest_B_Quantity > 1){
    numberOfIntToSend_A = BANDWIDTH / (avgDataSize_A + 28.0) * ((avgDataSize_B + 28.0)/(avgDataSize_A + 28.0 + avgDataSize_B + 28.0));
    numberOfIntToSend_B = BANDWIDTH / (avgDataSize_B + 28.0) * ((avgDataSize_A + 28.0)/(avgDataSize_A + 28.0 + avgDataSize_B + 28.0));
    
  }else{
    // Only received Interst from A
    if(downStreamInInterest_A_Quantity > 1){
      numberOfIntToSend_A = BANDWIDTH / (avgDataSize_A + 28.0);
      numberOfDataToSend_A = (BANDWIDTH - (numberOfIntToSend_A*28)) / avgDataSize_A;

      // To allow any other namespaces to also be allowed to pass through
      numberOfIntToSend_B = numberOfIntToSend_A;
      numberOfDataToSend_B = numberOfDataToSend_A;

    // Only received Interest from B
    }else if(downStreamInInterest_B_Quantity > 1){
      numberOfIntToSend_B = BANDWIDTH / (avgDataSize_B + 28.0);
      numberOfDataToSend_B = (BANDWIDTH - (numberOfIntToSend_B*28)) / avgDataSize_B;
      
      // To allow any other namespaces to also be allowed to pass through
      numberOfIntToSend_A = numberOfIntToSend_B;
      numberOfDataToSend_A = numberOfDataToSend_B;

    }else{
      // If not received any Interest packet from downstream.
      numberOfIntToSend_A = BANDWIDTH / (1024.0 + 28.0);
      numberOfIntToSend_B = BANDWIDTH / (1024.0 + 28.0);

      if(downStreamInData_A_Quantity > 1 && downStreamInData_B_Quantity > 1){
        avgDataSize_A = downStreamInData_A_Size / downStreamInData_A_Quantity;
        avgDataSize_B = downStreamInData_B_Size / downStreamInData_B_Quantity;

        numberOfDataToSend_A = BANDWIDTH / avgDataSize_A * (avgDataSize_B/(avgDataSize_A + avgDataSize_B));
        numberOfDataToSend_B = BANDWIDTH / avgDataSize_B * (avgDataSize_A/(avgDataSize_A + avgDataSize_B));

      }else if(downStreamInData_A_Quantity > 1){
        avgDataSize_A = downStreamInData_A_Size / downStreamInData_A_Quantity;
        numberOfDataToSend_A = BANDWIDTH / avgDataSize_A;
        numberOfDataToSend_B = numberOfDataToSend_A;

      }else if(downStreamInData_B_Quantity > 1){
        avgDataSize_B = downStreamInData_B_Size / downStreamInData_B_Quantity;
        numberOfDataToSend_B = BANDWIDTH / avgDataSize_B;
        numberOfDataToSend_A = numberOfDataToSend_B;

      }else{
        numberOfDataToSend_A = (BANDWIDTH - (numberOfIntToSend_A*28)) / 1024;
        numberOfDataToSend_B = (BANDWIDTH - (numberOfIntToSend_B*28)) / 1024;
      }
    }
  }

  downStreamInInterest_A_Quantity = 1.0;
  downStreamInInterest_A_Size = 1.0;
  downStreamInData_A_Quantity = 1.0;
  downStreamInData_A_Size = 1.0;
  downStreamInInterest_B_Quantity = 1.0;
  downStreamInInterest_B_Size = 1.0;
  downStreamInData_B_Quantity = 1.0;
  downStreamInData_B_Size = 1.0;

  upStreamInInterest_A_Quantity = 1.0;
  upStreamInInterest_A_Size = 1.0;
  upStreamInData_A_Quantity = 1.0;
  upStreamInData_A_Size = 1.0;
  upStreamInInterest_B_Quantity = 1.0;
  upStreamInInterest_B_Size = 1.0;
  upStreamInData_B_Quantity = 1.0;
  upStreamInData_B_Size = 1.0;

  timeSinceLastCalc = 0;
}

MulticastStrategySTraffic1_R1::MulticastStrategySTraffic1_R1(Forwarder& forwarder, const Name& name)
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
MulticastStrategySTraffic1_R1::getStrategyName()
{
  static Name strategyName("/localhost/nfd/strategy/multicast-STraffic-v1-R1/%FD%01");
  return strategyName;
}

void
MulticastStrategySTraffic1_R1::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                        const shared_ptr<pit::Entry>& pitEntry)
{
  bool doNormalStuff = false;
  std::string name = interest.getName().toUri();

  this->updateTime();

  if((inFace.getId() == 256)||inFace.getId() == 257){
  
    if((name.find(prefixA) != std::string::npos) && (numberOfIntToSend_A >= 0)){
      numberOfIntToSend_A--;
      downStreamInInterest_A_Quantity++;
      downStreamInInterest_A_Size += 28;
      doNormalStuff = true;
    }
    else if((name.find(prefixB) != std::string::npos) && (numberOfIntToSend_B >= 0)){
      numberOfIntToSend_B--;
      downStreamInInterest_B_Quantity++;
      downStreamInInterest_B_Size += 28;
      doNormalStuff = true;
    }

  }else if(inFace.getId() == 258){

    if(name.find(prefixA) != std::string::npos){
      upStreamInInterest_A_Quantity++;
      upStreamInData_A_Size += 28;
    }else if(name.find(prefixB) != std::string::npos){
      upStreamInInterest_B_Quantity++;
      upStreamInData_B_Size += 28;
    }
    doNormalStuff = true;

  }else{
    doNormalStuff = true;
  }

  if(timeSinceLastCalc > m_timeInterval){
      this->updateShaper();
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
MulticastStrategySTraffic1_R1::afterReceiveNack(const Face& inFace, const lp::Nack& nack,
                                    const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(inFace, nack, pitEntry);
}

void
MulticastStrategySTraffic1_R1::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                           const Face& inFace, const Data& data)
{
  bool doNormalStuff = false;
  std::string name = data.getName().toUri();

  this->updateTime();

  if(inFace.getId() == 256 || inFace.getId() == 257){

    if((name.find(prefixA) != std::string::npos) && (numberOfDataToSend_A >= 0)){
      numberOfDataToSend_A--;
      downStreamInData_A_Quantity++;
      downStreamInData_A_Size += data.getContent().size();
      doNormalStuff = true;
    }
    else if((name.find(prefixB) != std::string::npos) && (numberOfDataToSend_B >= 0)){
      numberOfDataToSend_B--;
      downStreamInData_B_Quantity++;
      downStreamInData_B_Size += data.getContent().size();
      doNormalStuff = true;
    }

  }else if(inFace.getId() == 258){

    if(name.find(prefixA) != std::string::npos){
      upStreamInData_A_Quantity++;
      upStreamInData_A_Size += data.getContent().size();
    }
    else if(name.find(prefixB) != std::string::npos){
      upStreamInData_B_Quantity++;
      upStreamInData_B_Size += data.getContent().size();
    }
    doNormalStuff = true;

  }else{
    doNormalStuff = true;
  }

  if(timeSinceLastCalc > m_timeInterval){
      this->updateShaper();
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
