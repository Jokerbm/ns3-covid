// หน้ากาก = 95 % ลดอัตรการแพร่เชื้อ 90 %
// วัคซีน
// 1. shinovac 2 เข็ม กัน 51%
// ประชาชน 7คน#
// 2. astra 2 เข็มกัน 79%
// ประชาชน 2คน#
// ทหาร 10คน
// 3. pfizer 2 เข็มกัน 95%
// ทหาร 10คน
// 4. moderna 1  94%
// ประชาชน 1คน #
// 5. shiv + astra 78%
// ประชาชน 37คน
// 6. astra + pfizser 90%
// ประชาชน  6คน
// 7. คนติดโควิต
// ประชาชน 1 คน
// 8. unvaccineted
// ประชาชน 26คน

#include <ostream>
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

// ตัวแปรประแภท วัคซีนและประชากร
#define TOTAL_MAN 100
#define PEOPLE_SHINOVAC 7
#define PEOPLE_ASTRA 2
#define SOLIDER_ASTRA 10
#define SOLIDER_Pfizer 10
#define PEOPLE_MODERNA 1
#define PEOPLE_SIAS 37
#define PEOPLE_ASPRI 6
#define PEOPLE_INFECT 1
#define PEOPLE_UNVAC 26
#define DURATION 100.0
#define X_BOX 100
#define Y_BOX 100
#define NodeSide 3.0

#define INFECTRAD 2
using namespace ns3;
using namespace std;
bool infected[101];
int twoshiv = 0;
int twoastra = 0;
int twopfizer = 0;
int twomoderna = 0;
int shivastra = 0;
int aspfizer = 0;
int covid = 0;
int unvac = 0;

// set RGB colors
struct rgb
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct rgb colors[] = {
    {255, 126, 172}, // sinovac 2 ประชาชน 7คน
    {255, 255, 0},   // astra 2 ประชาชน 2คน
    {135, 190, 219}, // moderna 2 ประชาชน 1 คน
    {216, 133, 219}, // si+as ประชาชน 37 คน
    {162, 218, 143}, // as+phi ประชาชน 6 คน
    {255, 0, 0},     // people ติดเชื้อ ประชาชน 1คน
    {3, 157, 215},   // unvacc ประชาชน 26คน
    {61, 34, 82},    // sol astra 10 คน
    {40, 71, 55},    // phizer 2 ทหาร 10 คน
};

// node class
class People
{
public:
    int people_count, solider_count;
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

    People(int people, int solider_count);
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

// สร้างคน 100 คน
People people(TOTAL_MAN, 80);
AnimationInterface *pAnim = 0;

People::People(int people, int solider)
{
    people_count = people;
    solider_count = solider;
    node.Create(people_count);
    stack.Install(node);
}

void People::setCSMA(int dataRate, int delay, int mtu)
{
    csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(dataRate)));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(delay)));
    csma.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    device = csma.Install(node);

    // ตั้งค่า handler กรณีมี packet เข้ามา
    NetDeviceContainer::Iterator i;
    for (i = device.Begin(); i != device.End(); ++i)
    {
        (*i)->SetPromiscReceiveCallback(MakeCallback(&People::receiveCOVID, this)); // some NetDevice method
    }
}

void People::setIPV4(string address, string netmask)
{
    ipv4.SetBase(Ipv4Address(address.c_str()), Ipv4Mask(netmask.c_str()));
    interface = ipv4.Assign(device);
    serverAddress = Address(interface.GetAddress(54));
}

void People::setUDPClient(int people_id, Time startTime)
{
    uint32_t packetSize = 1024;
    uint32_t maxPacketCount = 1000;
    Time interPacketInterval = MilliSeconds(50);
    UdpEchoClientHelper client(serverAddress, port);
    client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client.SetAttribute("Interval", TimeValue(interPacketInterval));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));
    apps = client.Install(node.Get(people_id));
    apps.Start(startTime);
    apps.Stop(Seconds(DURATION));
}

