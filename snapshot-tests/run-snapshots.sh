#!/bin/bash

if [[ ! -f ../standard/standard ]]; then
  echo "Please build the standard executable before running the snapshot tests.  Test failed."
  exit 1
fi

cp ../standard/standard standard
chmod +x ./standard

lastTurn=$(<"turns/turn")

for turn in $(seq 0 $lastTurn)
do
  echo -n "Replaying turn $turn..."
  cp -f turns/turn_$turn/game.in game.in
  cp -f turns/turn_$turn/players.in players.in
  cp -f turns/turn_$turn/orders.3 orders.3

  ./standard run &> engine-output.txt
  if [[ $? != 0 ]]; then
    echo "executable crashed. -- Test failed."
    cat engine-output.txt
    rm -f game.*
    rm -f players.*
    rm -f times.*
    rm -f orders.*
    rm -f template.*
    rm -f report.*
    rm -f engine-output.txt
    rm -f ./standard
    exit 1
  fi
    
  mkdir -p output/turn_$turn
  mv game.* output/turn_$turn
  mv players.* output/turn_$turn
  if [[ -f times.* ]]; then
    mv times.* output/turn_$turn
  fi
  mv orders.* output/turn_$turn
  mv template.* output/turn_$turn
  mv report.* output/turn_$turn
  mv engine-output.txt output/turn_$turn

  diff -ur turns/turn_$turn output/turn_$turn &> turn-difference.txt
  if [[ $? != 0 ]]; then
    echo "output differed. -- Test failed."
    cat turn-difference.txt
    rm -f turn-difference.txt
    rm -f ./standard
    rm -rf ./output
    exit 1
  fi

  echo "identical. -- Test succeeded."
  rm -f turn-difference.txt
done

rm -f ./standard
rm -rf ./output
exit 0
