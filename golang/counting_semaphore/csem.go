package main

import (
	"fmt"
	"sync"
	"time"
)

type Semaphore struct {
	counter int
	m       *sync.Mutex
	c       *sync.Cond
}

func New(size int) *Semaphore {
	m := sync.Mutex{}
	s := Semaphore{
		counter: size,
		c:       sync.NewCond(&m),
		m:       &m,
	}

	return &s
}

func (s *Semaphore) Post() {
	s.m.Lock()
	defer s.m.Unlock()

	s.counter += 1

	s.c.Signal()
}

func (s *Semaphore) Wait() {
	s.m.Lock()
	defer s.m.Unlock()

	for s.counter <= 0 {
		s.c.Wait()
	}

	s.counter -= 1
}

func (s *Semaphore) TryWait() bool {
	s.m.Lock()
	defer s.m.Unlock()

	if s.counter > 0 {
		s.counter -= 1
		return true
	}

	return false
}

func main() {
	s := New(3)

	var wg sync.WaitGroup

	for i := 0; i < 5; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			defer s.Post()

			if s.TryWait() {
				fmt.Printf("Call %v didn't block!\n", i)
				time.Sleep(1 * time.Second)
			} else {
				fmt.Printf("Call %v would block! Blocking now...\n", i)
				s.Wait()
				time.Sleep(1 * time.Second)
			}

			fmt.Printf("Call %v ending now...\n", i)
		}(i)
	}

	wg.Wait()
}
