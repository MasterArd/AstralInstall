package main
import (
	"log"
	"Backend/Network"
	"Backend/Protocol"
)

func main(){
	protocol.ProtocolInit()
	log.Println()
	protocol.RequestTest()
	log.Println()
	network.NetworkInit()
	log.Println()
	network.GithubPingTest()
	log.Println()
	
	
	network.GetLatest("https://github.com/MasterArd/example")
	//this is an example ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


	// now the big shi:
	protocol.FrontendLineReader()
}