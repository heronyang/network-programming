#!/bin/bash
# Usage: sh auto_check.sh
# Notice: please remove unwant folder in ras/ first, and your files should locate like this:
# .
# ├── auto_run.sh
# ├── client
# ├── client.c
# ├── example
# │   ├── bsd_example_out.txt
# │   ├── example_test.txt
# │   └── linux_example_out.txt
# ├── Makefile
# ├── result
# │   └── example.txt
# └── testing_data
    # ├── test1.txt
    # ├── test2.txt
    # ├── test3.txt
    # ├── test4.txt
    # ├── test5.txt
    # └── test6.txt

OUTPUT_DIR="./result"
SERVER_HOST="nplinux1.cs.nctu.edu.tw"
#SERVER_HOST="localhost"
SERVER_PORT="33916"

mkdir -p $OUTPUT_DIR   # create the output directory if not exists

if [ ! -f ./client ]; then
    gcc client.c -o client
fi

# Examples
echo "Testing example file..."
./client $SERVER_HOST $SERVER_PORT ./example/example_test.txt > $OUTPUT_DIR/example_test-out.txt
if diff $OUTPUT_DIR/example_test-out.txt ./example/linux_example_out.txt
then
    echo "pass!"
else
    echo "not pass!"
fi

# Test1 ~ Test2
for i in {1..6}
do
    echo "Generating test$i..."
    ./client $SERVER_HOST $SERVER_PORT ./testing_data/test$i.txt > $OUTPUT_DIR/test$i-out.txt
done
