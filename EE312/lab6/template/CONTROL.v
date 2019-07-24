module CONTROL(
  input wire CLK,
  input wire RSTn,
  input wire [31:0] IF_Inst,
  input wire [31:0] ID_Inst,
  input wire EX_SelectBit_Top,
  input wire EX_SelectBit_Bottom,
  input wire BranchOn,
  input wire [1:0] EX_isJump,
  input wire EX_isBranch,
  input wire [1:0] EX_Operation,
  output reg I_MEM_CSN,
  output reg D_MEM_CSN,
  output reg HALT,
  output reg DataStoreSelect,
  output reg HazardDetected,
  output reg isBranch,
  output reg [1:0] isJump,
  output reg [1:0] Operation,
  output reg [3:0] ALUopcode,
  output reg MEM_WEN,
  output reg [3:0] MEM_BE,
  output reg [2:0] MEM_OUT_PROCESS,
  output reg REG_WE
  );

/* HALT */
always@(*) begin
  if(~RSTn) HALT = 1'b0;
  else if(ID_Inst == 32'h00c00093 && IF_Inst == 32'h00008067) HALT = 1'b1;
end

/* CSN Control */
initial begin
  I_MEM_CSN = 1'b0;
  D_MEM_CSN = 1'b0;
end

/* Signal Generator */
always @(ID_Inst) begin
  case(ID_Inst[6:0])
  7'b1101111: begin //JAL
                isBranch = 1'd0;
                isJump = 2'd1;
                ALUopcode = 4'd0;
                MEM_WEN = 1'd1;
                REG_WE = (ID_Inst[11:7] == 5'd0) ? 1'd0 : 1'd1;
                Operation = 2'd0;
                DataStoreSelect = 1'd1;
              end
  7'b1100111: begin //JALR
                isBranch = 1'd0;
                isJump = 2'd2;
                ALUopcode = 4'd0;
                MEM_WEN = 1'd1;
                REG_WE = (ID_Inst[11:7] == 5'd0) ? 1'd0 : 1'd1;
                Operation = 2'd0;
                DataStoreSelect = 1'd1;
              end
  7'b1100011: begin //Branch
                isBranch = 1'd1;
                isJump = 2'd0;
                MEM_WEN = 1'd1;
                REG_WE = 1'd0;
                Operation = 2'd0;
                DataStoreSelect = 1'd1;
              end
  7'b0000011: begin //Load
                isBranch = 1'd0;
                isJump = 2'd0;
                ALUopcode = 4'd0;
                MEM_WEN = 1'd1;
                REG_WE = (ID_Inst[11:7] == 5'd0) ? 1'd0 : 1'd1;
                MEM_OUT_PROCESS = ID_Inst[14:12];
                DataStoreSelect = 1'd0;
                Operation = 2'd2;
              end
  7'b0100011: begin //Store
                isBranch = 1'd0;
                isJump = 2'd0;
                ALUopcode = 4'd0;
                MEM_WEN = 1'd0;
                MEM_BE = {ID_Inst[13],ID_Inst[13],ID_Inst[12] | ID_Inst[13],1'd1};
                REG_WE = 1'd0;
                DataStoreSelect = 1'd1;
                Operation = 2'd3;
              end
  7'b0010011: begin //Immediate Operation
                isBranch = 1'd0;
                isJump = 2'd0;
                MEM_WEN = 1'd1;
                REG_WE = (ID_Inst[11:7] == 5'd0) ? 1'd0 : 1'd1;
                DataStoreSelect = 1'd1;
                Operation = 2'd1;
                case(ID_Inst[14:12])
                3'b000: ALUopcode = 4'd0;
                3'b010: ALUopcode = 4'd1;
                3'b011: ALUopcode = 4'd2;
                3'b100: ALUopcode = 4'd3;
                3'b110: ALUopcode = 4'd4;
                3'b111: ALUopcode = 4'd5;
                3'b001: ALUopcode = 4'd6;
                3'b101: ALUopcode = {ID_Inst[30],~ID_Inst[30],~ID_Inst[30],~ID_Inst[30]};
                endcase
              end
  7'b0110011: begin //Normal Operation
                isBranch = 1'd0;
                isJump = 2'd0;
                MEM_WEN = 1'd1;
                REG_WE = (ID_Inst[11:7] == 5'd0) ? 1'd0 : 1'd1;
                DataStoreSelect = 1'd1;
                Operation = 2'd0;
                case(ID_Inst[14:12])
                3'b000: ALUopcode = {ID_Inst[30],1'b0,1'b0,ID_Inst[30]};
                3'b001: ALUopcode = 4'd6;
                3'b010: ALUopcode = 4'd1;
                3'b011: ALUopcode = 4'd2;
                3'b100: ALUopcode = 4'd3;
                3'b101: ALUopcode = {ID_Inst[30],~ID_Inst[30],~ID_Inst[30],~ID_Inst[30]};
                3'b110: ALUopcode = 4'd4;
                3'b111: ALUopcode = 4'd5;
                endcase
              end
  default: begin
            isBranch = 1'd0;
            isJump = 2'd0;
            MEM_WEN = 1'd1;
            REG_WE = 1'd0;
            DataStoreSelect = 1'd1;
            Operation = 2'd0;
           end
  endcase
end

/* Hazard Detect Unit */
always @(posedge CLK) begin
  if(HazardDetected == 1'd1) HazardDetected = 1'd0;
end
always @(negedge CLK) begin
  if(EX_isJump[1]) HazardDetected = 1'd1; // JALR
  else if(EX_isBranch && BranchOn) begin // do Branch
    if(EX_SelectBit_Top || ~EX_SelectBit_Bottom) HazardDetected = 1'd1;
    else HazardDetected = 1'd0;
  end
  else if(EX_isBranch && ~BranchOn) begin // do not Branch
    if(~EX_SelectBit_Top && EX_SelectBit_Bottom) HazardDetected = 1'd1;
    else HazardDetected = 1'd0;
  end
  else HazardDetected = 1'd0;
end

endmodule
