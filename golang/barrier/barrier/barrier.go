package barrier

type Barrier struct {
	count   uint
	barrier chan struct{}
	done    chan struct{}
}

func New(count uint) Barrier {
	barrier := make(chan struct{}, count)
	done := make(chan struct{}, count)

	go func() {
		for {

			var i uint
			for i = 0; i < count; i++ {
				<-barrier
			}
			for ; i > 0; i-- {
				done <- struct{}{}
			}
		}
	}()

	return Barrier{
		count,
		barrier,
		done,
	}
}

func (b *Barrier) ArriveAndWait() {
	b.barrier <- struct{}{}
	<-b.done
}
