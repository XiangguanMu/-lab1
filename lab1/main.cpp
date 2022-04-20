#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize 65536


class RF //register file
{
public:
    bitset<64> ReadData1, ReadData2;//预定义的数据类型，for eg bool->1byte(8位表示这个状态)，但是bitset可以直接操控一个二进制位
    RF()//构造函数
    {
        Registers.resize(32); //预设定有32个寄存器
        Registers[0] = bitset<64>(0);//把所有的位初始化为0
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<64> WrtData, bitset<1> WrtEnable) //WrtEnable 0 is read else is read and write
    {
        // TODO: implement!
        ReadData1 = Registers[RdReg1.to_ulong()];
        ReadData2 = Registers[RdReg2.to_ulong()];

        if(WrtEnable.to_ulong()==1){
            Registers[WrtReg.to_ulong()] = WrtData;
        }

    }

    void OutputRF() {
        ofstream rfout;
        rfout.open("RFresult.txt", std::ios_base::app);//指定的库，namespace命名空间
        if (rfout.is_open()) {
            rfout << "A state of RF:" << endl; // << 类似于fprintf(fp,"A state of");把内容输出到指定文件里
            for (int j = 0; j < 32; j++) {
                rfout << Registers[j] << endl; //将每个寄存器输出到文件
            }

        } else cout << "Unable to open file";//否则输出打不开文件
        rfout.close(); //关闭流
    }
private:
    vector<bitset<64> > Registers;
};


class ALU
{
public:
    bitset<64> ALUresult;
    bitset<64> ALUOperation(bitset<3> ALUOP, bitset<64> oprand1, bitset<64> oprand2)
    {
        // TODO: implement!
        if (ALUOP.to_string() == "000" || ALUOP.to_string() == "101")  // add/sd/ld
        {
            ALUresult = oprand1.to_ulong() + oprand2.to_ulong();
        }
        else if (ALUOP.to_string() == "001")  // sub
        {
            ALUresult = oprand1.to_ulong() - oprand2.to_ulong();
        }
        else if (ALUOP.to_string() == "010")  // and
        {
            ALUresult = oprand1 & oprand2;
        }
        else if (ALUOP.to_string() == "011")  // or
        {
            ALUresult = oprand1 | oprand2;
        }
        else if (ALUOP.to_string() == "011")  // xor
        {
            ALUresult = oprand1 ^ oprand2;
        }
        else if (ALUOP.to_string() == "110")  // jal
        {
            ;
        }
        return ALUresult;
    }
};


class INSMem
{
public:
    bitset<32> Instruction;
    INSMem()
    {
        IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i = 0;
        imem.open("imem.txt");
        if (imem.is_open())
        {
            while (getline(imem, line))
            {
                IMem[i] = bitset<8>(line.substr(0, 8));
                i++;
            }

        }
        else cout << "Unable to open file";
        imem.close();

    }

    bitset<32> ReadMemory(bitset<32> ReadAddress)
    {
        // TODO: implement!
        // (Read the byte at the ReadAddress and the following three byte).
        unsigned int address = ReadAddress.to_ulong();
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 8; j++){
                Instruction[31-(i*8+j)] = IMem[address+i][j];
            }
        }

        //
        return Instruction;
    }

private:
    vector<bitset<8> > IMem;

};

class DataMem
{
public:
    bitset<64> readdata;
    DataMem()
    {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open("dmem.txt");
        if (dmem.is_open())
        {
            while (getline(dmem, line))
            {
                DMem[i] = bitset<8>(line.substr(0, 8));
                i++;
            }
        }
        else cout << "Unable to open file";
        dmem.close();

    }
    bitset<64> MemoryAccess(bitset<64> Address, bitset<64> WriteData, bitset<1> readmem, bitset<1> writemem)
    {
        // TODO: implement!
        if(readmem[0] == 1){
            //返回address对应的数据
            string s;
            unsigned int address = Address.to_ulong();
            for(int i = 0; i < 8; i++){
                //取八个字节的数据
                s.append(DMem[address+i].to_string());
            }
            bitset<64>data(s);
            readdata = data;
            return readdata;
        }

        if(writemem[0] == 1){//返回？
            //写到对应地址的位置
            unsigned int address = Address.to_ulong();
            for(int i = 0; i < 64; i++){//big-endian
                DMem[address+i/8][i%8] = WriteData[i];
            }
            return WriteData;
        }


    }

