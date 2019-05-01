// ndn-kjevik-consumerPcon-RF.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

/**
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-consumerPcon-RF
 */

int
main(int argc, char* argv[])
{
  // Own variables
  std::string m_directory = "../../../Dropbox/00 - Utdanning/2017-06 UiO/Master/ndnSIM - Tracer/";
  std::string m_outputName = "ndn-kjevik-consumerPcon-RF-v3";
  double m_granularity = 0.25; // time in seconds

  std::string prefix = "/prefix";

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-kjevik-3-node.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "10000"); 
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll(prefix, "/localhost/nfd/strategy/best-route");

  // Getting containers for the consumer/producer
  Ptr<Node> consumer = Names::Find<Node>("C1");
  Ptr<Node> producer = Names::Find<Node>("P1");

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Size", DoubleValue(10000000000));
  consumerHelper.SetAttribute("PayloadSize", UintegerValue(1040));
  consumerHelper.Install(consumer);

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(prefix);
  producerHelper.SetAttribute("PayloadSize", StringValue("1040"));
  producerHelper.Install(producer);

  // Calculate and install FIBs
  ndn::FibHelper::AddRoute("C1", prefix, "R1", 1);
  ndn::FibHelper::AddRoute("R1", prefix, "P1", 1);

  Simulator::Stop(Seconds(20.0));

  ndn::L3RateTracer::InstallAll(m_directory + m_outputName + "-rate-trace.txt", Seconds(m_granularity));
  L2RateTracer::InstallAll(m_directory + m_outputName + "-drop-trace.txt", Seconds(m_granularity));
  ndn::CsTracer::InstallAll(m_directory + m_outputName + "-cs-trace.txt", Seconds(m_granularity));
  ndn::AppDelayTracer::InstallAll(m_directory + m_outputName + "-app-delays-trace.txt");

  cout << "The file(s) will be saved to " << m_directory << " with the common name: " << m_outputName << "\n";

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
