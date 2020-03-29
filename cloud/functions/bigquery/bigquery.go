package telemetry

import (
	"context"
	"encoding/json"
	"errors"
	"log"
	"os"
	"strings"
	"time"

  "cloud.google.com/go/bigquery"
  "cloud.google.com/go/functions/metadata"
)

// PubSubMessage is the payload of a Pub/Sub event.
type pubSubMessage struct {
	Attributes struct {
		DeviceId string `json:"deviceId"`
		ProjectId string `json:"projectId"`
	} `json:"attributes"`
	Data []byte `json:"data"`
}

type event struct {
	DeviceId   string    `bigquery:"device_id" json:-`
	Time       time.Time `bigquery:"time" json:-`
	TempIndoor float64   `bigquery:"temp_indoor" json:"sensors.temp.indoor"`
	TempProbe  float64   `bigquery:"temp_probe" json:"sensors.temp.probe"`
	Humidity   float64   `bigquery:"humidity" json:"sensors.humidity"`
	Pressure   float64   `bigquery:"pressure" json:"sensors.pressure"`
	EcValue    float64   `bigquery:"ec_value" json:"sensors.ec.value"`
	PhValue    float64   `bigquery:"ph_value" json:"sensors.ph.value"`
}

var datasetName = os.Getenv("DATASET")
var tableName = os.Getenv("TABLE")

// BigqueryPubSub consumes a Pub/Sub message and inserts a new Bigquery row.
func BigqueryPubSub(ctx context.Context, m pubSubMessage) error {
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
	e := event{
		DeviceId: m.Attributes.DeviceId,
		Time: meta.Timestamp,
	}
	if err := json.Unmarshal(m.Data, &e); err != nil {
		return err
	}

	client, err := bigquery.NewClient(ctx, m.Attributes.ProjectId)
	if err != nil {
		return err
	}
	defer client.Close()

	ins := client.Dataset(datasetName).Table(tableName).Inserter()
	if err := ins.Put(ctx, &e); err != nil {
		if pmErr, ok := err.(bigquery.PutMultiError); ok {
			for _, rowInsertionError := range pmErr {
				log.Println(rowInsertionError.Errors)
			}
		}
		return err
	}
	return nil
}