    void OutputDataMem()
    {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open())
        {
            for (int j = 0; j < 1000; j++)
            {
                dmemout << DMem[j] << endl;
            }

        }
        else cout << "Unable to open file";
        dmemout.close();

    }

private:
    vector<bitset<8> > DMem;

};


int main() {
    RF myRF;
    ALU myALU;
    INSMem myInsMem;
    DataMem myDataMem;

    // Control Registers
    bitset<32> PC;//program counter
    bitset<1> wrtEnable;
    bitset<1> isJType;
    bitset<1> isIType;
    bitset<1> isLoad;
    bitset<1> isStore;
    bitset<1> isBranch;
    bitset<1> isRType;
    bitset<3> aluOp;

    while (1) {
        // 1. Fetch Instruction
        bitset<32> instruction = myInsMem.ReadMemory(PC);

        // If current insturciton is "11111111111111111111111111111111", then break;
        if (myInsMem.Instruction.to_ulong() == 0xffffffff) {
            break;
        }

        // decode(Read RF)
        // Decoder
        isLoad = instruction.to_string().substr(25, 7) == string("0000011");
        isStore = instruction.to_string().substr(25, 7) == string("0100011");
        isJType = instruction.to_string().substr(25, 7) == string("1101111");
        isRType = instruction.to_string().substr(25, 7) == string("0110011");
        isBranch = instruction.to_string().substr(25, 7) == string("1100011");
        isIType = instruction.to_string().substr(25, 5) == string("00100") ||
                  instruction.to_string().substr(25, 5) == string("11000");
        wrtEnable = !(isStore.to_ulong() || isBranch.to_ulong());
        if (isRType[0] == 1) {//判断ALU的操作
            if (instruction.to_string().substr(17, 3) == string("000")) {
                if (instruction.to_string().substr(0, 7) == string("0000000"))
                    aluOp = bitset<3>("000");  //add
                else if (instruction.to_string().substr(0, 7) == string("0100000"))
                    aluOp = bitset<3>("001");  //sub
            } else if (instruction.to_string().substr(17, 3) == string("111")) {
                aluOp = bitset<3>("010");  //and
            } else if (instruction.to_string().substr(17, 3) == string("110")) {
                aluOp = bitset<3>("011");  //or
            } else if (instruction.to_string().substr(17, 3) == string("100")) {
                aluOp = bitset<3>("100");  //xor
            }
        } else if (isStore[0] == 1 || isLoad[0] == 1) {
            aluOp = bitset<3>("101"); //sw or lw
        } else if (isJType[0] == 1) {
            aluOp = bitset<3>("110"); //
            ///} else if (isBranch[0] == 1){
        } else {
            aluOp = bitset<3>("111");
        }

        // 2. Register File Instruction
        myRF.ReadWrite(
                (isJType[0]) ?
                bitset<5>(string("00000")) : bitset<5>(instruction.to_string().substr(12, 5)),
                (isIType[0] || isJType[0] || isLoad[0]) ?
                bitset<5>(string("00000")) : bitset<5>(instruction.to_string().substr(7, 5)),
                (isIType[0] || isRType[0] || isJType[0] || isLoad[0]) ? bitset<5>(instruction.to_string().substr(20, 5))
                                                                      : bitset<5>(string("00000")),
                bitset<64>(0), wrtEnable);


        // 3. Execuete alu operation
        bitset<64> tmp;
        if (isLoad[0] == 1 || isIType[0] == 1) {
            //imm[11:0]
            tmp = bitset<64>(instruction.to_string().substr(0, 12)); // if positive, 0 padded
            if (tmp[20]) {//这里有问题
                tmp = bitset<64>(string(52, '1') + tmp.to_string().substr(20, 12));
            }
        } else if (isStore[0] == 1) {//这里的temp有什么特殊含义？目前只知道代表了立即数
            //mm[11:5] rs2 rs1 010 imm[4:0]
            tmp = bitset<64>(instruction.to_string().substr(0, 7) +
                             instruction.to_string().substr(20, 5)); // if positive, 0 padded
            if (tmp[20]) {
                tmp = bitset<64>(string(52, '1') + tmp.to_string().substr(20, 12));
            }
            else if (isJType[0] == 1) {
                if (instruction[31])
                    tmp = bitset<64>(string(32, '1') + PC.to_string()); // R[rd] = PC + 4
                else
                    tmp = bitset<64>(string(32, '0') + PC.to_string());
            }
            else if (isBranch[0] == 1) {

            }

            myALU.ALUOperation(aluOp, myRF.ReadData1,
                               (isIType[0] || isJType[0] || isLoad[0] || isStore[0]) ? tmp : myRF.ReadData2);//

            // 4. Read/Write Mem(Memory Access)
            myDataMem.MemoryAccess(myALU.ALUresult, myRF.ReadData2, isLoad, isStore);

            // 5. Register File Update(Write Back)
            myRF.ReadWrite(
                    (isJType[0]) ? bitset<5>(string("00000")) : bitset<5>(instruction.to_string().substr(12, 5)),
                    (isJType[0] || isIType[0] || isLoad[0]) ? bitset<5>(string("00000")) : bitset<5>(
                            instruction.to_string().substr(7, 5)),
                    (isIType[0] || isRType[0] || isJType[0] || isLoad[0]) ? bitset<5>(
                            instruction.to_string().substr(20, 5)) : bitset<5>(string("00000")),
                    isLoad[0] ? myDataMem.readdata : myALU.ALUresult, wrtEnable);

            // Update PC
            if (isBranch[0] && myRF.ReadData1 == myRF.ReadData2) {
                bitset<32> addressExtend;
                // if (instruction[15] == true) {
                //     addressExtend = bitset<32>(string(14, '1') + instruction.to_string().substr(16, 16) + string("00"));
                // }
                // else {
                //     addressExtend = bitset<32>(string(14, '0') + instruction.to_string().substr(16, 16) + string("00"));
                // }

                //imm[12|10:5] rs2 rs1 000 imm(20)[4:1|11]
                if (instruction[0] == true)
                    addressExtend = bitset<32>(string(19, '1') + instruction.to_string().substr(0, 1) +
                                               instruction.to_string().substr(24, 1) +
                                               instruction.to_string().substr(2, 6) \
 + instruction.to_string().substr(20, 4) + string("0"));
                else
                    addressExtend = bitset<32>(string(19, '0') + instruction.to_string().substr(0, 1) +
                                               instruction.to_string().substr(24, 1) +
                                               instruction.to_string().substr(2, 6) \
 + instruction.to_string().substr(20, 4) + string("0"));

                //addressExtend = bitset<32>(tmp.to_string().substr(2, 30) + string("00"));
                PC = bitset<32>(PC.to_ulong() + addressExtend.to_ulong());
            } else if (isJType[0]) {
                bitset<32> addressExtend;
                //imm[20|10:1|11|19:12]
                if (instruction[0] == true)
                    addressExtend = bitset<32>(string(11, '1') + instruction.to_string().substr(0, 1) +
                                               instruction.to_string().substr(12, 8) +
                                               instruction.to_string().substr(11, 1) \
 + instruction.to_string().substr(1, 10) + string("0"));
                else
                    addressExtend = bitset<32>(string(11, '0') + instruction.to_string().substr(0, 1) +
                                               instruction.to_string().substr(12, 8) +
                                               instruction.to_string().substr(11, 1) \
 + instruction.to_string().substr(1, 10) + string("0"));
                PC = bitset<32>(PC.to_ulong() + addressExtend.to_ulong());
            } else {
                PC = bitset<32>(PC.to_ulong() + 4);
            }

            myRF.OutputRF(); // dump RF;
        }
        myDataMem.OutputDataMem(); // dump data mem

        return 0;
    }
}
