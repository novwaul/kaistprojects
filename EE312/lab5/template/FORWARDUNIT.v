module FORWARDUNIT(
  input wire CLK,
  input wire RSTn,
  input wire [1:0] EX_isJump,
  input wire M_DataStoreSelect,
  input wire M_REG_WE,
  input wire WB_REG_WE,
  input wire [1:0] EX_Operation,
  input wire [4:0] EX_REG_RA1,
  input wire [4:0] EX_REG_RA2,
  input wire [4:0] M_REG_WA,
  input wire [4:0] WB_REG_WA,
  output reg ALUarg1Select,
  output reg [1:0] ALUarg2Select,
  output reg [1:0] RegVal1Select,
  output reg [1:0] RegVal2Select
  );

  //ALU argument select bit for EX stage
  always @(*) begin
    if(EX_isJump != 2'd0) begin // jump
      ALUarg1Select = 1'd1;
      ALUarg2Select = 2'd1;
    end
    else begin
      case(EX_Operation)
      2'b00: begin //Norm
              ALUarg1Select = 1'd0;
              ALUarg2Select = 2'd0;
             end
      default: begin //Imm or Load or Store
              ALUarg1Select = 1'd0;
              ALUarg2Select = 2'd2;
             end
      endcase
    end
  end

  //Forwarding select bit
  always @(*) begin
    if(EX_isJump == 2'd0) begin
      //RegVal1
      if(EX_REG_RA1 != 5'd0 && (EX_REG_RA1 == M_REG_WA) && M_REG_WE) begin
        if(M_DataStoreSelect) RegVal1Select = 2'd1; //EX
        else RegVal1Select = 2'd2;//MEM
      end
      else if(EX_REG_RA1 != 5'd0 && (EX_REG_RA1 == WB_REG_WA) && WB_REG_WE) RegVal1Select = 2'd3;//WB
      else RegVal1Select = 2'd0;
      //RegVal2
      if(EX_REG_RA2 != 5'd0 && (EX_REG_RA2 == M_REG_WA) && M_REG_WE && ~(EX_Operation[1] ^ EX_Operation[0])) begin
       if(M_DataStoreSelect) RegVal2Select = 2'd1;
       else RegVal2Select = 2'd2;
      end
      else if(EX_REG_RA2 != 5'd0 && (EX_REG_RA2 == WB_REG_WA) && WB_REG_WE && ~(EX_Operation[1] ^ EX_Operation[0])) RegVal2Select = 2'd3;
      else RegVal2Select = 2'd0;
    end
  end

endmodule
