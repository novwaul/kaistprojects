/* 20160788 InJe Hwang*/
import Util._

object Main extends Homework03 {

  def run(str: String): String = {

    type Env = Map[String,FWAEValue]

    trait FWAEValue
    case class NumV(num : Int) extends FWAEValue
    case class FunV(params : List[String], body : FWAE, fenv : Env) extends FWAEValue
    case class RecV(renv : Env) extends FWAEValue //renv : recorded env

    def lookup(str : String, env : Env) : FWAEValue = {
      env.get(str) match {
        case Some(value) => value
        case None => error[FWAEValue]("Error: free identifier")
      }
    }

    def numOp(op : (Int, Int) => Int) : (FWAEValue, FWAEValue) => FWAEValue = {
      (_,_) match {
        case (NumV(n), NumV(m)) => NumV(op(n,m))
        case _ => error[FWAEValue]("Error: not a number")
      }
    }

    val add = numOp(_ + _)
    val sub = numOp(_ - _)

    def interp(arg : FWAE, env : Env) : FWAEValue = {
      arg match {
        case Num(num) => NumV(num)
        case Add(left, right) => add(interp(left, env), interp(right, env))
        case Sub(left, right) => sub(interp(left,env), interp(right, env))
        case With(name, value, body) => interp(body, env + (name -> interp(value, env)))
        case Id(x) => lookup(x, env)
        case App(func, args) => interp(func, env) match {
          case FunV(params,body,fenv) =>{
            if(params.size != args.size) error[FWAEValue]("Error: wrong arity")
            interp(body, fenv ++ (params zip args.map((input : FWAE) => interp(input, env))).toMap)
          }
          case _ => error[FWAEValue]("Error: not a function")
        }
        case Fun(params,body) => FunV(params,body,env)
        case Rec(rec) => RecV(rec map{ case(key, value) => (key, interp(value, env))})
        case Acc(expr, name) => {
          interp(expr, env) match{
            case NumV(num) => error[FWAEValue]("Error: not a record")
            case FunV(params,body,fenv) => error[FWAEValue]("Error: not a record")
            case RecV(renv) => renv.get(name) match {
              case Some(value) => value
              case None => error[FWAEValue]("Error: no such field")
            }
          }
        }
      }
    }// end interp

    def convert(input : FWAEValue) : String = {
      input match{
        case NumV(num) => num.toString
        case FunV(params, body, fenv) => "function"
        case RecV(renv) => "record"
      }
    }

    convert(interp(FWAE(str),Map()))
  }// end run

