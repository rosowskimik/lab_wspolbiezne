package main

import (
	"barrier/barrier"
	"fmt"
	"math/rand"
	"sync"
	"time"
)

func init() {
	rand.Seed(time.Now().Unix())
}

func main() {
	b := barrier.New(5)

	var wg sync.WaitGroup

	for i := 0; i < 5; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			time.Sleep(time.Duration(rand.Intn(4)) * time.Second)
			fmt.Printf("Thread %d arriving at barrier...\n", i)
			b.ArriveAndWait()
			fmt.Printf("Thread %d broke through the barrier...\n", i)
		}(i)
	}

	wg.Wait()

	for i := 0; i < 5; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			time.Sleep(time.Duration(rand.Intn(4)) * time.Second)
			fmt.Printf("Thread %d arriving at barrier...\n", i)
			b.ArriveAndWait()
			fmt.Printf("Thread %d broke through the barrier...\n", i)
		}(i)
	}
	wg.Wait()
}
