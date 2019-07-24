import scala.util.parsing.combinator._
import Util._

trait Homework03 {
  // FWAE type
  trait FWAE
  case class Num(num: Int) extends FWAE                                 // e ::= n
  case class Add(left: FWAE, right: FWAE) extends FWAE                  //     | {+ e e}
  case class Sub(left: FWAE, right: FWAE) extends FWAE                  //     | {- e e}
  case class With(name: String, value: FWAE, body: FWAE) extends FWAE   //     | {with {x e} e}
  case class Id(name: String) extends FWAE                              //     | x
  case class App(func: FWAE, args: List[FWAE]) extends FWAE             //     | {e e*}
  case class Fun(params: List[String], body: FWAE) extends FWAE         //     | {fun {x*} e}
  case class Rec(rec: Map[String, FWAE]) extends FWAE                   //     | {record {x e}*}
  case class Acc(expr: FWAE, name: String) extends FWAE                 //     | {access e x}

  // Check duplicated string values in a given string list.
  def dupCheck(ss: List[String]): Boolean = ss match {
    case h :: t => (t contains h) || dupCheck(t)
    case Nil => false
  }

  // Parser for FWAE
  object FWAE extends RegexParsers {
    def wrap[T](rule: Parser[T]): Parser[T] = "{" ~> rule <~ "}"
    lazy val int: Parser[Int] = """-?\d+""".r ^^ (_.toInt)
    lazy val str: Parser[String] = """[a-zA-Z0-9]+""".r
    lazy val expr: Parser[FWAE] =
      int                                       ^^ { case n => Num(n) }                        |
      wrap("+" ~> expr ~ expr)                  ^^ { case l ~ r => Add(l, r) }                 |
      wrap("-" ~> expr ~ expr)                  ^^ { case l ~ r => Sub(l, r) }                 |
      wrap("with" ~> wrap(str ~ expr) ~ expr)   ^^ { case x ~ i ~ b => With(x, i, b) }         |
      str                                       ^^ { case x => Id(x) }                         |
      wrap("fun" ~> wrap(rep(str)) ~ expr)      ^^ { case ps ~ b =>
        if (dupCheck(ps)) error(s"bad syntax: duplicate parameters: $ps")
        else Fun(ps, b)
      } |
      wrap("record" ~> rep(wrap(str ~ expr)))   ^^ { case ps =>
        val pList = ps.map { case f ~ e => (f,e) }
        if (dupCheck(pList.map { case (f, e) => f })) error(s"duplicate fields: $pList")
        Rec(pList.toMap)
      } |
      wrap("access" ~> expr ~ str)              ^^ { case e ~ x => Acc(e, x) }                 |
      wrap(expr ~ rep(expr))                    ^^ { case f ~ as => App(f, as) }
    def apply(str: String): FWAE = parse(expr, str).getOrElse(error(s"bad syntax: $str"))
  }

  // Evaluate a FWAE program contained in a string
  def run(str: String): String

  // Write your own tests
  def ownTests: Unit
}
