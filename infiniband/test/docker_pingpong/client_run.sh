#!/bin/bash

sleep 3
./pingpong -s $(getent hosts test_server)
