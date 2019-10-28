#! /bin/bash

test_cases=( "TEST_MESSAGE_PROCESSOR" "TEST_ADMISSION" "TEST_PROTECTOR_PLUGIN" "TEST_SUBSCRIPTION_FLOW"  "TEST_SUBSCRIPTION" "TEST_E2AP_INDICATION" "TEST_E2AP_CONTROL" "TEST_E2SM" "TEST_JSON" "TEST_X2_SGNB"  "TEST_SLIDING_WINDOW" "TEST_XAPP" )

# Run through test cases
for((i = 0; i <  ${#test_cases[@]}; i++)); do
    test="${test_cases[$i]}";
    echo "======================================"
    ./${test} > /dev/null

    if [ $? -eq 0 ]
    then
	echo -e "UNIT TEST CASE: ${test} \e[32m OK \e[0m ";
    else
	echo -e "UNIT TEST CASE: ${test} \e[31m  FAILED \e[0m";
    fi

    # valgrind -q  --tool=memcheck --leak-check=yes --track-origins=yes --leak-check=full ./${test} > /dev/null
    # if [ $? -eq 0 ]
    # then
    # 	echo -e "Valgrind Test on ${test}  \e[32m OK \e[0m ";
    # else
    # 	echo -e "Valgrind Test on  ${test} \e[31m FAILED \e[0m";
    # fi

done


#===============================
# Generate coverage report
cd ../
gcovr -r . --html > coverage_report.html
