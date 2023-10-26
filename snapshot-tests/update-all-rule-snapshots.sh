#!/bin/bash

echo "Updating all rules snapshots"
./update-rules-snapshot.sh
./update-rules-snapshot.sh basic
./update-rules-snapshot.sh fracas
./update-rules-snapshot.sh havilah
./update-rules-snapshot.sh kingdoms
./update-rules-snapshot.sh neworigins

exit 0
