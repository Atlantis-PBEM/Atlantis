#!/bin/bash

cp ../standard/standard standard
chmod +x ./standard

lastTurn=$(<"turns/turn")

for turn in $(seq 0 $lastTurn)
do
    echo "Replaying turn $turn..."

    cp turns/game_$turn.in game.in
    cp turns/players_$turn.in players.in
    cp turns/orders_$turn.3 orders.3

    ./standard run
    RESULT=$?

    rm -f game.*
    rm -f players.*
    rm -f times.*
    rm -f orders.*
    rm -f template.*
    rm -f report.*
    
    if [ $RESULT != 0 ]; then
        exit 1
    fi

    echo ""
done
