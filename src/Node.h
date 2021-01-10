
#ifndef __DEATHLYHALLOWS_SF2020_NODE_H_
#define __DEATHLYHALLOWS_SF2020_NODE_H_

#include <omnetpp.h>
#include <bits/stdc++.h>
#include "MyMessage_m.h"

using namespace omnetpp;
using namespace std;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  private:
    char flag, esc;
    vector<string> msgsVec;
    int nxtMsgIndex;            // TODO: To be initialized.
    bool isMeFinished;          // TODO: To be initialized.

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    string framming (string payload);
    bool getUserMsg (string& msg);
    int getCheckSum (string& msg, int sum=0);
    void addNoiseAndSend(cMessage *msg,int dest);
    bool checkError(string& frame, int checksum);
};

#endif
