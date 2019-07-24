import scala.util.parsing.combinator._
import Util._

trait Homework02 {
  // WAE abstract syntax trees
  trait WAE
  case class Num(num: Int) extends WAE                                 // e ::= n
  case class Add(left: WAE, right: WAE) extends WAE                    //     | {+ e e}
  case class Sub(left: WAE, right: WAE) extends WAE                    //     | {- e e}
  case class With(name: String, expr: WAE, body: WAE) extends WAE      //     | {with {x e} e}
  case class Id(id: String) extends WAE                                //     | x

  // Parser for WAE
  object WAE extends RegexParsers {
    def wrap[T](rule: Parser[T]): Parser[T] = "{" ~> rule <~ "}"
    lazy val int: Parser[Int] = """-?\d+""".r ^^ (_.toInt)
    lazy val str: Parser[String] = """[a-zA-Z0-9]+""".r
    lazy val expr: Parser[WAE] =
      int                                       ^^ { case n => Num(n) }                        |
      wrap("+" ~> expr ~ expr)                  ^^ { case l ~ r => Add(l, r) }                 |
      wrap("-" ~> expr ~ expr)                  ^^ { case l ~ r => Sub(l, r) }                 |
      wrap("with" ~> wrap(str ~ expr) ~ expr)   ^^ { case x ~ i ~ b => With(x, i, b) }         |
      str                                       ^^ { case x => Id(x) }
    def apply(str: String): WAE = parse(expr, str).getOrElse(error(s"bad syntax: $str"))
  }

  // 1. Implement the function freeIds, which takes a WAE and produces
  // a set of strings. The set should contain a string for each free
  // identifier in the given WAE.
  def freeIds(expr: WAE): Set[String]

  // 2. Implement the function bindingIds, which is like freeIds,
  // but the result set contains a string for each binding identifier
  // in the given WAE (whether or not the binding identifier is ever
  // referenced by a bound identifier).
  def bindingIds(expr: WAE): Set[String]

  // 3. Implement the function boundIds, which is like freeIds,
  // but the result list contains a string for each bound identifier
  // in the given WAE.
  def boundIds(expr: WAE): Set[String]

  // Write your own tests
  def ownTests: Unit
}
