
#ifndef __DEATHLYHALLOWS_SF2020_NODE_H_
#define __DEATHLYHALLOWS_SF2020_NODE_H_

#include <omnetpp.h>
#include <bitset>
using namespace omnetpp;
using namespace std;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  public:
    char stuffingByte1,stuffingByte2;

  private:
    bool inTransmission;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    string framming (string payload);
    string getUserMsg ();
    int getCheckSum (string msg);
    string addNoise(string msg);
    void startTransmission();
    void endTransmission();
};

#endif
