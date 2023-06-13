package atoms

import (
	"fmt"
	"math/rand"
	"time"
)

type Atom string

const (
	Hydrogen Atom = "Hydrogen"
	Oxygen        = "Oxygen"
)

const (
	HydrogenProducers int = 8
	OxygenProducers       = 5
)

func producer(id int, atom Atom, ch chan<- struct{}) {
	fmt.Printf("Created %c%d %s producer\n", atom[0], id, atom)

	var r int
	for {
		r = rand.Intn(25000)
		time.Sleep(time.Duration(r) * time.Millisecond)
		ch <- struct{}{}
	}
}

func createAtom(oChan, hChan <-chan struct{}) {
	oCount, hCount := 0, 0

	oRecv := func() {
		oCount++
		fmt.Println("Received oxygen")
	}
	hRecv := func() {
		hCount++
		fmt.Println("Received hydrogen")
	}

	for {
		select {
		case <-oChan:
			oRecv()
		case <-hChan:
			hRecv()
		}

		if oCount == 0 {
			if hCount < 2 {
				continue
			}

			<-oChan
			oRecv()
		}

		for hCount < 2 {
			<-hChan
			hRecv()
		}

		oCount, hCount = 0, 0
		fmt.Println("Water created!")
	}
}

func main() {
	oChan := make(chan struct{}, 1)
	hChan := make(chan struct{}, 2)

	for i := 0; i < HydrogenProducers; i++ {
		go producer(i, Hydrogen, hChan)
	}
	for i := 0; i < OxygenProducers; i++ {
		go producer(i, Oxygen, oChan)
	}

	createAtom(oChan, hChan)
}
