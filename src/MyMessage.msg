
packet MyMessage {
    @customize(true);  // see the generated C++ header for more info
   	int seqNum;
   	int msgType;
   	string frame;
    int checksum;
    bool lastMessage;
    bool startTransmission;
}
