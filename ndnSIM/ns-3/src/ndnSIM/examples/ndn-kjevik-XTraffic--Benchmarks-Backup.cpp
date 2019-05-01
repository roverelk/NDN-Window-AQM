// ndn-kjevik-XTraffic--Benchmarks.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {


/**
 * To run scenario and see what is happening, use the following command:
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-Pcon-SWvW-7C1R1P-DelayedEntry
 * 
 * PCON - Stop-and-Wait versus Window - 1 router with 56 applications.
 * Each of the applications wants a separate prefix and all get a stast window of 1 packet.
 * Each packet is 1040 kb, and the bandwidth is 5500 Kbps, hence it is room to only one packet from each per second initially.
 * 
 * Then after 4 seconds a 6th Consumer starts to send. This is more than the network is designed to handle, the goal is to see how it reacts.
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
  std::string m_outputName = "ndn-kjevik-XTraffic--Part1-Benchmark--CBR";
  double m_granularity = 0.25; // time in seconds

  std::string prefix[3] = {"/k/A", "/k/B"};


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
  /*
  for(int i = 0; i < 2; i++)
  {
    for(int j = 0; j < 2; j++)
    {
      ndn::StrategyChoiceHelper::Install(producers[j], prefix[i], "/localhost/nfd/strategy/best-route");
      ndn::StrategyChoiceHelper::Install(consumers[j], prefix[i], "/localhost/nfd/strategy/best-route");
    }
  }
  ndn::StrategyChoiceHelper::Install(router_R1[0], "/", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::Install(router_R2[0], "/", "/localhost/nfd/strategy/best-route");

  ndn::StrategyChoiceHelper::Install(router_R1[0], "/", "/localhost/nfd/strategy/multicast-Eirik-v5-R1");
  ndn::StrategyChoiceHelper::Install(router_R2[0], "/", "/localhost/nfd/strategy/multicast-Eirik-v5-R2");
  */
  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();
  // Installing applications
  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute("Frequency", DoubleValue(200));
  consumerHelper.SetPrefix(prefix[0]);
  consumerHelper.Install(consumers[0]);

  /*
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(100000)); // Making each node requesting 100 000 packets
  consumerHelper.SetAttribute("PayloadSize", UintegerValue(1024));
  consumerHelper.SetAttribute("Window", UintegerValue(1));

  consumerHelper.SetAttribute("CcAlgorithm", StringValue("AIMD")); // AIMD, BIC or CUBIC

  for(int i = 0; i < 1; i++)
  {
    consumerHelper.SetPrefix(prefix[i]);
    consumerHelper.Install(consumers[i]);
  }
  */
  // Set up the entry of link C3<-->R2 at 20 seconds, and stop after 40 seconds
  ndn::AppHelper consumerHelperXTraffic("ns3::ndn::ConsumerCbr");
  consumerHelperXTraffic.SetAttribute("Frequency", DoubleValue(200)); //number of Interest packets sent per second
  consumerHelperXTraffic.SetPrefix(prefix[1]);
  consumerHelperXTraffic.SetAttribute("StartTime", StringValue("10")); // Time s
  consumerHelperXTraffic.SetAttribute("StopTime", StringValue("70")); // Time s
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

  //Gives random packet sizes for P3.
  //ndn::AppHelper producerHelperRandom("ns3::ndn::ProducerRandomPktSize");
  /*
  ndn::AppHelper producerHelperRandom("ns3::ndn::Producer");
  producerHelperRandom.SetAttribute("PayloadSize", StringValue("1536"));
  ndnGlobalRoutingHelper.AddOrigins(prefix[2], producers[2]);
  producerHelperRandom.SetPrefix(prefix[2]);
  producerHelperRandom.Install(producers[2]);
  */

  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(90.0));

  ndn::L3RateTracer::InstallAll(m_directory + m_outputName + "--rate-trace.txt", Seconds(m_granularity));
  L2RateTracer::InstallAll(m_directory + m_outputName + "--drop-trace.txt", Seconds(m_granularity));
  ndn::CsTracer::InstallAll(m_directory + m_outputName + "--cs-trace.txt", Seconds(m_granularity));
  ndn::AppDelayTracer::InstallAll(m_directory + m_outputName + "--app-delays-trace.txt");


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