from subprocess import Popen, PIPE
import csv
import StringIO
import re


class Listserv(object):
    host = None
    port = None
    email = None
    password = None


    def __init__(self, *args, **kwargs):
        self.host = kwargs.get('host', None)
        self.port = kwargs.get('port', 2306)
        self.email = kwargs.get('email', None)
        self.password = kwargs.get('password', None)


    def run(self, cmd):
        """ Runs the supplied Listsrv command and returns the output """
        print "lcmdx %s:%s %s %s %s" % (self.host, self.port, self.email, self.password, cmd)
        data = Popen("lcmdx %s:%s %s %s %s" % (self.host, self.port, self.email, self.password, cmd), shell=True, stdout=PIPE).communicate()[0]

        return data


    def get_subscribers(self, list_name=None):
        """ Returns subscribers for the given list """
        return self.run("REVIEW %s MSG NOH" % list_name)


    def add_subscriber(self, list_name=None, email='', name=''):
        return self.run("ADD %s %s %s" % (list_name, email, name))





