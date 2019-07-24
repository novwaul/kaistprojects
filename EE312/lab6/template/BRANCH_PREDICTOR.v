module BRANCH_PREDICTOR(
  input wire CLK,
  input wire RSTn,
  input wire [11:0] IF_PC,
  input wire [11:0] ID_PC,
  input wire [11:0] EX_PC,
  input wire [11:0] store_target,
  input wire BranchOn,
  input wire isBranch,
  input wire EX_isBranch,
  output reg [11:0] BranchAddr,
  output reg TargetCTRL
  );
  reg [3:0] find_index;
  reg [3:0] store_index;
  reg [3:0] update_index;
  reg [7:0] find_tag;
  reg [7:0] store_tag;
  reg [7:0] update_tag;
  reg [11:0] branch_target_buffer[15:0];
  reg [7:0] tag_table[15:0];
  reg [1:0] branch_history_table[15:0];

  //initialize
  initial begin
    branch_history_table[0] <= 2'd0; tag_table[0] <= 8'd0;
    branch_history_table[1] <= 2'd0; tag_table[1] <= 8'd0;
    branch_history_table[2] <= 2'd0; tag_table[2] <= 8'd0;
    branch_history_table[3] <= 2'd0; tag_table[3] <= 8'd0;
    branch_history_table[4] <= 2'd0; tag_table[4] <= 8'd0;
    branch_history_table[5] <= 2'd0; tag_table[5] <= 8'd0;
    branch_history_table[6] <= 2'd0; tag_table[6] <= 8'd0;
    branch_history_table[7] <= 2'd0; tag_table[7] <= 8'd0;
    branch_history_table[8] <= 2'd0; tag_table[8] <= 8'd0;
    branch_history_table[9] <= 2'd0; tag_table[9] <= 8'd0;
    branch_history_table[10] <= 2'd0; tag_table[10] <= 8'd0;
    branch_history_table[11] <= 2'd0; tag_table[11] <= 8'd0;
    branch_history_table[12] <= 2'd0; tag_table[12] <= 8'd0;
    branch_history_table[13] <= 2'd0; tag_table[13] <= 8'd0;
    branch_history_table[14] <= 2'd0; tag_table[14] <= 8'd0;
    branch_history_table[15] <= 2'd0; tag_table[15] <= 8'd0;
  end

  //store
  always @(store_target) begin
    store_index = ID_PC[3:0];
    store_tag = ID_PC[11:4];
    if(RSTn) begin
      if(isBranch && (store_tag != tag_table[store_index])) begin // target is not saved yet
        branch_target_buffer[store_index] = store_target;
        tag_table[store_index] = store_tag;
        branch_history_table[store_index] = 2'd0;
      end
    end
  end

  //update
  always @(negedge CLK) begin
    update_index = EX_PC[3:0];
    update_tag = EX_PC[11:4];
    if(RSTn) begin
      if(EX_isBranch && BranchOn &&
        update_tag == tag_table[update_index] && branch_history_table[update_index] != 2'd3) // do branch
          branch_history_table[update_index] = branch_history_table[update_index] + 2'd1;
      else if(EX_isBranch && ~BranchOn &&
        update_tag == tag_table[update_index] && branch_history_table[update_index] != 2'd0) // do not branch
          branch_history_table[update_index] = branch_history_table[update_index] - 2'd1;
    end
  end

  //read
  always @(IF_PC) begin
    find_index = IF_PC[3:0];
    find_tag = IF_PC[11:4];
    if(find_tag == tag_table[find_index] &&
        branch_history_table[find_index] > 2'd1) begin
          BranchAddr = branch_target_buffer[find_index];
          TargetCTRL = 1'b1;
        end
    else TargetCTRL = 1'b0;
  end





endmodule