void People::setMobility()
{
    // เดินแบบ random
    mobility_move.SetMobilityModel("ns3::GaussMarkovMobilityModel",
                                   "Bounds", BoxValue(Box(0, X_BOX, 0, Y_BOX, 0, 10)),
                                   "TimeStep", TimeValue(MilliSeconds(10)),
                                   "Alpha", DoubleValue(0.85),
                                   "MeanVelocity", StringValue("ns3::UniformRandomVariable[Min=50|Max=100]"),
                                   "MeanDirection", StringValue("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
                                   "MeanPitch", StringValue("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
                                   "NormalVelocity", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"),
                                   "NormalDirection", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
                                   "NormalPitch", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));
    mobility_move.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                       "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + to_string(X_BOX) + "]"),
                                       "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + to_string(Y_BOX) + "]"),
                                       "Z", StringValue("ns3::UniformRandomVariable[Min=0|Max=10]"));
    for (int i = 0; i < 100; i++)
    {
        mobility_move.Install(node.Get(i));
    }
}

// ถ้าได้รับ packet จะมาทำ function นี้ ใช้คำนวณโอกาสติดได้
bool People::receiveCOVID(
    Ptr<NetDevice> dst_device,
    Ptr<const Packet> packet,
    uint16_t protocol,
    const Address &src,
    const Address &dst,
    BridgeNetDevice::PacketType packetType)
{
    Ptr<MobilityModel> dst_mob = dst_device->GetNode()->GetObject<MobilityModel>();
    double dst_x = dst_mob->GetPosition().x;
    double dst_y = dst_mob->GetPosition().y;

    Ptr<Node> src_node = node.Get(getNodeIdFromAddress(src));
    Ptr<MobilityModel> src_mob = src_node->GetObject<MobilityModel>();
    double src_x = src_mob->GetPosition().x;
    double src_y = src_mob->GetPosition().y;

    double distance = sqrt(pow(dst_x - src_x, 2) + pow(dst_y - src_y, 2));
    infected[79] = true;
    infected[80] = true;
    infected[85] = true;
    infected[82] = true;
    infected[99] = true;
    // เช็คดูว่าเป็น UDP ไหม (ดูขนาด packet) เพื่อกรอง arp ออก
    if (packet->GetSize() > 1000)
    {

        double random = ((double)rand() / RAND_MAX) * 100;
        // int dst_node_id = getNodeIdFromAddress(dst);
        int src_node_id = getNodeIdFromAddress(src);

        if (distance < INFECTRAD)
        {
            if (infected[src_node_id] == false)
            {
                if (0 <= src_node_id && src_node_id <= 6)
                {
                    if (random <= 2.45)
                    {

                        infected[src_node_id] = true;
                        pAnim->UpdateNodeColor(people.node.Get(src_node_id), colors[5].r, colors[5].g, colors[5].b);
                        twoshiv++;
                    }
                }

                else if (7 <= src_node_id && src_node_id <= 8)
                {
                    if (random <= 1.05)
                    {
                        twoastra++;

                        infected[src_node_id] = true;
                        pAnim->UpdateNodeColor(people.node.Get(src_node_id), colors[5].r, colors[5].g, colors[5].b);
                    }
                }
                else if (9 == src_node_id)
                {
                    if (random <= 0.3)
                    {
                        twomoderna++;

                        infected[src_node_id] = true;
                        pAnim->UpdateNodeColor(people.node.Get(src_node_id), colors[5].r, colors[5].g, colors[5].b);
                    }
                }
                else if (10 <= src_node_id && src_node_id <= 46)
                {
                    if (random <= 1.1)
                    {
                        shivastra++;

                        infected[src_node_id] = true;
                        pAnim->UpdateNodeColor(people.node.Get(src_node_id), colors[5].r, colors[5].g, colors[5].b);
                    }
                }
                else if (47 <= src_node_id && src_node_id <= 52)
                {

                    if (random <= 0.5)
                    {
                        aspfizer++;

                        infected[src_node_id] = true;
                        pAnim->UpdateNodeColor(people.node.Get(src_node_id), colors[5].r, colors[5].g, colors[5].b);
                    }
                }
                else if (53 <= src_node_id && src_node_id <= 79)
                {
                    if (random <= 5.0)
                    {

                        infected[src_node_id] = true;
                        pAnim->UpdateNodeColor(people.node.Get(src_node_id), colors[5].r, colors[5].g, colors[5].b);
                        unvac++;
                    }
                }
                else if (80 <= src_node_id && src_node_id <= 89)
                {
                    if (random <= 1.05)
                    {
                        twoastra++;

                        infected[src_node_id] = true;
                        pAnim->UpdateNodeColor(people.node.Get(src_node_id), colors[5].r, colors[5].g, colors[5].b);
                    }
                }
                else if (90 <= src_node_id && src_node_id <= 99)
                {

                    if (random <= 0.5)
                    {
                        twopfizer++;

                        infected[src_node_id] = true;
                        pAnim->UpdateNodeColor(people.node.Get(src_node_id), colors[5].r, colors[5].g, colors[5].b);
                    }
                }
            }
        }
    }
    return true;
}

