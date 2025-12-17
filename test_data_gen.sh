#!/bin/bash

for (( i = 1; i <= 10*$1; i = i+1)); do
  for (( j = 1; j <= 10*$1; j = j+1)); do
    echo $i","$j","1
  done
done

for (( i = 10*$1+1; i <= 20*$1; i = i+1)); do
  for (( j = 10*$1+1; j <= 20*$1; j = j+1)); do
    echo $i","$j","1
  done
done

for (( i = 1; i <= 10*$1; i = i+1)); do
  for (( j = 10*$1; j <= 20*$1; j = j+1)); do
    echo $i","$j","-1
  done
done

for (( i = 10*$1 + 1; i <= 20*$1; i = i+1)); do
  for (( j = 1; j <= 10*$1; j = j+1)); do
    echo $i","$j","-1
  done
done
