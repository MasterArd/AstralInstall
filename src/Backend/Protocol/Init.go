package protocol

import "log"

func ProtocolInit() {
	log.Println("Starting FrontendLineReader")
	FrontendLineReader()
}