int People::getNodeIdFromAddress(const Address &address)
{
    // เอา mac address มาหาว่าเป็น node ไหนเพื่อไปดึง pos(x, y)
    int found = 0;
    for (int i = 0; i < 100; i++)
    {
        if (operator==(address, node.Get(i)->GetDevice(1)->GetAddress()))
        {
            found = i;
            break;
        }
    }
    return found;
}

int main(int argc, char *argv[])
{
    printf("\n working \n")
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // setCSMA -> DataRate, Delay, Mtu
    people.setCSMA(5000000, 0, 1400);

    // setIPV4 -> ip, netmask
    people.setIPV4("10.10.200.0", "255.255.255.0");

    // แยกพ่อค้ากับลูกค้า (เคลื่อนไหวได้กับไม่ได้)
    people.setMobility();

    // set udp client
    for (int i = 0; i < 100; i++)
    {
        if (i != 54)
        {
            people.setUDPClient(i, Seconds(0.0));
        }
    }
    // for (int i = 0; i < 101; i++)
    // {
    //     infected[i] = false;
    // }

    Simulator::Schedule(Seconds(0.00),
                        []()
                        {
                            for (int i = 0; i < PEOPLE_SHINOVAC + 1; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[0].r, colors[0].g, colors[0].b);
                            }
                            for (int i = PEOPLE_SHINOVAC + 1; i < PEOPLE_SHINOVAC + PEOPLE_ASTRA + 1; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[1].r, colors[1].g, colors[1].b);
                            }
                            for (int i = PEOPLE_SHINOVAC + PEOPLE_ASTRA + 1; i < PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + 1; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[2].r, colors[2].g, colors[2].b);
                            }
                            for (int i = PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + 1; i < PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + 1; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[3].r, colors[3].g, colors[3].b);
                            }
                            for (int i = PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + 1; i < PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + 1; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[4].r, colors[4].g, colors[4].b);
                            }
                            for (int i = PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + 1; i < PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + PEOPLE_INFECT + 1; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[5].r, colors[5].g, colors[5].b);
                            }
                            for (int i = PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + PEOPLE_INFECT + 1; i < PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + PEOPLE_INFECT + PEOPLE_UNVAC + SOLIDER_ASTRA + 1; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[6].r, colors[6].g, colors[6].b);
                            }
                            for (int i = PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + PEOPLE_INFECT + PEOPLE_UNVAC + 1; i < PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + PEOPLE_INFECT + PEOPLE_UNVAC + SOLIDER_ASTRA + 1; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[7].r, colors[7].g, colors[7].b);
                            }
                            for (int i = PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + PEOPLE_INFECT + PEOPLE_UNVAC + SOLIDER_ASTRA + 1; i < PEOPLE_SHINOVAC + PEOPLE_ASTRA + PEOPLE_MODERNA + PEOPLE_SIAS + PEOPLE_ASPRI + PEOPLE_INFECT + PEOPLE_UNVAC + SOLIDER_ASTRA + SOLIDER_Pfizer; i++)
                            {
                                pAnim->UpdateNodeSize(i, NodeSide, NodeSide);
                                pAnim->UpdateNodeColor(people.node.Get(i), colors[8].r, colors[8].g, colors[8].b);
                            }
                        });

    // จบ Simulation
    Simulator::Stop(Seconds(DURATION));

    // ไฟล์ NetAnimation
    pAnim = new AnimationInterface("covid-model.xml");
    pAnim->SetMaxPktsPerTraceFile(1000000);

    Simulator::Run();
    Simulator::Destroy();
    printf("twoshiv : %d\ntwoastra : %d\ntwomoderna : %d\nshivastra : %d\naspfizer : %d\ntwopfizer : %d\nunvac : %d\n", twoshiv, twoastra, twomoderna, shivastra, aspfizer, twopfizer, unvac);

    return 0;
}
