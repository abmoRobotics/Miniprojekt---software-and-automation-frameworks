// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
// #include <strstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include "rapidxml-1.13/rapidxml.hpp"
#include "pugi/pugixml.hpp"


std::vector<std::vector<int>> readCSV()
{
    std::string line;                    /* string to hold each line */
    std::vector<std::vector<int>> array;      /* vector of vector<int> for 2d array */
    std::ifstream f;
    f.open("procssing_times_table.csv");

    if (!f.is_open()) {     /* validate file open for reading */
        perror (("error while opening file "));
    }

    while (std::getline(f, line)) {         /* read each line */
        std::string val;                     /* string to hold value */
        std::vector<int> row;                /* vector for row of values */
        std::stringstream s(line);          /* stringstream to parse csv */
        while (std::getline(s, val, ';')) {  /* for each value */
            //std::cout << val << std::endl;
            try{
                row.push_back(std::stoi(val));  /* convert to int, add to row */
            }
            catch (const std::exception& e) {
                std::cout << "ERROR" << std::endl;
            };
        }
        if(!row.empty()){
            array.push_back(row);}          /* add row to array */
        //std::cout << "\n"; 
    }
    f.close();

    std::cout << "complete array\n\n";
    std::cout << array.size() << std::endl;

    // for(auto& row : array) {           /* iterate over rows */
    //     for(auto& vald : row){        /* iterate over vals */
    //         std::cout << vald << "  ";        /* output value      */
    //     }
    //     std::cout << "\n";                   /* tidy up with '\n' */
    //     std::cout << "hej " << std::endl;
    // }
    return array;
};

void logData(int carrierID, int rfidTag, std::string time){
    std::ofstream logfile;
    logfile.open("logFile.csv", std::fstream::app);
    logfile << carrierID << ", ";
    logfile << rfidTag << ", ";
    logfile << time << "\n";
    logfile.close();

}

std::array<int,2> decodeXML(char buffer[]){
    // variables
    int carrierID;
    int rfidTag;
    std::string time;
    std::array<int,2> palletInfo{0,0};

    // load the XML file
    pugi::xml_document doc;
    std::cout << buffer << std::endl;
    
    if (!doc.load_string(buffer)) exit;
    pugi::xml_node tools = doc.child("Pallet_Data").child("Component");
    int i, j;

    bool dataRecieved = false;
     
    for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it)
    {
        std::string name;
        // std::cout << it->name() << ": ";
        // std::cout << it->child_value() << std::endl;
        name = it->name();
        if (name == "Station"){
            std::cout << name << ": " << it->child_value() << std::endl;
            carrierID = std::stoi(it->child_value());
        }
        else if (name == "rfid")
        {
            std::cout << name << ": " << it->child_value() << std::endl;
            rfidTag = std::stoi(it->child_value());
        }  
        else if (name == "Time")
        {
           std::cout << name << ": " << it->child_value() << std::endl;
           time = it->child_value();
           dataRecieved = true;
        }
        
    }

    if (dataRecieved){
        logData(carrierID,rfidTag,time);
        palletInfo[1] = carrierID - 1;
        palletInfo[0] = rfidTag - 1;
        return palletInfo;
    }
    return palletInfo;
}



#define PORT 8001
int main(int argc, char const *argv[])
{
    std::vector<std::vector<int>> carrierData;
    carrierData = readCSV();
    printf("Server started\n");
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
       
    // Set socket options. Allows for reusing the same adress, though it has been used by the same program previously.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    std::cout << "Connected" <<std::endl;

    Place:

    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    // int valread2;
    // valread2 = std::stoi(valread);

    // std::ofstream logfile;
    // logfile.open("logFile.csv", std::fstream::app);

    char returnMessage[] = "0";
    std::string Besked;
    // pugi::xml_document doc;
    // std::cout << buffer << std::endl;
    // // load the XML file
    
    // if (!doc.load_string(buffer)) return -1;
    // pugi::xml_node tools = doc.child("Pallet_Data").child("Component");
    // int i, j;
    // std::string p1 = "Station";
    // bool dataRecieved = false;

     
    // for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it)
    // {
    //     std::string name;
    //     // std::cout << it->name() << ": ";
    //     // std::cout << it->child_value() << std::endl;
    //     name = it->name();
    //     if (name == "Station"){
    //         std::cout << name << ": " << it->child_value() << std::endl;
    //         j = std::stoi(it->child_value()) - 1;
    //         logfile << it->child_value() << ", ";
    //     }
    //     else if (name == "rfid")
    //     {
    //         std::cout << name << ": " << it->child_value() << std::endl;
    //         i = std::stoi(it->child_value()) - 1;
    //         logfile << it->child_value() << ", ";
    //     }  
    //     else if (name == "Time")
    //     {
    //        std::cout << name << ": " << it->child_value() << std::endl;
    //        logfile << it->child_value() << "\n";
    //        dataRecieved = true;
    //     }
        
    // }
    // logfile.close();
    std::string message;
    std::array<int,2> carrierInfo;
    carrierInfo = decodeXML(buffer);
    int carrierID = carrierInfo[0];
    int rfidTag = carrierInfo[1];
    message = ("TIME#" + std::to_string(carrierData[carrierID][rfidTag]) + "ms");
    // if(dataRecieved){
    //     message = std::to_string(carrierData[i][j]);
    // }else{
    //     message = "0";
    // }    // std::cout << carrierData[i][j] << std::endl;

    // dataRecieved = false;
    strcpy(returnMessage,message.c_str());

    send(new_socket , returnMessage , strlen(returnMessage) , 0 );
    printf("Message sent\n");

    goto Place;

    return 0;
}