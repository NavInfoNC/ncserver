#!/usr/bin/python
# -*- coding: utf-8 -*-

import os, unittest, urllib
from os import system
import unittest

host = "http://127.0.0.1"

class Test(unittest.TestCase):
	def setUp(self):
		return

	def test_basic(self):
		o = urllib.urlopen(host + "/echo?city=%E5%8C%97%E4%BA%AC")
		s =  o.read()
		print s
		self.assertTrue(s.find("Request-Method: GET") != -1)
		self.assertTrue(s.find("city: 北京") != -1)

if __name__ == '__main__':
    unittest.main()