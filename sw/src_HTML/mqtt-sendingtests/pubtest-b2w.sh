#!/bin/bash

while true; do
    mosquitto_pub -h 10.159.177.113 -p 1883 -t "krl2475/b2w" -m '{"A": 12.3, "B": 45.6, "C": 1.2, "D": 3.4, "E": 1200, "F": 1300, "G": 0.5, "H": 0.6, "I": 0.05, "J": 0.06, "K": 123}'
    sleep 1
done

