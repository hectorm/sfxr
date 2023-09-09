#!/bin/sh

set -eu

DIR="$(CDPATH='' cd -- "$(dirname -- "${0:?}")" && pwd -P)"

: "${CONTAINER_IMAGE=docker.io/hectorm/wasm:latest}"
: "${CONTAINER_USER=$(id -u):$(id -g)}"
: "${CONTAINER_WORKDIR=${DIR:?}}"

exec docker run --rm \
	--user "${CONTAINER_USER:?}" \
	--workdir "${CONTAINER_WORKDIR:?}" \
	--env EM_CACHE=/tmp/ --env EM_PORTS=/tmp/ports \
	--mount type=bind,source="${CONTAINER_WORKDIR:?}",target="${CONTAINER_WORKDIR:?}" \
	--mount type=volume,source=emscripten-cache,target=/tmp/ \
	"${CONTAINER_IMAGE:?}" emmake make "${@}"
