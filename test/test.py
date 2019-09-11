#!/usr/bin/python2
import os, subprocess, string, sys, tempfile, shutil, signal, inspect
from getopt import getopt
from optparse import OptionParser
import multiprocessing as mp
import Queue
import pexpect
import re
import filecmp
import difflib

class Failure(Exception):
   def __init__(self, value, detail=None):
      self.value = value
      self.detail = detail
   def __str__(self):
      if self.detail is not None:
         return str(self.value) + "\n" + str(self.detail)
      else:
         return str(self.value)


class Test(object):

    IN_PROGRESS = 1
    PASSED      = 2
    FAILED      = 3

    name       = None
    desc       = None
    cmd        = None
    timeout    = None
    children   = list()
    open_files = list()
    point_val  = 0

    def __init__(self, project_path = None, test_path = None, log = None, use_gdb = False, use_valgrind = False):
        self.logfd        = log
        self.project_path = project_path
        self.notices      = list()
        self.use_gdb      = use_gdb
        self.use_valgrind = use_valgrind
        self.state        = Test.IN_PROGRESS
        self.test_path    = test_path

    def is_failed(self):
        return self.state == Test.FAILED

    def is_passed(self):
        return self.state == Test.PASSED

    def fail(self, reason = None):
        self.state = Test.FAILED
        if reason is not None:
            self.notices.append(reason)

    def warn(self, reason):
        self.notices.append(reason)

    def after(self):
        pass

    def done(self):
        self.logfd.flush()
        if not self.is_failed():
            self.state = Test.PASSED
    
    def points(self):
        return self.point_val

    def run_util(self, args):
        self.log(" ".join(args))
        child = subprocess.Popen(args, cwd=self.project_path, stdout=self.logfd,
                stderr=self.logfd)
        self.children.append(child)
        status = child.wait()
        return status

    def log(self, msg):
        self.logfd.write(msg + "\n")
        self.logfd.flush()

    def points(self):
        return self.point_val

    def terminate(self):
        if not self.state == Test.FAILED:
            self.fail("terminated")

        for f in self.open_files:
            try:
                fname = f.name
                f.close()
                os.remove(fname)
            except OSError:
                pass

        for child in self.children:
            try:
                os.kill(child.pid, signal.SIGTERM)
            except OSError:
                pass

    def __str__(self):
        s = "test " + self.name + " "
        if self.is_failed():
            s += "FAILED"
            if self.point_val > 0:
                s += " (0 of " + str(self.point_val) + ")"
        elif self.is_passed():
            s += "PASSED"
            if self.point_val > 0:
                s += " (" + str(self.point_val) + " of " + str(self.point_val) + ")"

        s += "\n"
        s += " (" + str(self.desc) + ")\n"

        if self.is_failed():
            if self.cmd is not None:
                s += "\n Test it yourself by running\n"
                s += " " + self.cmd + "\n"
        return s


# The build test is special in that it will try to build the project even if the
# test fails, so that other tests may be run
class BuildTest(Test):
   name    = "build"
   desc    = "build project using make"
   timeout = 10
   targets = None

   def run(self):
      self.make(self.targets)
      self.done()

   def make(self, files=[], required=True):
      failures = list()
      self.log("Trying to build project using make")
      status = self.run_util(["make", "MODE=DEBUG"])
      if status != 0:
         failures.append("make failed (error " + str(status) + ")")
      missing_files = []
      for f in files:
         if not os.path.exists(self.project_path + "/" + f):
            failures.append(
                  "required file '" + f + "' does not exist after make")
            missing_files.append(f)
      if required:
         for failure in failures:
            self.fail(failure)
      return (len(failures) == 0)

   def clean(self, files=[], required=True):
      failures = list()
      warnings = list()
      self.log("Trying to clean project using make clean")
      status = self.run_util(["make", "clean"])
      for pattern in files:
         for f in glob.glob(pattern):
            if os.path.exists(self.project_path + "/" + f):
               warnings.append(
                     "file '" + f + "' exists after make clean, removing")
               # os.remove(self.project_path + "/" + f)
      if required:
         for failure in failures:
            self.fail(failure)
         for warning in warnings:
            self.warn(warning)
      return (len(failures) == 0)


class HawknestBuild(BuildTest):
   name    = "hawknest-build"
   desc    = "Hawknest builds cleanly using make"
   timeout = 20
   targets = ["emu"]


