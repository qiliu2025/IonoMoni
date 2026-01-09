#-*- coding:utf-8 -*-
# =================================================================
# Provides command line operations and execution functions
# =================================================================
import logging
import os
import threading
import time
import unittest
import subprocess as sub
import concurrent.futures
from concurrent.futures import ThreadPoolExecutor,wait,ALL_COMPLETED
from threading import Thread
from multiprocessing import  Process

logger = logging.getLogger()
########################################################################
# jdhuang : 2021.11.05
# python2 thread running
########################################################################
class myThread(threading.Thread):
    def __init__(self, threadId, name, counter, cmd_list):
        threading.Thread.__init__(self)
        self.threadId = threadId
        self.name = name
        self.counter = counter
        self.cmd_list = cmd_list

    def run(self):
        # logging.info("start thread : " + self.name)
        # threadLock.acquire()
        multi_cmd(self.name, self.counter, self.cmd_list)
        # threadLock.release()

########################################################################
# jdhuang : 2023.09.18
# python2 thread running
########################################################################

class MyProcess(Process): #Inherited Process class
    def __init__(self, func, args):
        super(MyProcess,self).__init__()
        self.func = func
        self.args = args

    def run(self):
        self.func(*self.args)

########################################################################
# jdhuang : 2023.09.18
# python2 thread running
########################################################################
# Create a subclass of Thread
class MyThread3(Thread):
    def __init__(self, func, args):
        '''
        :param func: A callable object
        :param args: Parameters of a callable object
        '''
        Thread.__init__(self)   # Don't forget to call Thread's initialization method
        self.func = func
        self.args = args

    def run(self):
        self.func(*self.args)

########################################################################
# jdhuang : 2021.11.05
# multi cmd submit
# param[in] : cmd_list  The cmd list
########################################################################


def multi_cmd(name, counter, cmd_list):
    logging.info("%s: %s" % (name, str(counter)))
    for i in range(len(cmd_list)):
        os.system(cmd_list[i])

########################################################################
# jdhuang : 2021.11.05
# resize the cmd list when your number of cmd larger than thread number
# param[in] : list_info     The cmd list
# param[in] : per_list_len  The len you want to resize
########################################################################


def list_of_groups(list_info, per_list_len):
    '''
    :param list_info:     list of multi-cmd
    :param per_list_len:  len of list
    :return:
    '''
    list_of_group = zip(*(iter(list_info),) * per_list_len)
    end_list = [list(i) for i in list_of_group]
    count = len(list_info) % per_list_len
    end_list.append(list_info[-count:]) if count != 0 else end_list
    return end_list

#####################################################################
# jdhuang : 2023.09.18
# Change the path and run the command
#####################################################################
def change_path_and_run_cmd(path, cmd):
    # Rest for two seconds and wait for another process to start, otherwise switching paths is prone to errors
    # time.sleep(2)
    if os.path.exists(path) == False:
        os.makedirs(path)
    os.chdir(path)
    ret = os.system(cmd)
    return ret

#####################################################################
# jdhuang : 2023.09.18
# multi cmd exe, change path
# param[in] : cmd_list   The cmd  list
# param[in] : path_list  The path list
# param[in] : thread     The number of thread
#####################################################################
def execute_multi_path(cmd_list, path_list, thread):
    '''execute command by multi thread'''
    # mismatch
    if len(cmd_list) != len(path_list):
        return False

    # Judgment model
    if len(cmd_list) > thread:
        cmd_dict = dict()
        path_dict = dict()

        # sort
        nowThread = 0
        for i in range(len(cmd_list)):
            if nowThread not in cmd_dict:
                cmd_dict[nowThread] = list()
                path_dict[nowThread] = list()
            if len(cmd_dict[nowThread]) >= thread:
                nowThread = nowThread + 1
                if nowThread not in cmd_dict:
                    cmd_dict[nowThread] = list()
                    path_dict[nowThread] = list()
            cmd_dict[nowThread].append(cmd_list[i])
            path_dict[nowThread].append(path_list[i])

        # Multithreaded execution
        for idxThread in cmd_dict:
            nowCmdList = cmd_dict[idxThread]
            nowPathList = path_dict[idxThread]
            threads = []
            for i in range(len(nowCmdList)):
                # thread_i = MyThread3(change_path_and_run_cmd, (nowPathList[i],nowCmdList[i]))
                thread_i = MyProcess(change_path_and_run_cmd, (nowPathList[i],nowCmdList[i]))               
                threads.append(thread_i)
            for thread_i in threads:
                thread_i.start()
            for thread_i in threads:
                thread_i.join()
                

    elif len(cmd_list) < thread and len(cmd_list) > 0:
        threads = []
        for i in range(len(cmd_list)):
            # thread_i = MyThread3(change_path_and_run_cmd, (path_list[i],cmd_list[i]))
            thread_i = MyProcess(change_path_and_run_cmd, (path_list[i],cmd_list[i]))
            threads.append(thread_i)
        # Start process
        for thread_i in threads:
            thread_i.start()
        for thread_i in threads:
            thread_i.join()
    else:
        logger.info("No process to execute!")
        
    return True


