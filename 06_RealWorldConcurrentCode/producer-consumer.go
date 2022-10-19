package main

import (

	"fmt"

)

//  定义一个全局的channel
var stream = make(chan int, 10)
const n = 4

func produce() {
  for i := 0; ; i++ {
    fmt.Println("produce", i)
	// 向channel中写入数据
    stream <- i
  }
}

func consume() {
  for {
	// 从channel中读取数据
    x := <- stream
    fmt.Println("consume",x)
  }
}

func main() {
  // 产生n个生产者的协程
  for i := 0; i < n; i++ {
	// create a new grountine that call produce(),don't wait for it to finish
    go produce()
  }
  consume()
}
