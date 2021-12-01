#!/bin/bash

turn=$(<"turn")
nextTurn=$((turn + 1))

./standard run

mv orders.3 orders_$nextTurn.3
mv game.in game_$nextTurn.in
mv players.in players_$nextTurn.in

echo "$nextTurn" > "turn"

mv game.out game.in
mv players.out players.in
mv template.3 orders.3

rm -f times.*