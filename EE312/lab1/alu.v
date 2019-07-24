`timescale 1ns / 100ps

module ALU(A,B,OP,C,Cout);

	input [15:0]A;
	input [15:0]B;
	input [3:0]OP;
	output [15:0]C;
	output Cout;

	reg [15:0] C;
	reg Cout;
	//TODO
	always@(*) begin
	case(OP)
	 4'b0000://add
	  begin
	   C = A + B;
	   if( (A[15]&&B[15]&&!C[15]) || (!A[15]&&!B[15]&&C[15]) )
	    Cout = 1'b1;
	   else
	    Cout = 1'b0;
	  end
	 4'b0001://sub
	  begin
	   C = A - B;
	   if( (A[15]&&!B[15]&&!C[15]) || (!A[15]&&B[15]&&C[15]) )
	    Cout = 1'b1;
	   else
	    Cout = 1'b0;
	  end
	 4'b0010: C = A&B;//and
	 4'b0011: C = A|B;//or
	 4'b0100: C = ~(A&B);//nand
	 4'b0101: C = ~(A|B);//nor
	 4'b0110: C = A^B;//xor
	 4'b0111: C = ~(A^B);//xnor
	 4'b1000: C = A;//identity
	 4'b1001: C = ~A;//not
	 4'b1010: C = A >> 1;//logical right shift
	 4'b1011: C = $signed(A) >>>1;//arithmetic right shift
	 4'b1100: C = {A[0],A[15:1]};//rotate right
	 4'b1101: C = A << 1;//logical left shift
	 4'b1110: C = $signed(A) <<< 1;//arithmetic left shift
	 4'b1111: C = {A[14:0],A[15]};//rotate left
	endcase
	end
endmodule
