// pubsub converts a Pub/Sub event or state into a new Firestore and BigQuery database entry.
//
// The Firestore schema will be something like:
//
// devices            collection
//   {device_id}      document
//     telemetry			collection
//       {timestamp}  document
//         ec.a
//         ec.b
//         ph.a
//         ph.b
//         tank.a
//         tank.b
//         temp.indoor
//         temp.probe
//         humidity
//         pressure
package hydroponics

import (
	"context"
	"fmt"
	"log"
	"os"
	"reflect"
	"strings"
	"sync"
	"time"

	"cloud.google.com/go/bigquery"
	"cloud.google.com/go/firestore"
	"cloud.google.com/go/functions/metadata"
	pb "github.com/csobrinho/hydroponics/components/protos/go"
	"google.golang.org/protobuf/proto"
)

// pubSubMessage is the payload of a Pub/Sub event.
type pubSubMessage struct {
	Attributes struct {
		DeviceId  string `json:"deviceId"`
		ProjectId string `json:"projectId"`
	} `json:"attributes"`
	Data []byte `json:"data"`
}

type event struct {
	DeviceId   string    `firestore:"-"           bigquery:"device_id"`
	Timestamp  time.Time `firestore:"timestamp"   bigquery:"time"`
	TempIndoor float64   `firestore:"temp.indoor" bigquery:"temp_indoor"`
	TempProbe  float64   `firestore:"temp.probe"  bigquery:"temp_probe"`
	Humidity   float64   `firestore:"humidity"    bigquery:"humidity"`
	Pressure   float64   `firestore:"pressure"    bigquery:"pressure"`
	EcaValue   float64   `firestore:"ec.a"        bigquery:"eca_value"`
	EcbValue   float64   `firestore:"ec.b"        bigquery:"ecb_value"`
	PhaValue   float64   `firestore:"ph.a"        bigquery:"pha_value"`
	PhbValue   float64   `firestore:"ph.b"        bigquery:"phb_value"`
	TankaValue float64   `firestore:"tank.a"      bigquery:"tanka_value"`
	TankbValue float64   `firestore:"tank.b"      bigquery:"tankb_value"`
}

var datasetName = os.Getenv("DATASET")
var tableName = os.Getenv("TABLE")
var ignoreSuffix = os.Getenv("IGNORE_SUFFIX")

func getSuffix(s *pb.State) (string, error) {
	t := reflect.TypeOf(s.State).Elem()
	if !strings.HasPrefix(t.Name(), "State_") {
		return "", fmt.Errorf("state has unexpected type %T", s)
	}
	return strings.SplitN(t.Name(), "_", 2)[1], nil
}

func handleState(ctx context.Context, s *pb.State, m pubSubMessage, meta *metadata.Metadata, client *firestore.Client) error {
	suffix, err := getSuffix(s)
	log.Printf("state %s: %+v", suffix, s)
	if err != nil {
		return err
	}
	doc := client.Doc(fmt.Sprintf("devices/%s/state/%s", m.Attributes.DeviceId, strings.ToLower(suffix)))
	data := map[string]interface{}{
		"timestamp": meta.Timestamp,
		"data":      m.Data,
	}
	_, err = doc.Set(ctx, data, firestore.MergeAll)
	return err
}

func handleStateTelemetry(ctx context.Context, tpb *pb.StateTelemetry, m pubSubMessage, meta *metadata.Metadata, client *firestore.Client) error {
	log.Printf("State_Telemetry: %+v", tpb)
	e := &event{
		DeviceId:   m.Attributes.DeviceId,
		Timestamp:  meta.Timestamp,
		TempIndoor: float64(tpb.TempIndoor),
		TempProbe:  float64(tpb.TempProbe),
		Humidity:   float64(tpb.Humidity),
		Pressure:   float64(tpb.Pressure),
		EcaValue:   float64(tpb.EcA),
		EcbValue:   float64(tpb.EcB),
		PhaValue:   float64(tpb.PhA),
		PhbValue:   float64(tpb.PhB),
		TankaValue: float64(tpb.TankA),
		TankbValue: float64(tpb.TankB),
	}

	var wg sync.WaitGroup
	status := make(chan error, 2)
	finished := make(chan bool, 1)
	defer close(status)

	wg.Add(1)
	go func() {
		defer wg.Done()
		if err := handleStateTelemetryFirestore(ctx, e, client); err != nil {
			status <- err
		}
	}()

	wg.Add(1)
	go func() {
		defer wg.Done()
		if err := handleStateTelemetryBigQuery(ctx, e, m.Attributes.ProjectId); err != nil {
			status <- err
		}
	}()

	// Wait for all to complete in a go routine to avoid a deadlock.
	go func() {
		wg.Wait()
		close(finished)
	}()

	select {
	case <-finished:
	case err := <-status:
		return err
	}
	return nil
}

func handleStateTelemetryFirestore(ctx context.Context, e *event, client *firestore.Client) error {
	doc := client.Doc(fmt.Sprintf("devices/%s/telemetry/%d", e.DeviceId, e.Timestamp.Unix()))
	_, err := doc.Create(ctx, e)
	log.Printf("handleStateTelemetryFirestore err: %v", err)
	return err
}

func handleStateTelemetryBigQuery(ctx context.Context, e *event, projectId string) error {
	client, err := bigquery.NewClient(ctx, projectId)
	if err != nil {
		return err
	}
	defer func() {
		if err := client.Close(); err != nil {
			log.Fatalln(err)
		}
	}()

	ins := client.Dataset(datasetName).Table(tableName).Inserter()
	err = ins.Put(ctx, e)
	log.Printf("handleStateTelemetryBigQuery err: %v", err)
	if err != nil {
		if pmErr, ok := err.(bigquery.PutMultiError); ok {
			for _, rowInsertionError := range pmErr {
				log.Println(rowInsertionError.Errors)
			}
		}
		return err
	}
	return nil
}

// HandlePubSub consumes a Pub/Sub message and insert those values into the Firestore and BigQuery database.
func HandlePubSub(ctx context.Context, m pubSubMessage) error {
	if ignoreSuffix != "" && strings.HasSuffix(m.Attributes.DeviceId, ignoreSuffix) {
		return nil
	}
	meta, err := metadata.FromContext(ctx)
	if err != nil {
		return err
	}
	if m.Data == nil || len(m.Data) <= 0 {
		return fmt.Errorf("m.Data is empty")
	}
	var states pb.States
	if err := proto.Unmarshal(m.Data, &states); err != nil {
		return err
	}
	client, err := firestore.NewClient(ctx, m.Attributes.ProjectId)
	if err != nil {
		return err
	}
	defer func() {
		if err := client.Close(); err != nil {
			log.Fatalln(err)
		}
	}()

	for _, state := range states.State {
		switch state.State.(type) {
		case *pb.State_Telemetry:
			if err := handleStateTelemetry(ctx, state.GetTelemetry(), m, meta, client); err != nil {
				return err
			}
		default:
			if err := handleState(ctx, state, m, meta, client); err != nil {
				return err
			}
		}
	}
	return nil
}
