
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

string Node::addNoise(string msg)
{

}
void Node::startTransmission()
{
    inTransmission = true;
}
void Node::endTransmission()
{
    inTransmission = false;
}
void Node::isInTransmission()
{
    return inTransmission;
}
