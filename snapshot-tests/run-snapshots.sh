#!/bin/bash

failure=0

echo "Running standard snapshots"
./run-game-snapshots.sh
if [[ $? != 0 ]]; then
  failure=1
fi

echo "Running neworigin snapshots"
./run-game-snapshots.sh neworigins
if [[ $? != 0 ]]; then
  failure=1
fi

echo "Running all rules snapshots"
./run-rules-snapshot.sh
if [[ $? != 0 ]]; then
  failure=1
fi
./run-rules-snapshot.sh basic
if [[ $? != 0 ]]; then
  failure=1
fi
./run-rules-snapshot.sh fracas
if [[ $? != 0 ]]; then
  failure=1
fi
./run-rules-snapshot.sh havilah
if [[ $? != 0 ]]; then
  failure=1
fi
./run-rules-snapshot.sh kingdoms
if [[ $? != 0 ]]; then
  failure=1
fi
./run-rules-snapshot.sh neworigins
if [[ $? != 0 ]]; then
  failure=1
fi

exit $failure
