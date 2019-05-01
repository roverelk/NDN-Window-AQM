/** 
 * ndn-kjevik-XTraffic--AQM-Responsive.cpp
 * 
 * Author: Eirik Ketilsønn Kjevik, University of Oslo
 * Spring 2019
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

/**
 * To run scenario and see what is happening, use the following command:
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-XTraffic--AQM-Responsive
 *
 * Topology: See topology file.
 * 
 */

int
main(int argc, char* argv[])
{
  int runs = 20;
  int start_time_STraffic = 20;
  int start_time_XTraffic = 40;

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (runs);

  std::string topoFile = "src/ndnSIM/examples/topologies/topo-kjevik-2C1P-R-R-2P1C.txt";
  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName(topoFile);
  topologyReader.Read();
  cout << "Fetched the file: " << topoFile << "\n";

  // Own variables
  std::string m_directory = "../../../Dropbox/00 - Utdanning/2017-06 UiO/Master/ndnSIM - Tracer/";
  std::string m_outputName = "BTraffic-PCONvCBR200-Exponential-A1024Be1024Ce1024-Seed1-Run" + std::to_string(runs) 
                            + "-StartS" + std::to_string(start_time_STraffic) 
                            + "-StartX" + std::to_string(start_time_XTraffic);

  double m_granularity = 1; // time in seconds

  std::string prefix[3] = {"/k/A", "/k/B", "/k/C"};

  // Choose where to place AQM
  bool R1 = true; // write true or false
  bool R2 = true;


  // Getting containers for the consumer/producer
  Ptr<Node> producers[3] = {Names::Find<Node>("P1"), Names::Find<Node>("P2"), Names::Find<Node>("P3")};
  Ptr<Node> consumers[3] = {Names::Find<Node>("C1"), Names::Find<Node>("C2"), Names::Find<Node>("C3")};
  Ptr<Node> router_R1[1] = {Names::Find<Node>("R1")};
  Ptr<Node> router_R2[1] = {Names::Find<Node>("R2")};

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1000"); 
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  for (int i = 0; i < 3; i++){
    ndn::StrategyChoiceHelper::Install(producers[i], "/", "/localhost/nfd/strategy/best-route");
    ndn::StrategyChoiceHelper::Install(consumers[i], "/", "/localhost/nfd/strategy/best-route");
  }
  if(R1){
    ndn::StrategyChoiceHelper::Install(router_R1[0], "/", "/localhost/nfd/strategy/multicast-BTraffic-v1-R1");
  }else{
    ndn::StrategyChoiceHelper::Install(router_R1[0], "/", "/localhost/nfd/strategy/best-route");
  }
  if(R2){
    ndn::StrategyChoiceHelper::Install(router_R2[0], "/", "/localhost/nfd/strategy/multicast-BTraffic-v1-R2");
  }else{
    ndn::StrategyChoiceHelper::Install(router_R2[0], "/", "/localhost/nfd/strategy/best-route");
  }

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();
  
  // Installing applications
  // Consumer
  // Simulation: PCON with Cbr cross-traffic
  
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(4294967295));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  // Set up the entry of link C2<-->R2 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperSTraffic("ns3::ndn::ConsumerCbr");
  consumerHelperSTraffic.SetAttribute("Frequency", DoubleValue(200)); //number of Interest packets sent per second
  consumerHelperSTraffic.SetAttribute("Randomize", StringValue("exponential"));
  consumerHelperSTraffic.SetPrefix(prefix[1]);
  consumerHelperSTraffic.SetAttribute("StartTime", StringValue(std::to_string(start_time_STraffic))); // Time s
  consumerHelperSTraffic.SetAttribute("StopTime", StringValue(std::to_string(start_time_STraffic + 560))); // Time s
  consumerHelperSTraffic.Install(consumers[1]);

  // Set up the entry of link C2<-->R2 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerCbr");
  consumerHelperXTraffic.SetAttribute("Frequency", DoubleValue(200)); //number of Interest packets sent per second
  consumerHelperXTraffic.SetAttribute("Randomize", StringValue("exponential"));
  consumerHelperXTraffic.SetPrefix(prefix[2]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue(std::to_string(start_time_XTraffic))); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue(std::to_string(start_time_XTraffic + 560))); // Time s
  consumerHelperXTraffic.Install(consumers[2]);


  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  ndnGlobalRoutingHelper.AddOrigins(prefix[0], producers[0]);
  producerHelper.SetPrefix(prefix[0]);
  producerHelper.Install(producers[0]);
  
  ndn::AppHelper producerHelperSTraffic("ns3::ndn::Producer");
  producerHelperSTraffic.SetAttribute("PayloadSize", StringValue("1024"));
  ndnGlobalRoutingHelper.AddOrigins(prefix[1], producers[1]);
  producerHelperSTraffic.SetPrefix(prefix[1]);
  producerHelperSTraffic.Install(producers[1]);

  ndn::AppHelper producerHelperXTraffic("ns3::ndn::Producer");
  producerHelperXTraffic.SetAttribute("PayloadSize", StringValue("1024"));
  ndnGlobalRoutingHelper.AddOrigins(prefix[2], producers[2]);
  producerHelperXTraffic.SetPrefix(prefix[2]);
  producerHelperXTraffic.Install(producers[2]);

  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(start_time_XTraffic + 580)); //10 min

  ndn::L3RateTracer::InstallAll(m_directory + m_outputName + "--rate-trace.txt", Seconds(m_granularity));
  L2RateTracer::InstallAll(m_directory + m_outputName + "--drop-trace.txt", Seconds(m_granularity));
  ndn::CsTracer::InstallAll(m_directory + m_outputName + "--cs-trace.txt", Seconds(m_granularity));
  ndn::AppDelayTracer::InstallAll(m_directory + m_outputName + "--app-delays-trace.txt");


  cout << "The file(s) will be saved to " << m_directory << " with the common name: " << m_outputName << "\n\a";

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} //nmaespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}