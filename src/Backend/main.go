package main

import (
	network "Backend/Network"
	protocol "Backend/Protocol"
	"flag"
	"log"
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
		network.GetLatest("https://github.com/MasterArd/example")
	} else {
		protocol.FrontendLineReader()
	}
}
