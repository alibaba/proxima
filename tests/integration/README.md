How to run all integration test cases outside docker 
1、cd build/
2、sh ./tests/integration/run.sh

How to run one test suite outside docker
1、cd build/
2、sh ./tests/integration/run.sh test_admin_agent

How to run one test case outside docker
1、cd build/
2、sh ./tests/integration/run.sh test_admin_agent.TestAdminAgent.test_create_collection

##########################################
How to run integration test case in docker
1、cd build/
2、update the tests/integration/run.sh as following 
sudo docker run -it --net=host -v ${PWD}/..:/v ghcr.io/alibaba/proxima-be bash
3、sh ./tests/integration/run.sh
4、cd /v/tests/integration && sh ./script/setup_ci.sh build /v
5、export PYTHONPATH=/v/sdk/python; export BUILD_DIR_NAME=build; export SRC_PATH=/v
6、python3 src/case/test_xxx.py
