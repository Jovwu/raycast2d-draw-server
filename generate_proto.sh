#!/bin/bash

PROTO_DIR="proto"
INCLUDE_DIR="include"
SRC_DIR="src"

mkdir -p $INCLUDE_DIR
mkdir -p $SRC_DIR

for proto_file in $PROTO_DIR/*.proto; do
  protoc --cpp_out=$SRC_DIR --proto_path=$PROTO_DIR $proto_file
  base_name=$(basename $proto_file .proto)
  mv $SRC_DIR/$base_name.pb.h $INCLUDE_DIR/
done

echo "Proto files have been compiled and moved to the appropriate directories."