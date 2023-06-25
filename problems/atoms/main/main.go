package main

import (
	"math/rand"
	"problems/synth"
	"time"
)

const (
	OxygenProducers   = 5
	HydrogenProducers = 8
)

func init() {
	rand.Seed(time.Now().Unix())
}

func main() {
	oSend, hSend := synth.StartSynth(2 * time.Second)

	var i int
	for i = 0; i < OxygenProducers; i++ {
		go synth.InitProducer(39, i+1, synth.Oxygen, 12*time.Second, oSend)
	}
	for i = 0; i < HydrogenProducers; i++ {
		go synth.InitProducer(69, i+1, synth.Hydrogen, 10*time.Second, hSend)
	}

	select {}
}
