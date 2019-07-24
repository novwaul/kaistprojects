import Util._

/* 20160788 InJe Hwang */

object Main extends Homework08 {

  type Env = Map[String, KXCFAEV]
  type Cont = KXCFAEV => KXCFAEV

  trait KXCFAEV
  case class NumV(num : Int) extends KXCFAEV
  case class CloV(params : List[String], body : KXCFAE, env : Env) extends KXCFAEV
  case class ConV(cont : Cont) extends KXCFAEV
  case object ThrowV extends KXCFAEV

  def ADDV(left : KXCFAEV, right : KXCFAEV) : KXCFAEV = (left, right) match {
    case (NumV(l), NumV(r)) => NumV(l+r)
    case _ => error[KXCFAEV](s"$left or $right is not a number")
  }

  def SUBV(left : KXCFAEV, right : KXCFAEV) : KXCFAEV = (left, right) match {
    case (NumV(l), NumV(r)) => NumV(l-r)
    case _ => error[KXCFAEV](s"$left or $right is not a number")
  }

  def interpR(args : List[KXCFAE], params : List[String], body : KXCFAE, env : Env, fenv : Env, tryCont : Cont, cont : Cont) : KXCFAEV = {
    args match {
      case arg::argRemain => params match {
        case param::paramRemain => {
          interp(arg, env, tryCont, _ match {
            case ThrowV => tryCont(ThrowV)
            case v => interpR(argRemain, paramRemain, body, env, fenv + (param -> v), tryCont, cont)
          })
        }
        case Nil => error[KXCFAEV](s"wrong arity")
      }
      case Nil => params match {
        case param::t => error[KXCFAEV](s"wrong arity")
        case Nil => interp(body, fenv, tryCont, cont)
      }
    }
  }

  def interp(expr : KXCFAE, env : Env, tryCont : Cont, cont : Cont) : KXCFAEV = expr match {
    case Num(num) => cont(NumV(num))
    case Add(left, right) => interp(left, env, tryCont, _ match {
      case ThrowV => tryCont(ThrowV)
      case lv => interp(right, env, tryCont, _ match {
        case ThrowV => tryCont(ThrowV)
        case rv => cont(ADDV(lv, rv))
      })
    })
    case Sub(left, right) => interp(left, env, tryCont, _ match {
      case ThrowV => tryCont(ThrowV)
      case lv => interp(right, env, tryCont, _ match {
        case ThrowV => tryCont(ThrowV)
        case rv => cont(SUBV(lv,rv))
      })
    })
    case Id(name) => env.get(name) match {
      case Some(v) => cont(v)
      case None => error[KXCFAEV](s"$name is a free identifier")
    }
    case Fun(params, body) => cont(CloV(params, body, env))
    case App(fun, args) => interp(fun, env, tryCont, _ match{
      case CloV(params, body, fenv) => interpR(args, params, body, env, fenv, tryCont, cont)
      case ConV(k) =>{
        if(args.size == 1) interp(args(0), env, tryCont, k)
        else error[KXCFAEV](s"$args has wrong arity")
      }
      case ThrowV => tryCont(ThrowV)
      case _ => error[KXCFAEV](s"$fun is not a function nor a continuation")
    })
    case If0(cond, thenE, elseE) => interp(cond, env, tryCont, _ match {
      case NumV(0) => interp(thenE, env, tryCont, cont)
      case NumV(_) => interp(elseE, env, tryCont, cont)
      case ThrowV => tryCont(ThrowV)
      case _ => error[KXCFAEV](s"$cond is not a number")
    })
    case Withcc(name, body) => interp(body, env + (name -> ConV(cont)), tryCont, cont)
    case Try(tryE, catchE) => interp(tryE, env, _ match {
      case ThrowV => interp(catchE, env, tryCont, cont)
      case v => cont(v)
    }, cont)
    case Throw => tryCont(ThrowV)
  }

  def run(str : String) : String  = {
    interp(KXCFAE(str), Map(), (x : KXCFAEV) => x, (x : KXCFAEV) => x) match{
      case NumV(n) => n.toString
      case CloV(_, _, _) => "function"
      case ConV(_) => "continuation"
      case v => error[String](s"$v escaped, there is no enclosing try-catch")
    }
  }

