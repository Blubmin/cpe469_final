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
#include <bitset>
#include <curl/curl.h>
#include <math.h>

#define CHUNK_SIZE 4096
#define MAX_DIGITS 9

using namespace std;

ChordNode *node = NULL;
string rootDir = "";

int getNumChunks(int size)
{
	return ((size - 1) / CHUNK_SIZE) + 1;
}

void getFile(string fileName)
{
	FILE * file;
	file = fopen((rootDir + "/" + fileName).c_str(), "w");
	
	if (!file)
	{
		cout << "Unable to open file " << fileName << " for writing." << endl;
		return;
	}

	int file_size = atoi(node->get(fileName + ".size").c_str());
	int num_chunks = getNumChunks(file_size);
	CURL * curl = curl_easy_init();
	for (int i = 0; i < num_chunks; i++)
	{
		char *chunk;
		char num[MAX_DIGITS];
		

		sprintf(num, "%d", i);
		char * escaped_chunk;
		//strcpy(escaped_chunk, node->get(fileName + "_" + num).c_str());
		int c_size = CHUNK_SIZE;
		chunk = curl_easy_unescape(curl, node->get(fileName + "_" + num).c_str(), 0, &c_size);

		bitset<CHUNK_SIZE> bitset(node->get(fileName + "_" + num + ".bitset"));

		for (int j = 0; j < CHUNK_SIZE; j++)
		{
			if (bitset[j])
				chunk[j]--;
		}

		fwrite(chunk, 1, min(CHUNK_SIZE, file_size), file);
		file_size -= CHUNK_SIZE;
		curl_free(chunk);
	}
	curl_easy_cleanup(curl);

	fclose(file);
}

int chunkFile(string fileName)
{
    /*ifstream fin(fileName.c_str(), ifstream::binary);
    ofstream outfile;
    ostringstream convert;
    string numString;
    string tmpFname;

    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);

    char* fileBuffer = new char[length];
    char* tmpBuffer = new char[CHUNK_SIZE];

    int buf_i = 0, chunk_i = 0;

    while (length > CHUNK_SIZE)
    {
        convert.str("");
        convert.clear();
        convert << chunk_i;
        numString = convert.str();

        tmpFname = fileName;
        tmpFname.append("_");
        tmpFname.append(numString);

        //outfile.open(tmpFname, ios::out | ios::binary | ios::app);
        memcpy((void*)tmpBuffer, (void*)&fileBuffer[buf_i], CHUNK_SIZE);
        cout << tmpBuffer << endl;
        node->put(tmpFname, tmpBuffer);
        //outfile.write(tmpBuffer, CHUNK_SIZE);
        //outfile.close();

        length -= CHUNK_SIZE;
        buf_i += CHUNK_SIZE;
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

    return chunk_i;*/

    
    FILE * fin;
    fin = fopen(fileName.c_str(), "r");
    FILE * test = fopen("putTest.txt", "w");
    if (!fin)
    {
    	cout << "Unable to open " << fileName << endl;
    	return -1;
    }

    fseek(fin, 0, SEEK_END);
    int file_size = ftell(fin);
    int num_chunks = getNumChunks(file_size);
    fseek(fin, 0, SEEK_SET);
    cout << "Num Chunks: " << num_chunks << endl;

    for (int i = 0; i < num_chunks; i++)
    {
    	char chunk[CHUNK_SIZE];
    	bitset<CHUNK_SIZE> bitset(0);

    	for (int j = 0; j < CHUNK_SIZE; j++)
    		chunk[j] = 0;

    	fread(chunk, 1, CHUNK_SIZE, fin);

    	for (int j = 0; j < CHUNK_SIZE; j++)
    	{
    		if (!chunk[j])
    		{
    			chunk[j]++;
    			bitset[j] = 1;
    		}
    	}

    	fwrite(chunk, 1, CHUNK_SIZE, test);

    	CURL *curl = curl_easy_init();
    	char* escaped_chunk = curl_easy_escape(curl, chunk, CHUNK_SIZE);

    	string chunkName = fileName + "_";
    	char num[MAX_DIGITS];
    	sprintf(num, "%d", i);
    	node->put(chunkName + num, escaped_chunk);
    	node->put(chunkName + num + ".bitset", bitset.to_string());

    	curl_free(escaped_chunk);
    	curl_easy_cleanup(curl);
    }

    fclose(fin);
    fclose(test);

    return file_size;
}

void putFile(string fileName)
{
	ifstream file(fileName);
  if(file.good()) {
    int file_size = chunkFile(fileName);
    char num[MAX_DIGITS];
    sprintf(num, "%d", file_size);
    node->put(fileName + ".size", num);
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
					"3) Exit\n\n";
			cout << "---> ";
			cin >> entry;
			int chx = atoi(entry);

			switch (chx) {
    			case 0:
    				cout << "\n" << node->printStatus();
    				break;
    			case 1:
    				cout << "Filename? ";
    				cin >> key;
    				putFile(key);
    				break;
    			case 2:
    				cout << "Filename? ";
    				cin >> key;
    				getFile(key);
    				break;
    			case 3:
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
