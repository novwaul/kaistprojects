module RISCV_TOP (
	//General Signals
	input wire CLK,
	input wire RSTn,

	//I-Memory Signals
	output wire I_MEM_CSN,
	input wire [31:0] I_MEM_DI,//input from IM
	output reg [11:0] I_MEM_ADDR,//in byte address

	//D-Memory Signals
	output wire D_MEM_CSN,
	input wire [31:0] D_MEM_DI,
	output wire [31:0] D_MEM_DOUT,
	output wire [11:0] D_MEM_ADDR,//in word address
	output wire D_MEM_WEN,
	output wire [3:0] D_MEM_BE,

	//RegFile Signals
	output wire RF_WE,
	output wire [4:0] RF_RA1,
	output wire [4:0] RF_RA2,
	output wire [4:0] RF_WA1,
	input wire [31:0] RF_RD1,
	input wire [31:0] RF_RD2,
	output wire [31:0] RF_WD,
	output wire HALT,
	output reg [31:0] NUM_INST,
	output wire [31:0] OUTPUT_PORT
	);

	// TODO: implement multi-cycle CPU

	// datapath control signals
	wire [2:0] ImmOpCode;
	wire [3:0] ALUOpCode;
	wire ALUSrc;
	wire CondEnable;
	wire DoBranch;
	wire PCEnable;
	wire [1:0] PCSrc;
	wire [1:0] RegSrc;
	wire CountEnable;
	wire [1:0] SignExtension;
	wire ADDRSrc;

	//datapath args
	reg [31:0] IPU_DOUT;
	reg [31:0] Imm;
	reg [4:0] rs1;
	reg [4:0] rs2;
	reg [4:0] rd;
	reg [31:0] rs1_dout;
	reg [31:0] rs2_dout;
	wire [31:0] ALU_DIN;
	reg [31:0] ALU_DOUT;
	reg ALU_COND;
	reg [24:0] ImmArg;
	reg [11:0] nextPC;
	reg [11:0] curPC;
	wire [11:0] subsequentPC;
	wire [11:0] BJPC;
	wire [11:0] JALRPC;
	reg [31:0] result;

	//HALT args
	reg indicator;
	reg HaltEnable;

	//Control
	Control ControlUnit(CLK, RSTn, I_MEM_DI, ImmOpCode, ALUOpCode, ALUSrc,
		CondEnable, PCEnable, PCSrc, RegSrc, CountEnable, SignExtension, I_MEM_CSN, D_MEM_CSN, D_MEM_WEN, D_MEM_BE, RF_WE, ADDRSrc);

	//instruction counter
	always @ (posedge CLK) begin
	if(~RSTn) NUM_INST = 0;
	else if(CountEnable) NUM_INST = NUM_INST + 1;
	end

	//PC
	always @(posedge CLK) begin
	if(~RSTn) curPC <= 12'h000;
	else if(PCEnable) curPC <= nextPC;
	end

	//PC Forwarding MUX
	always @ (*) begin
	if(ADDRSrc) I_MEM_ADDR = nextPC & 12'hFFF;
	else I_MEM_ADDR = curPC & 12'hFFF;
	end

	//nextPC candidates
	assign subsequentPC = curPC + 4;
	assign BJPC = curPC + Imm[11:0];
	assign JALRPC = 32'hFFFFFFFE & (rs1_dout[11:0] + Imm[11:0]);

	//MUX
	always @ (*) begin
	case({PCSrc[1], PCSrc[0] | DoBranch})
	2'b01: nextPC = BJPC;
	2'b10: nextPC = JALRPC;
	default: nextPC = subsequentPC;
	endcase
	end

	//HALT
	always @ (*) begin
	if(~RSTn) begin
	indicator = 0;
	HaltEnable = 0;
	end
	else if(I_MEM_DI == 32'h00c00093) indicator = 1;
	else if(I_MEM_DI == 32'h00008067 && indicator == 1) HaltEnable = 1;
	else indicator = 0;
	end

	assign HALT = HaltEnable ? 1 : 0;

	//IPU store
	always @(posedge CLK) begin
	if(~RSTn) ImmArg <= 0;
	else ImmArg <= I_MEM_DI[31:7];
	end

	//IPU
	always @(*) begin
		case(ImmOpCode)
		3'b000: begin
						IPU_DOUT = ImmArg[24:13]; //I-type
						IPU_DOUT = $signed( IPU_DOUT << 20 ) >>> 20;
						end
		3'b001: IPU_DOUT = 32'h00000000 | ImmArg[17:13]; //Shift
		3'b010: begin
						IPU_DOUT = {ImmArg[24:18],ImmArg[4:0]}; //S-type
						IPU_DOUT = $signed( IPU_DOUT << 20 ) >>> 20;
						end
		3'b011: begin
						IPU_DOUT = {ImmArg[24],ImmArg[0],ImmArg[23:18],ImmArg[4:1],1'b0}; //B-type
						IPU_DOUT = $signed( IPU_DOUT << 19 ) >>> 19;
						end
		3'b100: begin
						IPU_DOUT = {ImmArg[24],ImmArg[12:5],ImmArg[13],ImmArg[23:14],1'b0}; // J-type
						IPU_DOUT = $signed( IPU_DOUT << 11 ) >>> 11;
						end
		endcase
	end

	//Reg store
	always @(posedge CLK) begin
	if(~RSTn) begin
	rs2 <= 0;
	rs1 <= 0;
	rd <= 0;
	end
	else begin
	rs2 <= I_MEM_DI[24:20];
	rs1 <= I_MEM_DI[19:15];
	rd <= I_MEM_DI[11:7];
	end
	end

	//Reg Inputs
	assign RF_RA2 = rs2;
	assign RF_RA1 = rs1;
	assign RF_WA1 = rd;

	//ALU store
	always @(posedge CLK) begin
	if(~RSTn) begin
	rs1_dout <= 0;
	rs2_dout <= 0;
	Imm <= 0;
	end
	else begin
	rs1_dout <= RF_RD1;
	rs2_dout <= RF_RD2;
	Imm <= IPU_DOUT;
	end
	end

	//MUX
	assign ALU_DIN = ALUSrc ? Imm : rs2_dout;

	//ALU
	always @(*) begin
		if(ALUOpCode < 4'b1010) begin //without branch
			casex(ALUOpCode)
			4'b0000: ALU_DOUT = $signed(rs1_dout) < $signed(ALU_DIN);//SLT
			4'b0001: ALU_DOUT = rs1_dout < ALU_DIN;//SLTU
			4'b0010: ALU_DOUT = rs1_dout ^ ALU_DIN;//XOR
			4'b0011: ALU_DOUT = rs1_dout | ALU_DIN;//OR
			4'b0100: ALU_DOUT = rs1_dout & ALU_DIN;//AND
			4'b0101: ALU_DOUT = rs1_dout << ALU_DIN;//SLL
			4'b0110: ALU_DOUT = rs1_dout >> ALU_DIN;//SRL
			4'b0111: ALU_DOUT = $signed(rs1_dout) >>> ALU_DIN;//SRA
			4'b1000: ALU_DOUT = rs1_dout + ALU_DIN;//ADD
			4'b1001: ALU_DOUT = rs1_dout - ALU_DIN;//SUB
			endcase
		ALU_COND = 0;
		end
		else begin //branch and unknown
			casex(ALUOpCode)
			4'b1010: ALU_COND = rs1_dout == ALU_DIN;//EQUAL
			4'b1011: ALU_COND = rs1_dout != ALU_DIN;//NOT EQUAL
			4'b1100: ALU_COND = $signed(rs1_dout) < $signed(ALU_DIN);//LESS THAN
			4'b1101: ALU_COND = $signed(rs1_dout) >= $signed(ALU_DIN);//GREATER EQUAL
			4'b1110: ALU_COND = rs1_dout < ALU_DIN;//LESS THAN U
			4'b1111: ALU_COND = rs1_dout >= ALU_DIN;//GREATER EQUAL U
			default: ALU_COND = 0; // unknown
			endcase
		ALU_DOUT = 0;
		end
	end

	// select signal for branch
	assign DoBranch = CondEnable & ALU_COND;

	//D_MEM Inputs
	assign D_MEM_ADDR = {ALU_DOUT[15:0] & 16'h3FFF};
	assign D_MEM_DOUT = rs2_dout;

	//MUX
	always @ (*) begin
	case(RegSrc)
	2'b00: result = ALU_DOUT;
	2'b01: result = SignExtension[1] ? ( SignExtension[0] ? $signed(D_MEM_DI << 16) >>> 16 : $signed(D_MEM_DI << 24) >>> 24) : D_MEM_DI;
	2'b10: result = subsequentPC;
	2'b11: result = ALU_COND;
	endcase
	end

	assign RF_WD = result;
	assign OUTPUT_PORT = result;
endmodule //
