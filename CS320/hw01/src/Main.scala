/* 20160788 InJe Hwang */
import Util._
object Main extends Homework01 {
  /* TODO Implement 10 missing functions */
  def dollar2won(dollar: Int): Int = {
    if(dollar < 0) error[Unit]("Error")
    1100*dollar
  }
  def volumeOfCuboid(a: Int, b: Int, c: Int): Int = {
    if(a < 0 || b < 0 || c < 0) error[Unit]("Error")
    a*b*c
  }
  def isEven(num: Int): Boolean = {
    if(num%2 == 0) true
    else false
  }
  def isOdd(num: Int): Boolean = {
    !isEven(num)
  }
  def gcd(a: Int, b: Int): Int = {
    /* seek greatest commom divisor by checking the divisors of smallNum */
    def seek_gcd(divisor: Int, operand: Int, smallNum: Int) : Int = {
      if(divisor == 0) operand // special case of gcd(0,0): by mathematical reasons,
      // 0 which is the top element in divisibility order should be gcd of 0 and 0
      else if(divisor == 1) divisor
      else if(operand % divisor == 0) divisor
      else seek_gcd( seek_next_divisor(divisor, smallNum), operand, smallNum)
    }
    /* seek divisor of smallNum for seek_gcd function */
    def seek_next_divisor(start_point: Int, smallNum: Int) : Int = {
      val divisor : Int = start_point - 1
      if(divisor == 1) divisor
      else if(smallNum % divisor == 0) divisor
      else seek_next_divisor(divisor, smallNum)
    }
    val abs_a : Int = {
      if(a < 0) -a
      else a
    }
    val abs_b : Int = {
      if(b < 0) -b
      else b
    }
    val smallNum : Int = {
      if(abs_a < abs_b) abs_a
      else abs_b
    }
    val bigNum : Int = {
      if(abs_a < abs_b) abs_b
      else abs_a
    }
    seek_gcd(smallNum, bigNum, smallNum)
  }
  def lcm(a: Int, b: Int): Int = {
    /* seek least common multiple by using gcd */
    def seek_lcm(smallNum: Int, bigNum: Int, factor: Int, gcd: Int): Int = {
      if (smallNum == 0) 0
      else if(factor*gcd % smallNum == 0 && factor*gcd % bigNum == 0) {factor*gcd}
      else seek_lcm(smallNum, bigNum, factor+1, gcd)
    }
    val abs_a : Int = {
      if(a < 0) -a
      else a
    }
    val abs_b : Int = {
      if(b < 0) -b
      else b
    }
    val smallNum : Int = {
      if(abs_a < abs_b) abs_a
      else abs_b
    }
    val bigNum : Int = {
      if(abs_a < abs_b) abs_b
      else abs_a
    }
    val greatest_common_divisor : Int = gcd(smallNum, bigNum)
    seek_lcm(smallNum, bigNum, 1 , greatest_common_divisor)
  }
  def numOfHomework(course: COURSE): Int = {
    course match {
      case CS320(quiz, homework) if(quiz < 0 || homework < 0) => error[Int]("Error")
      case CS311(homework) if(homework < 0) => error[Int]("Error")
      case CS330(projects, homework) if(projects < 0 || homework < 0) => error[Int]("Error")
      case CS320(quiz, homework) => homework
      case CS311(homework) => homework
      case CS330(projects, homework) => homework
    }
  }
  def hasProjects(course: COURSE): Boolean = {
    course match {
      case CS320(quiz, homework) if(quiz < 0 || homework < 0) => error[Boolean]("Error")
      case CS311(homework) if(homework < 0) => error[Boolean]("Error")
      case CS330(projects, homework) if(projects < 0 || homework < 0) => error[Boolean]("Error")
      case CS330(projects, homework) if (projects >= 2) => true
      case _ => false
    }
  }
  def namePets(pets: List[String]): List[String] = {
    pets.map {
     case "dog" => "happy"
     case "cat" => "smart"
     case "pig" => "pinky"
     case x => x
   }
  }
  def giveName(oldName: String, newName: String): List[String] => List[String] = (input_list: List[String]) => {
    input_list.map{
      case `oldName` => `newName`
      case x => x
    }
  }
  def ownTests(): Unit = {
    /* TODO Write your own tests */
    val cs311_normal : COURSE = CS311(5)
    val cs311_abnormal : COURSE = CS311(-1)
    val cs320_normal : COURSE = CS320(2,0)
    val cs320_abnormal : COURSE = CS320(-4,10)
    val cs330_normal_over2 : COURSE = CS330(3,10)
    val cs330_normal_under2 : COURSE = CS330(1,10)
    val f : List[String] => List[String] = giveName("cat","dog")
    val g : List[String] => List[String] = giveName("cow","monkey")
    println("---------------------------------------")
    println("            Test Homework01            ")
    println("---------------------------------------")
    println("Test 'dollar2won'")
    printf("Case1: 1$ to 1100Won ... ")
    test[Int](1100,dollar2won(1))
    printf("Case2: invalid -1$ input ... ")
    testExc[Int](dollar2won(-1),"Error")
    println("---------------------------------------")
    println("Test 'volumeOfCuboid'")
    printf("Case1: positive lengths ... ")
    test[Int](6,volumeOfCuboid(1,2,3))
    printf("Case2: negative lengths ... ")
    testExc[Int](volumeOfCuboid(-1,2,3),"Error")
    println("---------------------------------------")
    println("Test 'isEven'")
    printf("Case1: even number ... ")
    test[Boolean](true,isEven(4))
    printf("Case2: odd number ... ")
    test[Boolean](false,isEven(11))
    printf("Case3: negative even number ... ")
    test[Boolean](true,isEven(-4))
    printf("Case4: negative odd number ... ")
    test[Boolean](false,isEven(-11))
    println("---------------------------------------")
    println("Test 'isOdd'")
    printf("Case1: even number ... ")
    test[Boolean](true,isEven(4))
    printf("Case2: odd number ... ")
    test[Boolean](false,isEven(11))
    printf("Case3: negative even number ... ")
    test[Boolean](true,isEven(-4))
    printf("Case4: negative odd number ... ")
    test[Boolean](false,isEven(-11))
    println("---------------------------------------")
    println("Test 'gcd'")
    printf("Case1: gcd of 4 and 6 ... ")
    test[Int](2,gcd(4,6))
    printf("Case2: gcd of 12 and 6 ... ")
    test[Int](6,gcd(12,6))
    printf("Case3: gcd of 7 and 7 ... ")
    test[Int](7,gcd(7,7))
    printf("Case4: gcd of -7 and 2 ... ")
    test[Int](1,gcd(-7,2))
    printf("Case5: gcd of -12 and 8 ... ")
    test[Int](4,gcd(-12,8))
    printf("Case6: gcd of -12 and 6 ... ")
    test[Int](6,gcd(-12,6))
    printf("Case7: gcd of -12 and -10 ... ")
    test[Int](2,gcd(-12,-10))
    printf("Case8: gcd of -12 and -3 ... ")
    test[Int](3,gcd(-12,-3))
    printf("Case9: gcd of 0 and 5 ... ")
    test[Int](5,gcd(0,5))
    printf("Case10: gcd of 0 and -5 ... ")
    test[Int](5,gcd(0,5))
    printf("Case11: gcd of 0 and 0 ... ")
    test[Int](0,gcd(0,0))
    println("---------------------------------------")
    println("Test 'lcm'")
    printf("Case1: lcm of 4 and 6 ... ")
    test[Int](12,lcm(4,6))
    printf("Case2: lcm of 12 and 6 ... ")
    test[Int](12,lcm(12,6))
    printf("Case3: lcm of 7 and 7 ... ")
    test[Int](7,lcm(7,7))
    printf("Case4: lcm of -7 and 2 ... ")
    test[Int](14,lcm(-7,2))
    printf("Case5: lcm of -12 and 8 ... ")
    test[Int](24,lcm(-12,8))
    printf("Case6: lcm of -12 and 6 ... ")
    test[Int](12,lcm(-12,6))
    printf("Case7: lcm of -12 and -10 ... ")
    test[Int](60,lcm(-12,-10))
    printf("Case8: lcm of -12 and -3 ... ")
    test[Int](12,lcm(-12,-3))
    printf("Case9: lcm of 0 and 5 ... ")
    test[Int](0,lcm(0,5))
    printf("Case10: lcm of 0 and -5 ... ")
    test[Int](0,lcm(0,-5))
    printf("Case10: lcm of 0 and 0 ... ")
    test[Int](0,lcm(0,0))
    println("---------------------------------------")
    println("Test 'numOfHomework'")
    printf("Case1: non-negative input ... ")
    test[Int](5,numOfHomework(cs311_normal))
    printf("Case2: negative input ... ")
    testExc[Int](numOfHomework(cs311_abnormal),"Error")
    printf("Case3: zero homework ... ")
    test[Int](0,numOfHomework(cs320_normal))
    println("---------------------------------------")
    println("Test 'hasProjects'")
    printf("Case1: negative input ... ")
    testExc[Boolean](hasProjects(cs320_abnormal),"Error")
    printf("Case2: not CS330 ... ")
    test[Boolean](false, hasProjects(cs311_normal))
    printf("Case3: CS330 over 2 projects ... ")
    test[Boolean](true,hasProjects(cs330_normal_over2))
    printf("Case4: CS330 under 2 projects ... ")
    test[Boolean](false,hasProjects(cs330_normal_under2))
    println("---------------------------------------")
    println("Test 'namePets'")
    printf("Case1: List( \"dog\", \"cat\", \"pig\", \"cow\", \"monkey\") ... ")
    test[List[String]](List("happy","smart","pinky","cow","monkey"),namePets(List("dog", "cat", "pig", "cow", "monkey")))
    println("---------------------------------------")
    println("Test 'giveName'")
    printf("Case1: cat to dog with List(\"dog\", \"cat\", \"cow\") ... ")
    test[List[String]](List("dog", "dog", "cow"),f(List("dog", "cat", "cow")))
    printf("Case2: cow to monkey with List(\"dog\", \"cat\", \"cow\") ... ")
    test[List[String]](List("dog", "cat", "monkey"),g(List("dog", "cat", "cow")))
    println("---------------------------------------")
    println("END")
  }

}
