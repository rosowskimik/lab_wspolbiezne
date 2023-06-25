package utils

import (
	"fmt"
	"math/rand"
	"os"
	"time"
)

func init() {
	fmt.Fprint(os.Stderr, "\033[2J")
	os.Stderr.Sync()
}

func FprintfAt(x, y int, format string, a ...any) (n int, err error) {
	s := fmt.Sprintf(format, a...)

	n, err = fmt.Fprintf(os.Stderr, "\x1b[%d;%dH%s", y, x, s)
	if err != nil {
		os.Stderr.Sync()
	}
	return
}

func DurationN(maxDur time.Duration) time.Duration {
	nanos := rand.Int63n(maxDur.Nanoseconds())
	return time.Duration(nanos)
}
