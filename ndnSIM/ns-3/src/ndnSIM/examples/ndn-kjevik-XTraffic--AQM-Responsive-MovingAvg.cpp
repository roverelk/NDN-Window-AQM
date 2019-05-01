/** 
 * ndn-kjevik-XTraffic--AQM-Responsive-MovingAvg.cpp
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
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-XTraffic--AQM-Responsive-MovingAvg
 * 
 * Topology: See topology file.
 * 
 */

int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  std::string topoFile = "src/ndnSIM/examples/topologies/topo-kjevik-1C1P-R-R-1C1P.txt";
  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName(topoFile);
  topologyReader.Read();
  cout << "Fetched the file: " << topoFile << "\n";

  
  // Own variables
  std::string m_directory = "../../../Dropbox/00 - Utdanning/2017-06 UiO/Master/ndnSIM - Tracer/";
  std::string m_outputName = "ndn-kjevik-XTraffic--v3-9--Exponential-AQM-PCON-responsiveMovAQMatBoth";
  double m_granularity = 1; // time in seconds

  std::string prefix[2] = {"/k/A", "/k/B"};

  // Choose where to place AQM
  bool R1 = true; // write true or false
  bool R2 = true;


  // Getting containers for the consumer/producer
  Ptr<Node> producers[2] = {Names::Find<Node>("P1"), Names::Find<Node>("P2")};
  Ptr<Node> consumers[2] = {Names::Find<Node>("C1"), Names::Find<Node>("C2")};
  Ptr<Node> router_R1[1] = {Names::Find<Node>("R1")};
  Ptr<Node> router_R2[1] = {Names::Find<Node>("R2")};

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1000"); 
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  for (int i = 0; i < 2; i++){
    ndn::StrategyChoiceHelper::Install(producers[i], "/", "/localhost/nfd/strategy/best-route");
    ndn::StrategyChoiceHelper::Install(consumers[i], "/", "/localhost/nfd/strategy/best-route");
  }
  if(R1){
    ndn::StrategyChoiceHelper::Install(router_R1[0], "/", "/localhost/nfd/strategy/multicast-Eirik-v5-R1");
  }else{
    ndn::StrategyChoiceHelper::Install(router_R1[0], "/", "/localhost/nfd/strategy/best-route");
  }
  if(R2){
    ndn::StrategyChoiceHelper::Install(router_R2[0], "/", "/localhost/nfd/strategy/multicast-Eirik-v5-R2");
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
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(10000000));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  // Set up the entry of link C2<-->R2 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerCbr");
  consumerHelperXTraffic.SetAttribute("Frequency", DoubleValue(200)); //number of Interest packets sent per second
  consumerHelperXTraffic.SetAttribute("Randomize", StringValue("exponential"));
  consumerHelperXTraffic.SetPrefix(prefix[1]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue("20")); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue("580")); // Time s
  consumerHelperXTraffic.Install(consumers[1]);


  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  for (int i = 0; i < 2; i++)
  {
    ndnGlobalRoutingHelper.AddOrigins(prefix[i], producers[i]);
    producerHelper.SetPrefix(prefix[i]);
    producerHelper.Install(producers[i]);
  }

  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(600.0));

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