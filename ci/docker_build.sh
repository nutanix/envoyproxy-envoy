#!/bin/bash

set -ex

docker build --build-arg TARGETPLATFORM=linux/amd64 -f ci/Dockerfile-envoy -t ${DOCKER_PREFIX}/envoy:latest .
docker build --build-arg TARGETPLATFORM=linux/amd64 -f ci/Dockerfile-envoy-alpine -t ${DOCKER_PREFIX}/envoy-alpine:latest .