// Server side C/C++ program to communicate with Fiesto PLC
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include "pugi/pugixml.hpp"

// Reads the processing_times_table file and converts the input to a 2D-array.
std::vector<std::vector<int>> readCSV()
{
    std::string line;                    /* string to hold each line in the table*/
    std::vector<std::vector<int>> array; /* vector of vector<vector<int>> for 2d array */
    std::ifstream f;                     /* Creates an input stream*/
    f.open("procssing_times_table.csv"); /* Opens table with processing times */

    if (!f.is_open())
    { /* validate file open for reading */
        perror(("error while opening file "));
    }

    while (std::getline(f, line))
    {                              /* read each line */
        std::string val;           /* string to hold value */
        std::vector<int> row;      /* vector for row of values */
        std::stringstream s(line); /* stringstream to parse csv */
        while (std::getline(s, val, ';'))
        { /* for each value */
            //std::cout << val << std::endl;
            try
            {
                row.push_back(std::stoi(val)); /* convert to int, add to row.
                 Text entries will create an exception and be ignored */
            }
            catch (const std::exception &e)
            {
                //std::cout << "ERROR" << std::endl;
            };
        }
        if (!row.empty()) /* checks if row is empty*/
        {
            array.push_back(row); /* add row to array */
        }
    }
    f.close(); /* close file */

    return array;
};

// Outputs data to a csv logfile
void logData(int carrierID, int rfidTag, std::string time)
{
    std::ofstream logfile;                          /* create output stream */
    logfile.open("logFile.csv", std::fstream::app); /* open file */
    logfile << carrierID << ", ";
    logfile << rfidTag << ", ";
    logfile << time << "\n";
    logfile.close(); /* close file */
}

//Function for decoding XML data.
std::array<int, 2> decodeXML(char buffer[])
{
    // variables
    int carrierID;
    int rfidTag;
    std::string time;
    std::array<int, 2> palletInfo{0, 0};

    // load the XML file
    pugi::xml_document doc;           /* define xml document */
    std::cout << buffer << std::endl; /* output data */

    if (!doc.load_string(buffer)) /* try to load data as xml, fails if the data is not formatted correctly */
        exit;
    pugi::xml_node tools = doc.child("Pallet_Data").child("Component"); // Get the component child.

    bool dataRecieved = false;
    std::string name;

    for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it)
    {
        name = it->name();     /* get name of the current iteration of node in <Component> */
        if (name == "Station") /* check if the name of the node is Station */
        {
            std::cout << name << ": " << it->child_value() << std::endl;
            carrierID = std::stoi(it->child_value()); /* convert value to int */
        }
        else if (name == "rfid") /* check if the name of the node is rfid */
        {
            std::cout << name << ": " << it->child_value() << std::endl;
            rfidTag = std::stoi(it->child_value()); /* convert value to int */
        }
        else if (name == "Time") /* check if the name of the node is Time */
        {
            std::cout << name << ": " << it->child_value() << std::endl;
            time = it->child_value(); /* set time */
            dataRecieved = true;
        }
    }

    if (dataRecieved) /* log data if received */
    {
        logData(carrierID, rfidTag, time);
        palletInfo[1] = carrierID - 1;
        palletInfo[0] = rfidTag - 1;
        return palletInfo; /* returns information about the pallet, 
        so that the processing time can be returned later */
    }
    return palletInfo;
}

#define PORT 8001
int main(int argc, char const *argv[])
{
    // Reads the data from the processing_time_table
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
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
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
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    std::cout << "Connected" << std::endl;

    std::string message;
    std::array<int, 2> carrierInfo;

 
    // Reads data from PLC
    while (1)
    {
        // Reads data from tcp client
        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);

        //empties return message
        char returnMessage[] = "0";

        // Reads xml message from PLC, and returns carrierID and rfidTag.
        carrierInfo = decodeXML(buffer);
        int carrierID = carrierInfo[0];
        int rfidTag = carrierInfo[1];

        // Generates message containing process time for PLC
        message = ("TIME#" + std::to_string(carrierData[carrierID][rfidTag]) + "ms");

        // Converts to c_str
        strcpy(returnMessage, message.c_str());

        // Sends data to PLC
        send(new_socket, returnMessage, strlen(returnMessage), 0);
        printf("Message sent\n");
    }



    return 0;
}