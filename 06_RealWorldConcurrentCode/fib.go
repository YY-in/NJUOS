// Example from "The Go Programming Language"

package main

import (
  "fmt"
  "time"
)

func main() {
  go spinner(100 * time.Millisecond)
  const n = 45
  fibN := fib(n) // slow
  fmt.Printf("\rFibonacci(%d) = %d\n", n, fibN)
}


func spinner(delay time.Duration) {
  // deadloop
  for {
	// print character in turn 
    for _, r := range `-\|/` {
		// \r is used to returning the begining of the line
      fmt.Printf("\r%c", r)
      time.Sleep(delay)
    }
  }
}

//Solve Fibonacci with violence
func fib(x int) int {
  if x < 2 { return x }
  return fib(x - 1) + fib(x - 2)
}
