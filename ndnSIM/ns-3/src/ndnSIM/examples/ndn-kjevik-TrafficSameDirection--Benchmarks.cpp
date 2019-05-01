/** 
 * ndn-kjevik-TrafficSameDirection--Benchmarks.cpp
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
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-TrafficSameDirection--Benchmarks
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

  std::string topoFile = "src/ndnSIM/examples/topologies/topo-kjevik-2C-R-R-2P.txt";
  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName(topoFile);
  topologyReader.Read();
  cout << "Fetched the file: " << topoFile << "\n";

  
  // Own variables
  std::string m_directory = "../../../Dropbox/00 - Utdanning/2017-06 UiO/Master/ndnSIM - Tracer/";
  std::string m_outputName = "ndn-kjevik-STraffic-Benchmark-Part1--PCON";
  double m_granularity = 0.25; // time in seconds

  std::string prefix[2] = {"/k/A", "/k/B"};


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
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");


  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();
  
  // Each of the simulations has its own snippet of code . Un-comment the snippet you want to run.
  // Installing applications
  // Consumer
  // Simulation 01: CBR BENCHMARK
  /*
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute("Frequency", DoubleValue(200));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  // Set up the entry of link C2<-->R1 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerCbr");
  consumerHelperXTraffic.SetAttribute("Frequency", DoubleValue(200)); //number of Interest packets sent per second
  consumerHelperXTraffic.SetPrefix(prefix[1]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue("10")); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue("70")); // Time s
  consumerHelperXTraffic.Install(consumers[1]);
  */
  
  // Simulation 02: WINDOW BENCHMARK
  /*
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerWindow");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(100000));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  // Set up the entry of link C2<-->R2 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerWindow");
  consumerHelperXTraffic.SetAttribute("Size", DoubleValue(-1));
  consumerHelperXTraffic.SetAttribute("MaxSeq", UintegerValue(100000));
  consumerHelperXTraffic.SetPrefix(prefix[1]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue("10")); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue("70")); // Time s
  consumerHelperXTraffic.Install(consumers[1]);
  */

  // Simulation 03: PCON BENCHMARK
  
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(100000));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  // Set up the entry of link C2<-->R2 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerPcon");
  consumerHelperXTraffic.SetAttribute("Size", DoubleValue(-1));
  consumerHelperXTraffic.SetAttribute("MaxSeq", UintegerValue(100000));
  consumerHelperXTraffic.SetPrefix(prefix[1]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue("10")); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue("70")); // Time s
  consumerHelperXTraffic.Install(consumers[1]);
  

  // Simulation 04: Window with Cbr cross-traffic
  /*
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerWindow");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(100000));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  // Set up the entry of link C2<-->R2 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerCbr");
  consumerHelperXTraffic.SetAttribute("Frequency", DoubleValue(200)); //number of Interest packets sent per second
  consumerHelperXTraffic.SetPrefix(prefix[1]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue("10")); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue("70")); // Time s
  consumerHelperXTraffic.Install(consumers[1]);
  */

  // Simulation 05: PCON with Cbr cross-traffic
  /*
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(100000));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  // Set up the entry of link C2<-->R2 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerCbr");
  consumerHelperXTraffic.SetAttribute("Frequency", DoubleValue(200)); //number of Interest packets sent per second
  consumerHelperXTraffic.SetPrefix(prefix[1]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue("10")); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue("70")); // Time s
  consumerHelperXTraffic.Install(consumers[1]);
  */

  // Simulation 06: PCON with Windoow cross-traffic
  /*
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(100000));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  // Set up the entry of link C2<-->R2 at 10 seconds, and stop after 70 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerWindow");
  consumerHelperXTraffic.SetAttribute("Size", DoubleValue(-1));
  consumerHelperXTraffic.SetAttribute("MaxSeq", UintegerValue(100000));
  consumerHelperXTraffic.SetPrefix(prefix[1]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue("10")); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue("70")); // Time s
  consumerHelperXTraffic.Install(consumers[1]);
  */

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

  Simulator::Stop(Seconds(80.0));

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