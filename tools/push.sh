#!/usr/bin/env bash
set -o errexit

$HOME/src/google-cloud-sdk/bin/gcloud iot devices configs update --config-file="$1" --registry=registry --region=us-central1 --device="$2"