  def ownTests: Unit = {
    test(run("{try 7 catch 8}"), "7")
    test(run("{try {throw} catch 8}"), "8")
    test(run("{try {+ 1 {throw}} catch 8}"), "8")
    test(run("{{fun {f} {try {f 3} catch 8}} {fun {x} {throw}}}"), "8")
    test(run("{try {try {throw} catch 8} catch 9}"), "8")
    test(run("{try {try {throw} catch {throw}} catch 9}"), "9")
    test(run("{try {try 7 catch {throw}} catch 9}"), "7")
    test(run("{{withcc esc {try {{withcc k {esc k}} 0} catch {fun {x} 8}}} {fun {x} {throw}}}"), "8")

    // multiple arguments [5]
    test(run("{{fun {x y} {- y x}} 10 12}"), "2")
    test(run("{fun {} 12}"), "function")
    test(run("{fun {x} {fun {} x}}"), "function")
    test(run("{{{fun {x} {fun {} x}} 13}}"), "13")
    test(run("{withcc esc {{fun {x y} x} 1 {esc 3}}}"), "3") //ERR

    // exceptions [35]
    test(run("{+ {withcc k {k 5}} 4}"), "9")
    test(run("{{fun {f x} {f f x}} {fun {g y} {if0 {- y 1} 1 {+ y {g g {- y 1}}}}} 10}"), "55") // recursive function
    test(run("{withcc done {{fun {f x} {f f x}} {fun {g y} {if0 {- y 1} {done 100} {+ y {g g {- y 1}}}}} 10}}"), "100") // exit from recursive function using continuation
    test(run("{withcc k {- 0 {k 100}}}"), "100")
    test(run("{withcc k {k {- 0 100}}}"), "-100")
    test(run("{withcc k {k {+ 100 11}}}"), "111")
    test(run("{{fun {a b c} {- {+ {withcc k {+ {k 100} a}} b} c}} 100 200 300}"), "0")
    test(run("{withcc esc {{fun {x y} x} 1 {esc 3}}}"), "3") //ERR
    test(run("{{withcc esc {{fun {x y} {fun {z} {+ z y}}} 1 {withcc k {esc k}}}} 10}"), "20")
    test(run("{try {{fun {f x} {f f x}} {fun {g y} {if0 {- y 1} {throw} {+ y {g g {- y 1}}}}} 10} catch 110}"), "110") // exit from recursive function using try-catch
    test(run("{{fun {f x} {f f x}} {fun {g y} {if0 {- y 1} {throw} {try {+ y {g g {- y 1}}} catch y}}} 10}"), "54") // equal? for multiple recursive try-catch
    test(run("{withcc done {{fun {f x} {f f x}} {fun {g y} {if0 {- y 1} {throw} {try {+ y {g g {- y 1}}} catch {done y}}}} 10}}"), "2")
    test(run("{try {{fun {f x} {f f x}} {fun {g y} {if0 {- y 1} {throw} {try {+ y {g g {- y 1}}} catch {throw}}}} 10} catch 20110464}"), "20110464") // recursive try-catch throwing ("1")
    test(run("{try {{fun {x y z} {a b c}} 1 2 {throw}} catch 0}"), "0") // ERR
    test(run("{{fun {f} {try {f 3} catch 8}} {fun {x} {throw}}}"), "8")
    test(run("{try {- 0 {withcc k {+ 3 {k {throw}}}}} catch 89}"), "89")
    test(run("{try {+ 3 {withcc k {+ 1000 {k {throw}}}}} catch 11}"), "11")
    test(run("{{fun {x y z} {try {+ 1 {+ x {throw}}} catch {+ y z}}} 1 2 3}"), "5")
    test(run("{+ {try {- 10 {throw}} catch 3} 10}"), "13")
    test(run("{try {if0 0 {throw} {+ 1 2}} catch {if0 10 1 {try {throw} catch 54}}}"), "54")
    test(run("{try {withcc a {+ 1 {withcc b {throw}}}} catch 10}"), "10")
    test(run("{try {- 0 {throw}} catch 5}"), "5")
    test(run("{try {if0 {throw} 3 4} catch 5}"), "5")
    test(run("{try {{fun {x y} {try x catch y}} {throw} 0} catch -1}"), "-1")
    test(run("{try {try {throw} catch {throw}} catch 9}"), "9")
    test(run("{{withcc esc {try {{withcc k {esc k}} 0} catch {fun {x} 8}}} {fun {x} {throw}}}"), "8")
    test(run("{{withcc esc {try {{withcc k {try {esc k} catch {fun {x} {fun {y} 9}}}} 0} catch {fun {x} 8}}} {fun {x} {throw}}}"), "8")
    test(run("{withcc foo {{fun {x y} {y x}} {+ 2 3} {withcc bar {+ 1 {bar foo}}}}}"), "5")
    test(run("{try {withcc zzz {{fun {x y z w} {+ {+ x y} {+ z w}}} 1 2 {zzz 10} {throw}}} catch 42}"), "10")//ERR
    test(run("{try {withcc zzz {{fun {x y z w} {+ {+ x y} {+ z w}}} 1 2 {throw} {zzz 10}}} catch 42}"), "42")//ERR
    test(run("{try {withcc zzz {{fun {x y z w} {+ {w {+ x y}} {+ {throw} z}}} 1 2 3 zzz}} catch 42}"), "3")
    test(run("{withcc esc {try {+ {throw} {esc 3}} catch 4}}"), "4")
    test(run("{withcc esc {{fun {x y} {+ {+ x 3} y}} {withcc k {try {k {esc {throw}}} catch {k 5}}} 7}}"), "15")
    test(run("{try {withcc x {+ {x 1} {throw}}} catch 0}"), "1")
    test(run("{+ 12 {withcc k {+ 1 {k {{fun {} 7}}}}}}"), "19")

    // multiple arguments [6]
    test(run("{+ 999 {withcc done {{fun {f x} {f f x done}} {fun {g y z} {if0 {- y 1} {z 100} {+ y {g g {- y 1} z}}}} 10}}}"), "1099")
    test(run("{+ 999 {withcc done {{fun {f x} {f f x {fun {x} {if0 x {done {- 0 999}} 10000}}}} {fun {g y z} {if0 {- y 1} {z 100} {+ y {g g {- y 1} z}}}} 10}}}"), "11053")
    test(run("{+ 999 {withcc done {{fun {f x} {f f x {fun {x} {if0 x {done {- 0 999}} 10000}}}} {fun {g y z} {if0 {- y 1} {z 0} {+ y {g g {- y 1} z}}}} 10}}}"), "0")
    test(run("{withcc done {{fun {f x} {f f x {fun {x} {if0 x {fun {y} {fun {x} {+ x y}}} 10000}}}} {fun {g y z} {if0 {- y 1} {z 0} {{g g {- y 1} z} 32}}} 3}}"), "64")
    test(run("{{withcc done {{fun {f x} {f f x {fun {x} {if0 x {withcc k {fun {x} {fun {x} {fun {x} k}}}} 10000}}}} {fun {g y z} {if0 {- y 1} {z 0} {{g g {- y 1} z} 32}}} 3}} 5}"), "continuation")
    test(run("{{withcc done {{fun {f x} {f f x {fun {x} {if0 x {withcc k {fun {x} {fun {x} {fun {x} k}}}} 10000}}}} {fun {g y z} {if0 {- y 1} {z 0} {{g g {- y 1} z} 32}}} 4}} {fun {y} {fun {y} {fun {y} {fun {x} 42}}}}}"), "42")

    // exceptions [4]
    test(run("{try {{withcc done {{fun {f x} {f f x {fun {x} {if0 x {withcc k {fun {x} {fun {x} {fun {x} k}}}} {throw}}}}} {fun {g y z} {if0 {- y 1} {z 1} {{g g {- y 1} z} 32}}} 4}} {fun {y} {fun {y} {fun {y} {fun {x} 42}}}}} catch 4242}"), "4242")
    test(run("{withcc esc {{try {withcc done {{fun {f x} {f f x {fun {x} {if0 x {withcc k {fun {x} {fun {x} {fun {x} k}}}} {throw}}}}} {fun {g y z} {if0 {- y 1} {z 1} {{g g {- y 1} z} 32}}} 4}} catch esc} 33}}"), "33")
    test(run("{try {try {throw} catch {try {throw} catch {try {throw} catch {+ {withcc k {try {throw} catch {k 0}}} {throw}}}}} catch 0}"), "0")
    test(run("{try {{withcc done {{fun {f x} {f f x {fun {x} {if0 x {withcc k {fun {x} {fun {x} {fun {x} k}}}} 10000}}}} {fun {g y z} {if0 {- y 1} {z 0} {{g g {- y 1} z} 32}}} 4}} {fun {y} {fun {y} {fun {y} {throw}}}}} catch 4242}"), "4242")

  }
}
