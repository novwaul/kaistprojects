`include "vending_machine_def.v"

module vending_machine (

	clk,							// Clock signal
	reset_n,						// Reset signal (active-low)

	i_input_coin,				// coin is inserted.
	i_select_item,				// item is selected.
	i_trigger_return,			// change-return is triggered

	o_available_item,			// Sign of the item availability
	o_output_item,			// Sign of the item withdrawal
	o_return_coin,				// Sign of the coin return
	stopwatch,
	current_total,
	return_temp,
);

	// Ports Declaration
	// Do not modify the module interface
	input clk;
	input reset_n;

	input [`kNumCoins-1:0] i_input_coin;
	input [`kNumItems-1:0] i_select_item;
	input i_trigger_return;

	output reg [`kNumItems-1:0] o_available_item;
	output reg [`kNumItems-1:0] o_output_item;
	output reg [`kNumCoins-1:0] o_return_coin;

	output [3:0] stopwatch;
	output [`kTotalBits-1:0] current_total;
	output [`kTotalBits-1:0] return_temp;
	// Normally, every output is register,
	//   so that it can provide stable value to the outside.

//////////////////////////////////////////////////////////////////////	/

	//we have to return many coins
	reg [`kCoinBits-1:0] returning_coin_0;
	reg [`kCoinBits-1:0] returning_coin_1;
	reg [`kCoinBits-1:0] returning_coin_2;
	reg block_item_0;
	reg block_item_1;
	//check timeout
	reg [3:0] stopwatch;
	//when return triggered
	reg have_to_return;
	reg  [`kTotalBits-1:0] return_temp;
	reg [`kTotalBits-1:0] temp;
////////////////////////////////////////////////////////////////////////

	// Net constant values (prefix kk & CamelCase)
	// Please refer the wikepedia webpate to know the CamelCase practive of writing.
	// http://en.wikipedia.org/wiki/CamelCase
	// Do not modify the values.
	wire [31:0] kkItemPrice [`kNumItems-1:0];	// Price of each item
	wire [31:0] kkCoinValue [`kNumCoins-1:0];	// Value of each coin
	assign kkItemPrice[0] = 400;
	assign kkItemPrice[1] = 500;
	assign kkItemPrice[2] = 1000;
	assign kkItemPrice[3] = 2000;
	assign kkCoinValue[0] = 100;
	assign kkCoinValue[1] = 500;
	assign kkCoinValue[2] = 1000;


	// NOTE: integer will never be used other than special usages.
	// Only used for loop iteration.
	// You may add more integer variables for loop iteration.
	integer i, j, k,l,m,n;

	// Internal states. You may add your own net & reg variables.
	reg [`kTotalBits-1:0] current_total;
	reg [`kItemBits-1:0] num_items [`kNumItems-1:0];
	reg [`kCoinBits-1:0] num_coins [`kNumCoins-1:0];

	// Next internal states. You may add your own net and reg variables.
	reg [`kTotalBits-1:0] current_total_nxt;
	reg [`kItemBits-1:0] num_items_nxt [`kNumItems-1:0];
	reg [`kCoinBits-1:0] num_coins_nxt [`kNumCoins-1:0];

	// Variables. You may add more your own registers.
	reg [`kTotalBits-1:0] input_total, output_total, return_total_0,return_total_1,return_total_2;
	reg avail_temp[3:0];

	
	// Combinational logic for the next states
	always @(*) begin
		// TODO: current_total_nxt
		// You don't have to worry about concurrent activations in each input vector (or array).
		// Calculate the next current_total state. current_total_nxt =
	casex({have_to_return,i_select_item,i_input_coin})
	8'b1_0000_000:
	begin //time out
		if(current_total >= kkCoinValue[2] && num_coins_nxt[2] > 0) begin
		current_total_nxt = current_total - kkCoinValue[2];
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[1],num_coins[0]};
		num_coins_nxt[2] = num_coins[2] - 1;
		end
		else if(current_total >= kkCoinValue[1] && num_coins_nxt[1] > 0) begin
		current_total_nxt = current_total - kkCoinValue[1];
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[0]} = {num_coins[2],num_coins[0]};
		num_coins_nxt[1] = num_coins[1] - 1;
		end
		else if(current_total >= kkCoinValue[0] && num_coins_nxt[0] > 0) begin
		current_total_nxt = current_total - kkCoinValue[0];
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[1]} = {num_coins[2],num_coins[1]};
		num_coins_nxt[0] = num_coins[0] - 1;
		end
		else begin
		current_total_nxt = current_total;
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
	end
	8'bx_0000_001:
	begin // insert 100
		current_total_nxt = current_total + kkCoinValue[0];
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[1]} = {num_coins[2],num_coins[1]};
		num_coins_nxt[0] = num_coins[0] + 1;
	end
	8'bx_0000_010:
	begin// insert 500
		current_total_nxt = current_total + kkCoinValue[1];
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[0]} = {num_coins[2],num_coins[0]};
		num_coins_nxt[1] = num_coins[1] + 1;	
	end
	8'bx_0000_100:
	begin// insert 1000
		current_total_nxt = current_total + kkCoinValue[2];
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[1],num_coins[0]};
		num_coins_nxt[2] = num_coins[2] + 1;	
	end
	8'bx_0001_000:// order 400 
	begin
		if(current_total >= kkItemPrice[0] && num_items[0] > 0) begin
		current_total_nxt = current_total - kkItemPrice[0];
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1]} = {num_items[3],num_items[2],num_items[1]};
		num_items_nxt[0] = num_items[0] - 1;
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
		else begin
		current_total_nxt = current_total;
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
	end
	8'bx_0010_000://order 500
	begin
		if(current_total >= kkItemPrice[1] && num_items[1] > 0) begin
		current_total_nxt = current_total - kkItemPrice[1];
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[0]};
		num_items_nxt[1] = num_items[1] - 1;
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
		else begin
		current_total_nxt = current_total;
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
	end
	8'bx_0100_000://order 1000
	begin
		if(current_total >= kkItemPrice[2] && num_items[2] > 0) begin
		current_total_nxt = current_total - kkItemPrice[2];
		{num_items_nxt[3],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[1],num_items[0]};
		num_items_nxt[2] = num_items[2] - 1;
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
		else begin
		current_total_nxt = current_total;
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
	end
	8'bx_1000_000://order 2000
	begin
		if(current_total >= kkItemPrice[3] && num_items[3] > 0) begin
		current_total_nxt = current_total - kkItemPrice[3];
		{num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[2],num_items[1],num_items[0]};
		num_items_nxt[3] = num_items[3] - 1;
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
		else begin
		current_total_nxt = current_total;
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
		end
	end
	default:
	begin
		current_total_nxt = current_total;
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} = {num_items[3],num_items[2],num_items[1],num_items[0]};
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} = {num_coins[2],num_coins[1],num_coins[0]};
	end
	endcase
	end


	// Combinational logic for the outputs
	always @(*) begin
	// TODO: o_available_item
	{avail_temp[3],avail_temp[2],avail_temp[1],avail_temp[0]} = {1'b0,1'b0,1'b0,1'b0};
	for(l = 0; l < 4; l = l + 1) begin
		if(current_total >= kkItemPrice[l]) begin
		temp = current_total - kkItemPrice[l];
		for(i = 0; temp >= 1000; i = i + 1) temp = temp - 1000;
		for(j = 0; temp >= 500; j = j + 1) temp = temp - 500;
		for(k = 0; temp >= 100; k = k + 1) temp = temp - 100;
		avail_temp[l] = 1'b1 & (num_items[l]>0) &(
							(num_coins[2]>=i&&num_coins[1]>=j&&num_coins[0]>=k)||
							(num_coins[1]>=j+2*(i-num_coins[2])&&num_coins[0]>=k)||
							(num_coins[2]>=i&&num_coins[0]>=k+5*(j-num_coins[1]))||
							(num_coins[0]>=k+5*(j-num_coins[1])+10*(i-num_coins[2]))
							);
		end
	end
	o_available_item = {avail_temp[3],avail_temp[2],avail_temp[1],avail_temp[0]};
	
	// TODO: o_output_item
	casex({have_to_return,i_select_item})
	5'b1_0000:
	begin
		if(current_total >= kkCoinValue[2] && num_coins[2] > 0) o_return_coin = 3'b100;
		else if(current_total >= kkCoinValue[1] && num_coins[1] > 0) o_return_coin = 3'b010;
		else if(current_total >= kkCoinValue[0] && num_coins[0] > 0) o_return_coin = 3'b001;
		else o_return_coin = 3'b000;
	end
	5'bx_0001:
	begin
		if(current_total >= kkItemPrice[0]) o_output_item = {1'b0,1'b0,1'b0,1'b1&(num_items[0]>0)};
		else o_output_item = 4'b0000;
	end
	5'bx_0010:
	begin
		if(current_total >= kkItemPrice[1]) o_output_item = {1'b0,1'b0,1'b1&(num_items[1]>0),1'b0};
		else o_output_item = 4'b0000;
	end
	5'bx_0100:
	begin
		if(current_total >= kkItemPrice[2]) o_output_item = {1'b0,1'b1&(num_items[2]>0),1'b0,1'b0};
		else o_output_item = 4'b0000;
	end
	5'bx_1000:
	begin
		if(current_total >= kkItemPrice[3]) o_output_item = {1'b1&(num_items[3]>0),1'b0,1'b0,1'b0};
		else o_output_item = 4'b0000;
	end
	default: 
	begin
		o_output_item = 4'b0000;
		o_return_coin = 3'b000;
	end
	endcase
	end

	// Sequential circuit to reset or update the states
	always @(posedge clk) begin
		if (!reset_n) begin
			// TODO: reset all states.
		current_total <= `kTotalBits'd0;
		current_total_nxt <= `kTotalBits'd0;
		{num_items[3],num_items[2],num_items[1],num_items[0]} <= {`kItemBits'd`kEachItemNum,`kItemBits'd`kEachItemNum,`kItemBits'd`kEachItemNum,`kItemBits'd`kEachItemNum};
		{num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]} <= {`kItemBits'd`kEachItemNum,`kItemBits'd`kEachItemNum,`kItemBits'd`kEachItemNum,`kItemBits'd`kEachItemNum};
		{num_coins[2],num_coins[1],num_coins[0]} <= {`kCoinBits'd`kEachCoinNum,`kCoinBits'd`kEachCoinNum,`kCoinBits'd`kEachCoinNum};
		{num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]} <= {`kCoinBits'd`kEachCoinNum,`kCoinBits'd`kEachCoinNum,`kCoinBits'd`kEachCoinNum};
		stopwatch <= 4'd15;
		have_to_return <= 0;
		end
		else begin
			// TODO: update all states.
		current_total <= current_total_nxt;
		{num_items[3],num_items[2],num_items[1],num_items[0]} <= {num_items_nxt[3],num_items_nxt[2],num_items_nxt[1],num_items_nxt[0]};
		{num_coins[2],num_coins[1],num_coins[0]} <= {num_coins_nxt[2],num_coins_nxt[1],num_coins_nxt[0]};
/////////////////////////////////////////////////////////////////////////

			// decrease stopwatch
			//if you have to return some coins then you have to turn on the bit
		if(stopwatch > 1 && current_total > 0 && i_input_coin == 0 && i_select_item == 0 && i_trigger_return == 0) stopwatch = stopwatch - 4'd1; // if there is no input, countdown
		else if(i_trigger_return != 0 && current_total != 0) begin stopwatch = 4'd1; have_to_return = 1; end// if input is i_trigger_return, start returning changes
		else if(stopwatch != 1) begin stopwatch = 4'd15; have_to_return = 0; end// if input exists(without i_trigger_return), stop countdowning
		else if(stopwatch == 1 && (current_total == 0 || i_select_item != 0 || i_input_coin !=0)) begin stopwatch = 4'd15; have_to_return = 0; end// stop returning changes
		else have_to_return = 1;//stopwatch is 1 and there is no input
		

/////////////////////////////////////////////////////////////////////////
		end		   //update all state end
	end	   //always end

endmodule
