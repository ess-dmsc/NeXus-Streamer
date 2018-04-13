#!/bin/bash

mkdir -p build
cd build

echo "Run cmake..."
HDF5_ROOT=$HDF5_ROOT \
    cmake ../code \
    -DLibRDKafka_ROOT_DIR=$LibRDKafka_ROOT_DIR

echo "Build project"
make

echo "Run tests"
./unitTests ../code/data/ --gtest_output=xml:test_results.xml

echo "Create docs"
make doc

echo "Create package"
make package

cd ..

echo "Create RPM"
rm -rf package/*
mkdir -p package/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

cp build/NeXus-Streamer-0.1.0.tar.gz package/SOURCES/
cp code/NeXus-Streamer.spec package/SPECS/

rpmbuild \
    --define "_topdir $(pwd)/package" \
    --define "_version 0.1.0" \
    --define "_release $BUILD_NUMBER" \
-bb package/SPECS/NeXus-Streamer.spec

