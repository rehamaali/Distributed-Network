
packet MyMessage {
    @customize(true);  // see the generated C++ header for more info
   	int seqNum;
   	int msgType;	// 0->data, 1->ack, 2->nack
   	string frame;
    int checksum;
    bool lastMessage;
    bool startTransmission;
    int ack;
}
