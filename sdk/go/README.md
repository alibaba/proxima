
## Quickstart

```go
func ExampleNewClient() {
	protoc := be.GrpcProtocol
	if *Http {
		protoc = be.HttpProtocol
	}
	client, err := be.NewProximaSearchClient(protoc, &address)
	if err != nil {
		log.Fatal("Can't create ProximaClient instance.", err)
	}
}

func ExampleListCollections(client *be.ProximaSearchClient) {
	// List all collections
	collections, err := client.ListCollections()
	if err != nil {
		log.Fatal("Can't retrieve collections from Proxima Server.", err)
	}
	log.Printf("Collections (%d): \n", len(collections))
	for _, collection := range collections {
		log.Printf("%+v\n", collection)
	}

	// List all collections by Repo
	collections, err = client.ListCollections(be.ByRepo("repo"))
	if err != nil {
		log.Fatal("Can't retrieve collections from Proxima Server.", err)
	}
	log.Printf("Collections (%d): \n", len(collections))
	for _, collection := range collections {
		log.Printf("%+v\n", collection)
	}
}
```

## Howto

Please go through [sdk/references]() to get an idea how to use this package.

## Look and feel

## Release Rules:
Dev management: Using lightweight tag to release package, using annotation tag for develop.
