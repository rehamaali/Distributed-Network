
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


    enum EventType
{
    masterInit,
    readyToSend,
    frameArrival,
    checksumErr,
    timeout,
    ackTimeout,
    None

};

class Node : public cSimpleModule
{

  private:
    const static int MAXSEQNUM = ((1<<3)-1);
    const static int BUFCOUNT = ((MAXSEQNUM+1)>>1);
    char flag, esc;
    vector<string> msgsVec;

    int currentPeer;
    int nxtMsgIndex;            // TODO: To be initialized.
    bool isMeFinished;          // TODO: To be initialized.
    int isSource;
    bool noNAK;

    int ackExpected;
    int nextFrameToSend;
    int frameExpected;
    int tooFar;

    vector<string> outBuffer;
    vector<string> inBuffer;
    vector<bool> arrived;
    int currentBufCount;

    vector<cMessage*> frameTimer;
    cMessage* ackTimer, *networkReady;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    string framming (string payload);
    string unframe(string frame);
    bool getUserMsg (string& msg);
    int getCheckSum (string& msg, int sum=0);
    void addNoiseAndSend(MyMessage_Base *msg,int dest);
    void init(int peer);
    void sendMessage(int msgType, int frameNum, bool startTransmission=false);
    inline void inc(int&);
    inline bool inRange(int,int,int);
    void setTimer(cMessage*&, int);
    void resetTimer(cMessage*&);
    bool isTimeout(cMessage*&, cMessage*&);
};

#endif
