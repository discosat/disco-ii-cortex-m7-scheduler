#!/bin/bash
set -e
set +x

cp -r /tmp-src/* ${REPO_ROOT}/src

cd ${REPO_ROOT}/src/armgcc

./build.sh

if [ -d "${REPO_ROOT}/bin/debug" ]; then rm -r ${REPO_ROOT}/bin/debug; fi
if [ -d "${REPO_ROOT}/bin/release" ]; then rm -r ${REPO_ROOT}/bin/release; fi

mv debug ${REPO_ROOT}/bin
mv release ${REPO_ROOT}/bin
