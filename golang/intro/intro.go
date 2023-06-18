package main

import (
	"fmt"
	"time"
)

func say(s string) <-chan struct{} {
	ch := make(chan struct{}, 1)

	go func() {
		for i := 0; i < 5; i++ {
			time.Sleep(100 * time.Millisecond)
			fmt.Print(s)
		}
		ch <- struct{}{}
	}()

	return ch
}

func main() {
	ch := say("world!\n")
	fmt.Print("Hello ")
	<-ch
}
