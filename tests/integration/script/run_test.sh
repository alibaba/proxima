
# run test
# should run in container
#!/bin/bash
SE_DIR=$1
IT_DIR=${SE_DIR}/tests/integration

shift

if [ $# -eq 0 ]; then
	echo "Test all integration test"
	((cd $IT_DIR; python3 $IT_DIR/src/run.py $IT_DIR/src) || exit 1)
elif [ $# -gt 0 ]; then
    while [ $# -gt 0 ]; do
	    echo "Test specific integration test $1"
	    ((cd $IT_DIR; python3 $IT_DIR/src/run.py $IT_DIR/src $1) || exit 1)
        shift
    done
fi

