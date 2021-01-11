
#include "Node.h"


Define_Module(Node);
/*
 * Noise
 * Checksum
 * Framing (Before, After)
 */

void Node::initialize()
{
    flag = '$';      //Esc
    esc = '@';
    arrived.resize(BUFCOUNT);
    outBuffer.resize(BUFCOUNT);
    inBuffer.resize(BUFCOUNT);
    frameTimer.resize(BUFCOUNT);


    for(int i=0 ; i<arrived.size() ; ++i)   frameTimer[i] = NULL;
    ackTimer = NULL;
    networkReady = NULL;

    string fileName = "input/node" + to_string(getIndex()) + ".txt";
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
        if(type == -2)   // ready_to_send
        {
            resetTimer(networkReady);
            if(!isMeFinished  && currentBufCount < BUFCOUNT)
            {
                event = readyToSend;
            }
            msg = NULL;
        }
        else if(type == -1)  // ack_timeout
        {
            if(isTimeout(ackTimer, msg))
            {
                resetTimer(ackTimer);
                event = ackTimeout;
                msg = NULL;
            }
        }
        else   // frame_timeout
        {
            if(isTimeout(frameTimer[type], msg))
            {
                resetTimer(frameTimer[type]);
                timeoutFrameNum = type;
                event = timeout;
                msg = NULL;
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

        if(recMsg->getStartTransmission())
        {
            int peer = msg->getArrivalGate()->getIndex();
            init(peer);
        }

        int checkSum=getCheckSum(frame, recMsg->getChecksum());

        EV << "Received Frame(" << frame << ") Checksum : " << getCheckSum(frame) << endl;
        if(recMsg->getMsgType() == 0 && checkSum)
        {
            EV << "Checksum detected error at message \"" << frame << "\"" << endl;
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
            isMeFinished = getUserMsg(outBuffer[nextFrameToSend%BUFCOUNT]);
            sendMessage(0, nextFrameToSend, startTransmission);
            inc(nextFrameToSend);
            break;
        }
        case frameArrival:
        {
            string payload = unframe(recMsg->getFrame());
            recMsg->setFrame(payload.c_str());
            int msgType = recMsg->getMsgType();
            EV << "Received messaage with payload \"" << recMsg->getFrame() << "\", ";
            EV << "sequence number \"" << recMsg->getSeqNum() << "\", ";
            EV << "message type \"" << ((msgType==0)?"data":(msgType==1)?"ack":(msgType==2)?"nak":"terminate") << "\", ";
            EV << "at node \"" << getFullName() << "\"" << endl;

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
                        cout << "Inorder: " << getIndex() << " received " << inBuffer[frameExpected%BUFCOUNT]<< endl;
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
                EV << "Retransmitting message payload \"" << outBuffer[((recMsg->getAck()+1) % (MAXSEQNUM+1))%BUFCOUNT] <<"\"" << endl;
            }

            while(inRange(ackExpected, recMsg->getAck(), nextFrameToSend))
            {
                --currentBufCount;
                resetTimer(frameTimer[ackExpected%BUFCOUNT]);
                inc(ackExpected);
            }


            if(isMeFinished)
            {
                if((isSource==1 && recMsg->getLastMessage()) || recMsg->getMsgType() == 3)
                {
                    resetTimer(ackTimer);
                    resetTimer(networkReady);
                    for(int i=0 ; i<frameTimer.size() ; ++i)    resetTimer(frameTimer[i]);
                }

                if(isSource==1 && recMsg->getLastMessage())
                {
                    cout << "to Master" << endl;
                    isSource=2;
                    cMessage *msgToSent = new cMessage("");
                    sendMessage(3, 0, 0);    // send ack
                    send(msgToSent, "outs", gateSize("ins")-1);  // send to master
                }
            }

            break;
        }
        case checksumErr:
        {
            while(inRange(ackExpected, recMsg->getAck(), nextFrameToSend))
            {
                --currentBufCount;
                resetTimer(frameTimer[ackExpected%BUFCOUNT]);
                inc(ackExpected);
            }
            if(noNAK)
                sendMessage(2/*nack*/, 0 /*dummy*/, 0);
            break;
        }
        case timeout:
        {
            sendMessage(0/*data*/, timeoutFrameNum, 0);
            EV << "Time out for frame number \"" << timeoutFrameNum << "\" at node \"" << getFullName() << "\"" << endl;
            EV << "Retransmitting message payload \"" << outBuffer[timeoutFrameNum%BUFCOUNT] <<"\"" << endl;
            break;
        }
        case ackTimeout:
        {
            EV << "Time out for \"ack\" at node \"" << getFullName() << "\"" << endl;
            sendMessage(1, 0, 0);
            break;
        }
    }

    if(msg != NULL)
        delete msg;

    if(!isMeFinished && currentBufCount < BUFCOUNT)
    {
        setTimer(networkReady, -2);
    }
    else
    {
        resetTimer(networkReady);
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
    MyMessage_Base *msgToSent = new MyMessage_Base();

    msgToSent->setMsgType(msgType);

    if(msgType == 0)
    {
        frame = framming(outBuffer[frameNum%BUFCOUNT]);
        msgToSent->setFrame(frame.c_str());
        cout << frame << endl;
        EV << "Frame(" << msgToSent->getFrame() << ") Checksum: " << getCheckSum(frame) << endl;
//        EV << getIndex() << " sent " << frame << endl;
    }
    msgToSent->setSeqNum(frameNum);
    msgToSent->setAck((frameExpected + MAXSEQNUM)%(MAXSEQNUM+1));
    msgToSent->setChecksum(getCheckSum(frame));
    msgToSent->setLastMessage(isMeFinished);
    msgToSent->setStartTransmission(startTransmission);

    if(msgType == 2)    noNAK=false;
    addNoiseAndSend(msgToSent, currentPeer);
    if(msgType == 0)
    {
        setTimer(frameTimer[frameNum%BUFCOUNT], frameNum%BUFCOUNT);
    }
    resetTimer(ackTimer);
}

void Node::init(int peer)
{
    currentPeer = peer;
    nxtMsgIndex = 0;
    isMeFinished = 0;
    isSource = 0;
    noNAK = 1;

    ackExpected = 0;
    nextFrameToSend = 0;
    frameExpected = 0;
    tooFar = BUFCOUNT;

    currentBufCount = 0;
    for(int i=0 ; i<arrived.size() ; ++i)   arrived[i]=0, resetTimer(frameTimer[i]);
    resetTimer(ackTimer);
    resetTimer(networkReady);
}

string Node::framming(string payload)
{
    EV << "Framing: \"" << payload;
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
    EV << "\" -> \"" << frame << "\"" << endl;
    return frame;
}

string Node::unframe(string frame)
{
    string payload = "";
    int frame_size = frame.size();
    bool getcurrent = 0;
    for(int i=1;i<frame_size-1;i++)
    {
        if(frame[i]==esc && !getcurrent)
        {
            getcurrent=1;
        }
        else
        {
            payload+=frame[i];
            getcurrent=0;
        }
    }
    return payload;
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
       if(noiseType == 0 && msgContent.size()>0)       // modify one bit
       {
           EV << "Corruption: " << msgContent;
           int charInd = rand() % msgContent.size();
           bitset <8> toBeModified (msgContent[charInd]);
           int bitIndex = rand() % 8;
           toBeModified[bitIndex] = !toBeModified[bitIndex];
           msgContent[charInd] = (char) toBeModified.to_ulong();
           msg->setFrame(msgContent.c_str());
           EV << " -> " << msgContent << endl;
           send(msg,"outs",dest);
       }
       else if (noiseType == 1)     //delay
        {
           double delay = exponential(1 / par("lambdaNode").doubleValue());
           EV << "Delay: " << msgContent << " with delay (" << delay << ") -> Send at: " << simTime().dbl()+delay << endl;
           sendDelayed(msg,delay,"outs",dest);
        }
       else if (noiseType == 2)    // duplicate
        {
           EV << "Duplicate: " << msgContent << endl;
           send(msg,"outs",dest);
           MyMessage_Base *msg2 = msg->dup();
           send(msg2,"outs",dest);
        }
       else                        //loss
       {
           EV << "Loss: " << msgContent << endl;
           return;
       }
   }
   else  //don't add noise
       send(msg,"outs",dest);
}

void Node::setTimer(cMessage*& timer, int type)
{
    resetTimer(timer);
    timer = new cMessage(to_string(type).c_str());

    double time = simTime().dbl();

    if(type == -2)
    {
        time += par("networkReadyDelay").doubleValue();
    }
    else if(type == -1)
    {
        time += par("ackTimeout").doubleValue();
    }
    else
    {
        time += par("frameTimeout").doubleValue();
    }
    scheduleAt(time, timer);
}

void Node::resetTimer(cMessage*& timer)
{
    if(timer!=NULL)
    {
        cancelAndDelete(timer);
        timer = NULL;
    }
}

bool Node::isTimeout(cMessage*& timer, cMessage*& current)
{
    return timer == current;
}
