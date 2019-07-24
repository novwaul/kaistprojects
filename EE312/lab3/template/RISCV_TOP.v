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
	output wire HALT,                   // if set, terminate program
	output reg [31:0] NUM_INST,         // number of instruction completed
	output wire [31:0] OUTPUT_PORT      // equal RF_WD this port is used for test
	);

	assign OUTPUT_PORT = RF_WD;

	initial begin
		NUM_INST <= 0;
	end

	// Only allow for NUM_INST
	always @ (negedge CLK) begin
		if (RSTn) NUM_INST <= NUM_INST + 1;
	end

	// TODO: implement

	//control variable
	wire PCSrc; // select next pc
	wire [3:0] InsInfo; // instruction 000~111
	wire [3:0] ALUOp; // ALU operation
	wire [2:0] ImmOp; //IPU operation
	wire [2:0] BitOp; //BOU operation
	wire ALUSrc1, ALUSrc2; // select sources of ALU
	wire RegSrc; // select a source of data to store in register
	wire SelData; // select data

	//datapath variable
	reg [11:0] PCNext;
	wire [11:0] PCAdd;// PC + 4
	wire [31:0] ALUParam1;
	wire [31:0] ALUParam2;
	reg [31:0] ALUData;
	reg [31:0] Imm;
	reg [31:0] preResult;
	reg [31:0] Result;

	//ALU Mux
	assign ALUParam2 = ALUSrc2 ? RF_RD2 : Imm;
	assign ALUParam1 = ALUSrc1 ? I_MEM_ADDR : RF_RD1;

	//ALU
	always@(*) begin
	case(ALUOp)
	4'b0000: ALUData = ALUParam1 + ALUParam2; //ADD
	4'b0001: ALUData = ALUParam1 - ALUParam2; //SUB
	4'b0010: ALUData = ALUParam1 << ALUParam2; //SLL
	4'b0011: ALUData = $signed(ALUParam1) < $signed(ALUParam2); // SLT
	4'b0100: ALUData = (ALUParam1 < ALUParam2); // SLTU
	4'b0101: ALUData = ALUParam1 ^ ALUParam2; //XOR
	4'b0110: ALUData = ALUParam1 >> ALUParam2; //SRL
	4'b0111: ALUData = $signed(ALUParam1) >>> ALUParam2; //SRA
	4'b1000: ALUData = ALUParam1 | ALUParam2; //OR
	4'b1001: ALUData = ALUParam1 & ALUParam2; //AND
	4'b1010: ALUData = ALUParam2; //Push ALUParam2
	default: ALUData = ALUData;
	endcase
	end

	// Immediate produce unit (IPU)
	always@(*) begin
	case(ImmOp)
	3'b000: Imm = I_MEM_DI[31:12];
	3'b001: begin
		Imm = {I_MEM_DI[31],I_MEM_DI[19:12],I_MEM_DI[20],I_MEM_DI[30:21]};
		Imm = Imm << 12;
		Imm = $signed(Imm) >>> 11;
		end
	3'b010: begin
		Imm = I_MEM_DI[31:20];
		Imm = Imm << 20;
		Imm = $signed(Imm) >>> 20;
		end
	3'b011: begin
		Imm = {I_MEM_DI[31],I_MEM_DI[7],I_MEM_DI[30:25],I_MEM_DI[11:8]};
		Imm = Imm << 20;
		Imm = $signed(Imm) >>> 19;
		end
	3'b100: begin
		Imm = {I_MEM_DI[31:25],I_MEM_DI[11:7]};
		Imm = Imm << 20;
		Imm = $signed(Imm) >>> 20;
		end
	3'b101: Imm = I_MEM_DI[24:20] ;
	default: Imm = 32'd0;
	endcase
	end

	// Reg-Mem Mux
	always@(*) begin
	preResult = SelData ? ALUData : D_MEM_DI;
	end

	// Bit operation unit
	always@(*) begin
	case(BitOp)
	3'b000: Result = preResult & 32'hFFFFFFFE;
	3'b001: Result = $signed(preResult << 16) >>> 16;
	3'b010: Result = (preResult << 16) >> 16;
	3'b011: Result = $signed(preResult << 24) >>> 24;
	3'b100: Result = (preResult << 24) >> 24;
	default: Result = preResult;
	endcase
	end

	// program counter
	always@(posedge CLK) begin
	I_MEM_ADDR <= PCNext;
	end

	// PC Adder
	assign PCAdd = I_MEM_ADDR + 12'd4;

	// PC Mux
	always@(*) begin
	PCNext = ~RSTn ? 0 : (PCSrc ? Result & 12'hFFF : PCAdd);
	end

	// Reg Store Mux
	assign RF_WD = (InsInfo == 4'b0100) ? PCSrc : (RegSrc ? PCAdd : Result);

	// Reg Info
	assign RF_RA2 = I_MEM_DI[24:20];
	assign RF_RA1 = I_MEM_DI[19:15];
	assign RF_WA1 = I_MEM_DI[11:7];

	// Data to store MEM
	assign D_MEM_DOUT = RF_RD2;

	// Addr to store data in MEM
	assign D_MEM_ADDR = ALUData & 16'h3FFF;

	// select ALU param
	assign ALUparam2 = ALUSrc2 ? RF_RD2 : Imm;
	assign ALUparam1 = ALUSrc1 ? I_MEM_ADDR : RF_RD1;

	//Control unit
	CRTL control(RSTn, I_MEM_DI, RF_RD1, RF_RD2,
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

endmodule //
