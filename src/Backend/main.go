package main

import (
	//auth "Backend/Auth"
	common "Backend/Common"
	network "Backend/Network"
	protocol "Backend/Protocol"
	//"context"
	"flag"
	"fmt"
	//"log"
)

func main() {
	debugPtr := flag.Bool("debug", false, "Enable verbose debug logging")
	flag.Parse()
	if *debugPtr {
		network.GetLatest("https://github.com/hannes-swd/code-miner")
		fmt.Println("random seed is:", common.Seed())
		protocol.FrontendLineReader()
	} else {
		protocol.FrontendLineReader()
		
	}
}
