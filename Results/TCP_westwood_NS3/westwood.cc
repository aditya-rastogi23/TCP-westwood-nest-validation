#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
bool firstCwnd = true;
bool firstSshThr = true;
bool firstRtt = true;
bool firstRto = true;
bool firstMinRtt = true;
bool firstthrp=true;
Ptr<OutputStreamWrapper> cWndStream;
Ptr<OutputStreamWrapper> ssThreshStream;
Ptr<OutputStreamWrapper> rttStream;
Ptr<OutputStreamWrapper> rtoStream;
Ptr<OutputStreamWrapper> inFlightStream;
Ptr<OutputStreamWrapper> ThrpStream;

uint32_t cWndValue;
uint32_t ssThreshValue;
int32_t tempthrp=0;
// class Throughput : public Object
// {
// public:
 
//   static TypeId GetTypeId (void)
//   {
//     static TypeId tid = TypeId ("Throughput")
//       .SetParent<Object> ()
//       .SetGroupName ("BBR-test")
//       .AddConstructor<Throughput> ()
//       .AddTraceSource ("Throughput",
//                        "Tracing the Throughput",
//                        MakeTraceSourceAccessor (&Throughput::Thp),
//                        "ns3::TracedValueCallback::Int32")
//     ;
//     return tid;
//   }

//   Throughput () {}
//   TracedValue<int32_t> Thp;
// };
// Ptr<Throughput> myObject = CreateObject<Throughput> ();
// void
// ThroughputTrace (int32_t oldValue, int32_t newValue)
// {
//   //std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
//   if(firstthrp){
//     *ThrpStream->GetStream () << "0.0 " << " " << newValue << std::endl;
//     firstthrp=false;
//   }
//   else{
//   *ThrpStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newValue << std::endl;
//   myObject->Thp++;
//   }
// }
// static void
// TraceThroughput (std::string thrp_tr_file_name)
// {  
//   AsciiTraceHelper ascii;
//   ThrpStream = ascii.CreateFileStream (thrp_tr_file_name.c_str ());  
//   myObject->TraceConnectWithoutContext ("Throughput", MakeCallback (&ThroughputTrace));
//   //std::cout<< cWndValue<<std::endl;
//   myObject->Thp=5;
// }

// void
// ThroughputTrace (uint32_t oldValue, uint32_t newValue)
// {
//   //std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
 
//   *ThrpStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newValue << std::endl;
  
  
// }
std::string str;
double t=0.001;
uint32_t prev=0;
double prevtime=0; 
static void
TraceThroughput (std::string thrp_tr_file_name,Ptr<FlowMonitor> monitor)
{  
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  auto itr=stats.begin();
  itr++;
  double curtime=Simulator::Now ().GetSeconds ();
  std::cout<<  curtime<<" "<< 8*(itr->second.rxBytes-prev)/(1000*1000*(curtime-prevtime))<< std::endl;
  prevtime=curtime;   
  prev=itr->second.rxBytes;
  Simulator::Schedule (Seconds (t), &TraceThroughput, str + "throughput.data", monitor);
  
}

static void
CwndTracer (uint32_t oldval, uint32_t newval)
{
  if (firstCwnd)
    {
      *cWndStream->GetStream () << "0.0 " << oldval << std::endl;
      firstCwnd = false;
    }
  *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval/1448 << std::endl;
  cWndValue = newval;

  if (!firstSshThr)
    {
      *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << ssThreshValue << std::endl;
    }
}

static void
SsThreshTracer (uint32_t oldval, uint32_t newval)
{
  if (firstSshThr)
    {
      *ssThreshStream->GetStream () << "0.0 " << oldval << std::endl;
      firstSshThr = false;
    }
  *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  ssThreshValue = newval;

  if (!firstCwnd)
    {
      *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << cWndValue << std::endl;
    }
}

static void
RtoTracer (Time oldval, Time newval)
{
  if (firstRto)
    {
      *rtoStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRto = false;
    }
  *rtoStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}

static void
InFlightTracer (uint32_t old, uint32_t inFlight)
{
  *inFlightStream->GetStream () << Simulator::Now ().GetSeconds () << " " << inFlight << std::endl;
}

static void
RttTracer (Time oldval, Time newval)
{
  if (firstRtt)
    {
      *rttStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRtt = false;
    }
  *rttStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}

static void
TraceCwnd (std::string cwnd_tr_file_name)
{
  AsciiTraceHelper ascii;
  cWndStream = ascii.CreateFileStream (cwnd_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
}

static void
TraceSsThresh (std::string ssthresh_tr_file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream = ascii.CreateFileStream (ssthresh_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SsThreshTracer));
}

static void
TraceRtt (std::string rtt_tr_file_name)
{
  AsciiTraceHelper ascii;
  rttStream = ascii.CreateFileStream (rtt_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeCallback (&RttTracer));
}