class HawknestTest(BuildTest, Test):
    name      = None
    desc      = None
    timeout   = 5
    point_val = 0
    tester    = ""
    targets = ["tests"]
    
    def run(self): 

        self.log("Running Hawknest test ROM: " + self.test_path + "/" + self.name)

        is_success = self.run_util(["make", "tests"])
        if is_success:
            return

        fname = os.path.join(self.test_path, self.name + ".out")

        try :
            f = open(fname, "r")
            output = f.readlines()
        except:
            print("Testing Error: could not open solution file for " + self.name)
            self.fail("tester failed due to internal error")
            return

        args = ["bin/hawknest-gcc-debug",  "bin/" + self.name]

        tmpname = ('tmp' + self.name + '.out')
        of = open(tmpname, "w+")
        self.open_files.append(of)
        child = subprocess.Popen(args, cwd=self.project_path, stdout=of)
        self.children.append(child)
        child.wait()

        of.close()

        if not filecmp.cmp(fname, tmpname):
            s1 = open(fname).readlines()
            s2 = open(tmpname).readlines()
            diff = difflib.unified_diff(s1, s2)
            fail_str = "\tDiff of state dump:\n"
            for line in diff:
                fail_str += "\t\t" + line

            self.fail("Output does not match solution: \n" + fail_str)

        os.remove(tmpname)
        self.done()


parser = OptionParser(
      usage="Usage: %prog [options] tests...",
      description="Run specified tests on the project. "
      "By default, all tests are run.")
parser.add_option("-l", "--list",
      action="store_true", dest="print_list", default=False,
      help="Print a list of all available tests and their descriptions")
parser.add_option("-c", "--continue", action="store_true", dest="_continue",
      default=False, help="Continue testing after a test failure")
parser.add_option("-n", "--no-copy", action="store_false", dest="local",
      default=False,
      help="This will copy the project to a temporary directory for testing,"
      "e.g. for mitigating the performance of a network file system")
parser.add_option("-p", "--project-path", dest="project_path", default=os.getcwd(),
      help="Path to the directory containing the project to be tested "
      "(default: current directory)")
parser.add_option("-t", "--test-path", dest="tester_path", default="test",
      help="Path to the location of the test files")
parser.add_option("-q", "--quiet", action="store_true", dest="quiet",
      default=False)
parser.add_option("-m", "--no-timeout", action="store_true", dest="notimeout",
      default=False, help="Ignore timeouts on tests")
parser.add_option("-g", "--gdb", action="store_true", dest="gdb",
      default=False, help="Run project executable inside a gdb session. " +
      "implies -m")
parser.add_option("-v", "--valgrind", action="store_true", dest="valgrind",
      default=False, help="Run project executable inside a valgrind session. ")
parser.add_option("-b", "--no-build", action="store_false", dest="build",
      default=True,
      help="do not automatically run build test before running other tests")
parser.add_option("-f", "--factor", dest="factor", default=1,
      help="multiply all timeout lengths by FACTOR")

def quit_now(test):
    test.terminate()
    sys.exit()

def run_test(test, queue):
    signal.signal(signal.SIGTERM, lambda signum, frame: quit_now(test))
   # create a new process group so we can easily kill all children of this proc
    os.setpgrp()
    try:
        test.run()
    except Failure as f:
        test.fail(str(f.value))
        (type, value, tb) = sys.exc_info()
        # Don't print the stack trace, students find it unreadable
        traceback.print_exception(type, value, None, file=sys.stdout)
    except Exception as e:
        test.fail("Unexpected exception " + str(e))
        (type, value, tb) = sys.exc_info()
        # Don't print the stack trace, students find it unreadable
        traceback.print_exception(type, value, None, file=sys.stdout)
    finally:
        queue.put(test, block=True)


