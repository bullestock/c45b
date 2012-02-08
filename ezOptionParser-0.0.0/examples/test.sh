#!/bin/bash 

# Search array if contains element.
# elementExists 1 "1 2 3"
# echo $?
function elementExists {
	local arr=$2;
	if [ -z "$1" ]; then return; fi;
	for i in ${arr[@]}; do
		if [ $i == $1 ]; then return 1; fi;
	done;
	return 0;
}

# Allow user to run specific tests only, such as: test.sh "1 10 11"
if [ -z "$1" ]; then
	NTESTS=20
	TESTS=`seq 1 1 $NTESTS`
else
	TESTS="$1"
	NTESTS=`echo "$1" | wc -w`
fi

if [ -z $TMPDIR ]; then
	TMPDIR="."
fi

testctr=0

####################################################################
elementExists 1 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 1..."
	./short 2>&1 | grep "Expected 2 arguments" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
elementExists 2 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 2..."
	./short a 2>&1 | grep "but got 1" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
elementExists 3 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 3..."
	./short a b 2>&1 | grep "Second file: b" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
elementExists 4 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 4..."
	./short a -d b 2>&1 | grep "but got 1" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
elementExists 5 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 5..."
	./short -d a b 2>&1 | grep "\-d was set" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
elementExists 6 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 6..."
	./long -debug a b 2>&1 | grep "\-d was set" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
elementExists 7 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 7..."
	./long --dbg a b 2>&1 | grep "\-d was set" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
elementExists 8 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 8..."
	./long --debug a b 2>&1 | grep "\-d was set" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
elementExists 9 "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test 9..."
	./multi 2>&1 | grep "Missing required" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
CUR=10
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./multi -p 2>&1 | grep "unexpected number of arguments" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
CUR=11
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./multi -p 1,2 2>&1 | grep "unexpected number of arguments" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
CUR=12
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./multi -p 1,2,3 --point -1,1e6,-.314 2>&1 | grep "Point 1: \-1" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
CUR=13
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./pretty -p 2>&1 | grep "Not set" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
CUR=14
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./pretty -p -l 1:-9:0 2>&1 | grep "List: 1 -9 0" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
CUR=15
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./pretty a b -p -foo -l 1:-9:0 c d 2>&1 | grep "1: \-foo" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi
####################################################################
CUR=16
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./pretty a b -p -foo -l 1:-9:0 c d 2>&1 | grep "3: b" > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi

####################################################################
CUR=17
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./full -i in2.txt 2>&1 > tmp.$CUR
	cmp test$CUR.truth tmp.$CUR 2>&1 > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi

####################################################################
CUR=18
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./full -i in2.txt -d 987654321 -dbl 98.123e-120 +d +e tmp.$CUR 2>&1 > /dev/null
	cmp test$CUR.truth tmp.$CUR 2>&1 > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi

####################################################################
CUR=19
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./full -d 987654321 -dbl 98.123e-120 +d +e tmp.$CUR 2>&1 > /dev/null
	cmp test$CUR.truth tmp.$CUR 2>&1 > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi

####################################################################
CUR=20
elementExists $CUR "$TESTS"
if [ $? -eq 1 ]; then
	echo -n "Test $CUR..."
	./full +i in3.txt -d 987654321 -dbl 98.123e-120 -n 2020202 +d +e tmp.willnotbecreated 2>&1 > /dev/null
	cmp test$CUR.truth tmp.$CUR 2>&1 > /dev/null
	if [ "$?" -ne "0" ]; then
		echo FAILED
	else
		echo PASSED
		testctr=$((testctr+1))
	fi
fi

####################################################################
echo $testctr of $NTESTS tests passed.