# Hydroponics
Self managed IoT Hydroponics system

# Setup
## Google IoT

```shell script
PROJECT=iot-hydroponics
REGION=us-central1
gcloud config set project $PROJECT

# APIs
gcloud services enable cloudiot.googleapis.com
gcloud services enable pubsub.googleapis.com
gcloud services enable dataflow.googleapis.com

# Create the PubSub topic and subscription:
gcloud pubsub topics create telemetry
gcloud pubsub subscriptions create telemetry-sub --topic=telemetry

# IAM
gcloud projects add-iam-policy-binding $PROJECT --member=serviceAccount:cloud-iot@system.gserviceaccount.com --role=roles/pubsub.publisher

# BigQuey
bq --location=US mk --dataset dataset
bq mk -t --description "telemetry" --label organization:development dataset.config \
  sampling_temperature_ms:INTEGER,sampling_ph_ms:INTEGER,sampling_ec_ms:INTEGER

# Create the Cloud IoT Core registry:
REGISTRY=registry
gcloud iot registries create $REGISTRY \
  --project=$PROJECT --event-notification-config=topic=projects/iot-hydroponics/topics/telemetry --region=$REGION --log-level=debug

# Generate an Eliptic Curve (EC) private / public key pair:
openssl ecparam -genkey -name prime256v1 -noout -out certs/ec_private.pem
openssl ec -in certs/ec_private.pem -pubout -out certs/ec_public.pem

# Register the device using the keys you generated: 
gcloud iot devices create GreenMeanMachine --region=$REGION --registry=$REGISTRY --public-key path=certs/ec_public.pem,type=es256 --log-level=debug
```