static void
TraceRto (std::string rto_tr_file_name)
{
  AsciiTraceHelper ascii;
  rtoStream = ascii.CreateFileStream (rto_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTO", MakeCallback (&RtoTracer));
}

static void
TraceInFlight (std::string &in_flight_file_name)
{
  AsciiTraceHelper ascii;
  inFlightStream = ascii.CreateFileStream (in_flight_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/BytesInFlight", MakeCallback (&InFlightTracer));
}


int main(int argc,char *argv[]){
   // LogComponentEnable("TcpBbrv2",LOG_LEVEL_INFO);
    uint32_t nLeaf=1;
    std::string router_bandwidth = "10Mbps";
    std::string router_delay = "10ms";
    std::string leaf_bandwidth = "1000Mbps";
    std::string leaf_delay = "5ms";
    std::string queue_disc_type = "ns3::FifoQueueDisc";
    uint32_t size=1000;
    //double start_time=0.01;
    // double stop_time=70;
    uint32_t tcp_adu_size=1448;
    std::string transport_prot = "TcpWestwood";
    std::string recovery="ns3::TcpPrrRecovery";
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y-%I-%M-%S",timeinfo);
    std::string currentTime (buffer);
    Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (6291456));
    Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (4194304));
    Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType",
                      TypeIdValue (TypeId::LookupByName (recovery)));

    Config::SetDefault ("ns3::FifoQueueDisc::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, size)));
    transport_prot = std::string ("ns3::") + transport_prot;
  // Select TCP variant
    if (transport_prot.compare ("ns3::TcpWestwoodPlus") == 0)
    { 
      // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      // the default protocol type in ns3::TcpWestwood is WESTWOOD
      Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
    }
    else
    {
      TypeId tcpTid;
      NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (transport_prot, &tcpTid), "TypeId " << transport_prot << " not found");
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));
    }
    
    PointToPointHelper pointToPointRouter;
    pointToPointRouter.SetDeviceAttribute  ("DataRate", StringValue (router_bandwidth));
    pointToPointRouter.SetChannelAttribute ("Delay", StringValue (router_delay));

    PointToPointHelper pointToPointLeaf;
    pointToPointLeaf.SetDeviceAttribute  ("DataRate", StringValue (leaf_bandwidth));
    pointToPointLeaf.SetChannelAttribute ("Delay", StringValue (leaf_delay));
    
    PointToPointDumbbellHelper d (nLeaf, pointToPointLeaf,
                                nLeaf, pointToPointLeaf,
                                pointToPointRouter);

    // Install Stack
    InternetStackHelper stack;
    d.InstallStack (stack);

    // Assign IP Addresses
    d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

    uint16_t port = 50000;

     PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApps;
    for(uint32_t i=0;i<nLeaf;i++)
    sinkApps.Add(sink.Install (d.GetRight (i)));
    sinkApps.Start (Seconds (0.0));
    sinkApps.Stop (Seconds (61.0));

    
    BulkSendHelper source ("ns3::TcpSocketFactory",
                         Address());
  // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (0));
      source.SetAttribute ("SendSize", UintegerValue (tcp_adu_size));

    ApplicationContainer sourceApps; 
     for (uint32_t i = 0; i < nLeaf; ++i){
        AddressValue remoteAddress (InetSocketAddress (d.GetRightIpv4Address (i), port));
      source.SetAttribute ("Remote", remoteAddress);
      sourceApps = source.Install (d.GetLeft (i));
     }
  
    sourceApps.Start (Seconds (0.0));
    sourceApps.Stop (Seconds (60.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    std::string dir = "newresults/" + transport_prot.substr(5, transport_prot.length()) + "/" + currentTime + "/";
    std::string dirToSave = "mkdir -p " + dir;
    system (dirToSave.c_str ());
    str=dir;
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    Simulator::Schedule (Seconds (0 + 0.000001), &TraceCwnd, dir + "cwnd.data");
    Simulator::Schedule (Seconds (60 + 0.000001), &TraceSsThresh, dir + "ssth.data");
    Simulator::Schedule (Seconds (60 + 0.000001), &TraceRtt, dir + "rtt.data");
    Simulator::Schedule (Seconds (60 + 0.000001), &TraceRto, dir + "rto.data");
    Simulator::Schedule (Seconds (60 + 0.000001), &TraceInFlight, dir + "inflight.data");
    Simulator::Schedule (Seconds (60 + 0.000001), &TraceThroughput, dir + "throughput.data",monitor);
    
    
    //std::cout<<tempthrp<<std::endl;
    Simulator::Stop (Seconds (60 + 1));
    Simulator::Run ();
    
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
      std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
      std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
    }

    Simulator::Destroy ();
    return 0;
}
