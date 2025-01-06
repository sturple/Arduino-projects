from threading import Thread
import time

class ThreadWithReturnValue(Thread):
    
    def __init__(self, group=None, target=None, name=None,
                 args=(), kwargs={}, Verbose=None):
        Thread.__init__(self, group, target, name, args, kwargs)
        self._return = None

    def run(self):
        if self._target is not None:
            self._return = self._target(*self._args,
                                                **self._kwargs)
    def join(self, *args):
        Thread.join(self, *args)
        return self._return
    

class mySubClass():
    def foo(self,bar):
        print('hello yoda {0}'.format(bar))
        time.sleep(5)
        return "foo"


class myClass():
    def foo(self,bar):
        print('hello {0}'.format(bar))
        time.sleep(5)
        return "foo"
    
    def __init__(self) ->None:
        self._subclass = mySubClass()
        twrv = ThreadWithReturnValue(target=self._subclass.foo, args=('world 2!',))
        twrv.start()
        print('before the return')
        print (twrv.join())   # prints foo

if __name__ == "__main__":
    myClass()