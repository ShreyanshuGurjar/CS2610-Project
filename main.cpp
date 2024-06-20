#include <bits/stdc++.h>
#include <fstream>

using namespace std;



int main(){

  	ifstream icache;
 	ifstream dcache;
  	ifstream rf;
  	ofstream output;


  	icache.open("input/ICache.txt");
  	dcache.open("input/DCache.txt");
  	rf.open("input/RF.txt");
  	output.open("output/Output.txt");

  	
  	vector<int> data_cache(256);
  	for(int i=0; i<256; i++){
  		dcache >> hex >> data_cache[i];
  	}

  	vector<int> reg(16);
  	for(int i=0; i<16; i++){
  		rf >> hex >> reg[i];
  	}

  	vector<int> ins_cache(128);
  	for(int i=0; i<128; i++){
  		icache >> hex >> ins_cache[i];
  		ins_cache[i] = ins_cache[i] << 8;
  		int x;
  		icache >> hex >> x;
  		ins_cache[i] += x;
  	}

  	int PC = 0;
  	int opcode, A, B, imm, L1;
  	int LMD;
  	queue<int> ALU_out;


  	int IF=-1, ID=-1, EX=-1, MEM=-1, WB=-1;
  	int control_stall = 0, data_stall = 0;
  	int cycles = 0;
  	vector<int> reg_unavailable(16,0);

  	int arithmetic_ins=0;
  	int logical_ins=0;
  	int shift_ins=0;
  	int memory_ins=0;
  	int li_ins=0;
  	int control_ins=0;
  	int halt_ins=0; 
  	int data_stalls_count = 0;
  	int control_stalls_count = 0;
  	int halt = 0;
  	while(1){

  		if(WB != -1){
  			if(((WB&0xf000)>>12) == 10 || ((WB&0xf000)>>12) == 11){
  				reg_unavailable[(WB&0xf00)>>8]--;
  				reg[(WB&0xf00)>>8] = LMD;
  				// cout << LMD << " lmd\n";
  			}
  			else if(((WB&0xf000)>>12) <= 9){
  				reg_unavailable[(WB&0xf00)>>8]--;
  				reg[(WB&0xf00)>>8] = ALU_out.front()&0xff;
  				ALU_out.pop();
  			}
  			else if(((WB&0xf000)>>12)== 15 ){
  				cycles++;
  				halt_ins++;
  				break;
  			}
  		}

  		if(MEM != -1){
  			if(((MEM&0xf000)>>12) == 10){
  				LMD=ALU_out.front();
  				ALU_out.pop();
  			}
  			else if(((MEM&0xf000)>>12) == 11){
  				LMD = data_cache[ALU_out.front()&0xff];
  				ALU_out.pop();
  			}
  			
  			else if(((MEM&0xf000)>>12) == 12){
  				data_cache[ALU_out.front()] = reg[(MEM&(0xf00))>>8];
  				ALU_out.pop();
  			}
  			else if(((MEM&0xf000)>>12)== 13 || ((MEM&0xf000)>>12)== 14){
  				PC = ALU_out.front()&0xff;
  				ALU_out.pop();
  				control_stall = 0;
  			}
  			
  			WB = MEM;
  		}
  		else{
  			WB = -1;
  		}

  		if(EX != -1){
  			if(opcode == 0){			// ADD
  				
  				ALU_out.push(A+B);
  				arithmetic_ins++;
  			}
  			else if(opcode == 1){		// SUB
  			
  				ALU_out.push(A-B);
  				arithmetic_ins++;
  			}
  			else if(opcode == 2){		// MUL
  			
  				ALU_out.push(A*B);
  				arithmetic_ins++;
  			}
  			else if(opcode == 3){		// INC
  			
  				ALU_out.push(A+1);
  				arithmetic_ins++;
  			}
  			else if(opcode == 4){		// AND
  				
  				ALU_out.push(A&B);
  				logical_ins++;
  			}
  			else if(opcode == 5){		// OR
  				
  				ALU_out.push(A|B);
  				logical_ins++;
  			}
  			else if(opcode == 6){		// XOR
  				
  				ALU_out.push(A^B);
  				logical_ins++;
  			}
  			else if(opcode == 7){		// NOT
  				
  				ALU_out.push(~A);
  				logical_ins++;
  			}
  			else if(opcode == 8){		// SLLI
  			
  				ALU_out.push(A << imm);
  				shift_ins++;
  			}
  			else if(opcode == 9){		// SRLI
  				
  				ALU_out.push(A >> imm);
  				shift_ins++;
  			}
  			else if(opcode == 10){		// LI
  				
  				ALU_out.push(imm);
  				li_ins++;
  			}
  			else if(opcode == 11){		// LD
 
  				ALU_out.push(A + imm);
  				memory_ins++;
  			}
  			else if(opcode == 12){		// ST
  				ALU_out.push(A + imm);
  				memory_ins++;
  			}
  			else if(opcode == 13){		// JMP
  				ALU_out.push(PC + L1);
  				control_ins++;
  			}
  			else if(opcode == 14){		// BEQZ
   				if(A==0){
  					ALU_out.push(PC + L1);
  				}
  				else{
  					ALU_out.push(PC);
  				}
  				control_ins++;
  			}

  			if(opcode <=11) reg_unavailable[(EX&0xf00)>>8]++;
  			MEM = EX;
  		}
  		else{
  			MEM = -1;
  		}

  		if(ID != -1){

  			opcode = (ID&(0xf000))>>12;
  			data_stall=0;

  			if(opcode == 0 || opcode == 1 || opcode == 2 
  				|| opcode == 4 || opcode == 5 || opcode == 6){

  				if(reg_unavailable[(ID&(0xf0))>>4]==0 && reg_unavailable[ID&(0xf)]==0){
  					A = reg[(ID&(0xf0))>>4];
  				  	B = reg[ID&(0xf)];
  				}
  				else{
  					data_stall = 1;
  				}
  			}
  			else if(opcode == 3){
  				if(reg_unavailable[(ID&(0xf00))>>8]==0){

  					A = reg[(ID&(0xf00))>>8];
  				  	
  				}
  				else{
  					data_stall = 1;
  				}
  			}
  			else if(opcode == 7){
  				if(reg_unavailable[(ID&(0xf0))>>4]==0){  					
  					A = reg[(ID&(0xf0))>>4];
  				}
  				else{
  					data_stall = 1;
  				}
  			}
  			else if(opcode == 8 || opcode == 9 || opcode == 11){

  				if(reg_unavailable[(ID&(0xf0))>>4]==0){
  					A = reg[(ID&(0xf0))>>4];
  				  	imm = ID&(0xf);
  				}
  				else{
  					data_stall = 1;
  				}
  			}
  			else if(opcode == 12){

  				if(reg_unavailable[(ID&(0xf0))>>4]==0 && reg_unavailable[(ID&(0xf00))>>8]==0){
  					A = reg[(ID&(0xf0))>>4];
  				  	imm = ID&(0xf);
  				}
  				
  				else{
  					data_stall = 1;
  				}
  			}
  			else if(opcode == 10){
  				imm = ID&(0xff);
  			}
  			else if(opcode == 13){
  				control_stall = 1;  				
  				L1=(0x0ff0&ID)>> 4;  				
  			}

  			else if(opcode == 14){
  				control_stall = 1;
  				if(reg_unavailable[(ID&(0xf00))>>8]==0){
  					A = reg[(ID&(0xf00))>>8];
  				  	L1 = (0x0ff&ID);
  				}
  				else{
  					data_stall=1;
  				}
  			}

  			if(data_stall == 1){
  				EX = -1;
  				data_stalls_count++;
  			}
  			else{
  				EX = ID;
  			}
  			// if((ID&0xf000)>>12 <=11) reg_unavailable[(ID&0xf00)>>8]++;
  		}
  		else EX=-1;


	  	if(data_stall == 0 && !halt){
	  		if(control_stall != 1){	
 		  		IF = ins_cache[PC];
  		  		ID = IF;
  		  		if((IF&0xf000)>>12 == 15){
  		  			halt = 1;
  		  		}
		  		PC++;
	  		}
	  		else{
	  	  		control_stalls_count++;
	  	  		IF = -1;
	  	  		ID = IF;
	  		}
	  	}
	  	else{
	  		IF = -1;
	  	}

	 	cycles++;
	 	// cout << cycles << endl;
	 	// cout << hex << WB << " WB ";
	 	// cout << hex <<  MEM << " MEM ";
	 	// cout << hex <<  EX << " EX ";
	 	// cout << hex <<  ID << " ID ";
	 	// cout << hex << IF << " IF\n";
	 	// for(int i=0;i<16;i++) cout << hex << i << " : " << reg[i] << "\n";


  	}



  	ofstream dcache_out;
  	dcache_out.open("output/DCache.txt");

  	for(int i=0; i<256; i++){
  		dcache_out << hex << ((data_cache[i])>>4) << (data_cache[i]&0x0f) << endl;
  	}


  	int total_ins = arithmetic_ins+logical_ins+shift_ins+memory_ins+control_ins+li_ins+halt_ins;

  	double cycles_per_ins = (double)cycles/(double)total_ins;

  	output << "Total number of instructions executed        : " << total_ins << endl;
  	output << "Number of instructions in each class\n";
  	output << "Arithmetic instructions                      : " << arithmetic_ins << endl;
  	output << "Logical instructions                         : " << logical_ins << endl;
  	output << "Shift instructions                           : " << shift_ins << endl;
  	output << "Memory instructions                          : " << memory_ins << endl;
  	output << "Load immediate instructions                  : " << li_ins << endl;
  	output << "Control instructions                         : " << control_ins << endl;
  	output << "Halt instructions                            : " << halt_ins << endl;
  	output << "Cycles Per Instruction                       : " << cycles_per_ins << endl;
  	output << "Total number of stalls                       : " << (control_stalls_count + data_stalls_count)<< endl;
  	output << "Data stalls (RAW)                            : " << data_stalls_count << endl;
  	output << "Control stalls                               : " << control_stalls_count << endl;



}