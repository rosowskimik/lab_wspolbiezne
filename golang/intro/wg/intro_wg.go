package main

import (
	"fmt"
	"sync"
	"time"
)

func say(s string, wg *sync.WaitGroup) {
	defer wg.Done()

	for i := 0; i < 5; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()

			time.Sleep(100 * time.Millisecond)
			fmt.Print(s)
		}()
	}
}

func main() {
	var wg sync.WaitGroup

	wg.Add(1)
	go say("world!\n", &wg)
	fmt.Print("Hello ")
	wg.Wait()
}
