#include <ostream>
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

#define DURATION 10.0 // เวลาจำลอง 10 วินาที
#define N 100 // จำนวนคนทั้งหมด
#define M 80 // จำนวนลูกค้า
#define X_BOX 100 // กว้างแนวนอน
#define Y_BOX 50 // กว้างแนวตั้ง
#define INFECTRAD 2 // ระยะห่างที่ปลอดภัย (จำลอง 2 เมตร)
#define INFECTCHANCE 1.5 // โอกาสติด 1.5%

#define NodeSide 1.0 // ขนาดของจุดใน netanim

int infected_list[] = {0, 1, 2, 3, 4};

using namespace ns3;
using namespace std;

struct rgb {
  uint8_t r; 
  uint8_t g; 
  uint8_t b; 
};

struct rgb colors [] = {
  { 252, 44, 44 }, // ติดเชื้อ
  { 127, 255, 46 }, // ลูกค้า
  { 46, 131, 255 }  // พ่อค้า
};

class People {
  public:
    int people_count, customer_count;
    NodeContainer node;
    NetDeviceContainer device;
    Address serverAddress;
    Ipv4AddressHelper ipv4;
    InternetStackHelper stack;
    CsmaHelper csma;
    Ipv4InterfaceContainer interface;
    ApplicationContainer apps;
    MobilityHelper mobility_move, mobility_nomove;
    uint16_t port = 9;

    People(int people, int customer);
    void setCSMA(int dataRate, int delay, int mtu);
    void setIPV4(string address, string netmask);
    void setUDPClient(int people_id, Time startTime);
    void setMobility();
    bool receiveCOVID(
      Ptr<NetDevice> dst_device, 
      Ptr<const Packet> packet, 
      uint16_t protocol,
      const Address &src, 
      const Address &dst, 
      BridgeNetDevice::PacketType packetType);
    int getNodeIdFromAddress(const Address &src);
};

// สร้างคน 100 คน โดยแบ่งเป็นลูกค้า 80 พ่อค้า 20
People people(N, M);
AnimationInterface * pAnim = 0;
bool is_infected[N] = {false};

People::People(int people, int customer) {
  people_count = people;
  customer_count = customer;

  // สร้าง node จำนวน people โหนด
  node.Create (people_count);

  // ติดตั้ง internet stack
  stack.Install (node);
}

void People::setCSMA(int dataRate, int delay, int mtu) {
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delay)));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (mtu));
  device = csma.Install (node);

  // ตั้งค่า handler กรณีมี packet เข้ามา
  // NetDeviceContainer::Iterator i;
  // for (i = device.Begin (); i != device.End (); ++i) {
  //   (*i)->SetPromiscReceiveCallback (MakeCallback (&People::receiveCOVID, this));  // some NetDevice method
  // }
}

void People::setIPV4(string address, string netmask) {
  ipv4.SetBase (Ipv4Address(address.c_str()), Ipv4Mask(netmask.c_str()));
  interface = ipv4.Assign (device);
  serverAddress = Address(interface.GetAddress (1));
}

void People::setUDPClient(int people_id, Time startTime) { // เสมือนติดตั้งตัวแพร่เชื้อ (ให้เฉพาะคนติดโควิด)
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1000;
  Time interPacketInterval = MilliSeconds (50);
  UdpEchoClientHelper client (serverAddress, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  
  // ถ้าติดก็เปลี่ยนสีพร้อมลงตัวแพร่เชื้อ
  if (is_infected[people_id] == false) {
    is_infected[people_id] = true;
    apps = client.Install (node.Get (people_id));
    apps.Start (startTime);
    apps.Stop (Seconds (DURATION));
  }
  nt main (int argc, char *argv[]) {
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  // setCSMA -> DataRate, Delay, Mtu
  people.setCSMA(5000000, 0, 1400);

  // setIPV4 -> ip, netmask
  people.setIPV4("10.10.100.0", "255.255.255.0");

  // ไฟล์ NetAnimation
  pAnim = new AnimationInterface ("covid-model.xml");
  // pAnim->EnablePacketMetadata (true); // แสดงประเภท packet บนลูกศร
  pAnim->SetMaxPktsPerTraceFile (1000000);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}