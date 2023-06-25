package synth

import (
	"fmt"
	"problems/utils"
	"time"
)

type Atom int

const (
	Oxygen Atom = iota
	Hydrogen
)

var atomNames = map[Atom]string{
	Oxygen:   "Oxygen",
	Hydrogen: "Hydrogen",
}

type AtomCounter struct {
	count uint
	atom  Atom
}

func NewCounter(atom Atom) AtomCounter {
	return AtomCounter{
		count: 0,
		atom:  atom,
	}
}

func (c *AtomCounter) Increment() {
	c.count += 1
}

func (c *AtomCounter) Reset() {
	c.count = 0
}

func (c AtomCounter) String() string {
	return fmt.Sprintf("%s: %d", atomNames[c.atom], c.count)
}

func InitProducer(x, y int, product Atom, maxDur time.Duration, sender chan<- Atom) {
	char := atomNames[product][0]

	var dur time.Duration
	for {
		dur = utils.DurationN(maxDur)
		utils.FprintfAt(x, y, "%c: [ PRODUCING (%5dms) ]", char, dur.Milliseconds())
		time.Sleep(dur)
		utils.FprintfAt(x, y, "%c: [       SENDING       ]", char)
		sender <- product
	}
}

func StartSynth(prodDur time.Duration) (oSender, hSender chan<- Atom) {
	oxygen := make(chan Atom)
	hydrogen := make(chan Atom)

	go func() {
		oCounter := NewCounter(Oxygen)
		hCounter := NewCounter(Hydrogen)

		waterProduced := 0

		printSupplies := func() {
			utils.FprintfAt(1, 1, "%s", oCounter)
			utils.FprintfAt(1, 2, "%s", hCounter)
		}

		printState := func(msg string) {
			utils.FprintfAt(1, 4, "Synth state: %s\nProduced: %d", msg, waterProduced)
		}

		var dur time.Duration
		for {
			printSupplies()
			printState("WAITING FOR SUPPLIES")

			select {
			case <-oxygen:
				oCounter.Increment()

				for hCounter.count < 2 {
					printSupplies()
					<-hydrogen
					hCounter.Increment()
				}
			case <-hydrogen:
				hCounter.Increment()

				if oCounter.count == 0 {
					if hCounter.count == 1 {
						continue
					}

					printSupplies()

					<-oxygen
					oCounter.Increment()
				}
			}
			oCounter.Reset()
			hCounter.Reset()
			dur = utils.DurationN(prodDur)

			printSupplies()
			printState(fmt.Sprintf(" PRODUCING (%5dms) ", dur.Milliseconds()))

			time.Sleep(dur)
			waterProduced += 1
		}
	}()

	return oxygen, hydrogen
}
