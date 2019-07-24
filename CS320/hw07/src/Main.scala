import Util._
/* 20160788 InJe Hwang */
object Main extends Homework07 {

  trait CORELV
  case class NumV(n : Int) extends CORELV
  case class BoolV(n : Boolean) extends CORELV
  case class CloV(pararm : String, body : COREL, var fenv : Env) extends CORELV
  case class ConstructorV(x : String) extends CORELV
  case class VariantV(x : String, v : CORELV) extends CORELV

  type Env = Map[String, CORELV]

  case class TypeEnv(vars : Map[String, Type] = Map(), tbinds : Map[String, Map[String, Type]] = Map())
  {
    def addVar(x: String, ty : Type) : TypeEnv = copy(vars = vars + (x -> ty))
    def addTBind(x : String, cons : Map[String, Type]) = copy(tbinds = tbinds + (x -> cons))
  }

  def mustSame(lty : Type, rty : Type) : Type = {
    if(same(lty, rty)) lty
    else notype(s"$lty is not equal to $rty")
  }

  def typeChange(target : Type, from : Type, to : Type) : Type =
    target match{
      case NumT => NumT match {
        case `from` => to
        case _ => NumT
      }
      case BoolT => BoolT match {
        case `from` => to
        case _ => BoolT
      }
      case ArrowT(p, r) =>{
        val newP = {
          if(same(p, from)) to
          else typeChange(p, from, to)
        }
        val newR = {
          if (same(r, from)) to
          else typeChange(r, from, to)
        }
        ArrowT(newP, newR)
      }
      case PolyT(f, b) => {
        if(same(IdT(f), from)) PolyT(f, b)
        else PolyT(f, typeChange(b, from, to))
      }
      case IdT(a) => {
        if(same(IdT(a), from)) to
        else IdT(a)
      }
    }

  def same(lty : Type, rty : Type) : Boolean =
    (lty, rty) match{
      case (NumT, NumT) => true
      case (BoolT, BoolT) => true
      case (ArrowT(p1, r1), ArrowT(p2, r2)) => same(p1, p2) && same(r1, r2)
      case (PolyT(f1, b1), PolyT(f2, b2)) => {
        val newType = s"-$f1"
        same(typeChange(b1, IdT(f1), IdT(newType)), typeChange(b2, IdT(f2), IdT(newType)))
      }
      case (IdT(a), IdT(b)) => a == b
      case _ => false
    }

  def notype(msg : Any): Type = error[Type](s"no type: $msg")

  def validType(ty : Type, tyEnv : TypeEnv) : Type =
    ty match {
      case NumT => NumT
      case BoolT => BoolT
      case ArrowT(param, result) => ArrowT(validType(param, tyEnv),validType(result, tyEnv))
      case PolyT(forall, body) => PolyT(forall, validType(body, tyEnv.addTBind(forall, Map())))
      case IdT(name) => {
        if(tyEnv.tbinds.contains(name)) ty
        else notype(s"$name is a free type")
      }
      case _ => error[Type]("Cannot happen")
    }

