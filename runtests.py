#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import os
import subprocess
import enum
import argparse
#from termcolor import colored

# TODO:
# - Implement checking of return status of emulator:
#   // RUN: picco %s -o output.o
#   // RUN: plinker output.o
#   // RUN: mipselemu output.out
#   // RETURNED: 0
#
# - Implement CHECK NOT.

#===-----------------------------------------------------------------------===#
#  File check                                                                 #
#===-----------------------------------------------------------------------===#


@enum.unique
class TestStatus(enum.Enum):
    OK        = 0
    FAIL      = 1
    WARNING   = 2
    NORUN     = 3
    NOCHECK   = 4


class Colors:
    # Colors
    HEADER    = '\033[95m'
    OKBLUE    = '\033[94m'
    OKGREEN   = '\033[92m'
    WARNING   = '\033[93m'
    FAIL      = '\033[91m'
    ENDC      = '\033[0m'
    # Style
    BOLD      = "\033[1m"


class LineRecord:
    def __init__(self, value, status):
        self.value = value
        self.found = status


class CheckRecord:
    def __init__(self, value, status):
        self.value = value
        self.found = status


def addcwd(filename):
    return '{0}/{1}'.format(os.getcwd(), filename)


def print_log(filename, msg):
    log_record = '{0}\nTEST: {1}\n{2}'.format('-'*80, filename, msg)
    print(log_record, file=logfile)


class FileCheck:
    def __init__(self, filename):
        self.filename    = filename
        self.f           = open(filename, 'r', encoding='utf8')
        self.checks      = []
        self.output      = [] # list of LineRecord
        self.output_idx  = 0

    def get_output(self, command):
        # check_output() approach
        return subprocess.check_output(command,
                                       shell=True,
                                       universal_newlines=True,
                                       timeout=timeout_seconds)

    # process pipe approach
    def get_output_pipe(self, command):
        command = command.strip().split(' ')
        p = subprocess.Popen(command, stdout=subprocess.PIPE)
        outputb = p.communicate()[0]
        returncode = p.returncode
        return outputb.decode('utf-8')

    def run(self, command):
        global args
        # TODO: Run command and save output to the list 'output'
        command = command.replace('%s', self.filename)
        command = command.replace('picco ', addcwd('picco '))
        command = command.replace('plinker', addcwd('plinker'))
        command = command.replace('mipselemu', addcwd('mipselemu'))
        if args.v:
            print(command[1:], end='')
        try:
            output = self.get_output(command)
            for ln in output.split('\n'):
                self.output.append(LineRecord(ln, False))
        except subprocess.CalledProcessError as e:
            print_log(self.filename, e)
        except subprocess.TimeoutExpired as e:
            print_log(self.filename, e)

    def check_line(self, line):
        i = self.output_idx
        while i < len(self.output):
            if self.output[i].value == line.strip():
                self.output[i].found = True   # TODO: Remove
                self.output_idx = i
                return True
            i += 1
        return False

    def check_not_line(self, line):
        i = self.output_idx
        while i < len(self.output):
            if self.output[i].value != line.strip():
                self.output[i].found = True   # TODO: Remove
                self.output_idx = i
                return True
            i += 1
        return False

    # Flexible starts_with function
    def is_comment_starting_with(self, line, start_str):
        self.tmp_line = ''
        line = line.strip()             # Remove eventual blank characters
        if line.startswith('//'):
            line = line[2:].strip()     # Remove the '//' and strip
            if line.startswith(start_str):
                self.tmp_line = line[len(start_str):]
                return True
        return False

    # FIXME: Implement check of every single check line.
    def check(self):
        global args
        has_check_line = False
        has_run_line = False
        if args.v:
            print('TEST: %s' % self.filename)
        for line in self.f:
            if self.is_comment_starting_with(line, 'CHECK:'): # We have a comment
                line_to_check = self.tmp_line
                found = self.check_line(line_to_check)
                self.checks.append(CheckRecord(line_to_check, found))
                has_check_line = True
            elif self.is_comment_starting_with(line, 'CHECKNOT:'):
                line_to_check = self.tmp_line
                found = self.check_not_line(line_to_check)
                self.checks.append(CheckRecord(line_to_check, found))
                has_check_line = True
            elif self.is_comment_starting_with(line, 'RUN:'):
                line_to_check = self.tmp_line
                command_to_run = line[7:]
                self.run(command_to_run)
                has_run_line = True
        if not has_run_line:
            status = TestStatus.NORUN
        elif not has_check_line:
            status = TestStatus.NOCHECK
        else:
            # This can be changed to FAIL in following lines
            status = TestStatus.OK
        #for x in self.output:
        #    print x.value
        #    if not x.found:
        #        print("Error: line not found: " %s x.value)
        for x in self.checks:
            if not x.found:
                status = TestStatus.FAIL
        return status


