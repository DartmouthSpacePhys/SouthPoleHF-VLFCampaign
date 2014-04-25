#!/bin/bash

make
sudo chown root:adm acq_c
sudo chmod ugo=rx,u+s acq_c
