import Util._

/* 20160788 InJe Hwang */
object Main extends Homework02 {
  def run(str: String): List[Int] = {
    def binOp(
      op: (Int, Int) => Int,
      ls: List[Int],
      rs: List[Int]
    ): List[Int] = ls match {
      case Nil => Nil
      case l :: rest =>
      // TODO complete this function
      def f(r: Int): Int = op(l,r);
      rs.map(f) ++ binOp(op, rest, rs)
    }

  def add(left: Int, right: Int) : Int = {left + right}
  def sub(left: Int, right: Int) : Int = {left - right}
  def min(left: Int, right: Int) : Int = {
    if(left > right) right
    else left
  }
  def max(left: Int, right: Int) : Int = {
    if(left > right) left
    else right
  }
  def lookup(id: String, env: Map[String, List[Int]]) : List[Int] = {
    env.get(id) match{
      case Some(l) => l
      case None => error[List[Int]]("Error: free identifier")
    }
  }
  def interp(input: MUWAE, env: Map[String, List[Int]]) : List[Int] = {
     input match {
       case Num(nums) => nums
       case Add(left,right) => binOp(add,interp(left,env),interp(right,env))
       case Sub(left,right) => binOp(sub,interp(left,env),interp(right,env))
       case With(name,expr,body) => interp(body,env + (name -> interp(expr,env)))
       case Id(id) => lookup(id,env)
       case Min(left,mid,right) => binOp(min,interp(left,env),binOp(min,interp(mid,env),interp(right,env)))
       case Max(left,mid,right) => binOp(max,interp(left,env),binOp(max,interp(mid,env),interp(right,env)))
   }
  }
  interp(MUWAE(str), Map())
 }
  def ownTests: Unit = {
    /* tests offered in README */
    println("Test Start ...")
    test(run("{+ {1 2} {3 4}}"), List(4, 5, 5, 6))
    test(run("{- {+ {1 2} {3 4}} {1 2}}"), List(3, 2, 4, 3, 4, 3, 5, 4))
    test(run("{- {10 2 1} {3 2}}"), List(7, 8, -1, 0, -2, -1))
    test(run("{with {x {1 2}} {+ x {4 3}}}"), List(5, 4, 6, 5))
    test(run("{with {x 9} {+ x {with {x 3} x}}}"), List(12))
    test(run("{with {x 100} {+ x {with {y 3} x}}}"), List(200))
    test(run("{with {x 5} {+ x {with {x 3} 10}}}"), List(15))
    test(run("{with {x {7 5}} {+ x x}}"), List(14, 12, 12, 10))
    test(run("{with {x {1 2}} {+ x {4 3}}}"), List(5, 4, 6, 5))
    test(run("{with {x 2} {- {+ x x} x}}"), List(2))
    test(run("{+ {muwae-min 3 5 7} {muwae-min 10 100 1000}}"), List(13))
    test(run("{+ {muwae-min 9 3 7} {muwae-max 6 2 20}}"), List(23))
    test(run("{with {x 10} {muwae-max x 2 3}}"), List(10))
    test(run("{with {x 20} {with {y 5} {with {z {10 20}} {+ z {muwae-max {+ x y} 0 12}}}}}"), List(35, 45))
    test(run("{with {x 20} {with {y 5} {with {z {10 20}} {+ z {muwae-min {+ x y} 0 12}}}}}"), List(10, 20))
    test(run("{with {x {muwae-min 3 9 5}} {with {y {- x 3}} y}}"), List(0))
    test(run("{with {x {muwae-max 2 3 5}} {muwae-min x 7 6}}"), List(5))
    test(run("{with {x {muwae-max 9 7 10}} {muwae-max 8 x {+ 1 x}}}"), List(11))
    test(run("{- {muwae-min 6 4 5} {muwae-max 2 3 4}}"), List(0))
    test(run("{with {x {+ 7 2}} {muwae-min x 7 0}}"), List(0))
    test(run("{+ {muwae-min 9 3 7} {muwae-max 6 2 20}}"), List(23))
    test(run("{with {x {13}} {muwae-min x 1 12}}"), List(1))
    test(run("{with {x {muwae-min 2 1 3}} {+ x x}}"), List(2))
    test(run("{with {a 10} {with {b 19} {with {c 2} {muwae-min a b c}}}}"), List(2))
    test(run("{with {x 3} {muwae-max 3 4 {+ x x}}}"), List(6))
    test(run("{with {a 10} {with {b 19} {with {c 2} {muwae-max a b c}}}}"), List(19))
    test(run("{with {x {muwae-min 2 5 4}} {+ x x}}"), List(4))
    test(run("{with {x {muwae-max 2 5 4}} {+ x x}}"), List(10))
    test(run("{with {x {- 11 3}} {muwae-max x {+ x x} {- x x}}}"), List(16))
    test(run("{with {x {- 11 3}} {muwae-min x {+ x x} {- x x}}}"), List(0))
    test(run("{muwae-min {+ 4 4} {with {x 5} {+ x {with {x 3} 10}}} 3}"), List(3))
    test(run("{muwae-max {+ 4 4} {with {x 5} {+ x {with {x 3} 10}}} 3}"), List(15))
    test(run("{with {x {13}} {muwae-min x 1 12}}"), List(1))
    test(run("{with {x {10} } {muwae-max x 2 3}}"), List(10))
    test(run("{with {x {muwae-min 2 1 3}} {+ x x}}"), List(2))
    test(run("{with {x {muwae-max 2 1 3}} {+ x x}}"), List(6))
    test(run("{with {x 2} {muwae-min x 3 10}}"), List(2))
    test(run("{with {x 2} {muwae-max x 3 10}}"), List(10))
    test(run("{muwae-min {+ 4 4} 2 3} "), List(2))
    test(run("{muwae-max {+ 4 4} 2 3} "), List(8))
    test(run("{with {x 10} {muwae-min x 2 3}}"), List(2))
    test(run("{with {x 10} {muwae-max x 2 3}}"), List(10))
    test(run("{with {x {10}} {muwae-max x 2 3}}"), List(10))
    test(run("{muwae-min {+ 3 4} 5 6}"), List(5))
    test(run("{muwae-max {+ 3 4} 5 6}"), List(7))
    test(run("{with {x {10}} {muwae-min x {3} {5}}}"), List(3))
    test(run("{with {x {10}} {muwae-max x {3} {5}}}"), List(10))
    test(run("{muwae-min {3} 4 5}"), List(3))
    test(run("{muwae-max {3} 4 {5}}"), List(5))
    test(run("{+ {10 100 1000 10000} {muwae-min {- 3 4} 5 6}}"), List(9, 99, 999, 9999))
    /* my tests */
    test(run("12"), List(12))
    test(run("{1 2}"),List(1,2))
    test(run("{with {x {1 2}} {with {y {-1 -2}} {+ x y}}}"),List(0,-1,1,0)) // use two identifer
    test(run("{with {x {1 2 3 4}} {with {x {-1 -2 -3 -4}} x }}}"),List(-1,-2,-3,-4))// shadowing
    testExc[List[Int]](run("{with {x 10} {+ x y}}"),"Error:") // free identifier error
    testExc[List[Int]](run("x"),"Error:") // free identifier error
    println("Test End ...")
  }
}