  def Tcheck(expr : COREL, tyEnv : TypeEnv) : Type =
    expr match{
      case Num(_) => NumT
      case Bool(_) => BoolT
      case Add(l,r) =>{
        mustSame(Tcheck(l, tyEnv), NumT)
        mustSame(Tcheck(r, tyEnv), NumT)
      }
      case Sub(l,r) => {
        mustSame(Tcheck(l, tyEnv), NumT)
        mustSame(Tcheck(r, tyEnv), NumT)
      }
      case Equ(l,r) => {
        mustSame(Tcheck(l, tyEnv), NumT)
        mustSame(Tcheck(r, tyEnv), NumT)
        BoolT
      }
      case Id(name) => tyEnv.vars.getOrElse(name, notype(s"$name is a free type"))
      case Fun(param, paramty, body) => {
        validType(paramty, tyEnv)
        ArrowT(paramty, Tcheck(body, tyEnv.addVar(param, paramty)))
      }
      case App(funE, argE) =>
        Tcheck(funE, tyEnv) match {
          case ArrowT(param, result) => {
            val argT = Tcheck(argE, tyEnv)
            mustSame(argT, param)
            result
          }
          case _ => notype(s"$funE is not an arrow type")
        }
      case IfThenElse(testE, thenE, elseE) =>
        Tcheck(testE, tyEnv) match{
          case BoolT => mustSame(Tcheck(thenE, tyEnv), Tcheck(elseE, tyEnv))
          case _ => notype(s"$testE is not a bool type")
        }
      case Rec(fname, ftype, pname, ptype, body) => {
        val tyEnvA = tyEnv.addVar(fname, validType(ftype, tyEnv))
        val tyEnvB = tyEnvA.addVar(pname, validType(ptype, tyEnvA))
        mustSame(ftype, ArrowT(ptype, Tcheck(body, tyEnvB)))
      }
      case WithType(name, constructors, body) => {
        val tyEnvA = tyEnv.addTBind(name, constructors)
        constructors.foreach{ case (consName, inputTy) => validType(inputTy, tyEnvA) }
        val tyEnvAdd : Map[String, Type] = constructors.map{
          case (consName, inputTy) => (consName, ArrowT(inputTy, IdT(name)))
        }
        val NewVar : Map[String, Type] = tyEnvA.vars ++ tyEnvAdd
        val tyEnvB = tyEnvA.copy(vars = NewVar)
        val ty = Tcheck(body, tyEnvB)
        ty match{
          case(IdT(name)) => notype(s"$name escapes out of WithType")
          case _ => ty
        }
      }
      case Cases(name, dispatchE, cases) => {
        validType(IdT(name), tyEnv)
        mustSame(Tcheck(dispatchE, tyEnv), IdT(name))
        val nameDef : Map[String, Type] = tyEnv.tbinds.getOrElse(name, error[Map[String, Type]](s"no type: $name is a free type"))
        val (cons, (str, ex)) = cases.head
        val inputType = nameDef.getOrElse(cons, notype(s"cannot fine $cons"))
        val refType = Tcheck(ex, tyEnv.addVar(str, inputType))
        cases.foreach{
          case (cons, (str, ex)) => {
            val inputType = nameDef.getOrElse(cons, notype(s"cannot fine $cons"))
            mustSame(Tcheck(ex, tyEnv.addVar(str,inputType)), refType)
          }
        }
        refType
      }
      case TFun(name, ex) => {
        val tyEnvA = tyEnv.addTBind(name, Map())
        PolyT(name,Tcheck(ex, tyEnvA))
      }
      case TApp(body, ty) => {
        validType(ty, tyEnv)
        Tcheck(body, tyEnv) match{
          case PolyT(f, b) => typeChange(b, IdT(f), ty)
          case _ => notype(s"$body is not a PolyT")
        }
      }
    }

    def interp(expr : COREL, env : Env) : CORELV =
      expr match{
        case Num(n) => NumV(n)
        case Bool(b) => BoolV(b)
        case Add(l,r) => (interp(l,env),interp(r,env)) match {
          case (NumV(lv), NumV(rv)) => NumV(lv + rv)
          case _ => error[CORELV](s"$l or $r is not NumV")
        }
        case Sub(l,r) => (interp(l,env),interp(r,env)) match {
          case (NumV(lv), NumV(rv)) => NumV(lv - rv)
          case _ => error[CORELV](s"$l or $r is not NumV")
        }
        case Equ(l,r) => (interp(l,env),interp(r,env)) match {
          case(NumV(lv),NumV(rv)) => {
            if(lv == rv) BoolV(true)
            else BoolV(false)
          }
          case _ => error[CORELV](s"$l or $r is not NumV")
        }
        case Id(x) => env.getOrElse(x, error[CORELV](s"$x is a free identifier"))
        case Fun(param, _ , body) => CloV(param, body, env)
        case App(funE, argE) => interp(funE, env) match{
          case CloV(param, body, fenv) => {
            val argV = interp(argE, env)
            interp(body, fenv + (param -> argV))
          }
          case ConstructorV(x) => {
            val argV = interp(argE, env)
            VariantV(x, argV)
          }
          case _ => error[CORELV](s"$funE is not a CloV nor a ConstructorV")
        }
        case IfThenElse(testE, thenE, elseE) => interp(testE, env) match{
          case BoolV(true) => interp(thenE, env)
          case BoolV(false) =>interp(elseE, env)
          case _ => error[CORELV](s"$testE is not a BoolV")
        }
        case Rec(fname, _ , pname , _ , body) => {
          val tempCloV = CloV(pname, body, env)
          tempCloV.fenv = env + (fname -> tempCloV)
          tempCloV
        }
        case WithType(_ , constructors, body) => {
          val AddEnv = constructors.map{ case (x, v) => (x, ConstructorV(x)) }
          interp(body, env ++ AddEnv)
        }
        case Cases(_, dispatchE, cases) => {
          val Var = interp(dispatchE, env)
          Var match {
            case VariantV(x, _) =>{
              val (str, ex) : (String, COREL) = cases.getOrElse(x, error[(String, COREL)](s"cannot find a matched cases with this constructor $x"))
              interp(ex, env)
            }
            case _ => error[CORELV](s"$dispatchE is not a VariantV")
          }
        }
        case TFun(_ , ex) => interp(ex, env)
        case TApp(body, _ ) => interp(body, env)
      }

