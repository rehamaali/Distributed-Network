
#include "Node.h"


Define_Module(Node);

void Node::initialize()
{
    flag = char(27);      //Esc
    esc = '@';
    arrived.resize(BUFCOUNT);
    outBuffer.resize(BUFCOUNT);
    inBuffer.resize(BUFCOUNT);
    frameTimer.resize(BUFCOUNT);

    string fileName = "input/" + to_string(getIndex());
    ifstream infile(fileName);
    string msg;
    while(getline(infile, msg))
        msgsVec.push_back(msg);
    infile.close();
}

void Node::handleMessage(cMessage *msg)
{
    EventType event = None;
    MyMessage_Base* recMsg;
    int timeoutFrameNum;
    if(msg->isSelfMessage())
    {
        // TODO: Don't forget
        int type = atoi(msg->getName());
//        EV << "selfType: " << type << endl;
        if(type == -2)   // ready_to_send
        {
            if(!isMeFinished  && currentBufCount < BUFCOUNT)
            {
                event = readyToSend;
            }
        }
        else if(type == -1)  // ack_timeout
        {
            if(ackTimer.checkTimer(msg->getSendingTime().dbl()))
            {
                ackTimer.resetTimer();
                event = ackTimeout;
            }
        }
        else   // frame_timeout
        {
            if(frameTimer[type].checkTimer(msg->getSendingTime().dbl()))
            {
                frameTimer[type].resetTimer();
                timeoutFrameNum = type;
                event = timeout;
            }
        }
    }
    else if(msg->getArrivalGate()->getIndex()==gateSize("ins")-1)    // msg from master inform me to send
    {
        event = masterInit;
    }
    else
    {
        recMsg = check_and_cast<MyMessage_Base *>(msg);
        string frame = recMsg->getFrame();

        int checkSum=getCheckSum(frame, recMsg->getChecksum());
        if(recMsg->getMsgType() == 0 && checkSum)
        {
            event = checksumErr;
        }
        else
        {
            event = frameArrival;
        }
    }

    bool startTransmission = 0;
    switch(event)
    {
        case None:
            break;
        case masterInit:
        {
            int peer = atoi(msg->getName());
            init(peer);
            isSource = 1;
            startTransmission=1;
        }
        case readyToSend:
        {
            ++currentBufCount;
            sendMessage(0, nextFrameToSend, startTransmission);
            inc(nextFrameToSend);
            break;
        }
        case frameArrival:
        {
            if(recMsg->getStartTransmission())
            {
                int peer = msg->getArrivalGate()->getIndex();
                init(peer);
            }

            if(isMeFinished)
            {
                if(isSource && recMsg->getLastMessage())
                {
                    EV << "to Master" << endl;
                    cMessage *msgToSent = new cMessage("");
                    send(msgToSent, "outs", gateSize("ins")-1);  // send to master
                }
                else
                {
                    sendMessage(1, 0, 0);    // send ack
                }

                delete msg;
                return;
            }

            if(recMsg->getMsgType() == 0)
            {
                if(recMsg->getSeqNum() != frameExpected && noNAK)
                {
                    sendMessage(2, 0, 0);
                }
                else
                {
                    setTimer(ackTimer, -1);
                }

                if(inRange(frameExpected, recMsg->getSeqNum(), tooFar) && !arrived[recMsg->getSeqNum()%BUFCOUNT])
                {
                    arrived[recMsg->getSeqNum()%BUFCOUNT] = 1;
                    inBuffer[recMsg->getSeqNum()%BUFCOUNT] = recMsg->getFrame();
                    while(arrived[frameExpected%BUFCOUNT])
                    {
                        // TODO: Print Here for in-order receiving (in log file)
                        noNAK = 1;
                        arrived[frameExpected%BUFCOUNT] = 0;
                        inc(frameExpected);
                        inc(tooFar);
                        setTimer(ackTimer, -1);
                    }
                }
            }

            if(recMsg->getMsgType()==2 && inRange(ackExpected, (recMsg->getAck()+1) % (MAXSEQNUM+1), nextFrameToSend))
            {
                sendMessage(0, (recMsg->getAck()+1) % (MAXSEQNUM+1), 0);
            }

            while(inRange(ackExpected, recMsg->getAck(), nextFrameToSend))
            {
                --currentBufCount;
                frameTimer[ackExpected%BUFCOUNT].resetTimer();
                inc(ackExpected);
            }

            break;
        }
        case checksumErr:
        {
            if(noNAK)
                sendMessage(2/*nack*/, 0 /*dummy*/, 0);
            break;
        }
        case timeout:
        {
            sendMessage(0/*data*/, timeoutFrameNum, 0);
            break;
        }
        case ackTimeout:
        {

            sendMessage(1, 0, 0);
            break;
        }
    }

    delete msg;

    if(!isMeFinished && currentBufCount < BUFCOUNT)
    {
        cMessage *msg2 = new cMessage(to_string(-2).c_str());
        scheduleAt(simTime() + par("networkReadyDelay").doubleValue(), msg2);
    }


}

