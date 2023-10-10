#!/bin/bash

curTurn=$(< ./turn)
nextTurn=$((curTurn + 1))

if [[ ! -f ../../standard/standard ]]; then
  echo "Please build the standard executable before trying to create a new snapshot turn."
  exit 1
fi

if [[ ! -d turn_$nextTurn ]]; then
  mkdir turn_$nextTurn
  cp turn_$curTurn/game.out turn_$nextTurn/game.in
  cp turn_$curTurn/players.out turn_$nextTurn/players.in
  cp turn_$curTurn/template.3 turn_$nextTurn/orders.3
fi

echo "Modify the input orders for turn_$nextTurn to test desired changes"
echo "When that is complete, rerun this script."

diff turn_$curTurn/template.3 turn_$nextTurn/orders.3 &> /dev/null
if [[ $? = 0 ]];
then
  echo "No order changes have occured.   Exiting."
  exit 0
fi

echo "Running turn to produce new output"
cd turn_$nextTurn
../../../standard/standard run &> engine-output.txt

if [[ $? != 0 ]]; then
   echo "Game engine crashed while generating turn. Turn not generated."
   cat engine-output.txt
   rm -f *.out
   rm -f template.*
   rm -f report.*
   rm -f times.*
   rm -f engine-output.txt
fi

echo "$nextTurn" > ../turn

rm -f times.*
