import subprocess

basic_tests = [
    "test1", "test2", "test3", "test4", "test5"
]
BASIC_NEEDED = len(basic_tests)
BASIC_POINTS = 0.7

advanced_tests = [
    "testadvanced1", "testadvanced2", "testadvanced3", "testadvanced4", "testadvanced5", "testadvanced6", "testadvanced7"
]
ADVANCED_NEEDED = 4
ADVANCED_POINTS = 0.5

def run_test(test_name):
    try:
        command = f"make run_{test_name}"
        print(command)
        st = subprocess.run(command,shell=True)
        return st.returncode == 0
    except:
        return False


def run_tests(tests):
    passed=0
    for test in tests:
        if run_test(test):
            passed += 1

    return passed

def calc_grade(basic, advanced):
    return (basic * BASIC_POINTS) + (advanced * ADVANCED_POINTS)


if __name__ == "__main__":
    basic_passed = run_tests(basic_tests)
    advanced_passed = run_tests(advanced_tests)

    print(f"You are passing {basic_passed}/{len(basic_tests)} basic tests and {advanced_passed}/{len(advanced_tests)} advanced tests")

    print(f"Your (tentative) grade is a {calc_grade(basic_passed, advanced_passed)}")

    if basic_passed < BASIC_NEEDED:
        print(f"You must still complete {BASIC_NEEDED - basic_passed} basic tests in order to pass")
    
    if advanced_passed < ADVANCED_NEEDED:
        print(f"You must still complete {ADVANCED_NEEDED - advanced_passed} advanced tests in order to pass")

    if  advanced_passed >= ADVANCED_NEEDED and \
        basic_passed >= BASIC_NEEDED:
        print("You are currently passing all required tests and eligible to do the bonus assignments")
        print("NOTE: Bonus assignments are graded separately")
        print("NOTE: 0.5 point style points not considered in this script")