inline void Node::inc(int& x)
{
    ++x;
    x%=(MAXSEQNUM+1);
}


inline bool Node::inRange(int l, int x, int r)
{
    return (x>=l && x<r) || (r<l && (x>=l || x<r));
}

void Node::sendMessage(int msgType, int frameNum, bool startTransmission)
{
    string frame="";
    MyMessage_Base *msgToSent;

    if(msgType == 0)
    {
        isMeFinished = getUserMsg(outBuffer[nextFrameToSend%BUFCOUNT]);
        frame = framming(outBuffer[nextFrameToSend%BUFCOUNT]);
        EV << frame << endl;
    }
    msgToSent = new MyMessage_Base();
    msgToSent->setFrame(frame.c_str());
    msgToSent->setChecksum(getCheckSum(frame));
    msgToSent->setMsgType(msgType);
    msgToSent->setAck((frameExpected + MAXSEQNUM)%(MAXSEQNUM+1));
    msgToSent->setLastMessage(isMeFinished);
    msgToSent->setStartTransmission(startTransmission);

    if(msgType == 2)    noNAK=false;
    addNoiseAndSend(msgToSent, currentPeer);
    if(msgType == 0)
    {
        setTimer(frameTimer[frameNum%BUFCOUNT], frameNum%BUFCOUNT);
    }
    ackTimer.resetTimer();
}

void Node::init(int peer)
{
    currentPeer = peer;
    isMeFinished = 0;
    isSource = 0;
    nxtMsgIndex = 0;
    ackExpected = 0;
    nextFrameToSend = 0;
    frameExpected = 0;
    tooFar = BUFCOUNT;
    currentBufCount = 0;
    for(int i=0 ; i<arrived.size() ; ++i)   arrived[i]=0;
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

void Node::addNoiseAndSend(MyMessage_Base *msg, int dest)
{
   string msgContent = msg->getFrame();
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
           msg->setFrame(msgContent.c_str());
           send(msg,"outs",dest);
       }
       else if (noiseType == 1)     //delay
        {
           double delay = exponential(1 / par("lambdaNode").doubleValue());
           EV << "Delay Here" << endl;
           sendDelayed(msg,delay,"outs",dest);
        }
       else if (noiseType == 2)    // duplicate
        {
           EV << "Duplicate" << endl;
           send(msg,"outs",dest);
           MyMessage_Base *msg2 = msg->dup();
           send(msg2,"outs",dest);
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
    return getCheckSum(frame, checksum);
}

void Node::setTimer(Timer& timer, int type)
{
    double time;
    if(type == -1)
    {
        time = par("ackTimeout").doubleValue();
    }
    else
    {
        time = par("frameTimeout").doubleValue();
    }
    timer.setTimer(time);

    cMessage *msg = new cMessage(to_string(type).c_str());
    scheduleAt(simTime() + time, msg);
}

void Timer::setTimer(double time)
{
    timer = time;
}

void Timer::resetTimer()
{
    timer = -1;
}

double Timer::getTimer()
{
    return timer;
}

bool Timer::checkTimer(double time)
{
    return timer == time;
}
