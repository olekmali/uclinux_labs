#!/usr/bin/sh
while [ 1 ]
do
  # A Morse: dot-dash-
  echo 1
  sleep 0.1
  echo 0
  sleep 0.1
  # gap between letters (0.3 is 0.1+0.2)
  sleep 0.2
  # M Morse: dash-dash-
  echo 1
  sleep 0.3
  echo 0
  sleep 0.1
  echo 1
  sleep 0.3
  echo 0
  sleep 0.1
  # gap between words (0.7 is 0.1+0.6)
  sleep 0.6
done

