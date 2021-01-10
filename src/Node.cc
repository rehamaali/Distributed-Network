
#include "Node.h"


Define_Module(Node);

void Node::initialize()
{
    flag = char(27);      //Esc
    esc = '@';

    string fileName = "input/" + to_string(getIndex());
    ifstream infile(fileName);
    string msg;
    while(getline(infile, msg))
        msgsVec.push_back(msg);
    infile.close();
}

void Node::handleMessage(cMessage *msg)
{
    int peer = msg->getArrivalGate()->getIndex();
    bool startTransmission = 0;
    if(msg->getArrivalGate()->getIndex()==gateSize("ins")-1)    // msg from master inform me to send
    {
        peer = atoi(msg->getName());
        isMeFinished = 0;
        nxtMsgIndex = 0;
        startTransmission = 1;
    }
    else    // normal msg from a node
    {
        MyMessage_Base *recMsg = check_and_cast<MyMessage_Base *>(msg);

        if(recMsg->getStartTransmission())
        {
            isMeFinished = 0;
            nxtMsgIndex = 0;
        }


        //if(normal msg)
        //else if (positive ack msg)
        //else if (negative ack msg)
        // TODO: Handle Node Msg

        if(recMsg->getLastMessage() && isMeFinished)
        {
            cMessage *msgToSent = new cMessage("");
            send(msgToSent, "out", gateSize("ins")-1);
        }
        if(isMeFinished)
        {
            delete msg;
            return;
        }
    }

    // Enter here if I'm not finished so far.
    delete msg;
    string myPayload;
    // TODO: Check if I finished
    isMeFinished = getUserMsg(myPayload);

    string frame = framming(myPayload);
    MyMessage_Base msgToSent = new MyMessage_Base(frame.c_str());
    msgToSent->setChecksum(getCheckSum(frame));
    msgToSent->setLastMessage(isMeFinished);
    msgToSent->setStartTransmission(startTransmission);
    // TODO: Add Here Ack
    // TODO: Add Here NAck
    send(msgToSent,"outs",peer);
}

string Node::framming(string payload)
{
    string frame = "";
    frame +=  flag;
    int payload_size = payload.size();
    for(int i=0;i<payload_size;i++)
    {
        if(payload[i]==flag || payload[i]==esc){
            frame+=esc;
        }
        frame+=payload[i];
    }
    frame += flag;
    return frame;
}

bool Node::getUserMsg (string& msg)
{
    msg = msgsVec[nxtMsgIndex++];
    return nxtMsgIndex==msgsVec.size();
}

int Node::getCheckSum (string& msg, int sum)
{
    int msg_sz = msg.size();

    for(int i=0;i<msg_sz;i++)
    {
        sum+=(int)(msg[i]);
        int carry = sum>>8;     // Get Carry
        sum&=((1<<8)-1);        // Get Sum
        sum+=carry;             // Add Carry to Sum
    }
    sum^= ((1<<8)-1);           // Get 1's complement
    return sum;
}

void Node::addNoiseAndSend(cMessage *msg, int dest)
{
   string msgContent = msg->getName();

   int addNoise=rand()%10;
   if(addNoise>=8)   // add noise
   {
       int noiseType=rand()%4;
       if(noiseType == 0)       // modify one bit
       {
           int charInd = rand() % msgContent.size();
           bitset <8> toBeModified (msgContent[charInd]);
           int bitIndex = rand() % 8;
           toBeModified[bitIndex] = !toBeModified[bitIndex];
           msgContent[charInd] = (char) toBeModified.to_ulong();
           msg->setName(msgContent.c_str());
           send(msg,"outs",dest);
       }
       else if (noiseType == 1)     //delay
        {
           double delay = exponential(1 / par("lambdaNode").doubleValue());
           sendDelayed(msg,delay,"outs",dest);
        }
       else if (noiseType == 2)    // duplicate
        {
           send(msg,"outs",dest);
           send(msg,"outs",dest);
        }
       else                        //loss
       {
           return;
       }
   }
   else  //don't add noise
       send(msg,"outs",dest);
}

bool Node::checkError(string& frame, int checksum)
{
    int check = getCheckSum(frame, checksum);
    // TODO: Check for more
    return check;
}
