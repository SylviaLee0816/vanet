#include "application-control.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include "ns3/mobility-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ApplicationControl");

ApplicationControl::ApplicationControl ()
{
	NS_LOG_FUNCTION (this);
	Count1 = 0;
	Count2 = 0;
	Count3 = 0;
}

TypeId
ApplicationControl::GetTypeId (void)
{
  
  static TypeId tid = TypeId ("ns3::ApplicationControl")
  
    .SetParent<Object> ()
    .SetGroupName ("Wave")
    .AddConstructor<ApplicationControl> ();

  return tid;
}

void
ApplicationControl::CreateTxtNote (int packetSeq, double x, double y)
{
	std::string temp = "packet";
	std::stringstream stream;
	stream << packetSeq;
	std::string seq_temp = stream.str ();
	temp.append(seq_temp);
	temp.append(".txt");
	std::cout << temp << std::endl;
	std::ofstream outfile;
        outfile.open(temp.c_str());
	outfile << packetSeq  << "  " << x <<"   " << y << std::endl;

	   uint32_t nNodes = total.GetN ();
       for (uint32_t i = 0 ;i < nNodes; ++i)
        {
          Ptr<Node> p = total.Get (i);
          Ptr<Object> object = p;
     
          Ptr<MobilityModel> model = object->GetObject<MobilityModel> ();
         double temp =  sqrt(pow((model->GetPosition().x-x),2)+pow((model->GetPosition().y-y),2)) ;
          if(fabs(temp) >0.000001){
		outfile<< temp << std::endl;
           }
        }
        outfile << -1 << std::endl;
	outfile.close();}

void
ApplicationControl::WriteMessage (int packetSeq, double x, double y)
{

	std::string temp = "packet";
	std::stringstream stream;
	stream << packetSeq;
	std::string seq_temp = stream.str ();
	temp.append(seq_temp);
	temp.append(".txt");
	std::cout << temp << std::endl;
	std::ofstream outfile;
        outfile.open(temp.c_str(),std::ios::app);

	outfile << x <<"   " << y << std::endl;
	outfile.close();
    //std::ofstream outfile("packet"+packetSeq+".txt");
	//outfile <<" Receive A packet  "<< packetSeq <<"  " << x <<"   "<< y << std::endl;
}

void
ApplicationControl::CloseTxtNote (int packetSeq)
{

}

void 
ApplicationControl::SetNodeContainer(NodeContainer temp)
{

	total = temp;
}

}