def main(build_test, all_tests):

    (options, args) = parser.parse_args()
    if options.gdb:
        options.notimeout = True

    tempdir = None
    if options.local:
        tempdir = tempfile.mkdtemp()
        if not options.print_list:
            shutil.copytree(src=options.project_path, dst=tempdir + "/p", symlinks=True)
        project_path = tempdir + "/p"
    else:
        project_path = options.project_path

    log = sys.stdout

    if options.quiet:
        log = open("/dev/null", "w")

    _list = list()

    if options.build:
        _list.append(build_test)
    if len(args) == 0:
        _list.extend(all_tests)

    for test_name in args:
        if test_name == "all":
            _list.extend(all_tests)
        if test_name == "build":
            _list.append(build_test)
        else:
            match = None
            for test in all_tests:
                if test.name == test_name:
                    match = test
                    break
            if match is not None:
                _list.append(match)
            else:
                sys.stderr.write(test_name + " is not a valid test\n")
                exit(2)

    if options.print_list:
        for test in _list:
            print(test.name + "-" + test.desc)
        sys.exit(0)

    ran           = list()
    tests_passed  = 0
    points        = 0
    total_points  = 0
    tests_skipped = 0
    quitnow       = False

    for tester in _list:
        test = tester(project_path, log=log, use_gdb=options.gdb, use_valgrind=options.valgrind, test_path=options.tester_path)

        log.write("\n")
        log.write("*" * 70 + "\n")
        log.write("\n")
        log.write("Test [" + test.name + "]\n")
        log.write(str(test.desc)+ "\n")
        log.write("\n")
        log.write("*" * 70 + "\n")
        log.flush()

        # run the test in a new process
        # KCH: Note that this deadlocks with Python 3 implementation
        # of MP Queues, still not sure why, but suspect forking semantics
        result_queue = mp.Queue()
        p = mp.Process(target=run_test, args=(test,result_queue))
        p.start()

        if options.notimeout or test.timeout is None:
            timeout = None
        else:
            timeout = test.timeout * float(options.factor)
        try:
            # wait for the test result
            result = result_queue.get(block=True, timeout=timeout)
            p.join()
        except Queue.Empty:
            test.fail("Timelimit (" + str(timeout) + "s) exceeded (your program likely did not terminate)")
            result = test
        except KeyboardInterrupt:
            test.fail("User interrupted test")
            result = test
            quitnow = True
        finally:
            try:
                os.kill(p.pid, signal.SIGTERM)
            except OSError as e:
                pass

        result_queue.close()

        try:
            result.logfd = log
            result.after()
        except Exception as e:
            (type, value, tb) = sys.exc_info()
            traceback.print_exception(type, value, tb)

        ran.append(result)
        total_points += test.points()

        log.flush()
        if not result.is_failed():
            points += test.points()
            tests_passed += 1

        log.write("\n")
        log.write("\n")
        log.write(str(result) + "\n")

        if result.is_failed():
            log.write("Failure reason:\n")
            for n in result.notices:
                log.write("\t" + n + "\n")


        if result.is_failed() and not options._continue or quitnow:
            log.write("Skipped " + str(len(_list) - len(ran)) + " tests.\n")
            log.write("To keep testing after failing a test, use flag '-c' or '--continue'\n")
            sys.exit(1)
         
    log.write("*" * 70 + "\n")
    log.write("Summary:\n")

    for test in ran:
        log.write(str(test) + "\n")

    log.write("Passed " + str(tests_passed) + " of " + str(len(ran)) +
         " tests.\n")

    log.write("Overall " + str(tests_passed) + " of " + str(len(_list)) + "\n")

    if total_points > 0:
        log.write("Points " + str(points) + " of " + str(total_points) + "\n")

    if options.quiet:
        for test in ran:
            print(str(test))

        print("Overall " + str(tests_passed) + " of " + str(len(_list)))
        if total_points > 0:
            print("Points " + str(points) + " of " + str(total_points))

    if tempdir is not None:
        shutil.rmtree(tempdir)

    if tests_passed == len(_list):
        sys.exit(0)
    else:
        sys.exit(1)


def get_desc(name):
    desc = ""
    sfile = os.path.join(os.getcwd(), 'test', name)
    with open(sfile, 'r') as f:
        for line in f.readlines():
            if "DESC" in line:
                return line.split("DESC:")[1].strip()


if __name__ == "__main__":

    all_tests  = []
    build_test = HawknestBuild
    test_cnt   = 0

    for tfile in sorted(filter(lambda x: x.endswith('.s') == True, os.listdir('test'))):

        tname = os.path.splitext(tfile)[0]
        tdesc = get_desc(tfile)

        members = {
            'name': tname,
            'tester': 'test/' + tfile,
            'desc': tdesc,
            'timeout': 5,
            'point_val': 10 # default, but won't end up being this
        }
        newclass = type(tname, (HawknestTest,), members)
        all_tests.append(newclass)
        setattr(sys.modules[__name__], tname, newclass)

        test_cnt += 1

    test_val = 100/test_cnt

    for test in all_tests:
        test.point_val = test_val

    main(build_test, all_tests)
