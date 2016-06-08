/*
 *  example.cpp
 *  A simple main() to try out the Library
 *  Created by Laurent Vanni & Nicolas Goles Domic, 2010
 *
 */

#include <iostream>
#include <string>
#include <stdlib.h>
#include "./myMed/ChordNode.h"
#include <pthread.h>
#include "./myMed/ProtocolSingleton.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sstream>

using namespace std;

ChordNode *node = NULL;
string rootDir = "";

void getFile(string fileName)
{
	ofstream file;
	file.open((rootDir + "/" + fileName).c_str(), ios::out | ios::trunc);
	
	if (file.is_open())
	{
		int size = atoi(node->get(fileName + ".size").c_str());
		for (int i = 0; i < size; i++)
		{
			char num[32] = {};
			sprintf(num, "%d", i);
			string chunk = node->get(fileName + "_" + num);
			if (chunk != "null")
			{
				file << chunk;
			}
		}
		file.close();
	}
}

int chunkFile(string fileName)
{
    ifstream fin(fileName, ifstream::binary);
    ofstream outfile;
    ostringstream convert;
    string numString;
    string tmpFname;

    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);

    char* fileBuffer = new char[length];
    char* tmpBuffer = new char[4096];

    int buf_i = 0, chunk_i = 0;

    while (length > 4096)
    {
        convert.str("");
        convert.clear();
        convert << chunk_i;
        numString = convert.str();

        tmpFname = fileName;
        tmpFname.append("_");
        tmpFname.append(numString);

        //outfile.open(tmpFname, ios::out | ios::binary | ios::app);
        memcpy((void*)tmpBuffer, (void*)&fileBuffer[buf_i], 4096);
        node->put(tmpFname, tmpBuffer);
        //outfile.write(tmpBuffer, 4096);
        //outfile.close();

        length -= 4096;
        buf_i += 4096;
        chunk_i++;
    }

    if (length > 0)
    {
        convert.str("");
        convert.clear();
        convert << chunk_i;
        numString = convert.str();

        tmpFname = fileName;
        tmpFname.append("_");
        tmpFname.append(numString);

        //outfile.open(tmpFname, ios::out | ios::binary | ios::app);
        memcpy((void*)tmpBuffer, (void*)&fileBuffer[buf_i], length);
        node->put(tmpFname, tmpBuffer);
        //outfile.write(tmpBuffer, length);
        //outfile.close();
    }

    delete fileBuffer;
    delete tmpBuffer;

    return chunk_i;
}

void putFile(string fileName)
{
	ifstream file(fileName.c_str());
  if(file.good()) {
    int chunk_count = chunkFile(fileName);
    node->put(fileName + ".size", to_string(chunk_count));
  } else {
    throw invalid_argument("file does not exist or could not be opened");
  }

}

// This application receives args, "ip", "port", "overlay identifier (unique string)", "root directory)"
int main(int argc, char * const argv[]) 
{
	string backBone[] = {
			// user backbone
            "127x01",
	};
	
    Node *chord = NULL;

	if (argc >= 4) {
		// Create a test node
		node = P_SINGLETON->initChordNode(std::string(argv[1]), atoi(argv[2]), std::string("chordTestBed"), std::string(argv[3]));
		rootDir = argv[3];
        chord = NULL;
        
		// join to an existing chord
		if (argc == 5) {
			cout << "joining...\n";
			int i = 0;
		    chord = new Node(backBone[0], 5454);
			node->join(chord);
		}

		char entry[256];
		string key;
		string value;
		
		while (1) { // sleep...
			cout << "\n0) Print status\n" << 
					"1) Put\n" << 
					"2) Get\n" <<
					"3) Remove\n" << 
					"4) Exit\n\n";
			cout << "---> ";
			cin >> entry;
			int chx = atoi(entry);

			switch (chx) {
    			case 0:
    				cout << "\n" << node->printStatus();
    				break;
    			case 1:
    				cout << "Key = ";
    				cin >> key;
    				cout << "Value = ";
    				cin >> value;
    				node->put(key, value);
    				break;
    			case 2:
    				cout << "Key = ";
    				cin >> key;
    				cout << "\n" << node->get(key) << "------> found!" << endl;
    				break;
    			case 3:
    				cout << "Key = ";
    				cin >> key;
    				node->removekey(key);
    				break;
    			case 4:
    				node->shutDown();
    			default:
				break;
			}       
	    }
    } else {
		cout << "wrong parameters: test.out <hostname> <portNumber> <webContentDirectory> [--join]\n";
	}

    delete node;
    delete chord;
	return 0;
}
