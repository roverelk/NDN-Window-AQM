/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
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

#ifndef NFD_DAEMON_FW_MULTICAST_STRATEGY_HPP
#define NFD_DAEMON_FW_MULTICAST_STRATEGY_HPP

#include "strategy.hpp"
#include "process-nack-traits.hpp"
#include "retx-suppression-exponential.hpp"

namespace nfd {
namespace fw {

/** \brief a forwarding strategy that forwards Interest to all FIB nexthops
 */
class MulticastStrategyEirik2_R2 : public Strategy
                        , public ProcessNackTraits<MulticastStrategyEirik2_R2>
{
public:
  explicit
  MulticastStrategyEirik2_R2(Forwarder& forwarder, const Name& name = getStrategyName());

  static const Name&
  getStrategyName();

  void
  afterReceiveInterest(const Face& inFace, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry) override;

  void
  afterReceiveNack(const Face& inFace, const lp::Nack& nack,
                   const shared_ptr<pit::Entry>& pitEntry) override;

  void
  afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                   const Face& inFace, const Data& data) override;

private:
  friend ProcessNackTraits<MulticastStrategyEirik2_R2>;
  RetxSuppressionExponential m_retxSuppression;
  
  void 
  updateTime();
  
  void 
  updateBandwidth();

  //KJEVIK
  unsigned int numberOfInterestReceived = 1;
  unsigned int numberOfDataReceived = 1;
  unsigned int totSizeInterestReceived = 1;
  unsigned int totSizeDataReceived = 1;

  unsigned int BANDWIDTH = 12500; //1Mbps converted to bytes per second

  //time::steady_clock::time_point timeStart = time::steady_clock::now();
  time::steady_clock::time_point timeSinceLastPkt = time::steady_clock::now();
  unsigned int timeSinceLastCalc = 0;
  unsigned int m_timeInterval = 100000000; // 0.1 second in nanoseconds.

  signed int numberOfInterestToSend = 1000;
  signed int numberOfDataToSend = 1000;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  static const time::milliseconds RETX_SUPPRESSION_INITIAL;
  static const time::milliseconds RETX_SUPPRESSION_MAX;
};

} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_MULTICAST_STRATEGY_HPP
