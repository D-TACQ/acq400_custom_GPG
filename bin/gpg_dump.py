#!/usr/local/bin/python

import os

C24  = 2**24		# 24 bit count, include 0
C24M = C24-1		# 24 bit mask

count64 = 0
count240 = 0
with os.popen('gpg_dump') as pp:
	for line in pp:
#		print line.strip().split(' ')
		(count24, state) = line.strip().split(' ')
		count24 = int(count24)
		state = int(state,16)
		if count240 == C24M:
			if count24 == C24M:
				count64 += C24
			else:
				count64 += count24 + 1
		else:
			count64 += count24 - count240

		print("%8d s:%x %12d" % (count24, state, count64))
		count240 = count24

