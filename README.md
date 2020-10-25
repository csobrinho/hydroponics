# Hydroponics
Self managed IoT Hydroponics system

# Setup
## Google IoT

```shell script
PROJECT=iot-hydroponics
REGION=us-central1
DATASET=dataset
TABLE_TELEMETRY=telemetry
EVENT_TOPIC=telemetry
STATE_TOPIC=state

gcloud config set project ${PROJECT}

# APIs
gcloud services enable cloudiot.googleapis.com
gcloud services enable pubsub.googleapis.com
gcloud services enable dataflow.googleapis.com

# Create the PubSub topic and subscription:
gcloud pubsub topics create ${EVENT_TOPIC}
gcloud pubsub subscriptions create ${EVENT_TOPIC}-sub --topic=${EVENT_TOPIC} --expiration-period=never --message-retention-duration=10m
gcloud pubsub topics create ${STATE_TOPIC}
gcloud pubsub subscriptions create ${STATE_TOPIC}-sub --topic=${STATE_TOPIC} --expiration-period=never --message-retention-duration=10m

# IAM
gcloud projects add-iam-policy-binding ${PROJECT} --member=serviceAccount:cloud-iot@system.gserviceaccount.com --role=roles/pubsub.publisher

# BigQuey
bq --location=US mk ${DATASET}
bq --location=US mk ${DATASET}.${TABLE_TELEMETRY} cloud/bq/schema.json

# Create the Cloud IoT Core registry:
REGISTRY=registry
gcloud iot registries create ${REGISTRY} \
 --region=${REGION} \
 --event-notification-config=topic=projects/${PROJECT}/topics/telemetry \
 --state-pubsub-topic=projects/${PROJECT}/topics/state \
 --log-level=debug

# Generate an Eliptic Curve (EC) private / public key pair:
openssl ecparam -genkey -name prime256v1 -noout -out certs/ec_private.pem
openssl ec -in certs/ec_private.pem -pubout -out certs/ec_public.pem

# Register the device using the keys you generated: 
gcloud iot devices create GreenMeanMachine \
 --region=${REGION} \
 --registry=${REGISTRY} \
 --public-key=path=certs/ec_public.pem,type=es256 \
 --log-level=debug

# Cloud functions.
gcloud beta functions deploy pubsub \
 --region ${REGION} \
 --source cloud/functions/pubsub \
 --entry-point HandlePubSub \
 --trigger-topic ${EVENT_TOPIC} \
 --set-env-vars=DATASET=${DATASET},TABLE=${TABLE_TELEMETRY},IGNORE_SUFFIX="-test" \
 --ingress-settings internal-only \
 --max-instances 1 \
 --runtime go113 \
 --memory 128mb

gcloud beta functions deploy pubsub-state \
 --region ${REGION} \
 --source cloud/functions/pubsub \
 --entry-point HandlePubSub \
 --trigger-topic ${STATE_TOPIC} \
 --set-env-vars=DATASET=${DATASET},TABLE=${TABLE_TELEMETRY},IGNORE_SUFFIX="-test" \
 --ingress-settings internal-only \
 --max-instances 1 \
 --runtime go113 \
 --memory 128mb
