#!/bin/bash
sleep 5
./test_client $(getent hosts test_server)
