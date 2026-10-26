package main

import (
	auth "Backend/Auth"
	common "Backend/Common"
	network "Backend/Network"
	protocol "Backend/Protocol"
	"context"
	"flag"
	"fmt"
	"log"
)

func main() {
	debugPtr := flag.Bool("debug", false, "Enable verbose debug logging")
	flag.Parse()
	if *debugPtr {
		network.GetLatest("https://github.com/hannes-swd/code-miner")
		fmt.Println("random seed is:", common.Seed())
	} else {
		ctx := context.Background()
		token, user, err := auth.Login(ctx, "Ov23liUUhChOmyys6qVH", func(dc *auth.DeviceCode) {
			fmt.Printf("Go to %s and enter code: %s\n", dc.VerificationURI, dc.UserCode)
		})
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println("Logged in as", user.Login, "id", user.ID, token)
		protocol.FrontendLineReader()
		

	}
}
