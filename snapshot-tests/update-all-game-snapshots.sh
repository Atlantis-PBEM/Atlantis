#!/bin/bash

echo "Updating standard snapshots"
./update-game-snapshots.sh
echo "Updating neworigin snapshots"
./update-game-snapshots.sh neworigins

exit 0
