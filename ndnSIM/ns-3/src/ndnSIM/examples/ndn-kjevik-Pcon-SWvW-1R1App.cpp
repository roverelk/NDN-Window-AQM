// ndn-kjevik-Pcon-SWvW-1R1App.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

/**
 * To run scenario and see what is happening, use the following command:
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kjevik-Pcon-SWvW-1R1App
 * 
 * PCON - Stop-and-Wait versus Window - 1 router with 1 application.
 * The prefix is set to only wanting one interst.
 * 
 * Topology:
 * 
 *  +---+  5500 Kbps  +---+  5500 Kbps  +---+
 *  | C |<----------->| R |<----------->| P |
 *  +---+    10 ms    +---+    10 ms    +---+
 */

int
main(int argc, char* argv[])
{
  // Own variables
  std::string m_directory = "../../../Dropbox/00 - Utdanning/2017-06 UiO/Master/ndnSIM - Tracer/";
  std::string m_outputName = "ndn-kjevik-pcon-SWvW-1R1App-v3-1";
  double m_granularity = 0.25; // time in seconds

  std::string prefix = "/prefix";

  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("0.55Mbps"));
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
  ndn::StrategyChoiceHelper::InstallAll(prefix, "/localhost/nfd/strategy/best-route");

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerPcon");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Size", DoubleValue(-1));
  consumerHelper.SetAttribute("MaxSeq", UintegerValue(1250));
  consumerHelper.SetAttribute("PayloadSize", UintegerValue(1024));
  consumerHelper.SetAttribute("Window", UintegerValue(5));
  consumerHelper.Install(nodes.Get(0));

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");

  producerHelper.SetPrefix(prefix);
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(nodes.Get(2));

  Simulator::Stop(Seconds(30.0));

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
