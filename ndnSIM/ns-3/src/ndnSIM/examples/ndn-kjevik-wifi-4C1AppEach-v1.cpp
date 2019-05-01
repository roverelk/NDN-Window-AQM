/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * /////////////////////
 *
 * Med denne versjonen er planen å få to og to noder til å snakke sammen med hvert sitt oppsett.
 * så sette opp en konfigurasjon for C1 til R1 og så en egen en for R1 til P1. Må se hvordan
 * addressering kan gjennomføres på en god måte(?)
 * 
 * Har løst problemstillingen som ble beskrevet over. Måtte gjøre et par endringer fra kode
 * skrevet før for at det skulle gå opp. Primært:
 * -  I tillegg til denne filen har jeg måttett opprette en ny Forwarding Strategy, basert på 
 *    Multicast. Der har jeg måtett kommentere ut en kontroll som sjekker om Interest kommer inn
 *    på samme Face som den må sendes ut på. Den skulle ikke bli aktivert fordi det er et Ad Hoc
 *    nettverk, men der er det et flagg som ikke henger med skikkelig. Ved å kommentere vekk
 *    denne if-løkken kan FS Multicast i praksis fungere som en Broadcast startegi og kan 
 *    implementeres på alle trådløse noder jeg trenger i fremtiden.
 * -  Nodene har også fått fast posisjon, og ikke "random" som de var designet til å få først.
 *    
 * Problemstillinger som ikke er løst:
 * -  Få nodene til å bare snakke med en annen node ved å ha optimal trådløs avstand.
 **/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"

#include "ns3/ndnSIM-module.h"

using namespace std;
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ndn.WifiExample");

//
// DISCLAIMER:  Note that this is an extremely simple example, containing just 2 wifi nodes
// communicating directly over AdHoc channel.
//

// Ptr<ndn::NetDeviceFace>
// MyNetDeviceFaceCallback (Ptr<Node> node, Ptr<ndn::L3Protocol> ndn, Ptr<NetDevice> device)
// {
//   // NS_LOG_DEBUG ("Create custom network device " << node->GetId ());
//   Ptr<ndn::NetDeviceFace> face = CreateObject<ndn::MyNetDeviceFace> (node, device);
//   ndn->AddFace (face);
//   return face;
// }

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  //////////////////////

  // Variables
  //std::string v_prefix = "/prefix";

  std::string m_directory = "../../../Dropbox/00 - Utdanning/2017-06 UiO/Master/ndnSIM - Tracer/";
  std::string export_file_names = "kjevik-wifi-TEST-4C-OFDM48-80211g";
  std::string export_file_version = "v1";
  double v_recordGranularity = 1; // seconds
  double v_simulationTime = 500; // seconds

  //////////////////////
  
  /*
  * OfdmRate6Mbps
  * OfdmRate9Mbps
  * OfdmRate12Mbps
  * OfdmRate18Mbps
  * OfdmRate24Mbps
  * OfdmRate36Mbps
  * OfdmRate48Mbps
  * OfdmRate54Mbps
  */

  //std::string phyMode("DsssRate1Mbps");
  std::string phyMode("OfdmRate48Mbps");
  double rss = -80; // -dBm

  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211g);

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set("RxGain", DoubleValue (0));
  
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(rss));
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));

  wifiMac.SetType("ns3::AdhocWifiMac");
  

  ///////////////

  std::string topoFile = "src/ndnSIM/examples/topologies/topo-kjevik-4C-R-R-P-wifi.txt";
  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName(topoFile);
  topologyReader.Read();
  cout << "Fetched the file: " << topoFile << "\n";

  // NodeContainer nodes;
  // nodes.Create(3);

  Ptr<Node> producer = Names::Find<Node>("P1");
  NodeContainer consumers;
  consumers.Add(Names::Find<Node>("C1"));
  consumers.Add(Names::Find<Node>("C2"));
  consumers.Add(Names::Find<Node>("C3"));
  consumers.Add(Names::Find<Node>("C4"));
  NodeContainer routers;
  routers.Add(Names::Find<Node>("R1"));
  routers.Add(Names::Find<Node>("R2"));
  routers.Add(Names::Find<Node>("R3"));
  routers.Add(Names::Find<Node>("R4"));
  routers.Add(Names::Find<Node>("R5"));

  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhy, wifiMac, routers);
  
  //NetDeviceContainer wifiNetDevicesOne = wifi.Install(wifiPhyHelper, wifiMacHelper, wifiOne);
  //NetDeviceContainer wifiNetDevicesTwo = wifi.Install(wifiPhyHelper, wifiMacHelper, wifiTwo);

  // 2. Install Mobility model
  //mobility.Install(nodes);
  

  // 3. Install NDN stack
  NS_LOG_INFO("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  // ndnHelper.AddNetDeviceFaceCreateCallback (WifiNetDevice::GetTypeId (), MakeCallback
  // (MyNetDeviceFaceCallback));
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1000");
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::Install(producer, "/", "/localhost/nfd/strategy/multicast");
  for(int i = 0; i < 4; i++)
  {
    ndn::StrategyChoiceHelper::Install(consumers[i], "/", "/localhost/nfd/strategy/multicast");
  }
  for(int i = 0; i < 5; i++)
  {
    ndn::StrategyChoiceHelper::Install(routers[i], "/", "/localhost/nfd/strategy/multicast");
  } 
  

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();


  // 4. Set up applications
  NS_LOG_INFO("Installing Applications");

  std::string prefix[4] = {"/k/A", "/k/B", "/k/C", "/k/D"};


  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(250));
  consumerHelper.SetAttribute("PayloadSize", UintegerValue(1024));
  consumerHelper.SetAttribute("Window", UintegerValue(1));

  consumerHelper.SetAttribute("CcAlgorithm", StringValue("AIMD")); // AIMD, BIC or CUBIC

  for(int i = 0; i < 4; i++)
  {
    consumerHelper.SetAttribute("StartTime", StringValue(std::to_string(i))); // Time s
    consumerHelper.SetPrefix(prefix[i]);
    consumerHelper.Install(consumers[i]);
  }
  

  // Define attributes for producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  ndnGlobalRoutingHelper.AddOrigins("/k", producer);
  producerHelper.SetPrefix("/k");
  producerHelper.Install(producer);
  
  ndn::GlobalRoutingHelper::CalculateRoutes();

  ////////////////

  Simulator::Stop(Seconds(v_simulationTime));

  // Tracers
  L2RateTracer::InstallAll(m_directory + export_file_names + "-" + export_file_version + "-L2Trace.txt", Seconds(v_recordGranularity));
  ndn::L3RateTracer::InstallAll(m_directory + export_file_names + "-" + export_file_version + "-L3Trace.txt", Seconds(v_recordGranularity));
  ndn::CsTracer::InstallAll(m_directory + export_file_names + "-" + export_file_version + "-CsTrace-.txt", Seconds(v_recordGranularity));
  ndn::AppDelayTracer::InstallAll(m_directory + export_file_names + "-" + export_file_version + "-AppDelayTrace-.txt");


  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
