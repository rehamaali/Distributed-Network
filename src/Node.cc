
#include "Node.h"


Define_Module(Node);

void Node::initialize()
{
    // TODO - Generated method body
    stuffingByte1 = char(27);      //Esc
    stuffingByte2 = '@';
    inTransmission =false;

}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if(msg->getArrivalGate()->getIndex()==gateSize("ins")-1){   // msg from mater inform me to send

        string m = getUserMsg();
        m+=char(27);
        string l =getUserMsg();
        m+=l;
        string frame= framming(m);
        int ind = atoi(msg->getName());
        delete msg;
        msg = new cMessage(frame.c_str());
        send(msg,"outs",ind);

    }
   // else{  // normal msg from a node
        //if(normal msg)
        //else if (positive ack msg)
        //else  negative ack msg

    //}
}
string Node::framming(string payload)
{
    string frame = "";
    frame +=  stuffingByte1;
    int payload_size = payload.size();
    for(int i=0;i<payload_size;i++)
    {
        if(payload[i]==stuffingByte1||payload[i]==stuffingByte2){
            frame+=stuffingByte2;
        }
        frame+=payload[i];
    }
    frame += stuffingByte1;
    return frame;
}

string Node::getUserMsg ()
{
    string msg;
    cin>>msg;
    return msg;
}

int Node::getCheckSum (string msg)
{
    int msg_sz = msg.size();
    int sum=0;

    for(int i=0;i<msg_sz;i++)
    {
        sum+=(int)(msg[i]);
        int carry = sum>>8;
        sum&=((1<<8)-1);
        sum+=carry;
    }
    sum^= ((1<<8)-1);
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
void Node::startTransmission()
{
    inTransmission = true;
}
void Node::endTransmission()
{
    inTransmission = false;
}
bool Node::isInTransmission()
{
    return inTransmission;
}