  def ownTests: Unit = {
    test(run("{record {a 10} {b {+ 1 2}}}"), "record")
    test(run("{access {record {a 10} {b {+ 1 2}}} b}"), "3")
    test(run("{with {g {fun {r} {access r c}}} {g {record {a 0} {c 12} {b 7}}}}"), "12")
    test(run("{access {record {r {record {z 0}}}} r}"), "record")
    test(run("{access {access {record {r {record {z 0}}}} r} z}"), "0")
    test(run("{with {f {fun {a b} {+ a b}}} {with {g {fun {x} {- x 5}}} {with {x {f 2 5}} {g x}}}}"), "2")
    test(run("{with {f {fun {x y} {+ x y}}} {f 1 2}}"), "3")
    test(run("{with {f {fun {} 5}} {+ {f} {f}}}"), "10")
    test(run("{with {h {fun {x y z w} {+ x w}}} {h 1 4 5 6}}"), "7")
    test(run("{with {f {fun {} 4}} {with {g {fun {x} {+ x x}}} {with {x 10} {- {+ x {f}} {g 4}}}}}"), "6")
    test(run("{record {a 10} {b {+ 1 2}}}"), "record")
    test(run("{access {record {r {record {z 0}}}} r}"), "record")
    test(run("{access {access {record {r {record {z 0}}}} r} z}"), "0")
    test(run("{with {x 3} {with {y 5} {access {record {a x} {b y}} a}}}"), "3")
    test(run("{with {f {fun {a b} {+ {access a a} b}}} {with {g {fun {x} {+ 5 x}}} {with {x {f {record {a 10} {b 5}} 2}} {g x}}}}"), "17")
    test(run("{with {f {fun {a b c d e} {record {a a} {b b} {c c} {d d} {e e}}}} {access {f 1 2 3 4 5} c}}"), "3")
    test(run("{with {f {fun {a b c} {record {a a} {b b} {c c}}}} {access {f 1 2 3} b}}"), "2")
    test(run("{with {f {fun {a b c} {record {x a} {y b} {z c} {d 2} {e 3}}}} {access {f 1 2 3} y}}"), "2")
    test(run("{with {f {fun {a b c} {record {x a} {y b} {z c} {d 2} {e 3}}}} {access {f 1 2 3} d}}"), "2")
    test(run("{with {f {fun {x} {+ 5 x}}} {f {access {access {record {a {record {a 10} {b {- 5 2}}}} {b {access {record {x 50}} x}}} a} b}}}"), "8")
    test(run("{access {record {a 10} {b {+ 1 2}}} b}"), "3")
    test(run("{access {record {r {record {z 0}}}} r}"), "record")
    test(run("{access {access {record {r {record {z 0}}}} r} z}"), "0")
    test(run("{record {a 10}}"), "record")
    test(run("{access {record {a 10}} a}"), "10")
    test(run("{access {record {a {+ 1 2}}} a}"), "3")
    test(run("{fun {x} x}"), "function")
    test(run("{access {record {a {record {b 10}}}} a}"), "record")
    test(run("{access {access {record {a {record {a 10}}}} a} a}"), "10")
    test(run("{access {access {record {a {record {a 10} {b 20}}}} a} a}"), "10")
    test(run("{access {access {record {a {record {a 10} {b 20}}}} a} b}"), "20")
    test(run("{+ {access {record {a 10}} a} {access {record {a 20}} a}}"), "30")
    test(run("{+ {access {record {a 10}} a} {access {record {a 20}} a}}"), "30")
    test(run("{record {a 10}}"), "record")
    test(run("{record {a {- 2 1}}}"), "record")
    test(run("{access {record {a 10}} a}"), "10")
    test(run("{access {record {a {- 2 1}}} a}"), "1")
    test(run("{access {record {a {record {b 10}}}} a}"), "record")
    test(run("{access {access {record {a {record {a 10}}}} a} a}"), "10")
    test(run("{access {access {record {a {record {a 10} {b 20}}}} a} a}"), "10")
    test(run("{access {access {record {a {record {a 10} {b 20}}}} a} b}"), "20")
    test(run("{access {record {r {record {z 0}}}} r}"), "record")
    test(run("{access {access {record {r {record {z 0}}}} r} z}"), "0")
    test(run("{with {y {record {x 1} {y 2} {z 3}}} {access y y}}"), "2")
    test(run("{with {y {record {x 1} {y 2} {z 3}}} {access y z}}"), "3")
    test(run("{record {a 10} {b {+ 1 2}}}"), "record")
    test(run("{access {record {a 10} {b {+ 1 2}}} b}"), "3")
    test(run("{with {g {fun {r} {access r c}}} {g {record {a 0} {c 12} {b 7}}}}"), "12")
    test(run("{access {record {r {record {z 0}}}} r}"), "record")
    test(run("{access {access {record {r {record {z 0}}}} r} z}"), "0")
    /* my test */
    test(run("{with {f {fun {} 10} } {f}}"), "10")
    test(run("{with {f {fun {} 10} } f}"), "function")
    test(run("{with {s1 {fun {x} x} } {s1 10}}"), "10")
    test(run("{with {r {record {a 1}}} r}"),"record")
    testExc[String](run("{with {r {record {a 1}}} {r}}"),"not a function")
    testExc[String](run("{with {f {fun {x} {+ x 1}} } {f 1 2} }"), "wrong arity")
    testExc[String](run("{with {f {fun {} 10} } {f 1}}"), "wrong arity")
    testExc[String](run("{with {f {fun {x y z} 10} } {f 1}}"), "wrong arity")
    testExc[String](run("{access {record {a 1} {b a}} b}"), "free identifier")
    testExc[String](run("{access {record {a 1}} b}"), "no such field")
    testExc[String](run("{access {+ 1 2} b}"), "not a record")
    testExc[String](run("{with {x 3} {access {record {a 3} {b 3}} x}}"), "no such field")
  }// end ownTests
}
