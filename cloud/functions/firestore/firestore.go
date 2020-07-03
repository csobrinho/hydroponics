// firestore converts a Pub/Sub event into a new Firestore database entry.
//
// The schema will be something like:
//
// devices     collection
// {device_id} document
//   telemetry			collection
//     {timestamp}  document
//       ec.a
//       ec.b
//       ph.a
//       ph.b
//       tank.a
//       tank.b
//       temp.indoor
//       temp.probe
//       humidity
//       pressure
package firestore

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"strings"
	"time"

	"cloud.google.com/go/firestore"
	"cloud.google.com/go/functions/metadata"
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
	Timestamp  time.Time `firestore:"timestamp" json:-`
	TempIndoor float64   `firestore:"temp.indoor" json:"sensors.temp.indoor"`
	TempProbe  float64   `firestore:"temp.probe" json:"sensors.temp.probe"`
	Humidity   float64   `firestore:"humidity" json:"sensors.humidity"`
	Pressure   float64   `firestore:"pressure" json:"sensors.pressure"`
	EcaValue   float64   `firestore:"ec.a" json:"sensors.eca.value"`
	EcbValue   float64   `firestore:"ec.b" json:"sensors.ecb.value"`
	PhaValue   float64   `firestore:"ph.a" json:"sensors.pha.value"`
	PhbValue   float64   `firestore:"ph.b" json:"sensors.phb.value"`
	TankaValue float64   `firestore:"tank.a" json:"sensors.tanka.value"`
	TankbValue float64   `firestore:"tank.b" json:"sensors.tankb.value"`
}

// FirestorePubSub consumes a Pub/Sub message and insert those values into the Firestore database.
func FirestorePubSub(ctx context.Context, m pubSubMessage) error {
	if strings.HasSuffix(m.Attributes.DeviceId, "-test") {
		return nil
	}
	meta, err := metadata.FromContext(ctx)
	if err != nil {
		return err
	}
	if m.Data == nil || len(m.Data) <= 0 {
		return errors.New("m.Data is empty")
	}
	var e event
	if err := json.Unmarshal(m.Data, &e); err != nil {
		return err
	}
	client, err := firestore.NewClient(ctx, m.Attributes.ProjectId)
	if err != nil {
		return err
	}
	defer client.Close()

	telemetry := client.Doc(fmt.Sprintf("devices/%s/telemetry/%d", m.Attributes.DeviceId, meta.Timestamp.Unix()))
	e.Timestamp = meta.Timestamp
	_, err = telemetry.Create(ctx, e)
	if err != nil {
		return err
	}
	return nil
}
