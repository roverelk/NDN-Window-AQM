// ndn-kjevik-Pcon-SWvW-6C1R1P-DelayedEntry-CommonRoot.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {


/**
 * To run scenario and see what is happening, use the following command:
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-Pcon-SWvW-6C1R1P-DelayedEntry
 * 
 * PCON - Stop-and-Wait versus Window - 1 router with 56 applications.
 * Each of the applications wants a separate prefix and all get a stast window of 1 packet.
 * Each packet is 1040 kb, and the bandwidth is 5500 Kbps, hence it is room to only one packet from each per second initially.
 * 
 * Then after 4 seconds a 6th Consumer starts to send. This is more than the network is designed to handle, the goal is to see how it reacts.
 * 
 * Topology:
 * 
 *  +----+  1100 Kbps  
 *  | C1 |<---------------+ 
 *  +----+    10 ms       |
 *                        |
 *  +----+  1100 Kbps     |
 *  | C2 |<-------------+ |
 *  +----+    10 ms     | |
 *                      V V
 *  +----+  1100 Kbps  +-----+  5500 Kbps  +---+
 *  | C3 |<----------->|  R  |<----------->| P |
 *  +----+    10 ms    +-----+    10 ms    +---+
 *                      ^ ^ ^
 *  +----+  1100 Kbps   | | |
 *  | C4 |<-------------+ | |
 *  +----+    10 ms       | |
 *                        | |
 *  +----+  1100 Kbps     | |
 *  | C5 |<---------------+ |
 *  +----+    10 ms         |
 *                          / Connects after 4 seconds 
 *  +----+  1100 Kbps       |
 *  | C6 |<-----------------+
 *  +----+    10 ms  
 */

int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  std::string topoFile = "src/ndnSIM/examples/topologies/topo-kjevik-6C-R-P.txt";
  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName(topoFile);
  topologyReader.Read();
  cout << "Fetched the file: " << topoFile << "\n";

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "10000"); 
  ndnHelper.InstallAll();

  // Own variables
  std::string m_directory = "../../../Dropbox/00 - Utdanning/2017-06 UiO/Master/ndnSIM - Tracer/";
  std::string m_outputName = "ndn-kjevik-Pcon-SWvW-6C1R1P-DelayEntry-OneProducerApp";
  double m_granularity = 0.25; // time in seconds

  std::string prefix[6] = {"/k/A", "/k/B", "/k/C", "/k/D", "/k/E", "/k/F"};

  // Choosing forwarding strategy
  for(int i = 0; i < 6; i++)
  {
    ndn::StrategyChoiceHelper::InstallAll(prefix[i], "/localhost/nfd/strategy/best-route");
  }

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  
  // Getting containers for the consumer/producer
  Ptr<Node> producer = Names::Find<Node>("P1");
  Ptr<Node> consumers[6] = {Names::Find<Node>("C1"), Names::Find<Node>("C2"), Names::Find<Node>("C3"), 
    Names::Find<Node>("C4"), Names::Find<Node>("C5"), Names::Find<Node>("C6")};
  Ptr<Node> router = Names::Find<Node>("R1");

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(250));
  consumerHelper.SetAttribute("PayloadSize", UintegerValue(1040));
  consumerHelper.SetAttribute("Window", UintegerValue(1));

  for(int i = 0; i < 5; i++)
  {
    consumerHelper.SetPrefix(prefix[i]);
    consumerHelper.Install(consumers[i]);
  }

  // Set up the entry of link C6<-->R1 at 4 seconds
  consumerHelper.SetPrefix(prefix[5]);
  consumerHelper.SetAttribute("StartTime", StringValue("4")); // Time s
  consumerHelper.Install(consumers[5]);
  
  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1040"));

  ndnGlobalRoutingHelper.AddOrigins("/k", producer);
  producerHelper.SetPrefix("/k");
  producerHelper.Install(producer);


  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(120.0));

  ndn::L3RateTracer::InstallAll(m_directory + m_outputName + "-rate-trace.txt", Seconds(m_granularity));
  L2RateTracer::InstallAll(m_directory + m_outputName + "-drop-trace.txt", Seconds(m_granularity));
  ndn::CsTracer::InstallAll(m_directory + m_outputName + "-cs-trace.txt", Seconds(m_granularity));
  ndn::AppDelayTracer::InstallAll(m_directory + m_outputName + "-app-delays-trace.txt");

  cout << "The file(s) will be saved to " << m_directory << " with the common name: " << m_outputName << "\n";

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