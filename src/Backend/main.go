package main

import (
	common "Backend/Common"
	network "Backend/Network"
	protocol "Backend/Protocol"
	"flag"
	"log"
	"fmt"
)

func main() {
	debugPtr := flag.Bool("debug", false, "Enable verbose debug logging")
	flag.Parse()
	if *debugPtr {
		protocol.ProtocolInit()
		log.Println()
		protocol.RequestTest()
		log.Println()
		network.NetworkInit()
		log.Println()
		network.GithubPingTest()
		log.Println()
		network.GetLatest("https://github.com/hannes-swd/code-miner")
		fmt.Println("random seed is:",common.Seed())
	} else {
		protocol.FrontendLineReader()
	}
}
