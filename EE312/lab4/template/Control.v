module Control(
  input wire CLK,
  input wire RSTn,
  input wire [31:0] I_MEM_DI,
  output reg [2:0] ImmOpCode,
  output reg [3:0] ALUOpCode,
  output reg ALUSrc,
  output reg CondEnable,
  output reg PCEnable,
  output reg [1:0] PCSrc,
  output reg [1:0] RegSrc,
  output reg CountEnable,
  output reg [1:0] SignExtension,
  output reg ICSN,
  output reg DCSN,
  output reg DWEN,
  output reg [3:0] DBE,
  output reg RFWE,
  output reg ADDRSrc
  );
//args
reg [9:0] uPC;// meaning of numbers :: 0: IF, 1: ID, 2: EX, 3: MEM, 5: WB // stage, instruction, funct, mutation
reg [9:0] nextuPC;
reg [2:0] subsequentuPC;
reg [2:0] TempuPC;
reg [6:0] Opcode;
reg [2:0] funct3;
reg mutation;
reg jump;
reg [26:0] signal;// ImmOpCode,ALUOpCode,ALUSrc,CondEnable,PCEnable,PCSrc,RegSrc,CountEnable,SignExtension, ICSN, DCSN, DWEN, DBE, RFWE, ADDRSrc, jump
reg [2:0] inst;
reg [3:0] funct4;
reg [2:0] stage;

