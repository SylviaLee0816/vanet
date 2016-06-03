/*
 * 统计模块
 */


#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <math.h>
#include "ns3/simulator.h"



using namespace std;

int
main (int argc, char *argv[])
{
    double border1;
    double border2;
    double border3;
    double border4;
    
    double count1=0;
    double count2=0;
    double count3=0;
    double count4=0;
    
    double num1=0;
    double num2=0;
    double num3=0;
    double num4=0;
    
    double success1=0;
    double success2=0;
    double success3=0;
    double success4=0;
    
    int packetNum;
    
    std::cout << "Input your packetNum :" << std::endl;
    std::cin >> packetNum;

    
    std::cout << "Input your border-1 :" << std::endl;
    std::cin >> border1;
    
    std::cout << "Input your border-2 :" << std::endl;
    std::cin >> border2;
    
    std::cout << "Input your border-3 :" << std::endl;
    std::cin >> border3;
    
    std::cout << "Input your border-4 :" << std::endl;
    std::cin >> border4;
    
    
    int packetSeq;
    for(packetSeq=1;packetSeq<=packetNum;packetSeq++){
        std::string temp = "/Users/sylvia/documents/temp/temp/packet";
        std::stringstream stream;
        stream << packetSeq;
        std::string seq_temp = stream.str ();
        temp.append(seq_temp);
        temp.append(".txt");
        //std::cout << temp << std::endl;
        
        std::ifstream fp;
        fp.open(temp);
        
        if (fp==0 )
        {
            printf("cannot open this file\n");
            exit(0);
        }
        else{
            int packetSeq;
            fp>> packetSeq;
            
            double sendX,sendY;
            fp>> sendX;
            fp>> sendY;
            
            
            double data;
            while(fp>> data){
                // std::cout << data << std::endl;
                if(data==-1){
                    break;
                }
                if(data<=border1){
                    count1++; continue;
                }
                if(data>border1 && data<=border2){
                    count2++; continue;
                }
                if(data>border2 && data<=border3){
                    count3++; continue;
                }
                if(data>border3 && data<=border4){
                    count4++; continue;
                }
                
            }
            
            double x,y;
            double distance;
            
            
            while(fp>> x){
                fp>> y;
                
                distance = sqrt(pow(fabs(x-sendX),2)+pow(fabs(y-sendY),2));
                
                // std::cout << distance << std::endl;
                if(distance<=border1){
                    num1++; continue;
                }
                if(distance>border1 && distance<=border2){
                    num2++; continue;
                }
                if(distance>border2 && distance<=border3){
                    num3++; continue;
                }
                if(distance>border3 && distance<=border4){
                    num4++; continue;
                }
                
            }
            
            std::cout << "packetSeq: " <<packetSeq<< std::endl;
            if(count1!=0){
                std::cout << "<= border1: " << num1/count1<< std::endl;
                success1 +=num1/count1;
            }else{
                std::cout << "<= border1: " << "0" << std::endl;
            }
            if(count2!=0){
                std::cout << "border1 ~ border2: " << num2/count2<< std::endl;
                success2 +=num2/count2;
            }else{
                std::cout << "border1 ~ border2: " << "0" << std::endl;
            }
            if(count3!=0){
                std::cout << "border2 ~ border3: " << num3/count3<< std::endl;
                success3 +=num3/count3;
            }else{
                std::cout << "border2 ~ border3: " << "0"<< std::endl;
            }
            if(count4!=0){
                std::cout << "border3 ~ border4: " << num4/count4<< std::endl;
                success4 +=num4/count4;
            }else{
                std::cout << "border3 ~ border4: " << "0" << std::endl;
            }
            
            std::cout << "packetSeq: " <<packetSeq<< "  end"<<std::endl;
            std::cout << "    "<<std::endl;

            
            fp.close();
            
        }

    }
    
    
    std::cout << "Total-Result: "<< std::endl;
    std::cout << "<= border1: " << success1/packetNum<< std::endl;
    std::cout << "border1 ~ border2: " << success2/packetNum<< std::endl;
    std::cout << "border2 ~ border3: " << success3/packetNum<< std::endl;
    std::cout << "border3 ~ border4: " << success4/packetNum<< std::endl;



}

