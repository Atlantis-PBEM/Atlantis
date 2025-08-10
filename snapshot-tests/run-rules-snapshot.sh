#!/bin/bash

cleanup()
{
  game="$1"
  rm -f "${game}"
  rm -f "${game}.html"
  rm -f "${game}_intro.html"
  rm -f "${game}.html.strip"
  rm -f rules-difference.txt
  rm -f engine-output.txt
}

game="$1"
if [[ "$game" = "" || "$game" = "standard" ]]; then
  game=standard
  executable=../standard/standard
else
  executable="../$game/$game"
fi

[ -e "${executable}" ] || executable="../build/$game"

if [[ ! -f "${executable}" ]]; then
  echo "Please build the $game executable before running the rules snapshot tests.  Test failed."
  exit 1
fi

cp "$executable" "./$game"
chmod +x "./$game"

echo -n "Regenerating $game rules..."

cp "../$game/${game}_intro.html" .
if ! "./$game" genrules "${game}_intro.html" "${game}.css" "${game}.html" &> engine-output.txt ; then
  echo "executable crashed. -- Test failed."
  cat engine-output.txt
  cleanup "${game}"
  exit 1
fi

rm -f ./engine-output.txt

# Clean up the Last Change: timestamp line so that comparisons can work
sed -e 's/Last Change:.*$/Last Change: __STRIPPED_FOR_COMPARISON__/' "${game}.html" > "${game}.html.strip"

if ! diff -ur "rules/${game}.html" "${game}.html.strip" &> rules-difference.txt ; then
  echo "output differed. -- Test failed."
  cat rules-difference.txt
  cleanup "${game}"
  exit 1
fi

echo "identical. -- Test succeeded."
cleanup "${game}"
exit 0
