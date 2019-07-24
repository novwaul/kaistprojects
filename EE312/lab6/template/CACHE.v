module CACHE(
  input wire RSTn,
  input wire CLK,
  input wire [11:0] M_ALUout,
  input wire M_MEM_WEN,
  input wire [3:0] M_MEM_BE,
  input M_DataStoreSelect,
  input wire [31:0] M_StoreData,
  input wire [31:0] D_MEM_DI,
  output reg [11:0] D_MEM_ADDR,
  output reg D_MEM_WEN,
  output reg [3:0] D_MEM_BE,
  output reg [31:0] D_MEM_DOUT,
  output reg [31:0] CACHE_OUT,
  output reg STALL
  );
reg [127:0] cache [31:0];
reg [2:0] tag_table [31:0];
reg validbit_table [31:0];
reg MISS;
reg [31:0] tempOut;
reg [127:0] tempRead;
reg [31:0] tempChange;
reg [127:0] tempStore;
reg [11:0] DataAddr;
reg [2:0] MissCounter;
reg [2:0] WriteCounter;

wire [1:0] word_index = M_ALUout[3:2];
wire [4:0] block_index = M_ALUout[8:4];
wire [2:0] tag = M_ALUout[11:9];

always @(negedge CLK) begin
  /* read */
  if(~M_DataStoreSelect) begin
    /* read hit */
    if(tag_table[block_index] == tag && validbit_table[block_index] && ~STALL && ~MISS) begin
      tempOut = cache[block_index] >> (6'd32 * (2'd3 - word_index));
      STALL = 1'd1; // set stall for synchronize pipeline
    end
    /* return read hit value */
    else if(STALL && ~MISS) begin
      STALL = 1'd0;
      CACHE_OUT = tempOut;
    end
    /* read miss */
    else if(~STALL && ~MISS) begin
      MISS = 1'd1; // set miss to handle read miss
      STALL = 1'd1;
      DataAddr = {tag, block_index, 2'b00, M_ALUout[1:0]};
      tempRead = 128'd0;
    end
    /* handle read miss */
    else begin
      MissCounter = MissCounter + 3'd1;
      D_MEM_ADDR <= DataAddr;
      DataAddr <= DataAddr + 12'd4;
    end
  end
  /* write */
  else if(~M_MEM_WEN) begin
    /* write finish */
    if(WriteCounter == 3'd5) begin
      STALL = 1'd0;
      WriteCounter = 3'd0;
      D_MEM_WEN = 1'd1;
    end
    /* wirte cache and memory(write through  & write no-allocate) */
    else if(WriteCounter == 3'd0) begin
      STALL = 1'd1;
      /* write cache */
      if(tag_table[block_index] == tag && validbit_table[block_index]) begin
        if(word_index == 2'd3) begin
          tempChange = cache[block_index][31:0];
          if(M_MEM_BE[0]) tempChange[7:0] = M_StoreData[7:0];
          if(M_MEM_BE[1]) tempChange[15:8] = M_StoreData[15:8];
          if(M_MEM_BE[2]) tempChange[23:16] = M_StoreData[23:16];
          if(M_MEM_BE[3]) tempChange[31:24] = M_StoreData[31:24];
          cache[block_index][31:0] = tempChange;
        end
        else if(word_index == 2'd2) begin
          tempChange = cache[block_index][63:32];
          if(M_MEM_BE[0]) tempChange[7:0] = M_StoreData[7:0];
          if(M_MEM_BE[1]) tempChange[15:8] = M_StoreData[15:8];
          if(M_MEM_BE[2]) tempChange[23:16] = M_StoreData[23:16];
          if(M_MEM_BE[3]) tempChange[31:24] = M_StoreData[31:24];
          cache[block_index][63:32] = tempChange;
        end
        else if(word_index == 2'd1) begin
          tempChange = cache[block_index][95:64];
          if(M_MEM_BE[0]) tempChange[7:0] = M_StoreData[7:0];
          if(M_MEM_BE[1]) tempChange[15:8] = M_StoreData[15:8];
          if(M_MEM_BE[2]) tempChange[23:16] = M_StoreData[23:16];
          if(M_MEM_BE[3]) tempChange[31:24] = M_StoreData[31:24];
          cache[block_index][95:64] = tempChange;
        end
        else begin
          tempChange = cache[block_index][127:96];
          if(M_MEM_BE[0]) tempChange[7:0] = M_StoreData[7:0];
          if(M_MEM_BE[1]) tempChange[15:8] = M_StoreData[15:8];
          if(M_MEM_BE[2]) tempChange[23:16] = M_StoreData[23:16];
          if(M_MEM_BE[3]) tempChange[31:24] = M_StoreData[31:24];
          cache[block_index][127:96] = tempChange;
        end
      end
      /* write memory */
      D_MEM_WEN = 1'd0;
      D_MEM_ADDR = M_ALUout;
      D_MEM_BE = M_MEM_BE;
      D_MEM_DOUT = M_StoreData;
      WriteCounter = WriteCounter + 3'd1;
    end
    else WriteCounter = WriteCounter + 3'd1;
  end
end

/* read miss handler */
always @(posedge CLK) begin
  if(MissCounter > 3'd0) begin
    /* handle miss read over */
    if(MissCounter == 3'd6) begin
      MISS = 1'd0;
      MissCounter = 3'd0;
      tempOut = cache[block_index] >> (6'd32 * (2'd3 - word_index));
    end
    /* store in cache */
    else if(MissCounter == 3'd5) begin
      cache[block_index] = tempRead;
      tag_table[block_index] = tag;
      validbit_table[block_index] = 1'd1;
    end
    /* handle miss read */
    else begin
      tempRead = tempRead << 6'd32;
      tempRead = tempRead | D_MEM_DI;
    end
  end
end

/* intiialize */
always @(*) begin
  if(~RSTn) begin
    cache[0] <= 128'd0; cache[16] <= 128'd0;
    cache[1] <= 128'd0; cache[17] <= 128'd0;
    cache[2] <= 128'd0; cache[18] <= 128'd0;
    cache[3] <= 128'd0; cache[19] <= 128'd0;
    cache[4] <= 128'd0; cache[20] <= 128'd0;
    cache[5] <= 128'd0; cache[21] <= 128'd0;
    cache[6] <= 128'd0; cache[22] <= 128'd0;
    cache[7] <= 128'd0; cache[23] <= 128'd0;
    cache[8] <= 128'd0; cache[24] <= 128'd0;
    cache[9] <= 128'd0; cache[25] <= 128'd0;
    cache[10] <= 128'd0; cache[26] <= 128'd0;
    cache[11] <= 128'd0; cache[27] <= 128'd0;
    cache[12] <= 128'd0; cache[28] <= 128'd0;
    cache[13] <= 128'd0; cache[29] <= 128'd0;
    cache[14] <= 128'd0; cache[30] <= 128'd0;
    cache[15] <= 128'd0; cache[31] <= 128'd0;

    validbit_table[0] <= 1'd0; validbit_table[16] <= 1'd0;
    validbit_table[1] <= 1'd0; validbit_table[17] <= 1'd0;
    validbit_table[2] <= 1'd0; validbit_table[18] <= 1'd0;
    validbit_table[3] <= 1'd0; validbit_table[19] <= 1'd0;
    validbit_table[4] <= 1'd0; validbit_table[20] <= 1'd0;
    validbit_table[5] <= 1'd0; validbit_table[21] <= 1'd0;
    validbit_table[6] <= 1'd0; validbit_table[22] <= 1'd0;
    validbit_table[7] <= 1'd0; validbit_table[23] <= 1'd0;
    validbit_table[8] <= 1'd0; validbit_table[24] <= 1'd0;
    validbit_table[9] <= 1'd0; validbit_table[25] <= 1'd0;
    validbit_table[10] <= 1'd0; validbit_table[26] <= 1'd0;
    validbit_table[11] <= 1'd0; validbit_table[27] <= 1'd0;
    validbit_table[12] <= 1'd0; validbit_table[28] <= 1'd0;
    validbit_table[13] <= 1'd0; validbit_table[29] <= 1'd0;
    validbit_table[14] <= 1'd0; validbit_table[30] <= 1'd0;
    validbit_table[15] <= 1'd0; validbit_table[31] <= 1'd0;

    tag_table[0] <= 3'd0; tag_table[16] <= 3'd0;
    tag_table[1] <= 3'd0; tag_table[17] <= 3'd0;
    tag_table[2] <= 3'd0; tag_table[18] <= 3'd0;
    tag_table[3] <= 3'd0; tag_table[19] <= 3'd0;
    tag_table[4] <= 3'd0; tag_table[20] <= 3'd0;
    tag_table[6] <= 3'd0; tag_table[22] <= 3'd0;
    tag_table[5] <= 3'd0; tag_table[21] <= 3'd0;
    tag_table[7] <= 3'd0; tag_table[23] <= 3'd0;
    tag_table[8] <= 3'd0; tag_table[24] <= 3'd0;
    tag_table[9] <= 3'd0; tag_table[25] <= 3'd0;
    tag_table[10] <= 3'd0; tag_table[26] <= 3'd0;
    tag_table[11] <= 3'd0; tag_table[27] <= 3'd0;
    tag_table[12] <= 3'd0; tag_table[28] <= 3'd0;
    tag_table[13] <= 3'd0; tag_table[29] <= 3'd0;
    tag_table[14] <= 3'd0; tag_table[30] <= 3'd0;
    tag_table[15] <= 3'd0; tag_table[31] <= 3'd0;

    WriteCounter <= 3'd0;
    MissCounter <= 3'd0;
    MISS <= 1'd0;
    STALL <= 1'd0;
    D_MEM_WEN = 1'd1;
  end
end

endmodule
