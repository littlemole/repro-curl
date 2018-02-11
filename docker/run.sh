!/bin/bash

cd /opt/workspace/reprocurl
make clean
make -e
make -e test
make -e install

