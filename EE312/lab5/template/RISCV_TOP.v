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


	// TODO: implement
	reg StopCounting;
	reg [2:0] Synchronizer;
	/* IF Stage */
	reg [11:0] PC;
	reg [11:0] NextPC;
	reg [11:0] preNextPC;
	reg [11:0] FastJumpAddr;
	reg preNextPCCTRL;
	reg [11:0] TargetAddr;
	wire [11:0] BranchAddr;
	wire TargetAddrCTRL;
	reg [31:0] Inst;
	/* IF/ID STORE */
	reg ID_StopCounting;
	reg [11:0] ID_PC;
	reg [31:0] ID_Inst;
	reg ID_SelectBit_Top, ID_SelectBit_Bottom;
	/* ID Stage */
	reg [31:0] Imm;
	reg [11:0] store_target;
	wire DataStoreSelect;
	wire HazardDetected;
	wire isBranch;
	wire [1:0] isJump; // 2'b00: not Jump, 2'b01: JAL, 2'b10: JALR
	wire [1:0] Operation; // 2'b00: Normal, 2'b01: Imm, 2'b10: Load, 2'b11: Store
	wire [3:0] ALUopcode;
	wire MEM_WEN;
	wire [3:0] MEM_BE;
	wire [2:0] MEM_OUT_PROCESS;
	wire REG_WE;
	wire ID_HALT;
	reg [4:0] RegInput1;
	reg [4:0] RegInput2;
	/* ID/EX STORE */
	reg EX_SelectBit_Top, EX_SelectBit_Bottom;
	reg [11:0] EX_store_target;
	reg [2:0] EX_branch_opcode;
	reg EX_StopCounting;
	reg [1:0] EX_isJump;
	reg [3:0] EX_ALUopcode;
	reg EX_MEM_WEN;
	reg [3:0] EX_MEM_BE;
	reg EX_DataStoreSelect;
	reg [2:0] EX_MEM_OUT_PROCESS;
	reg EX_REG_WE;
	reg [4:0] EX_REG_WA;
	reg [4:0] EX_REG_RA1;
	reg [4:0] EX_REG_RA2;
	reg [31:0] EX_REG_RD1;
	reg [31:0] EX_REG_RD2;
	reg [31:0] EX_Imm;
	reg [11:0] EX_PC;
	reg [1:0] EX_Operation;
	reg EX_isBranch;
	reg EX_HALT;
	reg [31:0] MEM_STORE_DATA;
	/* EX Stage */
	reg [11:0] restoreAddr;
	reg BranchOn;
	reg [31:0] ALUarg1;
	reg [31:0] ALUarg2;
	reg [31:0] ALUout;
	wire ALUarg1Select;
	wire [1:0] ALUarg2Select;
	wire [1:0] RegVal1Select;
	wire [1:0] RegVal2Select;
	reg [31:0] RegVal1;
	reg [31:0] RegVal2;
	/* EX/MEM STORE */
	reg MEM_StopCounting;
	reg M_DataStoreSelect;
	reg M_REG_WE;
	reg [31:0] M_ALUout;
	reg M_MEM_WEN;
	reg [3:0] M_MEM_BE;
	reg [2:0] M_MEM_OUT_PROCESS;
	reg [4:0] M_REG_WA;
	reg [31:0] M_StoreData;
	reg [1:0] M_Operation;
	reg [1:0] M_isJump;
	reg M_isBranch;
	reg M_BranchOn;
	reg M_HALT;
	/* MEM Stage */
	reg [31:0] MEMout;
	reg [31:0] LoadData;
	/* MEM/WB STORE */
	reg WB_StopCounting;
	reg WB_REG_WE;
	reg [4:0] WB_REG_WA;
	reg [31:0] WB_LoadData;
	reg WB_isBranch;
	reg [1:0] WB_isJump;
	reg [1:0] WB_Operation;
	reg WB_BranchOn;
	reg [11:0] WB_StoreAddr;
	reg WB_HALT;
	/* WB Stage */
	reg [31:0] temp_RF_WD;


	assign OUTPUT_PORT = RF_WD;


	initial begin
	NUM_INST <= 0;
	end

	// Only allow for NUM_INST
	always @ (negedge CLK) begin
	if (RSTn && ~WB_StopCounting) NUM_INST <= NUM_INST + 1;
	end

	/*-------------------------------------------------------------------------*/
	/* IF Stage */
	initial begin
		Synchronizer <= 3'd0; //synchronize with RSTn. Limit updating PC at some point.
		WB_StopCounting <= 1'd0; //stop counting instruction number it stall or hazard occurs
	end

	//Synchronizer Eval
	always@(posedge CLK) begin
		if(~RSTn && Synchronizer != 3'd6) Synchronizer = Synchronizer + 3'd1;
		else if(RSTn) Synchronizer = 3'd0;
	end

	//Program Counter
	always@(posedge CLK) begin
		if(~RSTn && Synchronizer == 3'd1) PC <= 12'd0;
		else if(Synchronizer != 3'd6) PC <= NextPC & 12'hFFF;
	end

	always @(*) begin
		I_MEM_ADDR = PC;
		Inst = I_MEM_DI;
	end

	//Fast Jump
	always @(*) begin
		if(I_MEM_DI[6:0] == 7'b1101111) begin//JAL Instruction
			preNextPCCTRL = 1'b1;
			FastJumpAddr = PC + {I_MEM_DI[20],I_MEM_DI[30:21],1'b0};
		end
		else preNextPCCTRL = 1'b0;
	end

	//NextPC select MUX
	always @(*) begin
		if(HazardDetected) begin
			if(EX_isBranch && EX_SelectBit_Bottom && ~EX_SelectBit_Top) NextPC = EX_PC + 12'd4;
			else NextPC = restoreAddr;
		end
		else NextPC = preNextPC;
	end

	//preNextPC select MUX
	always@(*) begin
		if(preNextPCCTRL) preNextPC = FastJumpAddr;
		else preNextPC = TargetAddr;
	end

	//TargetAddr select Mux
	always@(*) begin
		if(TargetAddrCTRL) TargetAddr = BranchAddr;
		else TargetAddr = PC + 12'd4;
	end

	//NUM_INST controller
	always @(*) begin
		if(HazardDetected) StopCounting = 1'd1;
		else StopCounting = 1'd0;
	end
	//Branch predictor
	BRANCH_PREDICTOR branch_predictor(
																		CLK,
																		RSTn,
																		PC,
																		ID_PC,
																		EX_PC,
																		store_target,
																		BranchOn,
																		isBranch,
																		EX_isBranch,
																		BranchAddr,
																		TargetAddrCTRL
																		);


/*---------------------------------------------------------------------------*/

	//IF&ID STORE
	always @(posedge CLK) begin
		ID_StopCounting <= StopCounting;
		if(Synchronizer != 3'd6) begin
			if(~HazardDetected) begin
				ID_PC <= PC;
				ID_Inst <= Inst;
				ID_SelectBit_Top <= preNextPCCTRL;
				ID_SelectBit_Bottom <= TargetAddrCTRL;
			end
			else begin // flush
				ID_PC <= 12'd0;
				ID_Inst <= 32'b110011;
				ID_SelectBit_Top <= 1'd0;
				ID_SelectBit_Bottom <= 1'd0;
			end
		end
	end
	/*-------------------------------------------------------------------------*/
	/* ID Stage */

	//Register inputs
	always @(ID_Inst) begin
		RegInput1 = ID_Inst[19:15];
		RegInput2 = ID_Inst[24:20];
	end

	assign RF_RA1 = RegInput1;
	assign RF_RA2 = RegInput2;

	//Immediate Process Unit
	always @(*) begin
		case(ID_Inst[6:0])
		7'b1100111: Imm = ($signed(ID_Inst) >>> 20);//JALR
		7'b1100011: begin //Branch
								Imm = {ID_Inst[31],ID_Inst[7],ID_Inst[30:25],ID_Inst[11:8],1'd0};
								Imm = ($signed(Imm << 19) >>> 19);
								end
		7'b0000011: Imm = ($signed(ID_Inst) >>> 20);//Load
		7'b0100011: begin //Store
								Imm = {ID_Inst[31:25],ID_Inst[11:7]};
								Imm = ($signed(Imm << 20) >>> 20);
								end
		7'b0010011: Imm = ($signed(ID_Inst) >>> 20); //Immediate operation
		default: Imm = 32'd0;
		endcase
	end

	//Branch Address Generator
	always @(*) begin
		store_target = (ID_PC + Imm);
	end

	//INST_NUM controller
	always @(*) begin
		if(HazardDetected) ID_StopCounting = 1'd1;
	end

	wire [4:0] REG_WA = ID_Inst[11:7];
	wire [2:0] branch_opcode = ID_Inst[14:12];


	//Control unit
	CONTROL control(
									CLK,
									RSTn,
									Inst,
									ID_Inst,
									EX_SelectBit_Top,
									EX_SelectBit_Bottom,
									BranchOn,
									EX_isJump,
									EX_isBranch,
									EX_Operation,
									I_MEM_CSN,
									D_MEM_CSN,
									ID_HALT,
									DataStoreSelect,
									HazardDetected,
									isBranch,
									isJump,
									Operation,
								 	ALUopcode,
									MEM_WEN,
									MEM_BE,
									MEM_OUT_PROCESS,
									REG_WE
									);
/*--------------------------------------------------------------------------*/

	// ID&EX STORE
	always @(posedge CLK) begin
		EX_StopCounting <= ID_StopCounting;
		if(Synchronizer != 3'd6) begin
			if(~HazardDetected) begin
				EX_HALT <= ID_HALT;
				EX_ALUopcode <= ALUopcode;
				EX_MEM_WEN <= MEM_WEN;
				EX_MEM_BE <= MEM_BE;
				EX_DataStoreSelect <= DataStoreSelect;
				EX_MEM_OUT_PROCESS <= MEM_OUT_PROCESS;
				EX_REG_WE <= REG_WE;
				EX_REG_WA <= REG_WA;
				EX_REG_RA1 <= RF_RA1;
				EX_REG_RA2 <= RF_RA2;
				EX_REG_RD1 <= RF_RD1;
				EX_REG_RD2 <= RF_RD2;
				EX_Imm <= Imm;
				EX_Operation <= Operation;
				EX_isBranch <= isBranch;
				EX_isJump <= isJump;
				EX_branch_opcode <= branch_opcode;
				EX_store_target <= store_target;
				EX_SelectBit_Top <= ID_SelectBit_Top;
				EX_SelectBit_Bottom <= ID_SelectBit_Bottom;
				EX_PC <= ID_PC;
			end
			else begin //flush
				EX_HALT <= 1'd0;
				EX_isJump <= 2'd0;
				EX_ALUopcode <= 4'd0;
				EX_MEM_WEN <= 1'd1;
				EX_MEM_BE <= 4'd0;
				EX_DataStoreSelect <= 1'd1;
				EX_MEM_OUT_PROCESS <= 3'd0;
				EX_REG_WE <= 1'd0;
				EX_REG_WA <= 5'd0;
				EX_REG_RA1 <= 5'd0;
				EX_REG_RA2 <= 5'd0;
				EX_REG_RD1 <= 32'd0;
				EX_REG_RD2 <= 32'd0;
				EX_Imm <= 32'd0;
				EX_Operation <= 2'd0;
				EX_isBranch <= 1'd0;
				EX_branch_opcode <= 3'd0;
				EX_store_target <= 12'd0;
				EX_SelectBit_Top <= 1'd0;
				EX_SelectBit_Bottom <= 1'd0;
				EX_PC <= 12'd0;
			end
		end
	end

	/*-------------------------------------------------------------------------*/
	/* EX stage */


	//forwarding Reg1
	always @(*) begin
		if(RegVal1Select == 2'd1) RegVal1 = M_ALUout;
		else if(RegVal1Select == 2'd2) RegVal1 = MEMout;
		else if(RegVal1Select == 2'd3) RegVal1 = RF_WD;
		else RegVal1 = EX_REG_RD1;
	end

	//ALUarg1 Mux
	always @(*) begin
		if(ALUarg1Select == 1'd1) ALUarg1 = 32'd4;
		else ALUarg1 = RegVal1;
	end

	//forwarding Reg2
	always @(*) begin
		if(RegVal2Select == 2'd1) RegVal2 = M_ALUout;
		else if(RegVal2Select == 2'd2) RegVal2 = MEMout;
		else if(RegVal2Select == 2'd3) RegVal2 = RF_WD;
		else RegVal2 = EX_REG_RD2;
	end

	//ALUarg2 Mux
	always @(*) begin
		if(ALUarg2Select == 2'd1) ALUarg2 = EX_PC;
		else if(ALUarg2Select == 2'd2) ALUarg2 = EX_Imm;
		else ALUarg2 = RegVal2;
	end

	//Compare Unit
	always @(*) begin
		if(EX_isBranch) begin
			if(~EX_branch_opcode[2]) BranchOn = EX_branch_opcode[0] ^ (RegVal1 == RegVal2); //equal?
			else if(~EX_branch_opcode[1]) BranchOn = EX_branch_opcode[0] ^ ($signed(RegVal1) < $signed(RegVal2)); //less?
			else BranchOn = EX_branch_opcode[0] ^ (RegVal1 < RegVal2); //unsigned less?
		end
		else BranchOn = 1'd0;
	end

	//FAST Restore
	always @(*) begin
		if(EX_isJump[1]) begin // JALR case
			restoreAddr = ((RegVal1 + EX_Imm) & 32'hfffffffe);
		end
		else if(EX_isBranch) begin // branch case
			restoreAddr = EX_store_target;
		end
	end

	//ALU
	always @(*) begin
		case(EX_ALUopcode)
			4'b0000: ALUout = ALUarg1 + ALUarg2; //ADD
			4'b0001: ALUout = $signed(ALUarg1) < $signed(ALUarg2); //SetOnLessThan(SLT)
			4'b0010: ALUout = ALUarg1 < ALUarg2; //SLTU
			4'b0011: ALUout = ALUarg1 ^ ALUarg2;//XOR
			4'b0100: ALUout = ALUarg1 | ALUarg2; //OR
			4'b0101: ALUout = ALUarg1 & ALUarg2; //AND
			4'b0110: ALUout = ALUarg1 << ALUarg2; //ShiftLogicalLeft(SLL)
			4'b0111: ALUout = ALUarg1 >> ALUarg2; //SRL
			4'b1000: ALUout = $signed(ALUarg1) >>> ALUarg2; //SRA
			4'b1001: ALUout = ALUarg1 - ALUarg2; //SUB
			default: ALUout = ALUarg1;
		endcase
	end


	FORWARDUNIT fowunit(
		CLK,
		RSTn,
		EX_isJump,
		M_DataStoreSelect,
		M_REG_WE,
		WB_REG_WE,
		EX_Operation,
		EX_REG_RA1,
		EX_REG_RA2,
		M_REG_WA,
		WB_REG_WA,
		ALUarg1Select,
		ALUarg2Select,
		RegVal1Select,
		RegVal2Select
		);

		/*-----------------------------------------------------------------------*/

		//EX&MEM STORE
		always @( posedge CLK ) begin
			MEM_StopCounting <= EX_StopCounting;
			M_HALT <= EX_HALT;
			if(Synchronizer != 3'd6) begin
				M_ALUout <= ALUout;
				M_MEM_WEN <= EX_MEM_WEN;
				M_MEM_BE <= EX_MEM_BE;
				M_DataStoreSelect <= EX_DataStoreSelect;
				M_MEM_OUT_PROCESS <= EX_MEM_OUT_PROCESS;
				M_REG_WE <= EX_REG_WE;
				M_REG_WA <= EX_REG_WA;
				M_StoreData <= RegVal2;
				M_Operation <= EX_Operation;
				M_isJump <= EX_isJump;
				M_isBranch <= EX_isBranch;
				M_BranchOn <= BranchOn;
			end
		end
		/*-----------------------------------------------------------------------*/
		/* MEM stage */
		assign D_MEM_ADDR = M_ALUout & 16'h3fff;
		assign D_MEM_WEN = M_MEM_WEN;
		assign D_MEM_BE = M_MEM_BE;
		assign D_MEM_DOUT = M_StoreData;

		always@(*) begin
			case(M_MEM_OUT_PROCESS)
			3'b000: MEMout = $signed(D_MEM_DI << 24) >>> 24; //LB
			3'b001: MEMout = $signed(D_MEM_DI << 16) >>> 16; //LH
			3'b010: MEMout = D_MEM_DI; //LW
			3'b100: MEMout = D_MEM_DI[7:0]; //LBU
			3'b101: MEMout = D_MEM_DI[15:0]; //LHU
			default: MEMout = D_MEM_DI;
			endcase
		end

		always @(*) begin
			LoadData = M_DataStoreSelect ? M_ALUout : MEMout;
		end

		/*------------------------------------------------------------------------*/

		//MEM&WB STORE
		always @(posedge CLK) begin
			WB_StopCounting <= MEM_StopCounting;
			WB_HALT <= M_HALT;
			if(Synchronizer != 3'd6) begin
				WB_LoadData <= LoadData;
				WB_REG_WE <= M_REG_WE;
				WB_REG_WA <= M_REG_WA;
				WB_isJump <= M_isJump;
				WB_isBranch <= M_isBranch;
				WB_Operation <= M_Operation;
				WB_BranchOn <= M_BranchOn;
				WB_StoreAddr <= D_MEM_ADDR;
			end
		end
		/*------------------------------------------------------------------------*/
		/* Write Back Stage */
		assign RF_WE = WB_REG_WE;
		assign RF_WA1 = WB_REG_WA;

		assign HALT = WB_HALT;

		always @(*) begin
			if(WB_isJump != 2'd0) temp_RF_WD = WB_LoadData;
			else if(WB_isBranch) temp_RF_WD = WB_BranchOn;
			else if(WB_Operation != 2'd3) temp_RF_WD = WB_LoadData;
			else temp_RF_WD = WB_StoreAddr;
		end
		assign RF_WD = temp_RF_WD;

endmodule //
