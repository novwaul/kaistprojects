import scala.util.parsing.combinator._
import Util._

trait Homework04 {
  // BFAE type
  trait BFAE
  case class Num(num: Int) extends BFAE                                 // e ::= n
  case class Add(left: BFAE, right: BFAE) extends BFAE                  //     | {+ e e}
  case class Sub(left: BFAE, right: BFAE) extends BFAE                  //     | {- e e}
  case class Id(name: String) extends BFAE                              //     | x
  case class Fun(param: String, body: BFAE) extends BFAE                //     | {fun {x} e}
  case class App(fun: BFAE, arg: BFAE) extends BFAE                     //     | {e e}
  case class NewBox(expr: BFAE) extends BFAE                            //     | {newbox e}
  case class SetBox(box: BFAE, expr: BFAE) extends BFAE                 //     | {setbox e e}
  case class OpenBox(box: BFAE) extends BFAE                            //     | {openbox e}
  case class Seqn(left: BFAE, right: List[BFAE]) extends BFAE           //     | {seqn e e*}
  case class Rec(fields: Map[String, BFAE]) extends BFAE                //     | {rec {<id> <BFAE>}*}
  case class Get(record: BFAE, field: String) extends BFAE              //     | {get <BFAE> <id>}
  case class Set(record: BFAE, field: String, expr: BFAE) extends BFAE  //     | {set <BFAE> <id> <BFAE>}

  // Parser for BFAE
  object BFAE extends RegexParsers {
    def wrap[T](rule: Parser[T]): Parser[T] = "{" ~> rule <~ "}"
    lazy val int: Parser[Int] = """-?\d+""".r ^^ (_.toInt)
    lazy val str: Parser[String] = """[a-zA-Z0-9]+""".r
    lazy val expr: Parser[BFAE] =
      int                                       ^^ { case n => Num(n) }                        |
      wrap("+" ~> expr ~ expr)                  ^^ { case l ~ r => Add(l, r) }                 |
      wrap("-" ~> expr ~ expr)                  ^^ { case l ~ r => Sub(l, r) }                 |
      str                                       ^^ { case x => Id(x) }                         |
      wrap("fun" ~> wrap(str) ~ expr)           ^^ { case p ~ b => Fun(p, b) }                 |
      wrap("newbox" ~> expr)                    ^^ { case e => NewBox(e) }                     |
      wrap("setbox" ~> expr ~ expr)             ^^ { case b ~ e => SetBox(b, e) }              |
      wrap("openbox" ~> expr)                   ^^ { case e => OpenBox(e) }                    |
      wrap("seqn" ~> expr ~ rep(expr))          ^^ { case l ~ rs => Seqn(l, rs) }              |
      wrap("rec" ~> rep(wrap(str ~ expr)))      ^^ { case f =>
        Rec(f.foldLeft(Map[String, BFAE]()) { case (map, f ~ e) => map + (f -> e) })
      }                                                                                        |
      wrap("get" ~> expr ~ str)                 ^^ { case r ~ f => Get(r, f) }                 |
      wrap("set" ~> expr ~ str ~ expr)          ^^ { case r ~ f ~ e => Set(r, f, e) }          |
      wrap(expr ~ expr)                         ^^ { case f ~ a => App(f, a) }
    def apply(str: String): BFAE = parse(expr, str).getOrElse(error(s"bad syntax: $str"))
  }

  // Evaluate a BFAE program contained in a string
  def run(str: String): String

  // Write your own tests
  def ownTests: Unit
}
