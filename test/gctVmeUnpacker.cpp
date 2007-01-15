
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "EventFilter/GctRawToDigi/src/GctEvent.h"


// maximum event size in 32-bit words
#define MAXSIZE 1024

using std::ifstream;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

void readEvent(ifstream& file, unsigned char * data, unsigned int& evtSize);

int main(int argc, char* argv[]) {

  char* filename;
  vector<GctEvent> events;

  if (argc>0) { filename=argv[1]; }
  else { filename="test.dat"; }

  // open file
  ifstream inFile(filename);

  if (inFile.is_open()) {

    cout << "Reading events from " << filename << endl;

    unsigned char rawEvt[MAXSIZE];
    unsigned int evtSize;

    // event loop
    while ( ! inFile.eof() ) {

      // read an event
      readEvent(inFile, &rawEvt[0], evtSize);

      cout << "Event size is " << evtSize << endl;
      GctEvent evt(&rawEvt[0], evtSize);
      events.push_back(evt);

    }

    inFile.close();

  }
  else {
    cerr << "Could not open file " << filename << endl;
  }

  // now do something with the events
  vector<GctEvent>::const_iterator e;
  for (e=events.begin(); e!=events.end(); e++) {
    cout << (*e);
  }

  return 0;

};


void readEvent(ifstream& file, unsigned char * data, unsigned int& evtSize) {

  // read lines until one is blank
  // split each line and store in data
  string line;

  int i=0;

  cout << "reading event" << endl;
  getline(file, line);

  for (i=0; (line != "") && (i < MAXSIZE) ; i++) {

    // NB file is 32-bit words, data is char !!!

    std::istringstream iss(line);
    unsigned d;
    if ( !(iss >> std::hex >> d).fail() ) {
      data[4*i] = d&0xff;
      data[4*i+1] = (d>>8)&0xff;
      data[4*i+2] = (d>>16)&0xff;
      data[4*i+3] = (d>>24)&0xff;
    }
    else {
      cerr << "Invalid line entry!" << line <<  endl;
      exit(1); 
    }

    cout << "Line " << i<< " : " << std::hex << d << endl;
    //    cout << "Stored " << std::hex << data[4*i] << "." << data[4*i+1] << "." << data[4*i+2] << "." << data[4*i+3] << endl;
    //    unsigned tmp = data[4*i] + (data[4*i+1]<<8) + (data[4*i+2]<<16) + (data[4*i+3]<<24);
    //    cout << "Remade " << std::hex << tmp << endl;

    getline(file, line);

  }

  if (line != "") {
    cerr << "Found event larger than " << MAXSIZE << " bytes!" << endl;
  }

  evtSize = 4*i;

}
