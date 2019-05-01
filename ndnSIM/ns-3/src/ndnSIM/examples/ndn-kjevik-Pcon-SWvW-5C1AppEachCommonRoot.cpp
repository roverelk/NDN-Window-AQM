// ndn-kjevik-Pcon-SWvW-5C1AppEachCommonRoot.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {


/**
 * To run scenario and see what is happening, use the following command:
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-Pcon-SWvW-5C1AppEach
 * 
 * PCON - Stop-and-Wait versus Window - 1 router with 5 applications. All applications  has the same root name.
 * Each of the applications wants a separate prefix and all get a stast window of 1 packet.
 * Each packet is 1040 kb, and the bandwidth is 5500 Kbps, hence it is room to only one packet from each per second initially.
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
 *  +----+  1100 Kbps  +---+  5500 Kbps  +---+
 *  | C3 |<----------->| R |<----------->| P |
 *  +----+    10 ms    +---+    10 ms    +---+
 *                      ^ ^
 *  +----+  1100 Kbps   | |
 *  | C4 |<-------------+ |
 *  +----+    10 ms       |
 *                        |
 *  +----+  1100 Kbps     |
 *  | C5 |<---------------+ 
 *  +----+    10 ms    
 */

int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  std::string topoFile = "src/ndnSIM/examples/topologies/topo-kjevik-5C-R-P.txt";
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
  std::string m_outputName = "ndn-kjevik-Pcon-SWvW-5C1AppEachCommonRoot-v3-1-AIMD";
  double m_granularity = 0.25; // time in seconds

  std::string prefix[5] = {"/k/A", "/k/B", "/kC", "/k/D", "/k/E"};

  // Choosing forwarding strategy
  for(int i = 0; i < 5; i++)
  {
    ndn::StrategyChoiceHelper::InstallAll(prefix[i], "/localhost/nfd/strategy/best-route");
  }

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  
  // Getting containers for the consumer/producer
  Ptr<Node> producer = Names::Find<Node>("P1");
  Ptr<Node> consumers[5] = {Names::Find<Node>("C1"), Names::Find<Node>("C2"), Names::Find<Node>("C3"), Names::Find<Node>("C4"), Names::Find<Node>("C5")};


  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(250));
  consumerHelper.SetAttribute("PayloadSize", UintegerValue(1024));
  consumerHelper.SetAttribute("Window", UintegerValue(1));

  consumerHelper.SetAttribute("CcAlgorithm", StringValue("AIMD")); // AIMD, BIC or CUBIC

  for(int i = 0; i < 5; i++)
  {
    consumerHelper.SetPrefix(prefix[i]);
    consumerHelper.Install(consumers[i]);
  }
  
  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  for (int i = 0; i < 5; i++)
  {
    ndnGlobalRoutingHelper.AddOrigins(prefix[i], producer);
    producerHelper.SetPrefix(prefix[i]);
    producerHelper.Install(producer);
  }

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