
cplusplus {{
    #include <bitset>
	typedef  std::bitset<8> bits;
}}
class noncobject bits;



packet MyMessage {
    @customize(true);  // see the generated C++ header for more info
   	int Seq_Num;
   	int M_Type;
   	string frame;
    string M_Payload;
    bits check_sum;
}
