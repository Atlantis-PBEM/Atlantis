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

exit $failure
