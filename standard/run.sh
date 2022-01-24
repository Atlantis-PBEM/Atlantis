#!/bin/sh

rm -f game.*
rm -f report.*
rm -f times.*
./standard new
mv game.out game.in
./standard run