//microcode
always @(*) begin
stage = uPC[9:7];
inst = uPC[6:4];
funct4 = uPC[3:0];
if(uPC == 10'b111_111_111_1) signal = 27'bxxx_xxxx_x_0_0_00_xx_0_xx_0_1_x_xxxx_0_0_0; //initial
else if(inst == 3'b000) begin//ADD..
  case(stage)
  3'b000: signal = 27'bxxx_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0; //IF
  3'b001: signal = 27'bxxx_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0; //ID
  3'b010: begin // EX
          casex(funct4)
          4'b0000: signal = 27'bxxx_1000_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//ADD
          4'b0001: signal = 27'bxxx_1001_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SUB
          4'b001x: signal = 27'bxxx_0101_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SLL
          4'b010x: signal = 27'bxxx_0000_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SLT
          4'b011x: signal = 27'bxxx_0001_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SLTU
          4'b100x: signal = 27'bxxx_0010_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//XOR
          4'b1010: signal = 27'bxxx_0110_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SRL
          4'b1011: signal = 27'bxxx_0111_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SRA
          4'b110x: signal = 27'bxxx_0011_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//OR
          4'b111x: signal = 27'bxxx_0100_0_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//AND
          endcase
          end
  3'b100: signal = 27'bxxx_xxxx_x_0_1_00_xx_0_xx_0_1_x_xxxx_0_1_0; //WB
  endcase
end
else if(inst == 3'b001) begin//ADDI..
  case(stage)
  3'b000: signal = 27'bxxx_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;//IF
  3'b001: begin //ID
          casex(funct4)
          4'b11xx: signal = 27'b001_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;
          default: signal = 27'b000_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;
          endcase
          end
  3'b010: begin//EX
          casex(funct4)
          4'b000x: signal = 27'bxxx_1000_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//ADDI
          4'b001x: signal = 27'bxxx_0000_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SLTI
          4'b010x: signal = 27'bxxx_0001_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SLTIU
          4'b011x: signal = 27'bxxx_0010_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//XORI
          4'b100x: signal = 27'bxxx_0011_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//ORI
          4'b101x: signal = 27'bxxx_0100_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//ANDI
          4'b110x: signal = 27'bxxx_0101_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SLLI
          4'b1110: signal = 27'bxxx_0110_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SRLI
          4'b1111: signal = 27'bxxx_0111_1_x_0_xx_00_1_xx_1_1_x_xxxx_1_x_1;//SRAI
          endcase
          end
  3'b100: signal = 27'bxxx_xxxx_x_0_1_00_xx_0_xx_0_1_x_xxxx_0_1_0; //WB
  endcase
end
else if(inst == 3'b010) begin//SW..
  case(stage)
  3'b000: signal = 27'bxxx_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;//IF
  3'b001: signal = 27'b010_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;//ID
  3'b010: begin //EX
          case(funct4)
          4'b0000: signal = 27'bxxx_1000_1_x_0_xx_00_1_xx_1_0_0_0001_0_x_0;//SB
          4'b0010: signal = 27'bxxx_1000_1_x_0_xx_00_1_xx_1_0_0_0011_0_x_0;//SH
          4'b0100: signal = 27'bxxx_1000_1_x_0_xx_00_1_xx_1_0_0_1111_0_x_0;//SW
          endcase
          end
  3'b011: signal = 27'bxxx_xxxx_x_0_1_00_xx_0_xx_0_1_x_xxxx_0_1_1;//MEM
  endcase
end
else if(inst == 3'b011) begin//LW..
  case(stage)
  3'b000: signal = 27'bxxx_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;//IF
  3'b001: signal = 27'b000_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;//ID
  3'b010: signal = 27'bxxx_1000_1_x_0_xx_xx_0_xx_1_0_1_xxxx_0_x_0;//EX
  3'b011: begin//MEM
          case(funct4)
          4'b0000: signal = 27'bxxx_xxxx_x_x_0_xx_01_1_10_1_1_x_xxxx_1_x_0;//LB
          4'b0010: signal = 27'bxxx_xxxx_x_x_0_xx_01_1_11_1_1_x_xxxx_1_x_0;//LH
          default: signal = 27'bxxx_xxxx_x_x_0_xx_01_1_00_1_1_x_xxxx_1_x_0;//LW & LBU & LHU
          endcase
          end
  3'b100: signal = 27'bxxx_xxxx_x_0_1_00_xx_0_xx_0_1_x_xxxx_0_1_0;//WB
  endcase
end
else if(inst == 3'b100) begin//Branch
  case(stage)
  3'b000: signal = 27'bxxx_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;//IF
  3'b001: signal = 27'b011_xxxx_x_x_0_xx_xx_1_xx_1_1_x_xxxx_0_x_0;//ID
  3'b010: begin //EX
          case(funct4)
          4'b0000: signal = 27'bxxx_1010_0_1_1_00_11_0_xx_0_1_x_xxxx_0_1_1; //BEQ
          4'b0010: signal = 27'bxxx_1011_0_1_1_00_11_0_xx_0_1_x_xxxx_0_1_1; //BNEQ
          4'b0100: signal = 27'bxxx_1100_0_1_1_00_11_0_xx_0_1_x_xxxx_0_1_1; //BLT
          4'b0110: signal = 27'bxxx_1101_0_1_1_00_11_0_xx_0_1_x_xxxx_0_1_1; //BGE
          4'b1000: signal = 27'bxxx_1110_0_1_1_00_11_0_xx_0_1_x_xxxx_0_1_1; //BLTU
          4'b1010: signal = 27'bxxx_1111_0_1_1_00_11_0_xx_0_1_x_xxxx_0_1_1; //BGEU
          endcase
          end
  endcase
end
else if(inst == 3'b101) begin//JALR
  case(stage)
  3'b000: signal = 27'bxxx_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0;//IF
  3'b001: signal = 27'b000_xxxx_x_x_0_xx_xx_1_xx_1_1_x_xxxx_0_x_1;//ID
  3'b100: signal = 27'bxxx_xxxx_x_0_1_10_10_0_xx_0_1_x_xxxx_1_1_0;//WB
  endcase
end
else if(inst == 3'b110) begin//JAL
  case(stage)
  3'b000: signal = 27'bxxx_xxxx_x_x_0_xx_xx_0_xx_1_1_x_xxxx_0_x_0; //IF
  3'b001: signal = 27'b100_xxxx_x_x_0_xx_xx_1_xx_1_1_x_xxxx_0_x_1; //ID
  3'b100: signal = 27'bxxx_xxxx_x_0_1_01_10_0_xx_0_1_x_xxxx_1_1_0; //WB
  endcase
end
{ImmOpCode,ALUOpCode,ALUSrc,CondEnable,PCEnable,PCSrc,RegSrc,CountEnable,SignExtension,ICSN,DCSN,DWEN,DBE,RFWE,ADDRSrc,jump} = signal;
end

//SelectUnit
always @(*) begin
Opcode = I_MEM_DI[6:0];
funct3 = I_MEM_DI[14:12];
mutation = I_MEM_DI[30];
if(Opcode == 7'b1101111) begin //JAL
TempuPC = jump ? 3'b100 : subsequentuPC;
nextuPC = {TempuPC,7'b1100000};
end
else if(Opcode == 7'b1100111) begin//JALR
TempuPC = jump ? 3'b100 : subsequentuPC;
nextuPC = {TempuPC,7'b1010000};
end
else if(Opcode == 7'b1100011) begin //Branch
  TempuPC = jump ? 3'b000 : subsequentuPC;
  case(funct3)
  3'b000: nextuPC = {TempuPC,7'b1000000};//beq
  3'b001: nextuPC = {TempuPC,7'b1000010};//bneq
  3'b100: nextuPC = {TempuPC,7'b1000100};//blt
  3'b101: nextuPC = {TempuPC,7'b1000110};//bge
  3'b110: nextuPC = {TempuPC,7'b1001000};//bltu
  3'b111: nextuPC = {TempuPC,7'b1001010};//bgeu
  endcase
end
else if(Opcode == 7'b0000011) begin // Load
  case(funct3)
  3'b000: nextuPC = {subsequentuPC,7'b0110000};//lb
  3'b001: nextuPC = {subsequentuPC,7'b0110010};//lh
  3'b010: nextuPC = {subsequentuPC,7'b0110100};//lw
  3'b100: nextuPC = {subsequentuPC,7'b0110110};//lbu
  3'b101: nextuPC = {subsequentuPC,7'b0111000};//lhu
  endcase
end
else if(Opcode == 7'b0100011) begin // store
  TempuPC = jump ? 3'b000 : subsequentuPC;
  case(funct3)
  3'b000: nextuPC = {TempuPC,7'b0100000};//sb
  3'b001: nextuPC = {TempuPC,7'b0100010};//sh
  3'b010: nextuPC = {TempuPC,7'b0100100};//sw
  endcase
end
else if(Opcode == 7'b0010011) begin //Immediate
  TempuPC = jump ? 3'b100 : subsequentuPC;
  case(funct3)
  3'b000: nextuPC = {TempuPC,7'b0010000};//addi
  3'b010: nextuPC = {TempuPC,7'b0010010};//slti
  3'b011: nextuPC = {TempuPC,7'b0010100};//sltiu
  3'b100: nextuPC = {TempuPC,7'b0010110};//xori
  3'b110: nextuPC = {TempuPC,7'b0011000};//ori
  3'b111: nextuPC = {TempuPC,7'b0011010};//andi
  3'b001: nextuPC = {TempuPC,7'b0011100};//slli
  3'b101: nextuPC = {TempuPC,6'b001111,mutation};//srli & srai
  endcase
end
else if(Opcode == 7'b0110011) begin //arithmetic
  TempuPC = jump ? 3'b100 : subsequentuPC;
  case(funct3)
  3'b000: nextuPC = {TempuPC,6'b000000,mutation};//add & sub
  3'b001: nextuPC = {TempuPC,7'b0000010};//sll
  3'b010: nextuPC = {TempuPC,7'b0000100};//slt
  3'b011: nextuPC = {TempuPC,7'b0000110};//sltu
  3'b100: nextuPC = {TempuPC,7'b0001000};//xor
  3'b101: nextuPC = {TempuPC,6'b000101,mutation};//srl & sra
  3'b110: nextuPC = {TempuPC,7'b0001100};//or
  3'b111: nextuPC = {TempuPC,7'b0001110};//and
  endcase
end
end

//micro-PC
always @(posedge CLK) begin
if(~RSTn) uPC = 10'b111_111_111_1;
else uPC = nextuPC;
end

//ADDER
always @ (*) begin
if(uPC[9:7] == 3'b100) subsequentuPC = 3'd0;
else subsequentuPC = uPC[9:7] + 3'd1;
end


endmodule
