#!/bin/bash

TEMP_DIR=./temp

if [ -d $TEMP_DIR ]; then
    rm -rf $TEMP_DIR
fi
mkdir $TEMP_DIR

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

function CheckSuccess() {
    test_name=$1
    result=$2
    if [ $result != 0 ]; then
        echo ${red} ${test_name} FAIL ${reset}
        exit 1
    else
        echo ${green} ${test_name} SUCCESS ${reset}
    fi
}

../src/test/arraylist_test &> $TEMP_DIR/arraylist_test.log
CheckSuccess "ArrayList TEST" $?

../src/test/arraymap_test &> $TEMP_DIR/arraymap_test.log
CheckSuccess "ArrayMap TEST" $?

../src/test/arraypool_test &> $TEMP_DIR/arraypool_test.log
CheckSuccess "ArrayPool TEST" $?

../src/test/defer_test &> $TEMP_DIR/defer_test.log
CheckSuccess "Defer TEST" $?

../src/test/dev-tools_test &> $TEMP_DIR/dev-tools_test.log
CheckSuccess "Dev Tools TEST" $?

../src/test/huffman_test &> $TEMP_DIR/huffman_test.log
CheckSuccess "Huffman TEST" $?

../src/test/option-parser_test &> $TEMP_DIR/option-parser_test.log
CheckSuccess "Option Parser TEST" $?

../src/test/packer_test --data_path=../testdata/mytestdata &> $TEMP_DIR/packer_test.log
CheckSuccess "Packer TEST" $?

../src/test/topset_test &> $TEMP_DIR/topset_test.log
CheckSuccess "TopSet TEST" $?

