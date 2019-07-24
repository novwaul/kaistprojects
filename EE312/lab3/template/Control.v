module CRTL(RSTn, I_MEM_DI, RF_RD1, RF_RD2,
    PCSrc,
		InsInfo,
    ALUOp,
    ImmOp,
    BitOp,
    ALUSrc1,
    ALUSrc2,
    RegSrc,
    SelData,
    I_MEM_CSN,
    D_MEM_CSN,
    D_MEM_WEN,
    D_MEM_BE,
    RF_WE,
    HALT);
//input
input wire RSTn;
input wire [31:0] I_MEM_DI;
input wire [31:0] RF_RD1;
input wire [31:0] RF_RD2;
//output
output reg PCSrc; // select next pc
output reg [3:0] InsInfo; // instruction 000~111
output reg [3:0] ALUOp; // ALU operation
output reg [2:0] ImmOp; //IPU operation
output reg [2:0] BitOp; //BOU operation
output reg ALUSrc1, ALUSrc2; // select sources of ALU
output reg RegSrc; // select a source of data to store in register
output reg SelData; // select data
output reg I_MEM_CSN;
output reg D_MEM_CSN;
output reg D_MEM_WEN;
output reg [3:0] D_MEM_BE;
output reg RF_WE;
output reg HALT;
//interner variables
reg [1:0] preHalt; // see always block of "finalize"
reg [11:0] SignalSet;
reg UE, Lt, Eq;

// Top control
always@(*) begin
casex({RSTn,I_MEM_DI[6:0]})
8'b1_0110111: SignalSet = 12'b0_01_1_0000_0_1x_1;//LUI
8'b1_0010111: SignalSet = 12'b0_01_1_0001_0_01_1;//AUIPC
8'b1_1101111: SignalSet = 12'b0_01_1_0010_1_01_1;//JAL
8'b1_1100111: SignalSet = 12'b0_01_1_0011_1_00_1;//JALR
8'b1_1100011: SignalSet = 12'b0_01_0_0100_0_01_1;//BEQ , BLT , BLTU(rs1 < rs2) , BGE , BGEU(rs1 >= rs2)
8'b1_0000011: SignalSet = 12'b0_01_1_0101_0_00_0;//LB , LH , LW , LBU , LHU
8'b1_0100011: SignalSet = 12'b0_00_0_0110_0_00_1;//SB , SH , SW
8'b1_0010011: SignalSet = 12'b0_01_1_1000_0_00_1;//ADDI , SLTI , SLTIU , XORI , ORI, ANDI , SLLI , SRLI , SRAI
8'b1_0110011: SignalSet = 12'b0_01_1_1001_0_10_1;//ADD , SUB , SLL , SLT , SLTU ,XOR , SRL , SRA , OR , AND
default: SignalSet = 12'b0_01_0_xxxx_x_xx_x;//Do nothing
endcase
end
always@(*) begin
{D_MEM_WEN,RF_WE,InsInfo,RegSrc,ALUSrc2,ALUSrc1,SelData} = SignalSet[9:0];
I_MEM_CSN = ~RSTn ? 0 : SignalSet[11];
D_MEM_CSN = ~RSTn ? 0 : SignalSet[10];
end

// ALU control
always@(*) begin
casex({InsInfo,I_MEM_DI[14:12]})
7'b0000_xxx: ALUOp = 4'b1010; //Push ALUParam2
7'b0001_xxx: ALUOp = 4'b0000; //ADD
7'b001x_xxx: ALUOp = 4'b0000; //ADD
7'b0100_xxx: ALUOp = 4'b0000; //ADD
7'b0101_xxx: ALUOp = 4'b0000; //ADD
7'b0110_xxx: ALUOp = 4'b0000; //ADD
7'b1000_000: ALUOp = 4'b0000; //ADD
7'b1001_000: begin
       if(I_MEM_DI[31:25] == 7'b0100000) ALUOp = 4'b0001;//SUB
       else ALUOp = 4'b0000;//ADD
       end
7'b100x_010: ALUOp = 4'b0011; //SLT
7'b100x_011: ALUOp = 4'b0100; //SLTU
7'b100x_100: ALUOp = 4'b0101; //XOR
7'b100x_110: ALUOp = 4'b1000; //OR
7'b100x_111: ALUOp = 4'b1001; //AND
7'b100x_001: ALUOp = 4'b0010; //SLL
7'b100x_101: begin
       if(I_MEM_DI[31:25] == 7'b0100000) ALUOp = 4'b0111;//SRA
       else ALUOp = 4'b0110;//SRL
       end
default: ALUOp = 4'b1111;
endcase
end

// condition signal generator
always@(*) begin
casex({InsInfo,I_MEM_DI[14:12]})
7'b0100_11x: UE = 1;
default: UE = 0;
endcase
if(UE) Lt = RF_RD1 < RF_RD2;
else Lt = $signed(RF_RD1) < $signed(RF_RD2);
Eq = RF_RD1 == RF_RD2;
end

// Immediate produce unit Control
always@(*) begin
casex({InsInfo,I_MEM_DI[14:12]})
7'b000x_xxx: ImmOp = 3'b000;
7'b0010_xxx: ImmOp = 3'b001;
7'b0011_xxx: ImmOp = 3'b010;
7'b0100_xxx: ImmOp = 3'b011;
7'b0101_xxx: ImmOp = 3'b010;
7'b0110_xxx: ImmOp = 3'b100;
7'b1000_0x0: ImmOp = 3'b010;
7'b1000_011: ImmOp = 3'b010;
7'b1000_100: ImmOp = 3'b010;
7'b1000_11x: ImmOp = 3'b010;
7'b1000_x01: ImmOp = 3'b101;
default: ImmOp = 3'b111; //produce default Imm
endcase
end

// Bit operation unit control
always@(*) begin
casex({InsInfo,I_MEM_DI[14:12]})
7'b0011_xxx: BitOp = 3'b000;
7'b0101_000: BitOp = 3'b011;
7'b0101_001: BitOp = 3'b001;
7'b0101_100: BitOp = 3'b100;
7'b0101_101: BitOp = 3'b010;
default: BitOp = 3'b111; // do nothing
endcase
end

// memory store control
always@(*) begin
case({InsInfo,I_MEM_DI[14:12]})
7'b0110_000: D_MEM_BE = 4'b0001;
7'b0110_001: D_MEM_BE = 4'b0011;
7'b0110_010: D_MEM_BE = 4'b1111;
default: D_MEM_BE = 4'b0000;
endcase
end

// HALT signal generator
always@(*) begin
case(I_MEM_DI)
32'h00c00093: preHalt = 2'd1;
32'h00008067:
begin
if(preHalt[0]) preHalt = 2'd3;
else preHalt = 2'd0;
end
default: preHalt = 2'd0;
endcase
HALT = (preHalt == 2'd3) ? 1 : 0;
end

// Next PC control
always@(*) begin
casex({InsInfo,I_MEM_DI[14:12]})
7'b001x_xxx: PCSrc = 1'b1;
7'b0100_000: PCSrc = Eq;
7'b0100_001: PCSrc = !Eq;
7'b0100_1x0: PCSrc = Lt;
7'b0100_1x1: PCSrc = !Lt;
default: PCSrc = 1'b0;
endcase
end

endmodule
