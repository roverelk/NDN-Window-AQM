// ndn-kjevik-consumerWindow-P2P.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

/**
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-consumerWindow-P2P
 */

int
main(int argc, char* argv[])
{
  // Own variables
  std::string m_directory = "../../../Dropbox/00 - Utdanning/2017-06 UiO/Master/ndnSIM - Tracer/";
  std::string m_outputName = "ndn-kjevik-consumerWindow-P2P-v2";
  double m_granularity = 0.25; // time in seconds

  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxPackets", UintegerValue(20));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(3);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "10000"); // default ContentStore parameters
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerWindow");
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.SetAttribute("Size", DoubleValue(10000000000));
  consumerHelper.SetAttribute("PayloadSize", UintegerValue(1040));
  consumerHelper.Install(nodes.Get(0));

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1040"));
  producerHelper.Install(nodes.Get(2));

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
