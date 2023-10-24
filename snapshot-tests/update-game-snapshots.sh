#!/bin/bash

game=$1
if [[ $game = "" || $game = "standard" ]]; then
  game=standard
  executable=../standard/standard
  turndir=turns
else
  executable=../$game/$game
  turndir=${game}_turns
fi

if [[ ! -f $executable ]]; then
  echo "Please build the $game executable before updating the snapshot tests.  Not updated."
  exit 1
fi

cp $executable ./$game
chmod +x ./$game

if [[ ! -f $turndir/turn ]]; then
  echo "No turns defined for $game.  Not updated."
  exit 1
fi

lastTurn=$(<"$turndir/turn")

for turn in $(seq 0 $lastTurn)
do
  echo -n "Updating $game turn $turn..."

  if [[ ! -d $turndir/turn_$turn ]]; then
    echo "turn $turn missing. -- Not updated."
    exit 1
  fi

  cp -f $turndir/turn_$turn/game.in game.in
  cp -f $turndir/turn_$turn/players.in players.in
  cp -f $turndir/turn_$turn/orders.3 orders.3

  ./$game run &> engine-output.txt
  if [[ $? != 0 ]]; then
    echo "executable crashed. -- Not updated."
    cat engine-output.txt
    rm -f game.*
    rm -f players.*
    rm -f times.*
    rm -f orders.*
    rm -f template.*
    rm -f report.*
    rm -f engine-output.txt
    rm -r ./$game
    exit 1
  fi
    
  mkdir -p output/turn_$turn
  mv game.* output/turn_$turn
  mv players.* output/turn_$turn
  if [[ $(shopt -s nullglob; set -- times.*; echo $#) -ge 1 ]]; then
    mv times.* output/turn_$turn
  fi
  mv orders.* output/turn_$turn
  mv template.* output/turn_$turn
  mv report.* output/turn_$turn
  mv engine-output.txt output/turn_$turn

  mv $turndir/turn_$turn $turndir/turn_$turn.bak
  mv output/turn_$turn $turndir/turn_$turn

  echo "Snapshot updated."
done

rm -r ./$game
exit 0
