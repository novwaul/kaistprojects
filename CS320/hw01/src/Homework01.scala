trait Homework01 {
  // 1. Define the function "dollar2won", which consumes an integer number of dollars
  // and produces the won equivalent. Use the won/dollar conversion rate
  // of 1100 won per dollar.
  def dollar2won(dollar: Int): Int

  // 2. Write the function "volumeOfCuboid", which consumes three integer numbers
  // denoting lengths of three sides and produces the volume of the cuboid.
  def volumeOfCuboid(a: Int, b: Int, c: Int): Int

  // 3. Write the function "isEven", which consumes an integer number and returns
  // whether the number is even.
  def isEven(num: Int): Boolean

  // 4. Write the function "isOdd", which consumes an integer number and returns
  // whether the number is odd.
  def isOdd(num: Int): Boolean

  // 5. Write the function "gcd", which consumes two integer numbers and returns
  // the greatest common divisor of them.
  def gcd(a: Int, b: Int): Int

  // 6. Write the function "lcm", which consumes two integer numbers and returns
  // the least common multiple of them.
  def lcm(a: Int, b: Int): Int

  // You have a type "COURSE", which is either "CS320", "CS311", or "CS330".
  // "CS320" has two members: "quiz" for a number of quizzes and "homework"
  // for a number of programming assignments. CS311 has one member: "homework"
  // which is a number too. CS330 has two members: "projects" for a number of
  // projects and "homework" for a number of programming assignments.
  trait COURSE
  case class CS320(quiz: Int, homework: Int) extends COURSE
  case class CS311(homework: Int) extends COURSE
  case class CS330(projects: Int, homework: Int) extends COURSE

  // 7. Define the function numOfHomework, which consumes a course and produces
  // the number of programming assignments for the given course.
  def numOfHomework(course: COURSE): Int

  // 8. Define the function hasProjects, which consumes a course and produces
  // true only when the given course is CS330 with more than or equal to
  // two projects, otherwise produces false.
  def hasProjects(course: COURSE): Boolean

  // 9. Define the function namePets, which consumes a list of pets and produces
  // a corresponding list of pets with names; it names all occurrences of "dog"
  // with "happy", "cat" with "smart", "pig" with "pinky", and keeps the other pets
  // as unnamed. For example,
  //
  //   namePets(List("dog", "tiger", "cat")) == List("happy", "tiger", "smart")
  //
  def namePets(pets: List[String]): List[String]

  // 10. Generalize namePets to the function giveName. The new function consumes
  // two strings, called old and new. It produces a function that gets a list of
  // strings and replaces all occurrences of old by new in the list. For example,
  //
  //   val nameBears: List[String] => List[String] = giveName("bear", "pooh")
  //   nameBears(List("pig", "cat", "bear")) = List("pig", "cat", "pooh")
  //
  def giveName(oldName: String, newName: String): List[String] => List[String]

  // Write your own tests
  def ownTests: Unit
}