#####################################################################
# jdhuang : 2021.11.05
# multi cmd execute
# param[in] : cmd_list  The cmd list
# param[in] : thread    The number of thread
########################################################################


def execute_multi(cmd_list, thread):
    '''execute command by multi thread'''
    if len(cmd_list) > thread:
        new_list = list_of_groups(cmd_list, int(len(cmd_list)/thread)+1)
        thread = min(len(new_list), thread)
        #threadLock = threading.Lock()
        threads = []
        for i in range(thread):
            thread_i = myThread(i, "Thread_"+str(i), i, new_list[i])
            thread_i.start()
            threads.append(thread_i)
        for thread_i in threads:
            thread_i.join()
    elif len(cmd_list) <= thread and len(cmd_list) > 0:
        #threadLock = threading.Lock()
        threads = []
        for i in range(len(cmd_list)):
            newList = list()
            newList.append(cmd_list[i])
            thread_i = myThread(i, "Thread_"+str(i), i, newList)
            thread_i.start()
            threads.append(thread_i)
        for thread_i in threads:
            thread_i.join()
    else:
        logger.info("No process to execute!")

########################################################################
# split cmd when lager than thread number
########################################################################

def splitCMDs(list_info, per_list_len):
    '''
    :param list_info:   list
    :param per_list_len:  The length of each small list
    :return:
    '''
    listGroup = zip(*(iter(list_info),) *per_list_len)
    endList = [list(i) for i in listGroup] # i is a tuple
    count = len(list_info) % per_list_len
    endList.append(list_info[-count:]) if count !=0 else endList
    return endList

########################################################################
# run multi-cmd
########################################################################

def runCMDs(cmdList, timeOut=24*60*60):
    # runCMD(cmdList[0],"", timeOut, 1, False)
    for i in range(len(cmdList)):
        try:
            logging.info(f"Run cmd {cmdList[i]}")
            runCMD(cmdList[i],"", timeOut, 1, False)
        except:
            logging.error(f"{cmdList[i]} failed, please check!")

########################################################################
# run multi-cmd, thread is number
########################################################################

def multiRunCMD(cmdList,thread):
    '''execute command by multi thread'''

    if len(cmdList) > thread:
        newList = splitCMDs(cmdList,int(len(cmdList)/thread)+1)
        thread = min(len(newList),thread)
        # runCMDs(newList[0], timeOut=60)
        with ThreadPoolExecutor(max_workers=thread) as pool:
            all_works = [pool.submit(runCMDs, newList[i], timeOut=24*60*60) for i in range(thread)]
            wait(all_works,return_when=ALL_COMPLETED)
    elif len(cmdList) < thread and len(cmdList) > 0:
        for cmd in cmdList:
            try:
                runCMD(cmd)
            except:
                logging.error(f"{cmd} failed, please check!")
    else:
        logging.info("No process to execute!")

########################################################################
# run cmd
########################################################################

def runCMD(cmd,logFile ="",timeOut=24*60*60,repeatMaxiter=1,logInfo=False):
    ### test
    if logFile:
        cmd = cmd + " > "+logFile

    #print("begin carry on ",cmd)
    loggerFunc = logging.debug
    if logInfo:
        loggerFunc = logging.info

    loggerFunc("begin carry on "+cmd)

    timeBeg = time.time()

    ### Repeat Only when timeout(May be network or cycle
    for i  in range(1,repeatMaxiter+1):
        try:
            res = sub.Popen(cmd, shell=True, stdout=sub.PIPE, stderr=sub.PIPE)
            runingResult = res.wait(timeout = timeOut)
            break
        except sub.TimeoutExpired as e:
            res.kill()
            if  i < repeatMaxiter:
                logging.warning(f"Time Out!! {i} Repeat Carry on "+cmd)
                continue
            logging.warning(e,"Time Out! " + cmd)
            raise TimeoutError(f"Time out!! command is {cmd}")

    if runingResult !=0:
        wrongInfo = "Run wrong in cmd: "+cmd+"\n"
        wrongInfo += "DETAIL INFORMATION" + 30*"="+"\n"
        wrongInfo += "CMD STDERR CONTEXT:\n"
        wrongInfo += (res.stderr.read()).decode(errors="ignore")+"\n"
        wrongInfo += "CMD STDOUT CONTEXT:\n"
        wrongInfo += (res.stdout.read()).decode(errors="ignore")+"\n"
        wrongInfo += "DETAIL INFORMATION" + 30*"="+"\n"
        raise RuntimeError(wrongInfo)

    timeEnd = time.time()

    loggerFunc("finish! Totally spends {time:.3f} s".format(time=timeEnd-timeBeg))
