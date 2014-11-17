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

# Test1 ~ Test2
for i in {1..6}
do
    echo "Generating test$i..."
    ./client nplinux1.cs.nctu.edu.tw 33916 testing_data/test$i.txt  > result_archive/$i-out.txt;
done