    def run(str : String) : String = {
      interp(COREL(str), Map()) match {
        case NumV(n) => n.toString
        case BoolV(b) => b.toString
        case CloV(a,b,c) => "function"
        case ConstructorV(x) => "constructor"
        case VariantV(str, v) => "variant"
        case _ => error[String]("Cannot happen")
      }

    }

    def typeCheck(expr : String) : Type = {
      val tyEnv = TypeEnv()
      Tcheck(COREL(expr), tyEnv)
    }

    def ownTests : Unit = {
      // type check
      test(typeCheck("{tyfun {a} 3}"), Type("{^ a num}"))
      test(typeCheck("{tyfun {a} {tyfun {b} 3}}"), Type("{^ a {^ b num}}"))
      test(typeCheck("{tyfun {a} {fun {x: a} x}}"), Type("{^ a {a -> a}}"))
      test(typeCheck("{tyfun {a} {tyfun {b} {fun {x: {^ a {^ b a}}} x}}}"), Type("{^ a {^ b {{^ a {^ b a}} -> {^ a {^ b a}}}}}"))
      test(typeCheck("{@ {tyfun {a} {tyfun {b} {fun {x: {^ a {^ b a}}} x}}} num}"), Type("{^ b {{^ a {^ b a}} -> {^ a {^ b a}}}}"))
      test(typeCheck("{fun {x: {^ a a}} x}"), Type("{{^ a a} -> {^ a a}}"))
      testExc(typeCheck("{fun {x: {^ a {a -> b}}} x}"), "free")
      testExc(typeCheck("{tyfun {a} {fun {x: b} x}}"), "free")
      testExc(typeCheck("{@ {tyfun {a} {fun {x: b} x}} num}"), "free")
      testExc(typeCheck("{tyfun {a} {fun {x: a} {tyfun {b} {fun {y: b} {+ x y}}}}}"), "no")
      test(typeCheck("{tyfun {a} 3}"), Type("{^ a num}"))
      test(typeCheck("{tyfun {a} {tyfun {b} 3}}"), Type("{^ a {^ b num}}"))
      test(typeCheck("{tyfun {a} {fun {x: a} x}}"), Type("{^ a {a -> a}}"))
      test(typeCheck("{@ {tyfun {a} {fun {x: a} x}} {^ b {b -> b}}}"), Type("{{^ b {b -> b}} -> {^ b {b -> b}}}"))
      test(typeCheck("{tyfun {a} {tyfun {b} 3}}"), Type("{^ a {^ b num}}"))
      test(typeCheck("{tyfun {a} {fun {x: a} x}}"), Type("{^ a {a -> a}}"))
      test(typeCheck("{tyfun {a} {tyfun {b} {fun {x: a} x}}}"), Type("{^ a {^ b {a -> a}}}"))
      test(typeCheck("{if true {tyfun {a} {fun {x: a} x}} {tyfun {b} {fun {y: b} y}}}"), Type("{^ a {a -> a}}"))
      test(typeCheck("{if true {tyfun {b} {fun {y: b} y}} {tyfun {a} {fun {x: a} x}}}"), Type("{^ b {b -> b}}"))
      test(typeCheck("{if {= 8 8} {tyfun {a} {tyfun {b} {fun {x: a} x}}} {tyfun {b} {tyfun {a} {fun {x: b} x}}}}"), Type("{^ a {^ b {a -> a}}}"))
      test(typeCheck("{tyfun {a} {fun {x: a} {tyfun {b} {fun {y: a} {if true x y}}}}}"), Type("{^ a {a -> {^ b {a -> a}}}}"))
      test(typeCheck("{tyfun {a} {fun {a: {num -> num}} {fun {x: a} x}}}"), Type("{^ a {{num -> num} -> {a -> a}}}"))
      test(typeCheck("{fun {a: {num -> num}} {tyfun {a} {fun {x: a} x}}}"), Type("{{num -> num} -> {^ a {a -> a}}}"))
      test(typeCheck("{@ {tyfun {a} {fun {x: {^ a {a -> a}}} x}} num}"), Type("{{^ a {a -> a}} -> {^ a {a -> a}}}"))
      test(typeCheck("{@ {tyfun {a} {fun {x: a} 5}} num}"), Type("{num -> num}"))
      test(typeCheck("{if {= 8 10} {tyfun {a} {tyfun {b} {fun {x: a} {fun {y: b} y}}}} {tyfun {b} {tyfun {a} {fun {x: b} {fun {y: a} y}}}}}"), Type("{^ a {^ b {a -> {b -> b}}}}"))
      test(typeCheck("{@ {tyfun {a} {fun {a: a} {{fun {x: {^ a {a -> a}}} {{@ x num} 10}} {tyfun {b} {fun {b: b} b}}}}} {num -> num}}"), Type("{{num -> num} -> num}"))
      test(typeCheck("{@ {tyfun {a} {fun {a: a} {{fun {x: {^ a {a -> a}}} {{@ x num} 10}} {tyfun {b} {fun {b: b} b}}}}} num}"), Type("{num -> num}"))
      test(typeCheck("{@ {tyfun {a} {fun {a: a} {{fun {x: {^ a {a -> a}}} {{@ x num} 10}} {tyfun {a} {fun {a: a} a}}}}} num}"), Type("{num -> num}"))
      test(typeCheck("{tyfun {a} 3}"), Type("{^ a num}"))
      test(typeCheck("{tyfun {a} {tyfun {b} 3}}"), Type("{^ a {^ b num}}"))
      test(typeCheck("{tyfun {a} {fun {x: a} x}}"), Type("{^ a {a -> a}}"))
      test(typeCheck("{if true {tyfun {a} {fun {x: a} x}} {tyfun {b} {fun {y: b} y}}}"), Type("{^ a {a -> a}}"))
      test(typeCheck("{if true {tyfun {b} {fun {y: b} y}} {tyfun {a} {fun {x: a} x}}}"), Type("{^ b {b -> b}}"))
      test(typeCheck("{if true {tyfun {a} {tyfun {b} {fun {x: a} x}}} {tyfun {b} {tyfun {a} {fun {x: b} x}}}}"), Type("{^ a {^ b {a -> a}}}"))
      test(typeCheck("{tyfun {a} {fun {x: a} {tyfun {b} {fun {y: a} {if true x y}}}}}"), Type("{^ a {a -> {^ b {a -> a}}}}"))
      test(typeCheck("{fun {x: {^ a a}} x}"), Type("{{^ a a} -> {^ a a}}"))
      test(typeCheck("{@ {tyfun {a} {tyfun {b} {fun {x: {^ a {^ b a}}} x}}} num}"), Type("{^ b {{^ a {^ b a}} -> {^ a {^ b a}}}}"))
      test(typeCheck("{{@ {@ {tyfun {a} {tyfun {b} {fun {x: a} x}}} num} num} 10}"), Type("num"))
      test(typeCheck("{withtype {foo {a num} {b num}} {cases foo {a 3} {a {n} {+ n 3}} {b {n} {+ n 4}}}}"), Type("num"))

      test(run("{{{@ {tyfun {a} {fun {f: a} f}} {num -> num}} {fun {x: num} x}} 10}"), "10")
      test(run("{@ {tyfun {a} {fun {f: a} f}} {num -> num}}"), "function")
      test(run("{@ {@ {tyfun {a} {tyfun {b} 3}} num} num}"), "3")
      test(run("{tyfun {a} {fun {x: b} x}}"), "function")
      test(run("{{fun {x: num} {{fun {f: {num -> num}} {+ {f 1} {{fun {x: num} {f 2}} 3}}} {fun {y: num} {+ x y}}}} 0}"), "3")
      test(run("{@ {tyfun {a} {fun {x: a} x}} num}"), "function")
      test(run("{tyfun {a} {tyfun {b} 3}}"), "3")
      test(run("{{{@ {tyfun {a} {fun {f: a} f}} {num  -> num}} {fun {x: num} x}} 10}"), "10")
      test(run("{@ {tyfun {a} {fun {f: a} f}} {num -> num}}"), "function")
      test(run("{@ {@ {tyfun {a} {tyfun {b} 3}} num} num}"), "3")
      test(run("{@ {tyfun {a} {fun {f: a} f}} {num -> num}}"), "function")
      test(run("{{@ {if true {tyfun {a} {fun {x: a} x}} {tyfun {b} {fun {x: b} b}}} x} 30}"), "30")
      test(run("{{fun {x: {^ a {a -> a}}} {{@ x num} 10}} {tyfun {b} {fun {y: b} y}}}"), "10")
      test(run("{@ {tyfun {a} {fun {x: a} 5}} num}"), "function")
      test(run("{@ {tyfun {a} {fun {x: {^ a {a -> a}}} x}} num}"), "function")
      test(run("{{{@ {@ {tyfun {a} {tyfun {b} {fun {x: {a -> b}} x}}} num} num} {fun {x: num} {+ 5 x}}} 3}"), "8")
      test(run("{{{@ {@ {tyfun {a} {tyfun {a} {fun {x: {a -> a}} x}}} num} num} {fun {x: num} {+ 5 x}}} 3}"), "8")
      test(run("{@ {@ {tyfun {a} {tyfun {b} 3}} num} num}"), "3")
      test(run("{{@ {@ {tyfun {a} {tyfun {b} {fun {x: a} x}}} num} num} 10}"), "10")
      test(run("{with {f: {num -> num} {recfun {f: {num -> num} x: num} {if {= x 0} 0 {+ {f {- x 1}} x}}}} {f 10}}"), "55")
    }
}
