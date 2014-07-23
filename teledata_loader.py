#!/usr/bin/python

import struct
import sys
import cx_Oracle
import time
import argparse

COORD_MULT = 10**7

PROV_HEADER_STR = '<iQ'
DEV_HEADER_STR = '<qQ'
POINT_BLOCK_STR = '<iiBii'

parser = argparse.ArgumentParser()
parser.add_argument('--output_file', dest='fname', type=str, required=True, 
                    help='Output file name')
parser.add_argument('--tablespace_name', dest='tablespace_name', type=str,
                    default='TELEDATA2013I', required=False,
                    help='Tablespace name')
parser.add_argument('-n', dest='records_number', type=int, default=0, 
                    required=False, help='Number of records to select from DB')


args = parser.parse_args()

oracle_conn_str = u'gps/z@GEOORACLESTD'
oracle_conn = cx_Oracle.connect(oracle_conn_str)
oracle_cursor = oracle_conn.cursor()

fout = open(args.fname, 'wb')
if not args.records_number:
    print 'Number of records to select not set'
    print 'Executing "select count(*) from {0}". \
    Please wait a few minutes.'.format(TABLESPACE_NAME)
    oracle_cursor.execute("""select count(*) from {0}""".format(
                                                        args.tablespace_name))
    args.records_number = oracle_cursor.fetchone()[0] 
print "Requested {0} records from the database".format(args.records_number)
# writing provider header (there will be the only provider in .bin file)
fout.write(struct.pack(PROV_HEADER_STR, 1, 1))

# writing device header 
fout.write(struct.pack(DEV_HEADER_STR, 1, args.records_number))

# getting and writing points
oracle_cursor.execute("""select * from {0}""".format(args.tablespace_name))

skipped = 0
estimated_time = 0
iter_start = time.time()
for i in xrange(args.records_number):
    if i > 0 and i % 10**5 == 0:
        iter_end = time.time()
        estimated_time = (iter_end - iter_start) * \
                                (args.records_number - i) / 10**5
        print "Writing record #" + str(i) + '.', \
                "Skipped {0} broken points.".format(skipped), \
                "ETA: " + \
                        time.ctime(time.time() + estimated_time)
        iter_start = time.time()
    curr_point = oracle_cursor.fetchone()
    try:
        fout.write(struct.pack(POINT_BLOCK_STR, \
                    int(curr_point[3] * COORD_MULT), \
                    int(curr_point[4] * COORD_MULT),\
                    curr_point[5], \
                    int(time.mktime(curr_point[8].timetuple())),\
                    int(time.mktime(curr_point[8].timetuple()))))
    except (struct.error, TypeError), e:
        skipped += 1
        continue

print "Skipped {0} records".format(skipped)
fout.close()
