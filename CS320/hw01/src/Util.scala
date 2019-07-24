import scala.reflect.ClassTag

object Util {
  // Tests
  def test[T](input: =>T, output: =>T): Unit = try {
    if (input == output) println("PASS")
    else println(s"FAIL: $input is not equal to $output")
  } catch {
    case e: Throwable => println(s"FAIL: ${e.getMessage}")
  }

  // Define errors
  case class PLError(msg: String) extends Error(s"[ERROR] $msg")
  def error[T](msg: String): T = throw PLError(msg)

  // Tests for exceptions
  def testExc[T](input: =>T, msg: String): Unit = try {
    println(s"FAIL: it should throw an error but result is $input")
  } catch {
    case e: Throwable =>
      val eMsg = e.getMessage
      if (eMsg.contains(msg)) println("PASS")
      else println(s"""FAIL: "$eMsg" does not contain "$msg"""")
  }

  // Cast types with error messages for failure cases
  def cast[T](
    v: Any,
    msg: String
  )(implicit tag: ClassTag[T]): T = v match {
    case v: T => v
    case v => error(msg)
  }
}