#===-----------------------------------------------------------------------===#
# Driver code                                                                 #
#===-----------------------------------------------------------------------===#


# Returns sorted list of *.ext files at path.
def get_files(path, ext):
    try:
        files = []
        for name in os.listdir(path):
            filename = os.path.join(path, name)
            if os.path.isfile(filename) and filename.endswith(ext):
                files.append(filename)
        return sorted(files)
    except FileNotFoundError as e:
        print('error: %s' % e, file=sys.stderr)
        return []


def print_fail(msg):
    print(Colors.FAIL + msg + Colors.ENDC)


def print_norun(msg):
    print(Colors.HEADER + msg + Colors.ENDC)


def print_summary(results):
    test_pass = 0
    test_fail = 0

    if len(results) > 1:
        print('%s' % ('-'*80))
        print(' SUMMARY')
        print('%s' % ('-'*80))

    for result in results:
        status = result[0]
        input_file = result[1].filename
        if status == TestStatus.OK:
            test_pass += 1
            print('PASS:  %s' % input_file)
        elif status == TestStatus.FAIL:
            test_fail += 1
            print_fail('FAIL:  %s' % input_file)
        elif status == TestStatus.NORUN:
            print_norun('NORUN: %s' % input_file)
        elif status == TestStatus.NOCHECK:
            print_norun('NOCHECK: %s' % input_file)

    if len(results) > 1:
        #print('%s' % ('-'*80))
        print('\nPASS: %d' % test_pass)
        print('FAIL: %d' % test_fail)


def check_file(input_file):
    file_check = FileCheck(input_file)
    status = file_check.check()
    return (status, file_check)
    #if status == TestStatus.OK:
    #    test_pass += 1
    #    print("PASS:  %s" % input_file)
    #elif status == TestStatus.FAIL:
    #    test_fail += 1
    #    print_fail("FAIL:  %s" % input_file)
    #elif status == TestStatus.NORUN:
    #    print_norun("NORUN: %s" % input_file)
    #elif status == TestStatus.NOCHECK:
    #    print_norun("NOCHECK: %s" % input_file)


if __name__ == '__main__':
    global args
    global logfile
    global timeout_seconds

    argparser = argparse.ArgumentParser()
    argparser.add_argument('-v', default=None, action='count', help='Verbose')
    argparser.add_argument('-l', help='Log filename', type=str)
    argparser.add_argument('-t', help='Timeout in seconds', type=int)
    argparser.add_argument('inputfile', nargs='*', default=None)
    args = argparser.parse_args()

    if args.t:
        timeout_seconds = args.t
    else:
        timeout_seconds = 10

    if args.l:
        log_filename = args.l
    else:
        log_filename = 'runtests.log'

    logfile = open(log_filename, 'w')
    results = []

    if args.inputfile and os.path.isfile(args.inputfile[0]):
        input_file = args.inputfile[0]
        results.append(check_file(input_file))
    else:
        if args.inputfile:
            input_dir = args.inputfile[0]
        else:
            input_dir = 'test/c'
        files = get_files(addcwd(input_dir), '.c')
        for input_file in files:
            results.append(check_file(input_file))

    print_summary(results)

    if not logfile is None:
        logfile.close()
