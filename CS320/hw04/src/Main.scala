/* 20160788 InJe Hwang*/
import Util._

object Main extends Homework04 {

  def run(str: String): String = {

    type Env = Map[String,Int]
    type Sto = Map[Int,BFAEValue]

    trait BFAEValue
    case class NumV(num : Int) extends BFAEValue
    case class FunV(param : String, body : BFAE, fenv : Env) extends BFAEValue
    case class RecV(renv : Env) extends BFAEValue
    case class BoxV(addr : Int) extends BFAEValue

    def lookup(str : String, env : Env, sto : Sto) : BFAEValue = {
      env.get(str) match {
        case Some(addr) => sto.get(addr) match{
          case Some(value) => value
          case None => error[BFAEValue]("Error: dangling pointer")
        }
        case None => error[BFAEValue]("Error: free identifier")
      }
    }

    def numOp(op : (Int, Int) => Int) : (BFAEValue, BFAEValue) => BFAEValue = {
      (_,_) match {
        case (NumV(n), NumV(m)) => NumV(op(n,m))
        case _ => error[BFAEValue]("Error: not a number")
      }
    }

    val add = numOp(_ + _)
    val sub = numOp(_ - _)

    def newAddr(sto : Sto) : Int = {
      if(sto.isEmpty) 0
      else {
        val (addr, value) = sto.maxBy{case (addr, value) => addr}
        addr + 1
      }
    }

    def record(keys : List[String], values : List[BFAE], env : Env, sto : Sto) : (Env, Sto) = {
      keys match{
        case t::krest => {
          values match {
            case r::vrest =>{
              val (value, vs) = interp(r, env, sto)
              val newaddr = newAddr(vs)
              val (nrec, ns) = record(krest, vrest, env , vs + (newaddr -> value))
              (nrec + (t -> newaddr), ns)
            }
            case Nil => error[(Env, Sto)]("Error: not enough value")
          }
        }
        case Nil => (Map(), sto)
      }
    }

    def interp(arg : BFAE, env : Env, sto : Sto) : (BFAEValue, Sto) = {
      arg match {
        case Num(num) => (NumV(num), sto)
        case Add(left, right) => {
          val (lv, ls) = interp(left, env, sto)
          val (rv, rs) = interp(right, env, ls)
          (add(lv, rv), rs)
        }
        case Sub(left, right) => {
          val (lv, ls) = interp(left, env, sto)
          val (rv, rs) = interp(right, env, ls)
          (sub(lv, rv), rs)
        }
        case Id(x) => (lookup(x, env, sto), sto)
        case Fun(param,body) => (FunV(param,body,env), sto)
        case App(func, arg) => {
          val (fv, fs) = interp(func, env, sto)
          fv match {
            case FunV(param,body,fenv) =>{
              val (argv, args) = interp(arg,env,fs)
              val newaddr = newAddr(args)
              interp(body, fenv + (param -> newaddr), args + (newaddr -> argv))
            }
            case _ => error[(BFAEValue, Sto)]("Error: not a function")
          }
        }
        case NewBox(expr) => {
          val (value, vs) = interp(expr,env, sto)
          val addr = newAddr(vs)
          (BoxV(addr), vs + (addr -> value))
        }
        case SetBox(box, expr) => {
          val (bv, bs) = interp(box, env, sto)
          bv match {
            case BoxV(addr) => {
              val (value, vs) = interp(expr, env, bs)
              (value, vs + (addr -> value))
            }
            case _ => error[(BFAEValue, Sto)]("Error: not a box")
          }
        }
        case OpenBox(expr) => {
          val (bv, bs) = interp(expr, env, sto)
          bv match {
            case BoxV(addr) => {
              bs.get(addr) match {
                case Some(value) => (value, bs)
                case None => error[(BFAEValue, Sto)]("Error: dangling pointer")
              }
            }
            case _ => error[(BFAEValue, Sto)]("Error: not a box")
          }
        }
        case Seqn(left, right) => {
          val (lv, ls) = interp(left, env, sto)
          right match{
            case Nil => (lv, ls)
            case t::rest => interp(Seqn(t, rest), env, ls)
          }
        }
        case Rec(rec) => {
          val keylist = (rec.keys).toList
          val valuelist = (rec.values).toList
          val (nrec, ns) = record(keylist, valuelist, env, sto)
          (RecV(nrec), ns)
        }
        case Get(expr, name) => {
          val (rec, rs) = interp(expr, env, sto)
          rec match{
            case RecV(renv) => renv.get(name) match {
              case Some(addr) => rs.get(addr) match{
                case Some(value) => (value, rs)
                case None => error[(BFAEValue, Sto)]("Error: dangling pointer")
              }
              case None => error[(BFAEValue, Sto)]("Error: no such field")
            }
            case _ => error[(BFAEValue, Sto)]("Error: not a record")
          }
        }
        case Set(record, field, expr) => {
          val (rv, rs) = interp(record, env, sto)
          rv match{
            case RecV(renv) => renv.get(field) match{
              case Some(addr) => {
                val (nv, ns) = interp(expr, env, rs)
                (nv, ns + (addr -> nv))
              }
              case None => error[(BFAEValue, Sto)]("Error: no such field")
            }
            case _ => error[(BFAEValue, Sto)]("Error: not a record")
          }
        }
      }
    }// end interp

    def convert(input : (BFAEValue,Sto)) : String = {
      input match{
        case (NumV(num), sto) => num.toString
        case (FunV(params, body, fenv), sto) => "function"
        case (RecV(renv), sto) => "record"
        case (BoxV(addr), sto) => "box"
        case (_, _) => "Error: not a BFAEValue"
      }
    }

    convert(interp(BFAE(str),Map(),Map()))
  }// end run

  def ownTests: Unit = {
    test(run("{seqn 1 2}"), "2")
    test(run("{{fun {b} {openbox b}} {newbox 10}}"), "10")
    test(run("{{fun {b} {seqn {setbox b 12} {openbox b}}} {newbox 10}}"), "12")
    test(run("{{fun {b} {seqn {setbox b 12} {openbox b}}} {newbox 10}}"), "12")
    test(run("{{fun {b} {openbox b}} {seqn {newbox 9} {newbox 10}}}"), "10")
    test(run("{{{fun {b} {fun {a} {openbox b}}} {newbox 9}} {newbox 10}}"), "9")
    test(run("{{fun {b} {seqn {setbox b 2} {openbox b}}} {newbox 1}}"), "2")
    test(run("{{fun {b} {seqn {setbox b {+ 2 {openbox b}}} {setbox b {+ 3 {openbox b}}} {setbox b {+ 4 {openbox b}}} {openbox b}}} {newbox 1}}"), "10")
    test(run("{{fun {r} {get r x}} {rec {x 1}}}"), "1")
    test(run("{{fun {r} {seqn {set r x 5} {get r x}}} {rec {x 1}}}"), "5")
    test(run("{{{{{fun {g} {fun {s} {fun {r1} {fun {r2} {+ {get r1 b} {seqn {{s r1} {g r2}} {+ {seqn {{s r2} {g r1}} {get r1 b}} {get r2 b}}}}}}}} {fun {r} {get r a}}} {fun {r} {fun {v} {set r b v}}}} {rec {a 0} {b 2}}} {rec {a 3} {b 4}}}"), "5")
    test(run("{fun {x} x}"), "function")
    test(run("{newbox 1}"), "box")
    test(run("{rec}"), "record")
    testExc[String](run("{{fun {r} {set r y 4}} {rec {x 1}}}"),"no such field") 
  }// end ownTests
}
